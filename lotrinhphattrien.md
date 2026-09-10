# Lộ Trình Phát Triển Thực Tế: MB BUDDY (MB Junior)
## Hệ Thống Thiết Bị AI Đồng Hành & Rèn Luyện Thói Quen (Firmware + Backend + App)


lệnh nạp code: .\flash_firmware.ps1 -Port COM3
lệnh đọc debug: .\monitor.bat COM3

---

## 📱 Cấu Trúc 5 Màn Hình Chính & Luồng Nhiệm Vụ Chi Tiết

Giao diện trên thiết bị được tổ chức theo kiến trúc **5 Màn Hình Chính (Main Swipe Carousel)**, có thể vuốt qua lại mượt mà bằng cảm ứng. Các luồng chi tiết (Quiz, thưởng, nuôi heo) là **Màn Hình Phụ (Sub-flow Screens)** nằm bên trong *Today's Quest*.

```
                                  [ 5 MÀN HÌNH CHÍNH (VUỐT NGANG) ]
   +----------------+   +----------------+   +----------------+   +----------------+   +----------------+
   |  Màn 1: CHÀO   |   | Màn 2: TODAY'S |   | Màn 3: AI      |   | Màn 4: SAVINGS |   | Màn 5: FAMILY  |
   |   BUDDY HOME   | < |     QUEST      | > |     TUTOR      | > |  (DREAM GOAL)  | > |     MOMENT     |
   +----------------+   +----------------+   +----------------+   +----------------+   +----------------+
                                |
                                v (Bấm chọn nhiệm vụ)
                     [ LUỒNG PHỤ: MISSION FLOW ]
                     +-----------------------------------+
                     | 2.1. View Mission Details         |
                     | 2.2. Do Mission (Trắc nghiệm Quiz)|
                     | 2.3. Mission Completed            |
                     | 2.4. Nhận Bắp (+2 Corn)           |
                     | 2.5. Cho Heo Ăn (Feed Piggy)      |
                     | 2.6. Tăng Cấp (+30 XP & Level Up) |
                     +-----------------------------------+
```

---

### 1. Chi Tiết Kịch Bản & Dữ Liệu 5 Màn Hình Chính

#### 🟢 Màn 1: Buddy Home (Màn Chào & Đồng Hồ)
- **Hiển thị:** 
  - Đồng hồ số thời gian thực (đồng bộ NTP qua Wi-Fi).
  - Nhân vật chú heo hồng 3D với các trạng thái động (chớp mắt, vẫy tay).
  - Câu chào ngữ cảnh (Sáng: *"Chào ngày mới, Alex! Sẵn sàng phiêu lưu chưa?"*, Tối: *"Đã đến giờ chuẩn bị đi ngủ rồi nhé"*).
  - Trạng thái hệ thống trên thanh status bar: Mức pin %, icon Wi-Fi/Bluetooth.
- **Nguồn dữ liệu:** `SystemTime`, `BatteryADC`, `WiFiStatus`, `PiggyMoodState`.

#### 🟢 Màn 2: Today's Quest (Danh Sách Nhiệm Vụ Trong Ngày)
- **Hiển thị:** 
  - Danh sách 3–5 nhiệm vụ hàng ngày (Ví dụ: 1. Bài tập toán; 2. Đọc sách 15 phút; 3. Vận động 20 phút).
  - Tiến độ (ví dụ: `8/15 phút`, `0/20 phút`) và dấu tích xanh ✅ khi hoàn thành.
  - Dòng nhắc: *"Còn 2 nhiệm vụ nữa để mở quà bí mật từ Heo Con!"*.
- **Tương tác:** Chạm vào bất kỳ nhiệm vụ nào để mở **Luồng Phụ (Sub-flow)**:
  - *2.1. Chi tiết nhiệm vụ:* Tên bài toán, số câu cần làm (10 bài), nút "Bắt đầu ngay".
  - *2.2. Màn hình làm bài:* Câu hỏi (`8 x 6 = ?`) và 4 ô chọn đáp án cảm ứng (`42`, `48`, `56`, `40`).
  - *2.3. Chúc mừng hoàn thành:* Hiển thị điểm số (`Completed 10/10`).
  - *2.4. Nhận thưởng bắp:* Hiển thị `+2 Corn` bay vào kho thức ăn.
  - *2.5. Nuôi heo (Piggy Farm):* Nút kéo bắp cho heo ăn, biểu cảm heo vui sướng (`Yummy! Cảm ơn bạn!`).
  - *2.6. Tăng cấp:* Hiệu ứng ngôi sao `+30 XP`, thanh cấp độ `Level 7 (250/300 XP)`.

#### 🟢 Màn 3: AI Tutor (Gia Sư Trợ Lý Giọng Nói AI)
- **Hiển thị:**
  - Robot thông minh AI với mắt led chuyển động theo giọng nói.
  - Lời mời: *"Mình có thể giúp gì cho bạn hôm nay?"*.
  - 3 nút bấm cảm ứng bên dưới: 
    - 🎙 **Micro:** Chạm/giữ để hỏi đáp trực tiếp bài tập bằng giọng nói (Voice-to-Voice).
    - 📷 **Camera:** Bật camera quét bài tập/vật thể (nếu có module camera).
    - ⌨️ **Bàn phím ảo/Quiz Mode:** Chuyển sang chế độ đố vui luyện tập phản xạ.
- **Xử lý:** Kích hoạt luồng WebSocket streaming âm thanh gửi đến LLM, nhận phản hồi giọng nói và hiển thị text câu trả lời trực tiếp trên màn hình.

#### 🟢 Màn 4: Savings (Mục Tiêu Tiết Kiệm - Dream Goal & Saving Missions)
- **Hiển thị:**
  - Tên mục tiêu tiết kiệm: *"Mua điện thoại mới"* hoặc *"Mua xe đạp mới"*.
  - Hình ảnh minh họa mục tiêu (điện thoại/xe đạp).
  - Thanh tiến độ hình tròn hoặc bán nguyệt: `% Hoàn thành (Ví dụ: 44%)`.
  - Số tiền: `1.250.000đ / 2.000.000đ` (hoặc 3.000.000đ).
  - Danh sách các **Saving Missions** (Nhiệm vụ kiếm tiền thưởng tiết kiệm: *"Dọn dẹp phòng +20.000đ"*, *"Đạt điểm 10 Toán +50.000đ"* do bố mẹ giao từ App).

#### 🟢 Màn 5: Family Moment (Hộp Thư Kết Nối Gia Đình)
- **Trạng thái chưa có dữ liệu (Placeholder):**
  - Hiển thị avatar bố mẹ (icon tròn) và biểu tượng trái tim ❤️ ở trạng thái chờ: *"Đang đợi lời nhắn yêu thương từ bố mẹ..."*.
- **Trạng thái khi nhận được tin nhắn (Từ App bố mẹ qua MQTT):**
  - Avatar người gửi (Mẹ / Bố).
  - Nội dung tin nhắn: *"Mẹ rất tự hào về con! Hãy cố gắng nhé!"*.
  - Nút chạm để thả tim ❤️ gửi phản hồi ngược lại về điện thoại của bố mẹ.

---

## 🗄 Cấu Trúc Dữ Liệu Thực Tế (Data Schema) Trên Thiết Bị & Backend

### 1. Dữ liệu Quest & Thói quen (`quest_data.json` / SQLite / NVS)
```json
{
  "quests": [
    {
      "id": "q_math_01",
      "title": "Bài tập Toán",
      "type": "quiz",
      "target": 10,
      "current": 10,
      "completed": true,
      "reward_corn": 2,
      "reward_xp": 30,
      "questions": [
        {
          "q": "8 x 6 = ?",
          "options": [42, 48, 56, 40],
          "answer_index": 1
        }
      ]
    },
    {
      "id": "q_read_02",
      "title": "Đọc sách 15 phút",
      "type": "timer",
      "target": 15,
      "current": 8,
      "completed": false,
      "reward_corn": 1,
      "reward_xp": 20
    }
  ]
}
```

### 2. Dữ liệu Mục tiêu Tiết kiệm (`saving_goal.json`)
```json
{
  "goal_name": "Mua điện thoại mới",
  "target_amount": 2000000,
  "current_amount": 1250000,
  "currency": "VND",
  "saving_missions": [
    {
      "title": "Rửa bát giúp mẹ",
      "reward_money": 20000,
      "status": "available"
    }
  ]
}
```

### 3. Dữ liệu Tin nhắn Gia đình (`family_moment.json`)
```json
{
  "has_message": true,
  "sender": "Mẹ",
  "avatar": "mom_avatar.png",
  "message": "Mẹ rất tự hào về con! Cố gắng hoàn thành bài toán nhé!",
  "timestamp": 1741138200,
  "liked": false
}
```

---

## 🚀 Kế Hoạch Triển Khai Thực Tế Từng Bước

### GIAI ĐOẠN 1: LẬP TRÌNH 5 MÀN HÌNH CHÍNH & LUỒNG NHIỆM VỤ (FIRMWARE)
*Thời gian: 3 tuần — Thực hiện trực tiếp trên bo mạch ESP32-S3 Display 2.8" hiện có*

* **Bước 1.1: Khởi tạo Bộ Quản Lý Giao Diện (Screen Manager):**
  - Viết `ScreenManager` bằng C++/LVGL: Hỗ trợ chuyển đổi giữa 5 màn hình chính bằng cử chỉ vuốt ngang (Swipe Left/Right) và chuyển màn hình bằng sự kiện.
* **Bước 1.2: Lập trình Màn 1 (Buddy Home) & Màn 3 (AI Tutor):**
  - Màn 1: Đọc giờ hệ thống, hiển thị chú heo động, kết nối trạng thái Wi-Fi/Pin thật.
  - Màn 3: Dựng robot AI gia sư, gán sự kiện bấm nút Micro để bắt đầu thu âm Voice.
* **Bước 1.3: Lập trình Màn 2 (Today's Quest) & Toàn Bộ Luồng Phụ (Sub-flow):**
  - Xây dựng danh sách quest thật từ bộ nhớ Flash.
  - Lập trình bộ máy thi trắc nghiệm (Quiz Engine): Hiển thị câu hỏi `8 x 6`, bắt sự kiện cảm ứng 4 nút chọn đáp án, kiểm tra đúng/sai, chuyển tiếp câu hỏi.
  - Lập trình màn hình nhận thưởng bắp, cơ chế kéo bắp cho heo ăn và thanh tăng cấp XP.
* **Bước 1.4: Lập trình Màn 4 (Savings) & Màn 5 (Family Moment):**
  - Màn 4: Hiển thị mục tiêu 2.000.000đ, tính toán vẽ vòng cung tiến độ `%`.
  - Màn 5: Hiển thị icon chờ khi chưa có tin nhắn; hỗ trợ nhận cấu trúc dữ liệu JSON tin nhắn để hiển thị ngay lời nhắn của bố mẹ.

---

### GIAI ĐOẠN 2: TÍCH HỢP VOICE AI GIA SƯ (BACKEND AI + WEBSOCKET)
*Thời gian: 2–3 tuần*

* **Bước 2.1: WebSocket Audio Streaming trên Firmware:**
  - Kết nối mic I2S (ES8311), nén Opus và stream lên server qua WebSocket khi bấm giữ nút Mic trên Màn 3.
  - Nhận luồng âm thanh Opus trả về từ server, giải mã và phát trực tiếp ra loa với độ trễ thấp.
* **Bước 2.2: Xây dựng AI Tutor Backend Server:**
  - Tiếp nhận audio stream -> Nhận dạng giọng nói Tiếng Việt (STT).
  - Tích hợp LLM (GPT-4o-mini / Claude 3.5 Sonnet) với System Prompt gia sư trẻ em: Hướng dẫn bé tự suy nghĩ giải bài tập, phản hồi ngắn gọn, thân thiện.
  - Chuyển văn bản thành giọng đọc trẻ em/hoạt hình cảm xúc (TTS).
* **Bước 2.3: Tool Calling trên Thiết bị (Function Calling):**
  - Bé nói: *"Buddy ơi, mở bài tập toán cho tớ"* -> AI nhận diện ý định và gửi lệnh JSON yêu cầu thiết bị tự động chuyển sang Màn 2 (Làm bài trắc nghiệm).

---

### GIAI ĐOẠN 3: XÂY DỰNG HỆ THỐNG BACKEND & MQTT SYNC CHO SAVINGS VÀ FAMILY
*Thời gian: 3 tuần*

* **Bước 3.1: Cơ Sở Dữ Liệu & REST API (Backend Service):**
  - API quản lý tài khoản gia đình, danh sách nhiệm vụ của bé, số dư tiết kiệm.
  - API tạo mục tiêu tiết kiệm mới (VD: Mua điện thoại 2 triệu).
* **Bước 3.2: Kênh MQTT Đồng Bộ Thời Gian Thực:**
  - Cấu hình MQTT Client trên ESP32-S3 kết nối tới Backend Broker (EMQX / HiveMQ / Mosquitto).
  - Khi bố mẹ gửi tin nhắn -> Server publish topic `device/{id}/family` -> Thiết bị nhận ngay và cập nhật Màn 5.
  - Khi bé hoàn thành bài toán trên thiết bị -> Thiết bị publish topic `device/{id}/quest_done` -> Báo về điện thoại bố mẹ.
* **Bước 3.3: Cấu hình Wi-Fi qua Bluetooth (BLE Provisioning):**
  - Tích hợp chuẩn nạp Wi-Fi BLE để bố mẹ không phải nhập mật khẩu rườm rà.

---

### GIAI ĐOẠN 4: ỨNG DỤNG PHỤ HUYNH TRÊN ĐIỆN THOẠI (PARENT MOBILE APP)
*Thời gian: 3 tuần*

* **Bước 4.1: Màn hình Giao Nhiệm Vụ & Báo Cáo:**
  - Bố mẹ chọn bài tập toán/đọc sách/việc nhà để gửi xuống thiết bị của con.
  - Xem biểu đồ thói quen hoàn thành của con theo tuần.
* **Bước 4.2: Màn hình Quản Lý Tiết Kiệm (Savings & Goal):**
  - Thiết lập mục tiêu mua sắm cho con (Tên món đồ, hình ảnh, giá tiền: 2.000.000đ).
  - Duyệt thưởng tiền vào heo đất khi con làm tốt việc nhà.
* **Bước 4.3: Màn hình Gửi Tin Nhắn & Thả Tim (Family Cheer):**
  - Soạn tin nhắn gửi tức thì về Màn 5 của thiết bị.
  - Nhận thông báo khi con bấm nút thả tim ❤️ đáp lại.

---

### GIAI ĐOẠN 5: TỐI ƯU PIN & ĐÓNG GÓI SẢN PHẨM THỰC TẾ
*Thời gian: 3–4 tuần*

* **Bước 5.1: Tối ưu Pin & Sleep Mode:**
  - Tự động tắt màn hình sau 30s không thao tác, đưa chip về Light Sleep (giữ kết nối MQTT để nhận tin nhắn từ bố mẹ).
  - Chạm cảm ứng màn hình để bật sáng lại ngay lập tức.
* **Bước 5.2: OTA Firmware Update:**
  - Cho phép nâng cấp firmware, tải thêm bộ câu hỏi mới qua Wi-Fi.
* **Bước 5.3: Chuẩn Bị Phần Cứng Thương Mại:**
  - Thiết kế mạch PCB tròn nhỏ gọn theo hình mặt gấu, gắn màn hình tròn IPS/AMOLED và đóng vỏ silicone hoàn chỉnh.
