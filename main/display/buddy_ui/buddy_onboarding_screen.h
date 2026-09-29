#ifndef BUDDY_ONBOARDING_SCREEN_H
#define BUDDY_ONBOARDING_SCREEN_H

#include <string>
#include <functional>
#include <lvgl.h>

class BuddyOnboardingScreen {
public:
    BuddyOnboardingScreen();
    ~BuddyOnboardingScreen();

    void Create(lv_obj_t* parent);
    void Show();
    void Hide();
    bool IsVisible() const { return is_visible_; }

    void SetOnNameConfirmed(std::function<void(const std::string& name)> cb) {
        on_name_confirmed_ = cb;
    }

private:
    static void OnConfirmClicked(lv_event_t* e);
    static void OnQuickNameClicked(lv_event_t* e);

    lv_obj_t* root_ = nullptr;
    lv_obj_t* bg_img_ = nullptr;
    lv_obj_t* piggy_img_ = nullptr;
    lv_obj_t* speech_bubble_ = nullptr;
    lv_obj_t* speech_label_ = nullptr;
    lv_obj_t* name_card_ = nullptr;
    lv_obj_t* name_label_ = nullptr;
    lv_obj_t* quick_names_box_ = nullptr;
    lv_obj_t* btn_confirm_ = nullptr;

    std::string selected_name_ = "Minh";
    bool is_visible_ = false;
    std::function<void(const std::string& name)> on_name_confirmed_;
};

#endif // BUDDY_ONBOARDING_SCREEN_H
