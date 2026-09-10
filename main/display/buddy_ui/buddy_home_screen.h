#ifndef BUDDY_HOME_SCREEN_H
#define BUDDY_HOME_SCREEN_H

#include <string>
#include <vector>
#include <cstdint>
#include <lvgl.h>

class BuddyHomeScreen {
public:
    BuddyHomeScreen();
    ~BuddyHomeScreen();

    void Create(lv_obj_t* parent);
    void UpdateTime(const std::string& time_str);
    void SetGreeting(const std::string& title, const std::string& subtitle);
    void SetBatteryLevel(int level, bool charging);
    void SetWifiStatus(bool connected, int rssi);
    void SetSpeechText(const std::string& text);

    void CheckDayNightTransition(int hour);
    void SetDayMode(bool is_day);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnScreenTouchCb(lv_event_t* e);
    void HandleCharacterTouch();

    lv_obj_t* container_ = nullptr;
    lv_obj_t* bg_img_ = nullptr;
    
    // Top Bar HUD
    lv_obj_t* top_bar_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* wifi_label_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;

    // Interactive Speech Bubble
    lv_obj_t* speech_bubble_ = nullptr;
    lv_obj_t* speech_label_ = nullptr;

    // Bottom Level & XP Badge
    lv_obj_t* level_badge_ = nullptr;
    lv_obj_t* level_label_ = nullptr;

    bool is_day_mode_ = true;
    int current_quote_idx_ = 0;
    std::vector<std::string> interactive_quotes_;
};

#endif // BUDDY_HOME_SCREEN_H
