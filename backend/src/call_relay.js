const ws = require('ws');
const url = require('url');

// Active call sessions indexed by deviceId
// session = { deviceWs: WebSocket, parentWs: WebSocket, state: 'idle' | 'calling' | 'active', caller: string }
const sessions = new Map();

function getOrCreateSession(deviceId) {
    if (!sessions.has(deviceId)) {
        sessions.set(deviceId, {
            deviceId,
            deviceWs: null,
            parentWs: null,
            state: 'idle',
            caller: null,
            startTime: null
        });
    }
    return sessions.get(deviceId);
}

function attachCallRelayServer(httpServer) {
    const wss = new ws.Server({ noServer: true });

    wss.on('connection', (socket, req) => {
        const parsedUrl = url.parse(req.url, true);
        const query = parsedUrl.query;
        const clientType = query.type || 'parent'; // 'device' or 'parent'
        const deviceId = query.deviceId || 'default';

        console.log(`📞 [CallRelay] New connection: type=${clientType}, deviceId=${deviceId}`);

        const session = getOrCreateSession(deviceId);

        if (clientType === 'device') {
            if (session.deviceWs && session.deviceWs.readyState === ws.OPEN) {
                session.deviceWs.close();
            }
            session.deviceWs = socket;
        } else {
            if (session.parentWs && session.parentWs.readyState === ws.OPEN) {
                session.parentWs.close();
            }
            session.parentWs = socket;
        }

        // Notify parent about device presence status
        if (clientType === 'parent') {
            const isDeviceOnline = session.deviceWs && session.deviceWs.readyState === ws.OPEN;
            socket.send(JSON.stringify({
                type: 'device_status',
                online: isDeviceOnline,
                callState: session.state,
                deviceId
            }));
        } else if (clientType === 'device' && session.parentWs && session.parentWs.readyState === ws.OPEN) {
            session.parentWs.send(JSON.stringify({
                type: 'device_status',
                online: true,
                callState: session.state,
                deviceId
            }));
        }

        socket.on('message', (data, isBinary) => {
            // 1. Binary Data -> Pure Audio Stream Relay (Ultra Low Latency)
            if (isBinary) {
                if (session.state === 'active') {
                    const target = (clientType === 'device') ? session.parentWs : session.deviceWs;
                    if (target && target.readyState === ws.OPEN) {
                        target.send(data, { binary: true });
                    }
                }
                return;
            }

            // 2. Text Data -> JSON Signaling Control
            try {
                const msg = JSON.parse(data.toString('utf-8'));
                console.log(`📩 [CallRelay ${clientType} -> ${deviceId}]`, msg);

                switch (msg.type) {
                    case 'call_request': {
                        session.state = 'calling';
                        session.caller = msg.caller || (clientType === 'parent' ? 'Phụ huynh' : 'Bé');
                        const target = (clientType === 'device') ? session.parentWs : session.deviceWs;
                        if (target && target.readyState === ws.OPEN) {
                            target.send(JSON.stringify({
                                type: 'incoming_call',
                                caller: session.caller,
                                from: clientType,
                                deviceId
                            }));
                        }

                        // Also publish to MQTT topic for device standby listening
                        if (clientType === 'parent') {
                            const { publishToDevice } = require('./mqtt');
                            publishToDevice(deviceId, 'call/request', {
                                type: 'incoming_call',
                                caller: session.caller,
                                from: 'parent',
                                deviceId
                            });
                        }
                        break;
                    }

                    case 'call_accept': {
                        session.state = 'active';
                        session.startTime = Date.now();
                        const notifyMsg = JSON.stringify({
                            type: 'call_connected',
                            deviceId,
                            startTime: session.startTime
                        });
                        if (session.deviceWs && session.deviceWs.readyState === ws.OPEN) session.deviceWs.send(notifyMsg);
                        if (session.parentWs && session.parentWs.readyState === ws.OPEN) session.parentWs.send(notifyMsg);
                        const { publishToDevice } = require('./mqtt');
                        publishToDevice(deviceId, 'call/accept', { type: 'call_connected', deviceId });
                        console.log(`✅ [CallRelay] Call ACTIVE for device ${deviceId}`);
                        break;
                    }

                    case 'call_reject': {
                        session.state = 'idle';
                        const rejectMsg = JSON.stringify({
                            type: 'call_rejected',
                            reason: msg.reason || 'Bận / Từ chối cuộc gọi',
                            from: clientType
                        });
                        if (session.deviceWs && session.deviceWs.readyState === ws.OPEN) session.deviceWs.send(rejectMsg);
                        if (session.parentWs && session.parentWs.readyState === ws.OPEN) session.parentWs.send(rejectMsg);
                        const { publishToDevice } = require('./mqtt');
                        publishToDevice(deviceId, 'call/reject', { type: 'call_rejected', deviceId, reason: msg.reason || 'Bận' });
                        break;
                    }

                    case 'call_end': {
                        session.state = 'idle';
                        const endMsg = JSON.stringify({
                            type: 'call_ended',
                            from: clientType,
                            duration: session.startTime ? Math.floor((Date.now() - session.startTime) / 1000) : 0
                        });
                        if (session.deviceWs && session.deviceWs.readyState === ws.OPEN) session.deviceWs.send(endMsg);
                        if (session.parentWs && session.parentWs.readyState === ws.OPEN) session.parentWs.send(endMsg);
                        const { publishToDevice } = require('./mqtt');
                        publishToDevice(deviceId, 'call/end', { type: 'call_ended', deviceId });
                        console.log(`⏹️ [CallRelay] Call ENDED for device ${deviceId}`);
                        break;
                    }

                    case 'ping': {
                        socket.send(JSON.stringify({ type: 'pong' }));
                        break;
                    }
                }
            } catch (err) {
                console.error(`❌ [CallRelay] JSON parse error:`, err.message);
            }
        });

        socket.on('close', () => {
            console.log(`🔌 [CallRelay] Connection closed: type=${clientType}, deviceId=${deviceId}`);
            if (clientType === 'device') {
                session.deviceWs = null;
                if (session.parentWs && session.parentWs.readyState === ws.OPEN) {
                    session.parentWs.send(JSON.stringify({ type: 'device_status', online: false, deviceId }));
                }
            } else {
                session.parentWs = null;
            }

            if (session.state !== 'idle') {
                session.state = 'idle';
                const endMsg = JSON.stringify({ type: 'call_ended', reason: 'Disconnected' });
                if (session.deviceWs && session.deviceWs.readyState === ws.OPEN) session.deviceWs.send(endMsg);
                if (session.parentWs && session.parentWs.readyState === ws.OPEN) session.parentWs.send(endMsg);
            }
        });
    });

    return wss;
}

module.exports = {
    attachCallRelayServer,
    sessions
};
