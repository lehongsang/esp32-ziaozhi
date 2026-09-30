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
    const badge = document.getElementById('questCountBadge');
    if (badge) badge.innerText = `${quests ? quests.length : 0} việc`;

    if (!quests || quests.length === 0) {
        container.innerHTML = '<p class="placeholder-text">Chưa có nhiệm vụ nào hôm nay</p>';
        return;
    }

    container.innerHTML = quests.map(q => `
        <div class="quest-item ${q.completed ? 'completed' : ''}">
            <div style="display:flex; align-items:center; gap:8px;">
                <button type="button" class="quest-btn-check" onclick="toggleQuest('${q.id}')" title="Đánh dấu hoàn thành">
                    ${q.completed ? '✓' : ''}
                </button>
                <div>
                    <span style="${q.completed ? 'text-decoration: line-through; opacity: 0.7;' : ''}; font-weight:600;">${q.title}</span>
                    ${q.scheduled_time ? `<div style="font-size:11px; color:#F59E0B; margin-top:2px;">⏰ Hạn: ${q.scheduled_time} (Nhắc trước ${q.remind_before || 30}p)</div>` : ''}
                </div>
            </div>
            <div style="display:flex; align-items:center; gap:10px;">
                <span style="color:#94A3B8; font-size:12px;">${q.completed ? 'Đã xong' : (q.progress_text || '')}</span>
                <button type="button" class="btn-del-quest" onclick="deleteQuest('${q.id}')" title="Xóa việc này">🗑️</button>
            </div>
        </div>
    `).join('');
}

async function deleteQuest(questId) {
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/quests/${questId}`, { method: 'DELETE' });
        const data = await res.json();
        if (data.success) {
            renderQuests(data.quests);
            log(`[Nhiệm Vụ] Đã xóa nhiệm vụ thành công`, 'success');
        }
    } catch (err) {
        log('Lỗi xóa nhiệm vụ: ' + err.message, 'amber');
    }
}

async function clearAllQuests() {
    if (!confirm('Bạn có chắc muốn xóa TOÀN BỘ danh sách nhiệm vụ cũ không?')) return;
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/quests`, { method: 'DELETE' });
        const data = await res.json();
        if (data.success) {
            renderQuests(data.quests);
            log(`[Nhiệm Vụ] Đã xóa sạch toàn bộ danh sách việc cũ!`, 'success');
        }
    } catch (err) {
        log('Lỗi xóa danh sách: ' + err.message, 'amber');
    }
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
    const scheduledTime = document.getElementById('questTimeInput') ? document.getElementById('questTimeInput').value : '15:00';
    const remindBefore = document.getElementById('questRemindInput') ? parseInt(document.getElementById('questRemindInput').value, 10) : 30;
    if (!title) return;

    const id = 'q_' + Date.now();
    try {
        const res = await fetch(`/api/device/${DEVICE_ID}/quests`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                id, 
                title, 
                scheduled_time: scheduledTime,
                remind_before: remindBefore,
                duration: 20,
                reward_stars: 1,
                category: 'habit'
            })
        });
        const data = await res.json();
        if (data.success) {
            renderQuests(data.quests);
            document.getElementById('questTitleInput').value = '';
            log(`[Nhiệm Vụ] Đã giao: "${title}" (Hạn: ${scheduledTime}, Nhắc trước: ${remindBefore}p)`, 'success');
        }
    } catch (err) {
        log('Lỗi giao nhiệm vụ: ' + err.message, 'amber');
    }
});

function setQuickQuest(id, title, prog) {
    document.getElementById('questTitleInput').value = title;
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

// ==========================================================================
// Voice Call System (WebSocket Audio Streaming & Signaling)
// ==========================================================================
let callWs = null;
let callState = 'idle'; // 'idle' | 'calling' | 'incoming' | 'active'
let callTimerInterval = null;
let callStartTime = 0;
let audioContext = null;
let mediaStream = null;
let scriptProcessor = null;
let isMicMuted = false;
let nextPlayTime = 0;

function initCallWebSocket() {
    const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${location.host}/call?type=parent&deviceId=${DEVICE_ID}`;
    
    callWs = new WebSocket(wsUrl);
    callWs.binaryType = 'arraybuffer';

    callWs.onopen = () => {
        log('📞 Đã kết nối kênh thoại Voice Call Relay!', 'success');
    };

    callWs.onmessage = (event) => {
        // Binary Audio Packet from ESP32 -> Play in Browser
        if (event.data instanceof ArrayBuffer) {
            if (callState === 'active') {
                playIncomingAudioChunk(event.data);
            }
            return;
        }

        // Text JSON Signaling Messages
        try {
            const msg = JSON.parse(event.data);
            handleCallSignaling(msg);
        } catch (e) {
            console.error('Call JSON parse error:', e);
        }
    };

    callWs.onclose = () => {
        console.warn('Call WS closed, reconnecting in 3s...');
        if (callState !== 'idle') {
            endCallLocally('Mất kết nối máy chủ');
        }
        setTimeout(initCallWebSocket, 3000);
    };
}

function handleCallSignaling(msg) {
    console.log('📞 [Call Event]:', msg);

    switch (msg.type) {
        case 'device_status': {
            const statusDot = document.getElementById('statusDot');
            const statusText = document.getElementById('deviceStatusText');
            if (msg.online) {
                if (statusDot) statusDot.className = 'status-dot online';
                if (statusText) statusText.innerText = 'ESP32 Buddy • Trực Tuyến';
            } else {
                if (statusDot) statusDot.className = 'status-dot';
                if (statusText) statusText.innerText = 'ESP32 Buddy • Ngoại Tuyến';
            }
            break;
        }

        case 'incoming_call': {
            // Child is calling parent
            callState = 'incoming';
            showCallModal({
                title: `${msg.caller || 'Bé Minh'} đang gọi...`,
                status: 'Cuộc gọi đến từ thiết bị Buddy',
                mode: 'incoming'
            });
            log(`🔔 [Cuộc gọi] Bé đang gọi cho Bố/Mẹ từ thiết bị!`, 'pink');
            break;
        }

        case 'call_connected': {
            // Call accepted and active!
            callState = 'active';
            callStartTime = Date.now();
            showCallModal({
                title: 'Bé Minh (Buddy ESP32)',
                status: 'Đang đàm thoại 2 chiều',
                mode: 'active'
            });
            startCallTimer();
            startAudioStream();
            log('🎙️ [Cuộc gọi] Kết nối thành công! Đang đàm thoại 2 chiều với bé.', 'success');
            break;
        }

        case 'call_rejected': {
            log(`❌ [Cuộc gọi] Cuộc gọi bị từ chối: ${msg.reason || 'Bận'}`, 'amber');
            endCallLocally('Bé từ chối hoặc đang bận');
            break;
        }

        case 'call_failed': {
            log(`⚠️ [Cuộc gọi] Không thể gọi: ${msg.reason}`, 'amber');
            alert(msg.reason);
            endCallLocally(msg.reason);
            break;
        }

        case 'call_ended': {
            log(`⏹️ [Cuộc gọi] Cuộc gọi đã kết thúc. Thời lượng: ${msg.duration || 0}s`, 'info');
            endCallLocally('Cuộc gọi kết thúc');
            break;
        }
    }
}

// UI Actions
function startParentCall() {
    if (!callWs || callWs.readyState !== WebSocket.OPEN) {
        alert('Chưa kết nối được với máy chủ cuộc gọi! Vui lòng thử lại sau vài giây.');
        return;
    }

    callState = 'calling';
    showCallModal({
        title: 'Bé Minh (Buddy ESP32)',
        status: 'Đang đổ chuông thiết bị...',
        mode: 'calling'
    });

    callWs.send(JSON.stringify({
        type: 'call_request',
        caller: 'Mẹ'
    }));

    log('📞 [Cuộc gọi] Đang gọi tới thiết bị của bé...', 'info');
}

function acceptIncomingCall() {
    if (callWs && callWs.readyState === WebSocket.OPEN) {
        callWs.send(JSON.stringify({ type: 'call_accept' }));
    }
}

function rejectIncomingCall() {
    if (callWs && callWs.readyState === WebSocket.OPEN) {
        callWs.send(JSON.stringify({ type: 'call_reject', reason: 'Bố Mẹ đang bận' }));
    }
    endCallLocally();
}

function endCall() {
    if (callWs && callWs.readyState === WebSocket.OPEN) {
        callWs.send(JSON.stringify({ type: 'call_end' }));
    }
    endCallLocally();
}

function endCallLocally(statusMsg) {
    callState = 'idle';
    stopAudioStream();
    stopCallTimer();

    const statusEl = document.getElementById('callStatusText');
    if (statusEl && statusMsg) {
        statusEl.innerText = statusMsg;
    }

    setTimeout(() => {
        document.getElementById('callModal').style.display = 'none';
    }, 1200);
}

function showCallModal({ title, status, mode }) {
    const modal = document.getElementById('callModal');
    document.getElementById('callTitle').innerText = title;
    document.getElementById('callStatusText').innerText = status;
    modal.style.display = 'flex';

    const callingActions = document.getElementById('callingActions');
    const incomingActions = document.getElementById('incomingActions');
    const activeActions = document.getElementById('activeActions');
    const timerEl = document.getElementById('callTimer');
    const equalizer = document.getElementById('callEqualizer');

    callingActions.style.display = (mode === 'calling') ? 'flex' : 'none';
    incomingActions.style.display = (mode === 'incoming') ? 'flex' : 'none';
    activeActions.style.display = (mode === 'active') ? 'flex' : 'none';
    timerEl.style.display = (mode === 'active') ? 'block' : 'none';
    equalizer.style.display = (mode === 'active') ? 'flex' : 'none';
}

function startCallTimer() {
    stopCallTimer();
    const timerEl = document.getElementById('callTimer');
    timerEl.innerText = '00:00';

    callTimerInterval = setInterval(() => {
        const diff = Math.floor((Date.now() - callStartTime) / 1000);
        const mins = String(Math.floor(diff / 60)).padStart(2, '0');
        const secs = String(diff % 60).padStart(2, '0');
        timerEl.innerText = `${mins}:${secs}`;
    }, 1000);
}

function stopCallTimer() {
    if (callTimerInterval) {
        clearInterval(callTimerInterval);
        callTimerInterval = null;
    }
}

function toggleMuteMic() {
    isMicMuted = !isMicMuted;
    const muteBtn = document.getElementById('btnMuteMic');
    if (muteBtn) {
        muteBtn.className = isMicMuted ? 'btn-circle btn-call-mute muted' : 'btn-circle btn-call-mute';
        muteBtn.innerHTML = isMicMuted ? '<span>🔇</span>' : '<span>🎙️</span>';
    }
}

// Audio Stream Capture & Playback
async function startAudioStream() {
    try {
        window.AudioContext = window.AudioContext || window.webkitAudioContext;
        audioContext = new AudioContext({ sampleRate: 16000 });
        nextPlayTime = audioContext.currentTime;

        mediaStream = await navigator.mediaDevices.getUserMedia({
            audio: {
                sampleRate: 16000,
                channelCount: 1,
                echoCancellation: true,
                noiseSuppression: true,
                autoGainControl: true
            }
        });

        const micSource = audioContext.createMediaStreamSource(mediaStream);
        scriptProcessor = audioContext.createScriptProcessor(1024, 1, 1);

        scriptProcessor.onaudioprocess = (e) => {
            if (callState !== 'active' || isMicMuted) return;

            const inputData = e.inputBuffer.getChannelData(0);
            // Convert Float32 to 16-bit PCM buffer
            const pcm16 = new Int16Array(inputData.length);
            for (let i = 0; i < inputData.length; i++) {
                let s = Math.max(-1, Math.min(1, inputData[i]));
                pcm16[i] = s < 0 ? s * 0x8000 : s * 0x7FFF;
            }

            if (callWs && callWs.readyState === WebSocket.OPEN) {
                callWs.send(pcm16.buffer);
            }
        };

        micSource.connect(scriptProcessor);
        scriptProcessor.connect(audioContext.destination);

    } catch (err) {
        console.error('Audio capture failed:', err);
        log(`⚠️ Không thể truy cập Micro: ${err.message}`, 'amber');
    }
}

function playIncomingAudioChunk(arrayBuffer) {
    if (!audioContext) return;

    try {
        const int16Array = new Int16Array(arrayBuffer);
        const float32Array = new Float32Array(int16Array.length);
        for (let i = 0; i < int16Array.length; i++) {
            float32Array[i] = int16Array[i] / 32768.0;
        }

        const audioBuffer = audioContext.createBuffer(1, float32Array.length, 16000);
        audioBuffer.copyToChannel(float32Array, 0);

        const source = audioContext.createBufferSource();
        source.buffer = audioBuffer;
        source.connect(audioContext.destination);

        const now = audioContext.currentTime;
        if (nextPlayTime < now) {
            nextPlayTime = now + 0.02;
        }
        source.start(nextPlayTime);
        nextPlayTime += audioBuffer.duration;

    } catch (err) {
        console.error('Error playing audio chunk:', err);
    }
}

function stopAudioStream() {
    if (scriptProcessor) {
        scriptProcessor.disconnect();
        scriptProcessor = null;
    }
    if (mediaStream) {
        mediaStream.getTracks().forEach(track => track.stop());
        mediaStream = null;
    }
    if (audioContext) {
        audioContext.close().catch(() => {});
        audioContext = null;
    }
}

// Initialize Voice Call Relay
initCallWebSocket();

