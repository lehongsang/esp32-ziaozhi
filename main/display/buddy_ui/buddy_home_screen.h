#ifndef BUDDY_HOME_SCREEN_H
#define BUDDY_HOME_SCREEN_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>
#include <lvgl.h>

class BuddyHomeScreen {
public:
    BuddyHomeScreen();
    ~BuddyHomeScreen();

    void Create(lv_obj_t* parent);
    void UpdateTime(const std::string& time_str);
    void SetGreeting(const std::string& child_name, int total_quests);
    void SetQuestSummary(int total_quests, int completed_quests, const std::string& child_name = "Minh");
    void SetBatteryLevel(int level, bool charging);
    void SetWifiStatus(bool connected, int rssi);
    void SetSpeechText(const std::string& text);

    void CheckDayNightTransition(int hour);
    void SetDayMode(bool is_day);

    void SetOnViewQuests(std::function<void()> cb) { on_view_quests_ = cb; }
    void SetOnTalkBuddy(std::function<void()> cb) { on_talk_buddy_ = cb; }

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnViewQuestsClicked(lv_event_t* e);
    static void OnTalkBuddyClicked(lv_event_t* e);
    static void OnAvatarTouchCb(lv_event_t* e);
    void HandleAvatarTouch();

    lv_obj_t* container_ = nullptr;
    lv_obj_t* bg_img_ = nullptr;
    
    // Top Bar HUD
    lv_obj_t* top_bar_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* wifi_label_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;

    // Greeting & Mascot Header
    lv_obj_t* header_card_ = nullptr;
    lv_obj_t* avatar_box_ = nullptr;
    lv_obj_t* avatar_icon_ = nullptr;
    lv_obj_t* greeting_title_ = nullptr;
    lv_obj_t* greeting_sub_ = nullptr;

    // Action Buttons
    lv_obj_t* btn_container_ = nullptr;
    lv_obj_t* btn_view_quests_ = nullptr;
    lv_obj_t* btn_talk_buddy_ = nullptr;

    // Bottom Progress Bar & Star
    lv_obj_t* progress_card_ = nullptr;
    lv_obj_t* progress_ratio_label_ = nullptr;
    lv_obj_t* progress_bar_ = nullptr;
    lv_obj_t* star_icon_ = nullptr;

    bool is_day_mode_ = true;
    std::string child_name_ = "Minh";
    int total_quests_ = 3;
    int completed_quests_ = 2;

    std::function<void()> on_view_quests_;
    std::function<void()> on_talk_buddy_;
};

#endif // BUDDY_HOME_SCREEN_H
