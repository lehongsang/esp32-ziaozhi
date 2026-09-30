const sqlite3 = require('sqlite3').verbose();
const path = require('path');
const fs = require('fs');

const dataDir = process.env.DATA_DIR || path.resolve(__dirname, '../data');
if (!fs.existsSync(dataDir)) {
    fs.mkdirSync(dataDir, { recursive: true });
}
const dbPath = path.join(dataDir, 'buddy.sqlite');

const db = new sqlite3.Database(dbPath, (err) => {
    if (err) {
        console.error('❌ Error connecting to SQLite database:', err.message);
    } else {
        console.log('📦 Connected to SQLite database:', dbPath);
        initTables();
    }
});

function initTables() {
    db.serialize(() => {
        // 1. Devices Table
        db.run(`CREATE TABLE IF NOT EXISTS devices (
            id TEXT PRIMARY KEY,
            name TEXT DEFAULT 'MB Buddy',
            pairing_code TEXT DEFAULT '123456',
            battery INTEGER DEFAULT 95,
            level INTEGER DEFAULT 1,
            xp INTEGER DEFAULT 120,
            last_seen DATETIME DEFAULT CURRENT_TIMESTAMP
        )`);

        // 2. Quests Table (Today's Quests)
        db.run(`CREATE TABLE IF NOT EXISTS quests (
            id TEXT PRIMARY KEY,
            device_id TEXT,
            title TEXT,
            progress_text TEXT,
            scheduled_time TEXT DEFAULT '',
            start_time TEXT DEFAULT '',
            duration INTEGER DEFAULT 20,
            reward_stars INTEGER DEFAULT 1,
            category TEXT DEFAULT 'habit',
            remind_before INTEGER DEFAULT 30,
            completed INTEGER DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )`);

        // Migration helper if columns don't exist yet
        const columnsToAdd = [
            `ALTER TABLE quests ADD COLUMN scheduled_time TEXT DEFAULT ''`,
            `ALTER TABLE quests ADD COLUMN start_time TEXT DEFAULT ''`,
            `ALTER TABLE quests ADD COLUMN duration INTEGER DEFAULT 20`,
            `ALTER TABLE quests ADD COLUMN reward_stars INTEGER DEFAULT 1`,
            `ALTER TABLE quests ADD COLUMN category TEXT DEFAULT 'habit'`,
            `ALTER TABLE quests ADD COLUMN remind_before INTEGER DEFAULT 30`
        ];
        columnsToAdd.forEach(sql => {
            db.run(sql, () => {}); // Ignore error if column exists
        });

        // 3. Savings & Dream Goal Table
        db.run(`CREATE TABLE IF NOT EXISTS savings (
            device_id TEXT PRIMARY KEY,
            current_amount INTEGER DEFAULT 850000,
            target_amount INTEGER DEFAULT 2000000,
            goal_type INTEGER DEFAULT 1,
            goal_name TEXT DEFAULT 'Smart Robot',
            currency TEXT DEFAULT 'd',
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )`);

        // 4. Savings History Table
        db.run(`CREATE TABLE IF NOT EXISTS savings_history (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT,
            amount INTEGER,
            action TEXT,
            note TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )`);

        // 5. Family Messages Table (Family Moment)
        db.run(`CREATE TABLE IF NOT EXISTS family_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT,
            sender TEXT,
            sender_role TEXT,
            avatar TEXT,
            message TEXT,
            timestamp INTEGER,
            liked INTEGER DEFAULT 0
        )`);

        // Seed default device 'default' if not exists
        const defaultDeviceId = 'default';
        db.get(`SELECT id FROM devices WHERE id = ?`, [defaultDeviceId], (err, row) => {
            if (!row) {
                console.log('🌱 Seeding initial demo data for device:', defaultDeviceId);
                db.run(`INSERT INTO devices (id, name, battery, level, xp) VALUES (?, ?, ?, ?, ?)`,
                    [defaultDeviceId, 'MB Buddy S3', 98, 3, 240]);

                // Initial Quests according to AC02 MB Junior Mockup (3 Việc hôm nay)
                const initialQuests = [
                    ['q_math', defaultDeviceId, 'Học Toán', '', '17:00 - 17:20', '17:00', 20, 1, 'math', 1],
                    ['q_read', defaultDeviceId, 'Đọc sách 15 phút', '', '19:30 - 19:45', '19:30', 15, 1, 'read', 1],
                    ['q_pack', defaultDeviceId, 'Chuẩn bị cặp', '0/1', '21:00 - 21:10', '21:00', 10, 1, 'pack', 0]
                ];
                initialQuests.forEach(q => {
                    db.run(`INSERT INTO quests (id, device_id, title, progress_text, scheduled_time, start_time, duration, reward_stars, category, completed) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`, q);
                });

                // Initial Savings
                db.run(`INSERT INTO savings (device_id, current_amount, target_amount, goal_type, goal_name, currency) VALUES (?, ?, ?, ?, ?, ?)`,
                    [defaultDeviceId, 850000, 2000000, 1, 'Smart Robot', 'd']);

                // Initial Family Message
                db.run(`INSERT INTO family_messages (device_id, sender, sender_role, avatar, message, timestamp, liked) VALUES (?, ?, ?, ?, ?, ?, ?)`,
                    [defaultDeviceId, 'Mom', 'mom', 'mom_avatar.png', 'Mẹ rất tự hào về con! Hoàn thành nốt việc chuẩn bị cặp nhé! 💖', Math.floor(Date.now() / 1000), 0]);
            }
        });
    });
}

module.exports = db;
