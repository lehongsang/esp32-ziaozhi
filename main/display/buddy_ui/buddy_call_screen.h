#ifndef BUDDY_CALL_SCREEN_H
#define BUDDY_CALL_SCREEN_H

#include <string>
#include <functional>
#include <lvgl.h>

class BuddyCallScreen {
public:
    BuddyCallScreen();
    ~BuddyCallScreen();

    void Create(lv_obj_t* parent);
    void SetParentStatus(bool online);
    void SetOnCallParent(std::function<void(const std::string& parent_role)> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnCallMomBtnCb(lv_event_t* e);
    static void OnCallDadBtnCb(lv_event_t* e);

    lv_obj_t* container_ = nullptr;
    lv_obj_t* header_pill_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* main_card_ = nullptr;
    lv_obj_t* family_img_ = nullptr;
    lv_obj_t* status_badge_ = nullptr;
    lv_obj_t* status_dot_ = nullptr;
    lv_obj_t* status_text_ = nullptr;
    lv_obj_t* desc_label_ = nullptr;

    lv_obj_t* btn_call_mom_ = nullptr;
    lv_obj_t* btn_call_dad_ = nullptr;

    bool is_online_ = true;
    std::function<void(const std::string& parent_role)> on_call_parent_cb_;
};

#endif // BUDDY_CALL_SCREEN_H
