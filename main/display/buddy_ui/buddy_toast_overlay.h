#ifndef BUDDY_TOAST_OVERLAY_H
#define BUDDY_TOAST_OVERLAY_H

#include <string>
#include <lvgl.h>

enum class ToastType {
    kMessage,
    kNewQuest,
    kReminder,
    kReward,
    kWarning,
    kInfo
};

class BuddyToastOverlay {
public:
    static BuddyToastOverlay& GetInstance();

    void Initialize(lv_obj_t* root_layer);
    void Show(const std::string& title, const std::string& body, ToastType type = ToastType::kMessage, uint32_t duration_ms = 4000);
    void Hide();

private:
    BuddyToastOverlay();
    ~BuddyToastOverlay();

    static void OnToastClickedCb(lv_event_t* e);
    static void AutoHideTimerCb(lv_timer_t* timer);

    lv_obj_t* container_ = nullptr;
    lv_obj_t* icon_label_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* body_label_ = nullptr;
    lv_timer_t* hide_timer_ = nullptr;
    bool is_visible_ = false;
};

#endif // BUDDY_TOAST_OVERLAY_H
