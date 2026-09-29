const DEVICE_ID = 'default';

function log(msg, type = 'info') {
    const consoleBody = document.getElementById('consoleLogs');
    const line = document.createElement('div');
    line.className = `log-line ${type}`;
    const timeStr = new Date().toLocaleTimeString();
    line.innerText = `[${timeStr}] ${msg}`;
    consoleBody.appendChild(line);
    consoleBody.scrollTop = consoleBody.scrollHeight;
}

function clearLogs() {
    document.getElementById('consoleLogs').innerHTML = '';
}

function formatVND(amount) {
    return amount.toLocaleString('vi-VN') + 'đ';
}

// 1. Fetch & Render Initial State
async function loadInitialState() {
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/state`);
        const data = await res.json();

        // Render Family Message (Màn 5)
        if (data.family_message) {
            renderFamilyMessage(data.family_message);
        }

        // Render Savings (Màn 4)
        if (data.savings) {
            renderSavings(data.savings);
        }

        // Render Quests (Màn 2)
        if (data.quests) {
            renderQuests(data.quests);
        }

        log('Đã tải trạng thái ban đầu của thiết bị thành công!', 'success');
    } catch (e) {
        log('Lỗi khi tải trạng thái ban đầu: ' + e.message, 'amber');
    }
}

function renderFamilyMessage(msg) {
    const roleMap = { mom: '👩 Mom', dad: '👨 Dad', family: '🏡 Family' };
    document.getElementById('currentSenderRole').innerText = roleMap[msg.sender_role] || msg.sender;
    document.getElementById('currentMessageText').innerText = msg.message;
    document.getElementById('currentMessageTime').innerText = new Date(msg.timestamp * 1000).toLocaleTimeString();
    
    const heartEl = document.getElementById('heartIndicator');
    if (msg.liked) {
        heartEl.className = 'heart-indicator liked';
        heartEl.innerText = '💖 Đã thả tim!';
    } else {
        heartEl.className = 'heart-indicator';
        heartEl.innerText = '🤍 Chưa thả tim';
    }
}

function renderSavings(savings) {
    const cur = savings.current_amount || 0;
    const tgt = savings.target_amount || 2000000;
    const pct = Math.min(100, Math.max(0, (cur / tgt) * 100)).toFixed(1);

    document.getElementById('savingsCurrent').innerText = formatVND(cur);
    document.getElementById('savingsTarget').innerText = `Mục tiêu: ${formatVND(tgt)}`;
    document.getElementById('savingsProgressFill').style.width = `${pct}%`;
    document.getElementById('currentGoalName').innerText = `🎯 Mục tiêu: ${savings.goal_name || 'Dream Goal'}`;
    document.getElementById('currentGoalPercent').innerText = `${pct}%`;
}

function renderQuests(quests) {
    const container = document.getElementById('questListContainer');
    if (!quests || quests.length === 0) {
        container.innerHTML = '<p class="placeholder-text">Chưa có nhiệm vụ nào hôm nay</p>';
        return;
    }

    container.innerHTML = quests.map(q => `
        <div class="quest-item ${q.completed ? 'completed' : ''}">
            <div style="display:flex; align-items:center; gap:8px;">
                <button type="button" class="quest-btn-check" onclick="toggleQuest('${q.id}')">
                    ${q.completed ? '✓' : ''}
                </button>
                <span>${q.title}</span>
            </div>
            <span style="color:#94A3B8; font-size:12px;">${q.completed ? 'Đã xong' : (q.progress_text || '')}</span>
        </div>
    `).join('');
}

// 2. Actions: Family Message
document.getElementById('sendMessageForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const role = document.querySelector('input[name="senderRole"]:checked').value;
    const msg = document.getElementById('messageInput').value.trim();
    if (!msg) return;

    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/family/message`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ sender_role: role, message: msg })
        });
        const data = await res.json();
        if (data.success) {
            renderFamilyMessage(data.message);
            document.getElementById('messageInput').value = '';
            log(`[Hộp Thư] Đã gửi tin nhắn tới thiết bị: "${msg}"`, 'pink');
        }
    } catch (err) {
        log('Lỗi gửi tin nhắn: ' + err.message, 'amber');
    }
});

function setQuickMsg(text) {
    document.getElementById('messageInput').value = text;
}

// 3. Actions: Savings
async function depositMoney(amount) {
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/savings/deposit`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ amount, note: 'Parent Reward' })
        });
        const data = await res.json();
        if (data.success) {
            renderSavings(data.savings);
            log(`[Heo Đất] Đã thưởng +${formatVND(amount)} vào Heo Đất!`, 'amber');
        }
    } catch (err) {
        log('Lỗi nạp tiền: ' + err.message, 'amber');
    }
}

function depositCustomMoney() {
    const input = document.getElementById('customDepositInput');
    const amt = parseInt(input.value, 10);
    if (!isNaN(amt) && amt > 0) {
        depositMoney(amt);
        input.value = '';
    }
}

async function changeGoal() {
    const select = document.getElementById('goalSelect');
    const opt = select.options[select.selectedIndex];
    const goalType = parseInt(select.value, 10);
    const goalName = opt.text.split('(')[0].trim();
    const targetAmount = parseInt(opt.getAttribute('data-price'), 10);

    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/savings/goal`, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ goal_type: goalType, goal_name: goalName, target_amount: targetAmount })
        });
        const data = await res.json();
        if (data.success) {
            renderSavings(data.savings);
            log(`[Heo Đất] Đã đổi mục tiêu tiết kiệm thành: ${goalName} (${formatVND(targetAmount)})`, 'amber');
        }
    } catch (err) {
        log('Lỗi đổi mục tiêu: ' + err.message, 'amber');
    }
}

// 4. Actions: Quests
async function toggleQuest(questId) {
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/quests/${questId}/toggle`, { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            renderQuests(data.quests);
            log(`[Nhiệm Vụ] Đã cập nhật trạng thái nhiệm vụ`, 'success');
        }
    } catch (err) {
        log('Lỗi cập nhật nhiệm vụ: ' + err.message, 'amber');
    }
}

document.getElementById('addQuestForm').addEventListener('submit', async (e) => {
    e.preventDefault();
    const title = document.getElementById('questTitleInput').value.trim();
    const prog = document.getElementById('questProgInput').value.trim() || '0/1';
    if (!title) return;

    const id = 'q_' + Date.now();
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/quests`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id, title, progress_text: prog })
        });
        const data = await res.json();
        if (data.success) {
            renderQuests(data.quests);
            document.getElementById('questTitleInput').value = '';
            log(`[Nhiệm Vụ] Đã giao nhiệm vụ mới: "${title}"`, 'success');
        }
    } catch (err) {
        log('Lỗi giao nhiệm vụ: ' + err.message, 'amber');
    }
});

function setQuickQuest(id, title, prog) {
    document.getElementById('questTitleInput').value = title;
    document.getElementById('questProgInput').value = prog;
}

// 5. Connect SSE Real-time Stream
function connectSSE() {
    const sse = new EventSource('/api/events');

    sse.onmessage = (event) => {
        try {
            const msg = JSON.parse(event.data);
            const { type, data } = msg;

            if (type === 'device_connected') {
                log(`🔌 Thiết bị ESP32-S3 (${data.clientId}) vừa kết nối MQTT!`, 'success');
            } else if (type === 'device_disconnected') {
                log(`❌ Thiết bị ESP32-S3 (${data.clientId}) ngắt kết nối MQTT!`, 'amber');
            } else if (type === 'family_love_received') {
                log(`💖 Bé vừa bấm "Send Love ❤️" trên ESP32-S3!`, 'pink');
                const heartEl = document.getElementById('heartIndicator');
                heartEl.className = 'heart-indicator liked';
                heartEl.innerText = '💖 Đã thả tim!';
            } else if (type === 'quest_completed') {
                log(`🎉 Bé vừa hoàn thành nhiệm vụ "${data.questId}" trên thiết bị!`, 'success');
                loadInitialState();
            } else if (type === 'goal_changed') {
                log(`🎯 Bé vừa đổi mục tiêu thành "${data.goal_name}" trên thiết bị!`, 'amber');
                loadInitialState();
            }
        } catch (e) {
            console.warn(e);
        }
    };

    sse.onerror = () => {
        console.warn('SSE stream error, retrying in 3s...');
        setTimeout(connectSSE, 3000);
    };
}

// Start
loadInitialState();
connectSSE();
