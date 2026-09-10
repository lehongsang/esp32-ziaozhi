#ifndef FAMILY_MOMENT_SCREEN_H
#define FAMILY_MOMENT_SCREEN_H

#include <string>
#include <functional>
#include <cstdint>

#include <lvgl.h>

class FamilyMomentScreen {
public:
    FamilyMomentScreen();
    ~FamilyMomentScreen();

    void Create(lv_obj_t* parent);
    void SetMessage(const std::string& sender, const std::string& message);
    void ClearMessage(); // Returns to placeholder state
    void OnLikeClicked(std::function<void(bool liked)> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    lv_obj_t* container_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* msg_card_ = nullptr;
    lv_obj_t* sender_label_ = nullptr;
    lv_obj_t* msg_label_ = nullptr;
    lv_obj_t* btn_like_ = nullptr;
    lv_obj_t* like_icon_ = nullptr;
    lv_obj_t* placeholder_box_ = nullptr;

    bool is_liked_ = false;
    std::function<void(bool liked)> on_like_click_;
};

#endif // FAMILY_MOMENT_SCREEN_H
