#ifndef BUDDY_PIGGY_SCREEN_H
#define BUDDY_PIGGY_SCREEN_H

#include <string>
#include <vector>
#include <lvgl.h>

class BuddyPiggyScreen {
public:
    BuddyPiggyScreen();
    ~BuddyPiggyScreen();

    void Create(lv_obj_t* parent);
    void UpdateTime(const std::string& time_str);
    void RefreshClock();
    void SetGreeting(const std::string& child_name);
    void SetSpeechText(const std::string& text);
    void SetBatteryLevel(int level, bool charging);
    void SetWifiStatus(bool connected, int rssi);
    void SetDayMode(bool is_day);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnPiggyTouchCb(lv_event_t* e);
    static void OnClockTimerCb(lv_timer_t* timer);
    void HandlePiggyTouch();

    lv_obj_t* container_ = nullptr;
    lv_obj_t* bg_img_ = nullptr;
    lv_obj_t* top_bar_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_timer_t* clock_timer_ = nullptr;
    lv_obj_t* wifi_label_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;

    // Speech Bubble
    lv_obj_t* speech_bubble_ = nullptr;
    lv_obj_t* speech_label_ = nullptr;

    // 3D Piggy Mascot
    lv_obj_t* piggy_box_ = nullptr;
    lv_obj_t* piggy_img_ = nullptr;

    // Hint text at bottom
    lv_obj_t* swipe_hint_ = nullptr;

    std::string child_name_ = "Minh";
    bool is_day_mode_ = true;
    int touch_count_ = 0;
};

#endif // BUDDY_PIGGY_SCREEN_H
