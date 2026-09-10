#ifndef BUDDY_SCREEN_TYPES_H
#define BUDDY_SCREEN_TYPES_H

#include <cstdint>

enum class BuddyScreenId : uint8_t {
    kScreenHome = 0,     // Màn 1: Buddy Home (Chào & Đồng hồ)
    kScreenQuest = 1,    // Màn 2: Today's Quest (Nhiệm vụ hàng ngày)
    kScreenTutor = 2,    // Màn 3: AI Tutor (Gia sư trợ lý giọng nói)
    kScreenSavings = 3,  // Màn 4: Savings Dream Goal (Mục tiêu tiết kiệm)
    kScreenFamily = 4,   // Màn 5: Family Moment (Hộp thư gia đình)
    kScreenSettings = 5, // Màn 6: Settings (Cài đặt & Cấu hình Wi-Fi)
    kScreenCount = 6
};

enum class QuestStatus : uint8_t {
    kNotStarted = 0,
    kInProgress = 1,
    kCompleted = 2
};

#endif // BUDDY_SCREEN_TYPES_H
