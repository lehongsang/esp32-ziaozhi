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
    void SetMessage(const std::string& sender, const std::string& message, const std::string& timestamp = "Today");
    void ClearMessage(); // Returns to placeholder state
    void OnLikeClicked(std::function<void(bool liked)> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnLikeButtonEventCb(lv_event_t* e);
    void UpdateLikeButtonState();

    lv_obj_t* container_ = nullptr;
    
    // Top Header Pill
    lv_obj_t* header_pill_ = nullptr;
    lv_obj_t* title_label_ = nullptr;

    // View: Active Message Card
    lv_obj_t* msg_card_ = nullptr;
    lv_obj_t* sender_badge_ = nullptr;
    lv_obj_t* sender_label_ = nullptr;
    lv_obj_t* time_label_ = nullptr;
    lv_obj_t* msg_label_ = nullptr;
    lv_obj_t* btn_like_ = nullptr;
    lv_obj_t* like_icon_ = nullptr;
    lv_obj_t* like_text_ = nullptr;

    // View: Placeholder / Empty Card
    lv_obj_t* placeholder_card_ = nullptr;

    bool has_message_ = true;
    bool is_liked_ = false;
    std::string sender_name_ = "Mom";
    std::string message_body_ = "Con yêu ơi! Hôm nay con học rất chăm chỉ. Bố mẹ tự hào về con nhiều lắm!";
    std::string timestamp_ = "Today, 08:30";
    std::function<void(bool liked)> on_like_click_;
};

#endif // FAMILY_MOMENT_SCREEN_H
