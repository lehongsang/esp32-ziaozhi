#ifndef BUDDY_SCREEN_TYPES_H
#define BUDDY_SCREEN_TYPES_H

#include <cstdint>

enum class BuddyScreenId : uint8_t {
    kScreenPiggy = 0,    // Màn 1: Chú lợn chào & tương tác chính ban đầu
    kScreenHome = 1,     // Màn 2: Hôm nay có mấy việc (Thiết kế y hệt ảnh 3 - Chú gấu 3D)
    kScreenQuest = 2,    // Màn 3: Mission - Danh sách công việc chi tiết
    kScreenTutor = 3,    // Màn 4: AI Tutor (Gia sư trợ lý giọng nói)
    kScreenSavings = 4,  // Màn 5: Savings Dream Goal (Mục tiêu tiết kiệm)
    kScreenFamily = 5,   // Màn 6: Family Moment (Hộp thư gia đình)
    kScreenCount = 6
};

enum class QuestStatus : uint8_t {
    kNotStarted = 0,
    kInProgress = 1,
    kCompleted = 2
};

#endif // BUDDY_SCREEN_TYPES_H
