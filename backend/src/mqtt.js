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

function createMqttWsServer() {
    const wss = new ws.Server({ noServer: true });
    wss.on('connection', (socket) => {
        const stream = ws.createWebSocketStream(socket);
        aedes.handle(stream);
    });
    return wss;
}

aedes.on('client', (client) => {
    console.log(`🔌 [MQTT] Client connected: ${client ? client.id : 'unknown'}`);
    broadcastEvent('device_connected', { clientId: client.id, timestamp: Date.now() });

    if (client && client.id) {
        const devId = client.id.replace('buddy_', '');
        publishToDevice(devId, 'time/set', { timestamp: Date.now(), timezone_offset: 420 });
    }
});

aedes.on('clientDisconnect', (client) => {
    console.log(`❌ [MQTT] Client disconnected: ${client ? client.id : 'unknown'}`);
    broadcastEvent('device_disconnected', { clientId: client.id, timestamp: Date.now() });
});

// Handle incoming messages from Device
aedes.on('publish', (packet, client) => {
    // If packet comes from internal server publish (client is null/undefined), do not re-process
    if (!client) {
        return;
    }

    const topic = packet.topic;

    // Handle binary voice call audio packet from ESP32 -> Web Browser
    if (topic.includes('/call/audio/up')) {
        const parts = topic.split('/');
        const deviceId = parts[1] || 'default';
        const { sessions } = require('./call_relay');
        let sess = (sessions && sessions.has(deviceId)) ? sessions.get(deviceId) : null;
        if (!sess && sessions && sessions.has('default')) {
            sess = sessions.get('default');
        }
        if (!sess && sessions) {
            for (const s of sessions.values()) {
                if (s.parentWs && s.parentWs.readyState === 1) {
                    sess = s;
                    break;
                }
            }
        }
        if (sess && sess.parentWs && sess.parentWs.readyState === 1) {
            sess.parentWs.send(packet.payload, { binary: true });
        }
        return;
    }

    const payloadStr = packet.payload.toString('utf-8');
    console.log(`📥 [MQTT Device -> Server] [${topic}]:`, payloadStr);

    try {
        const payload = JSON.parse(payloadStr);
        const parts = topic.split('/');
        // Format: buddy/{deviceId}/{action}/{subaction...}
        if (parts[0] === 'buddy' && parts.length >= 3) {
            const deviceId = parts[1];
            const channel = parts[2];

            if (channel === 'time' && (parts[3] === 'get' || parts[3] === 'sync' || parts[3] === 'request' || !parts[3])) {
                // Device requested real time sync
                publishToDevice(deviceId, 'time/set', { timestamp: Date.now(), timezone_offset: 420 });
                console.log(`⏰ Synchronized real time to device ${deviceId}: ${new Date().toLocaleString('vi-VN')}`);
            } else if (channel === 'quests' && parts[3] === 'completed') {
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
            } else if (channel === 'call') {
                const subAction = parts[3] || 'request';
                console.log(`📞 [MQTT Call] ${subAction} from device ${deviceId}:`, payload);
                broadcastEvent('call_signal', { deviceId, subAction, payload });

                const { sessions } = require('./call_relay');
                let sess = (sessions && sessions.has(deviceId)) ? sessions.get(deviceId) : null;
                if (!sess && sessions && sessions.has('default')) {
                    sess = sessions.get('default');
                }
                if (!sess && sessions) {
                    for (const s of sessions.values()) {
                        if (s.parentWs && s.parentWs.readyState === 1) {
                            sess = s;
                            break;
                        }
                    }
                }

                if (sess && sess.parentWs && sess.parentWs.readyState === 1) {
                    if (subAction === 'request') {
                        sess.state = 'calling';
                        sess.caller = payload.caller || 'Bé Minh';
                        sess.parentWs.send(JSON.stringify({
                            type: 'incoming_call',
                            caller: sess.caller,
                            from: 'device',
                            deviceId
                        }));
                    } else if (subAction === 'accept') {
                        sess.state = 'active';
                        sess.startTime = Date.now();
                        sess.parentWs.send(JSON.stringify({ type: 'call_connected', deviceId, startTime: sess.startTime }));
                    } else if (subAction === 'reject' || subAction === 'end') {
                        sess.state = 'idle';
                        sess.parentWs.send(JSON.stringify({ type: 'call_ended', deviceId, reason: payload.reason || 'Ended' }));
                    }
                }
            } else if (channel === 'status') {
                // Periodic status heartbeat from device
                const { battery, level, xp } = payload;
                db.run(`UPDATE devices SET battery = ?, level = ?, xp = ?, last_seen = CURRENT_TIMESTAMP WHERE id = ?`,
                    [battery, level, xp, deviceId]);
                broadcastEvent('device_status', { deviceId, battery, level, xp });
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
    createMqttWsServer,
    publishToDevice,
    setDashboardBroadcast,
    broadcastEvent
};
