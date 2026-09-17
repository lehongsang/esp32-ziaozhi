const express = require('express');
const router = express.Router();
const db = require('./db');
const { publishToDevice, setDashboardBroadcast } = require('./mqtt');

// SSE Clients for Live Web Dashboard
const sseClients = new Set();

setDashboardBroadcast((type, data) => {
    const payload = JSON.stringify({ type, data, timestamp: Date.now() });
    for (const client of sseClients) {
        client.write(`data: ${payload}\n\n`);
    }
});

// 1. SSE Stream for Web Dashboard
router.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.flushHeaders();

    sseClients.add(res);
    req.on('close', () => sseClients.delete(res));
});

// 2. Get Full State (Used by device on boot or web dashboard)
router.get('/device/:id/state', (req, res) => {
    const deviceId = req.params.id || 'default';

    db.get(`SELECT * FROM devices WHERE id = ?`, [deviceId], (err, device) => {
        if (err || !device) {
            device = { id: deviceId, name: 'MB Buddy S3', battery: 95, level: 3, xp: 240 };
        }

        db.all(`SELECT * FROM quests WHERE device_id = ? ORDER BY id ASC`, [deviceId], (err, quests) => {
            db.get(`SELECT * FROM savings WHERE device_id = ?`, [deviceId], (err, savings) => {
                db.get(`SELECT * FROM family_messages WHERE device_id = ? ORDER BY id DESC LIMIT 1`, [deviceId], (err, message) => {
                    res.json({
                        device,
                        quests: quests || [],
                        savings: savings || { current_amount: 850000, target_amount: 2000000, goal_type: 1, goal_name: 'Smart Robot', currency: 'd' },
                        family_message: message || { sender: 'Mom', sender_role: 'mom', message: 'Mẹ rất tự hào về con! 💖', timestamp: Math.floor(Date.now() / 1000), liked: 0 }
                    });
                });
            });
        });
    });
});

// 3. Quests APIs (Màn 2)
router.get('/device/:id/quests', (req, res) => {
    const deviceId = req.params.id || 'default';
    db.all(`SELECT * FROM quests WHERE device_id = ?`, [deviceId], (err, rows) => {
        if (err) return res.status(500).json({ error: err.message });
        res.json(rows || []);
    });
});

router.post('/device/:id/quests', (req, res) => {
    const deviceId = req.params.id || 'default';
    const { id, title, progress_text } = req.body;
    if (!id || !title) return res.status(400).json({ error: 'Missing id or title' });

    db.run(`INSERT OR REPLACE INTO quests (id, device_id, title, progress_text, completed) VALUES (?, ?, ?, ?, 0)`,
        [id, deviceId, title, progress_text || '0/1'], (err) => {
            if (err) return res.status(500).json({ error: err.message });

            // Fetch full list and push down to device via MQTT
            db.all(`SELECT * FROM quests WHERE device_id = ?`, [deviceId], (err, allQuests) => {
                publishToDevice(deviceId, 'quests/set', { quests: allQuests });
                res.json({ success: true, quests: allQuests });
            });
        });
});

router.post('/device/:id/quests/:questId/toggle', (req, res) => {
    const deviceId = req.params.id || 'default';
    const questId = req.params.questId;

    db.get(`SELECT completed FROM quests WHERE id = ? AND device_id = ?`, [questId, deviceId], (err, row) => {
        if (!row) return res.status(404).json({ error: 'Quest not found' });
        const newStatus = row.completed ? 0 : 1;
        db.run(`UPDATE quests SET completed = ? WHERE id = ? AND device_id = ?`, [newStatus, questId, deviceId], () => {
            db.all(`SELECT * FROM quests WHERE device_id = ?`, [deviceId], (err, allQuests) => {
                publishToDevice(deviceId, 'quests/set', { quests: allQuests });
                res.json({ success: true, quests: allQuests });
            });
        });
    });
});

// 4. Savings APIs (Màn 4)
router.get('/device/:id/savings', (req, res) => {
    const deviceId = req.params.id || 'default';
    db.get(`SELECT * FROM savings WHERE device_id = ?`, [deviceId], (err, row) => {
        if (err) return res.status(500).json({ error: err.message });
        res.json(row || {});
    });
});

router.post('/device/:id/savings/deposit', (req, res) => {
    const deviceId = req.params.id || 'default';
    const { amount, note } = req.body;
    const addAmt = parseInt(amount, 10);
    if (isNaN(addAmt)) return res.status(400).json({ error: 'Invalid amount' });

    db.get(`SELECT * FROM savings WHERE device_id = ?`, [deviceId], (err, savings) => {
        const current = (savings ? savings.current_amount : 0) + addAmt;
        const target = savings ? savings.target_amount : 2000000;
        const goalType = savings ? savings.goal_type : 1;
        const goalName = savings ? savings.goal_name : 'Smart Robot';
        const currency = savings ? savings.currency : 'd';

        db.run(`INSERT OR REPLACE INTO savings (device_id, current_amount, target_amount, goal_type, goal_name, currency, updated_at)
                VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)`,
            [deviceId, current, target, goalType, goalName, currency], () => {
                db.run(`INSERT INTO savings_history (device_id, amount, action, note) VALUES (?, ?, ?, ?)`,
                    [deviceId, addAmt, addAmt >= 0 ? 'deposit' : 'withdraw', note || 'Parent reward']);

                const payload = {
                    current_amount: current,
                    target_amount: target,
                    goal_type: goalType,
                    goal_name: goalName,
                    currency: currency
                };
                // Push real-time to ESP32
                publishToDevice(deviceId, 'savings/set', payload);
                res.json({ success: true, savings: payload });
            });
    });
});

router.put('/device/:id/savings/goal', (req, res) => {
    const deviceId = req.params.id || 'default';
    const { goal_type, goal_name, target_amount } = req.body;

    db.get(`SELECT * FROM savings WHERE device_id = ?`, [deviceId], (err, savings) => {
        const current = savings ? savings.current_amount : 0;
        const currency = savings ? savings.currency : 'd';

        db.run(`INSERT OR REPLACE INTO savings (device_id, current_amount, target_amount, goal_type, goal_name, currency, updated_at)
                VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)`,
            [deviceId, current, target_amount || 2000000, goal_type || 1, goal_name || 'Dream Goal', currency], () => {
                const payload = {
                    current_amount: current,
                    target_amount: target_amount || 2000000,
                    goal_type: goal_type || 1,
                    goal_name: goal_name || 'Dream Goal',
                    currency: currency
                };
                publishToDevice(deviceId, 'savings/set', payload);
                res.json({ success: true, savings: payload });
            });
    });
});

// 5. Family Messages APIs (Màn 5)
router.get('/device/:id/family/message', (req, res) => {
    const deviceId = req.params.id || 'default';
    db.get(`SELECT * FROM family_messages WHERE device_id = ? ORDER BY id DESC LIMIT 1`, [deviceId], (err, row) => {
        if (err) return res.status(500).json({ error: err.message });
        res.json(row || {});
    });
});

router.post('/device/:id/family/message', (req, res) => {
    const deviceId = req.params.id || 'default';
    const { sender, sender_role, message } = req.body;
    if (!message) return res.status(400).json({ error: 'Message content is required' });

    const role = sender_role || (sender === 'Dad' ? 'dad' : (sender === 'Family' ? 'family' : 'mom'));
    const senderName = sender || (role === 'dad' ? 'Dad' : (role === 'family' ? 'Family' : 'Mom'));
    const timestamp = Math.floor(Date.now() / 1000);

    db.run(`INSERT INTO family_messages (device_id, sender, sender_role, avatar, message, timestamp, liked) VALUES (?, ?, ?, ?, ?, ?, 0)`,
        [deviceId, senderName, role, `${role}_avatar.png`, message, timestamp], function (err) {
            if (err) return res.status(500).json({ error: err.message });

            const payload = {
                id: this.lastID,
                sender: senderName,
                sender_role: role,
                message: message,
                timestamp: timestamp,
                liked: false
            };
            // Push real-time to ESP32
            publishToDevice(deviceId, 'family/message', payload);
            res.json({ success: true, message: payload });
        });
});

module.exports = router;
