const Aedes = require('aedes');
const net = require('net');
const db = require('./db');

const aedes = new Aedes();
const mqttPort = process.env.MQTT_PORT || 1883;

// Global broadcast callback to SSE / Web Dashboard
let dashboardBroadcast = null;

function setDashboardBroadcast(cb) {
    dashboardBroadcast = cb;
}

function broadcastEvent(type, data) {
    if (dashboardBroadcast) {
        dashboardBroadcast(type, data);
    }
}

const ws = require('ws');

// Start MQTT TCP Server (with error handling for cloud platforms)
const server = net.createServer(aedes.handle);

server.listen(mqttPort, () => {
    console.log(`📡 Embedded MQTT TCP Broker running on port: ${mqttPort}`);
}).on('error', (err) => {
    console.warn(`⚠️ MQTT TCP port ${mqttPort} not available (${err.message}). Using WebSocket MQTT fallback.`);
});

function attachWebSocketServer(httpServer) {
    const wss = new ws.Server({ server: httpServer, path: '/mqtt' });
    wss.on('connection', (socket) => {
        const stream = ws.createWebSocketStream(socket);
        aedes.handle(stream);
    });
    console.log(`🌐 MQTT over WebSockets attached to HTTP Server at: /mqtt`);
}

aedes.on('client', (client) => {
    console.log(`🔌 [MQTT] Client connected: ${client ? client.id : 'unknown'}`);
    broadcastEvent('device_connected', { clientId: client.id, timestamp: Date.now() });
});

aedes.on('clientDisconnect', (client) => {
    console.log(`❌ [MQTT] Client disconnected: ${client ? client.id : 'unknown'}`);
    broadcastEvent('device_disconnected', { clientId: client.id, timestamp: Date.now() });
});

// Handle incoming messages from Device
aedes.on('publish', (packet, client) => {
    if (!client) return; // Ignore internal broker publishes
    const topic = packet.topic;
    const payloadStr = packet.payload.toString('utf-8');

    console.log(`📥 [MQTT Device -> Server] [${topic}]:`, payloadStr);

    try {
        const payload = JSON.parse(payloadStr);
        const parts = topic.split('/');
        // Format: buddy/{deviceId}/{action}/{subaction...}
        if (parts[0] === 'buddy' && parts.length >= 3) {
            const deviceId = parts[1];
            const channel = parts[2];

            if (channel === 'quests' && parts[3] === 'completed') {
                // Quest Completed by child
                const questId = payload.quest_id;
                db.run(`UPDATE quests SET completed = 1, progress_text = '' WHERE id = ? AND device_id = ?`,
                    [questId, deviceId], (err) => {
                        if (!err) {
                            console.log(`🎉 Quest ${questId} marked COMPLETED on DB for ${deviceId}`);
                            broadcastEvent('quest_completed', { deviceId, questId, timestamp: Date.now() });
                        }
                    });
            } else if (channel === 'family' && parts[3] === 'like') {
                // Child tapped "Send Love ❤️"
                db.run(`UPDATE family_messages SET liked = 1 WHERE device_id = ?`, [deviceId], (err) => {
                    if (!err) {
                        console.log(`💖 Device ${deviceId} sent LOVE heart to family!`);
                        broadcastEvent('family_love_received', { deviceId, timestamp: Date.now() });
                    }
                });
            } else if (channel === 'savings' && parts[3] === 'goal_changed') {
                // Child changed Dream Goal on device
                const { goal_type, goal_name, target_amount } = payload;
                db.run(`UPDATE savings SET goal_type = ?, goal_name = ?, target_amount = ? WHERE device_id = ?`,
                    [goal_type, goal_name, target_amount, deviceId], (err) => {
                        if (!err) {
                            console.log(`🎯 Goal changed to ${goal_name} (${target_amount}) for ${deviceId}`);
                            broadcastEvent('goal_changed', { deviceId, goal_type, goal_name, target_amount });
                        }
                    });
            } else if (channel === 'status') {
                // Periodic status heartbeat
                const { battery, level, xp } = payload;
                db.run(`UPDATE devices SET battery = ?, level = ?, xp = ?, last_seen = CURRENT_TIMESTAMP WHERE id = ?`,
                    [battery, level, xp, deviceId]);
                broadcastEvent('device_status', { deviceId, battery, level, xp });

                // Auto hydrate full state to newly connected device
                db.all(`SELECT id, title, progress_text, scheduled_time, start_time, duration, reward_stars, category, completed FROM quests WHERE device_id = ? ORDER BY id ASC`, [deviceId], (err, quests) => {
                    if (!err && quests && quests.length > 0) {
                        const formattedQuests = quests.map(q => ({
                            id: q.id,
                            title: q.title,
                            progress_text: q.progress_text,
                            scheduled_time: q.scheduled_time || '',
                            start_time: q.start_time || '',
                            duration: q.duration || 20,
                            reward_stars: q.reward_stars || 1,
                            category: q.category || 'habit',
                            completed: q.completed === 1
                        }));
                        publishToDevice(deviceId, 'quests/set', { quests: formattedQuests });
                    }
                });

                db.get(`SELECT goal_type, goal_name, current_amount, target_amount, currency FROM savings WHERE device_id = ?`, [deviceId], (err, sav) => {
                    if (!err && sav) {
                        publishToDevice(deviceId, 'savings/set', sav);
                    }
                });

                db.get(`SELECT sender, message, timestamp FROM family_messages WHERE device_id = ? ORDER BY id DESC LIMIT 1`, [deviceId], (err, msg) => {
                    if (!err && msg) {
                        publishToDevice(deviceId, 'family/message', msg);
                    }
                });
            }
        }
    } catch (e) {
        console.warn('⚠️ Error parsing MQTT packet payload JSON:', e.message);
    }
});

// Helper to push updates down to Device
function publishToDevice(deviceId, subTopic, payloadObj) {
    const topic = `buddy/${deviceId}/${subTopic}`;
    const payloadStr = JSON.stringify(payloadObj);
    aedes.publish({
        topic,
        payload: Buffer.from(payloadStr),
        qos: 1,
        retain: false
    }, (err) => {
        if (err) {
            console.error(`❌ Failed to publish to ${topic}:`, err);
        } else {
            console.log(`📤 [MQTT Server -> Device] [${topic}]:`, payloadStr);
        }
    });
}

module.exports = {
    aedes,
    attachWebSocketServer,
    publishToDevice,
    setDashboardBroadcast,
    broadcastEvent
};
