# MB Buddy Backend - Tài Liệu Kỹ Thuật & Đặc Tả API (API Documentation)

Hệ thống Backend của **MB Buddy** (`buddy-backend`) cung cấp 4 kênh giao tiếp chính:
1. **REST APIs (`/api/*`)**: Quản lý thiết bị, nhiệm vụ (Quests), tiết kiệm/heo đất (Savings & Dream Goal), tin nhắn gia đình (Family Moments).
2. **Server-Sent Events (SSE - `/api/events`)**: Luồng cập nhật thời gian thực từ thiết bị lên Parent Web Dashboard.
3. **MQTT Broker (TCP Port 1883 & WebSocket `/mqtt`)**: Kênh truyền thông 2 chiều giữa ESP32 và Server/Dashboard với độ trễ thấp.
4. **WebSocket Voice Call Relay (`ws://.../call`)**: Kênh thoại trực tiếp (Voice Call) giữa Phụ huynh (Web/App) và Bé (ESP32) với luồng âm thanh Binary trực tiếp.

---

## 🌐 1. Cấu Hình & Endpoint Chung

- **📑 Swagger Interactive UI**: `http://<server-ip>:3000/api-docs` *(Xem và test trực tiếp trên trình duyệt)*
- **📄 OpenAPI 3.0 Spec (JSON)**: `http://<server-ip>:3000/api/swagger.json`
- **HTTP REST API Base URL**: `http://<server-ip>:3000/api`
- **SSE Event Stream**: `http://<server-ip>:3000/api/events`
- **MQTT Broker (TCP)**: `mqtt://<server-ip>:1883`
- **MQTT Broker (WebSocket)**: `ws://<server-ip>:3000/mqtt`
- **Voice Call Relay (WebSocket)**: `ws://<server-ip>:3000/call`
- **Web Parent Dashboard**: `http://<server-ip>:3000/`

---

## 📡 2. Chi Tiết REST APIs (`/api/*`)

### 2.1. Lấy Toàn Bộ Trạng Thái Thiết Bị (Full Sync)
Được ESP32 gọi khi khởi động hoặc Web Dashboard tải lần đầu để đồng bộ dữ liệu.

- **Method**: `GET`
- **Endpoint**: `/api/device/:id/state`
- **URL Params**:
  - `id` *(string)*: Mã định danh thiết bị (ví dụ: `default` hoặc `MB_BUDDY_01`).
- **Response `200 OK`**:
```json
{
  "device": {
    "id": "default",
    "name": "MB Buddy S3",
    "pairing_code": "123456",
    "battery": 95,
    "level": 3,
    "xp": 240,
    "last_seen": "2026-10-09 10:00:00"
  },
  "quests": [
    {
      "id": "q_math",
      "device_id": "default",
      "title": "Học Toán",
      "progress_text": "",
      "scheduled_time": "17:00 - 17:20",
      "start_time": "17:00",
      "duration": 20,
      "reward_stars": 1,
      "category": "math",
      "remind_before": 30,
      "completed": 1
    }
  ],
  "savings": {
    "device_id": "default",
    "current_amount": 850000,
    "target_amount": 2000000,
    "goal_type": 1,
    "goal_name": "Smart Robot",
    "currency": "d",
    "updated_at": "2026-10-09 10:00:00"
  },
  "family_message": {
    "id": 1,
    "device_id": "default",
    "sender": "Mom",
    "sender_role": "mom",
    "avatar": "mom_avatar.png",
    "message": "Mẹ rất tự hào về con! 💖",
    "timestamp": 1775707200,
    "liked": 1
  }
}
```

---

### 2.2. Quản Lý Nhiệm Vụ Hôm Nay (Quests - Màn 2)

#### a. Lấy danh sách nhiệm vụ
- **Method**: `GET`
- **Endpoint**: `/api/device/:id/quests`
- **Response `200 OK`**: Mảng danh sách các nhiệm vụ (`Quests[]`).

#### b. Thêm mới hoặc cập nhật nhiệm vụ
- **Method**: `POST`
- **Endpoint**: `/api/device/:id/quests`
- **Headers**: `Content-Type: application/json`
- **Request Body**:
```json
{
  "id": "q_eng_01",
  "title": "Luyện nghe Tiếng Anh",
  "progress_text": "0/1",
  "scheduled_time": "20:00 - 20:30",
  "start_time": "20:00",
  "duration": 30,
  "reward_stars": 2,
  "category": "english",
  "remind_before": 15
}
```
- **Response `200 OK`**:
```json
{
  "success": true,
  "quests": [ /* Toàn bộ danh sách nhiệm vụ sau khi thêm/sửa */ ]
}
```
*(Tự động phát MQTT topic `buddy/:id/quests/set` để ESP32 cập nhật màn hình ngay lập tức)*.

#### c. Đảo trạng thái hoàn thành (Toggle Quest)
- **Method**: `POST`
- **Endpoint**: `/api/device/:id/quests/:questId/toggle`
- **Response `200 OK`**: `{ "success": true, "quests": [...] }`

#### d. Xóa 1 nhiệm vụ
- **Method**: `DELETE`
- **Endpoint**: `/api/device/:id/quests/:questId`
- **Response `200 OK`**: `{ "success": true, "quests": [...] }`

#### e. Xóa toàn bộ nhiệm vụ của thiết bị
- **Method**: `DELETE`
- **Endpoint**: `/api/device/:id/quests`
- **Response `200 OK`**: `{ "success": true, "quests": [] }`

---

### 2.3. Quản Lý Heo Đất & Ước Mơ (Savings - Màn 4)

#### a. Lấy thông tin tiết kiệm & mục tiêu hiện tại
- **Method**: `GET`
- **Endpoint**: `/api/device/:id/savings`
- **Response `200 OK`**:
```json
{
  "device_id": "default",
  "current_amount": 850000,
  "target_amount": 2000000,
  "goal_type": 1,
  "goal_name": "Smart Robot",
  "currency": "d"
}
```

#### b. Nạp / Rút tiền tiết kiệm (Thưởng nhiệm vụ / Gửi quà)
- **Method**: `POST`
- **Endpoint**: `/api/device/:id/savings/deposit`
- **Request Body**:
```json
{
  "amount": 50000,
  "note": "Thưởng dọn phòng sạch sẽ"
}
```
- **Response `200 OK`**:
```json
{
  "success": true,
  "savings": {
    "current_amount": 900000,
    "target_amount": 2000000,
    "goal_type": 1,
    "goal_name": "Smart Robot",
    "currency": "d"
  }
}
```
*(Tự động phát MQTT topic `buddy/:id/savings/set` xuống ESP32)*.

#### c. Thay đổi Mục tiêu Ước Mơ (Dream Goal)
- **Method**: `PUT`
- **Endpoint**: `/api/device/:id/savings/goal`
- **Request Body**:
```json
{
  "goal_type": 2,
  "goal_name": "Xe đạp thể thao",
  "target_amount": 2500000
}
```
- **Response `200 OK`**: `{ "success": true, "savings": { ... } }`

---

### 2.4. Tin Nhắn Yêu Thương Gia Đình (Family Moments - Màn 5)

#### a. Lấy tin nhắn mới nhất
- **Method**: `GET`
- **Endpoint**: `/api/device/:id/family/message`
- **Response `200 OK`**:
```json
{
  "id": 1,
  "device_id": "default",
  "sender": "Mom",
  "sender_role": "mom",
  "avatar": "mom_avatar.png",
  "message": "Mẹ rất tự hào về con! 💖",
  "timestamp": 1775707200,
  "liked": 0
}
```

#### b. Gửi tin nhắn động viên mới xuống thiết bị
- **Method**: `POST`
- **Endpoint**: `/api/device/:id/family/message`
- **Request Body**:
```json
{
  "sender": "Bố",
  "sender_role": "dad",
  "message": "Hôm nay con hoàn thành bài kiểm tra rất tốt, cuối tuần bố dẫn đi chơi nhé!"
}
```
- **Response `200 OK`**:
```json
{
  "success": true,
  "message": {
    "id": 2,
    "sender": "Bố",
    "sender_role": "dad",
    "message": "...",
    "timestamp": 1775707320,
    "liked": false
  }
}
```
*(Tự động phát MQTT topic `buddy/:id/family/message` xuống ESP32 để rung chuông / hiển thị pop-up)*.

---

## ⚡ 3. Server-Sent Events (SSE) `/api/events`

Web Dashboard kết nối tới SSE Stream để nhận cập nhật tự động khi có bất kỳ thay đổi nào từ phía thiết bị ESP32:
- **Format**: `data: {"type": "<EVENT_TYPE>", "data": {...}, "timestamp": 1775707200000}\n\n`
- **Các sự kiện (Event Types)**:
  - `device_connected`: ESP32 vừa kết nối MQTT.
  - `device_disconnected`: ESP32 ngắt kết nối.
  - `quest_completed`: Bé hoàn thành một nhiệm vụ trên màn hình.
  - `family_love_received`: Bé nhấn nút thả tim ❤️ trên màn hình Moment.
  - `goal_changed`: Bé chọn đổi mục tiêu ước mơ mới từ màn hình thiết bị.
  - `device_status`: Cập nhật dung lượng pin, cấp độ (level), điểm kinh nghiệm (xp).
  - `call_signal`: Trạng thái gọi điện từ thiết bị.

---

## 📻 4. Giao Thức MQTT Topics (ESP32 <-> Backend)

Tiền tố topic mặc định: `buddy/{deviceId}/...`

### 4.1. Chiều Server -> ESP32 (Downlink)
| Topic | Payload Format | Ý nghĩa / Hành động của ESP32 |
|---|---|---|
| `buddy/{id}/time/set` | `{"timestamp": 1775707200000, "timezone_offset": 420}` | Đồng bộ giờ thực tế (RTC / SNTP) cho đồng hồ |
| `buddy/{id}/quests/set` | `{"quests": [ ... ]}` | Cập nhật toàn bộ danh sách nhiệm vụ trên UI |
| `buddy/{id}/savings/set` | `{"current_amount": 900000, "target_amount": 2000000, ...}` | Cập nhật heo đất và thanh tiến độ mục tiêu |
| `buddy/{id}/family/message`| `{"id": 2, "sender": "Mom", "message": "...", ...}` | Hiển thị tin nhắn gia đình và biểu tượng chuông báo |
| `buddy/{id}/call/request` | `{"type": "incoming_call", "caller": "Mẹ", "deviceId": "..."}` | Bật chuông cuộc gọi đến từ phụ huynh |
| `buddy/{id}/call/accept` | `{"type": "call_connected", "deviceId": "..."}` | Báo cuộc gọi đã được kết nối |
| `buddy/{id}/call/reject` | `{"type": "call_rejected", "reason": "Bận"}` | Báo cuộc gọi bị từ chối |
| `buddy/{id}/call/end` | `{"type": "call_ended", "deviceId": "..."}` | Kết thúc cuộc gọi |
| `buddy/{id}/call/audio/down`| Raw Binary Audio (PCM / Opus stream) | Dữ liệu âm thanh trực tiếp từ Phụ huynh phát ra loa ESP32 |

### 4.2. Chiều ESP32 -> Server (Uplink)
| Topic | Payload Format | Ý nghĩa / Hành động của Backend |
|---|---|---|
| `buddy/{id}/time/get` | `{}` | ESP32 yêu cầu lấy giờ hiện tại khi vừa có Wifi |
| `buddy/{id}/quests/completed` | `{"quest_id": "q_math"}` | Bé đánh dấu hoàn thành nhiệm vụ trên màn hình cảm ứng |
| `buddy/{id}/family/like` | `{"action": "love"}` | Bé chạm vào icon gửi tim cho ba mẹ |
| `buddy/{id}/savings/goal_changed`| `{"goal_type": 2, "goal_name": "Xe đạp", "target_amount": 2500000}` | Bé chọn mục tiêu mới trên màn hình Buddy |
| `buddy/{id}/status` | `{"battery": 88, "level": 3, "xp": 260}` | Gửi thông số pin và cấp độ định kỳ |
| `buddy/{id}/call/request` | `{"caller": "Bé Minh"}` | Bé nhấn gọi ba mẹ từ thiết bị |
| `buddy/{id}/call/accept` | `{}` | Bé nhấn chấp nhận cuộc gọi |
| `buddy/{id}/call/reject` | `{"reason": "Đang bận học"}` | Bé từ chối cuộc gọi |
| `buddy/{id}/call/end` | `{}` | Bé nhấn cúp máy |
| `buddy/{id}/call/audio/up` | Raw Binary Audio (PCM / Opus stream) | Âm thanh thu từ micro ESP32 đẩy thẳng lên trình duyệt Phụ huynh |

---

## 📞 5. Giao Thức WebSocket Voice Call Relay (`/call`)

- **URL**: `ws://<server-ip>:3000/call?type=<parent|device>&deviceId=<deviceId>`
  - `type`: `parent` (Trình duyệt Web Dashboard) hoặc `device` (ESP32).
  - `deviceId`: ID của thiết bị (mặc định `default`).

### Luồng Tín Hiệu Cuộc Gọi (JSON Control Signals):
1. **Yêu cầu gọi đi**: Gửi `{"type": "call_request", "caller": "Mẹ"}`
   - Phía nhận được thông điệp: `{"type": "incoming_call", "caller": "Mẹ", "from": "parent", "deviceId": "..."}`
2. **Chấp nhận cuộc gọi**: Gửi `{"type": "call_accept"}`
   - Cả 2 bên nhận được: `{"type": "call_connected", "deviceId": "...", "startTime": 1775707200000}`
3. **Từ chối cuộc gọi**: Gửi `{"type": "call_reject", "reason": "Bận"}`
   - Cả 2 bên nhận được: `{"type": "call_rejected", "reason": "...", "from": "..."}`
4. **Kết thúc cuộc gọi**: Gửi `{"type": "call_end"}`
   - Cả 2 bên nhận được: `{"type": "call_ended", "duration": 45}`
5. **Kiểm tra trạng thái thiết bị**:
   - Khi `parent` kết nối, server trả về: `{"type": "device_status", "online": true, "callState": "idle", "deviceId": "..."}`

### Luồng Âm Thanh Trực Tiếp (Binary Frame Streaming):
- Khi trạng thái là `active`, các khung dữ liệu nhị phân (`Binary WebSocket frames` chứa raw audio PCM 16kHz 16-bit Mono) được chuyển tiếp lập tức hai chiều giữa trình duyệt Phụ huynh và ESP32 với độ trễ cực thấp (< 50ms).
