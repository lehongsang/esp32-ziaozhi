#include "buddy_home_screen.h"
#include "assets/buddy_assets.h"

#include <ctime>
#include <esp_log.h>

#define TAG "BuddyHomeScreen"

BuddyHomeScreen::BuddyHomeScreen() {
    interactive_quotes_ = {
        "Hello Alex! Ready for a brand new adventure today? ☀️",
        "I love learning and playing with you every day! 🐷",
        "Complete today's quests to earn yummy corn rewards! 🌽",
        "Hehe, you just tapped me! That tickles! ✨",
        "Swipe next to ask AI Tutor about your homework! 🎙️",
        "Would you like to hear a wonderful story? 📖",
        "Alex, let's build great daily habits together! 🌟"
    };
}

BuddyHomeScreen::~BuddyHomeScreen() {}

void BuddyHomeScreen::Create(lv_obj_t* parent) {
    // 1. Root Container
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Check initial system time for Day (06:00 - 18:00) vs Night (18:00 - 06:00)
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    int hour = (tm_info != nullptr) ? tm_info->tm_hour : 9;
    is_day_mode_ = (hour >= 6 && hour < 18);

    // 3. 3D Background Image Layer (320x240 RGB565)
    bg_img_ = lv_image_create(container_);
    lv_image_set_src(bg_img_, is_day_mode_ ? &buddy_bg_day : &buddy_bg_night);
    lv_obj_set_size(bg_img_, 320, 240);
    lv_obj_align(bg_img_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(bg_img_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bg_img_, OnScreenTouchCb, LV_EVENT_CLICKED, this);

    // 4. Top Status HUD Bar - Centered Clock Pill
    top_bar_ = lv_obj_create(container_);
    lv_obj_remove_style_all(top_bar_);
    lv_obj_set_size(top_bar_, 100, 24);
    lv_obj_align(top_bar_, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(top_bar_, lv_color_hex(0x0A0E1A), 0);
    lv_obj_set_style_bg_opa(top_bar_, LV_OPA_60, 0);
    lv_obj_set_style_radius(top_bar_, 12, 0);
    lv_obj_set_style_border_color(top_bar_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(top_bar_, LV_OPA_20, 0);
    lv_obj_set_style_border_width(top_bar_, 1, 0);
    lv_obj_set_flex_flow(top_bar_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top_bar_, LV_OBJ_FLAG_SCROLLABLE);

    // 4.1 Time Label Centered (inherits system text font)
    time_label_ = lv_label_create(top_bar_);
    lv_label_set_text(time_label_, is_day_mode_ ? "09:30" : "20:45");
    lv_obj_set_style_text_color(time_label_, lv_color_hex(0xFFFFFF), 0);

    // 5. Interactive Floating Speech Bubble with Circular Marquee (inherits system text font)
    speech_bubble_ = lv_obj_create(container_);
    lv_obj_remove_style_all(speech_bubble_);
    lv_obj_set_size(speech_bubble_, 290, 26);
    lv_obj_align(speech_bubble_, LV_ALIGN_TOP_MID, 0, 34);
    lv_obj_set_style_bg_color(speech_bubble_, lv_color_hex(0x101726), 0);
    lv_obj_set_style_bg_opa(speech_bubble_, LV_OPA_70, 0);
    lv_obj_set_style_radius(speech_bubble_, 10, 0);
    lv_obj_set_style_border_color(speech_bubble_, is_day_mode_ ? lv_color_hex(0xFFD166) : lv_color_hex(0x06D6A0), 0);
    lv_obj_set_style_border_opa(speech_bubble_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(speech_bubble_, 1, 0);
    lv_obj_set_style_pad_hor(speech_bubble_, 8, 0);
    lv_obj_set_flex_flow(speech_bubble_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(speech_bubble_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(speech_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    speech_label_ = lv_label_create(speech_bubble_);
    lv_obj_set_width(speech_label_, 274);
    lv_label_set_long_mode(speech_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(speech_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(speech_label_, is_day_mode_ ? "Hello Alex! Ready for a brand new adventure today? ☀️" : "Goodnight Alex, sweet dreams under the stars! 🌙");
    lv_obj_set_style_text_color(speech_label_, is_day_mode_ ? lv_color_hex(0xFFF3B0) : lv_color_hex(0x90E0EF), 0);
    lv_obj_set_style_text_color(speech_label_, is_day_mode_ ? lv_color_hex(0xFFF3B0) : lv_color_hex(0x90E0EF), 0);

    // 6. Bottom Level & XP Badge (Floating Pill - inherits system text font)
    level_badge_ = lv_obj_create(container_);
    lv_obj_remove_style_all(level_badge_);
    lv_obj_set_size(level_badge_, 200, 20);
    lv_obj_align(level_badge_, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_bg_color(level_badge_, lv_color_hex(0x050814), 0);
    lv_obj_set_style_bg_opa(level_badge_, LV_OPA_70, 0);
    lv_obj_set_style_radius(level_badge_, 10, 0);
    lv_obj_set_style_border_color(level_badge_, lv_color_hex(0xFFB703), 0);
    lv_obj_set_style_border_opa(level_badge_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(level_badge_, 1, 0);
    lv_obj_set_style_pad_hor(level_badge_, 6, 0);
    lv_obj_set_flex_flow(level_badge_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(level_badge_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(level_badge_, LV_OBJ_FLAG_SCROLLABLE);

    level_label_ = lv_label_create(level_badge_);
    lv_label_set_text(level_label_, "Lv.3  *  120/200 XP");
    lv_obj_set_style_text_color(level_label_, lv_color_hex(0xFFD166), 0);

    ESP_LOGI(TAG, "BuddyHomeScreen created with %s mode, Noto Sans font and circular scrolling text", is_day_mode_ ? "DAY" : "NIGHT");
}

void BuddyHomeScreen::OnScreenTouchCb(lv_event_t* e) {
    auto* self = static_cast<BuddyHomeScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->HandleCharacterTouch();
    }
}

void BuddyHomeScreen::HandleCharacterTouch() {
    if (interactive_quotes_.empty()) return;
    
    current_quote_idx_ = (current_quote_idx_ + 1) % interactive_quotes_.size();
    if (speech_label_) {
        lv_label_set_text(speech_label_, interactive_quotes_[current_quote_idx_].c_str());
    }

    // Bounce / highlight speech bubble on touch
    if (speech_bubble_) {
        lv_obj_set_style_border_color(speech_bubble_, lv_color_hex(0xFF006E), 0);
        lv_obj_set_style_border_width(speech_bubble_, 2, 0);
        
        // Simple scale/glow feedback
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, speech_bubble_);
        lv_anim_set_time(&a, 300);
        lv_anim_set_values(&a, 2, 1);
        lv_anim_set_custom_exec_cb(&a, [](lv_anim_t* anim, int32_t val) {
            auto* bubble = static_cast<lv_obj_t*>(anim->var);
            if (bubble) {
                lv_obj_set_style_border_width(bubble, val, 0);
            }
        });
        lv_anim_start(&a);
    }
}

void BuddyHomeScreen::UpdateTime(const std::string& time_str) {
    if (time_label_) {
        lv_label_set_text(time_label_, time_str.c_str());
    }

    // Parse hour from string if format is "HH:MM" or check system clock
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    if (tm_info) {
        CheckDayNightTransition(tm_info->tm_hour);
    }
}

void BuddyHomeScreen::CheckDayNightTransition(int hour) {
    bool should_be_day = (hour >= 6 && hour < 18);
    if (should_be_day != is_day_mode_) {
        SetDayMode(should_be_day);
    }
}

void BuddyHomeScreen::SetDayMode(bool is_day) {
    is_day_mode_ = is_day;
    if (bg_img_) {
        lv_image_set_src(bg_img_, is_day_mode_ ? &buddy_bg_day : &buddy_bg_night);
    }

    if (speech_bubble_ && speech_label_) {
        lv_obj_set_style_border_color(speech_bubble_, is_day_mode_ ? lv_color_hex(0xFFD166) : lv_color_hex(0x06D6A0), 0);
        lv_obj_set_style_text_color(speech_label_, is_day_mode_ ? lv_color_hex(0xFFF3B0) : lv_color_hex(0x90E0EF), 0);
        lv_label_set_text(speech_label_, is_day_mode_ ? "Chào Alex! Cùng bắt đầu ngày mới tràn đầy năng lượng nhé! ☀️" : "Chúc Alex ngủ ngon và có những giấc mơ thật đẹp nhé! 🌙");
    }

    ESP_LOGI(TAG, "Switched to %s mode", is_day_mode_ ? "DAY" : "NIGHT");
}

void BuddyHomeScreen::SetGreeting(const std::string& title, const std::string& subtitle) {
    SetSpeechText(title);
}

void BuddyHomeScreen::SetSpeechText(const std::string& text) {
    if (speech_label_) {
        lv_label_set_text(speech_label_, text.c_str());
    }
}

void BuddyHomeScreen::SetBatteryLevel(int level, bool charging) {
    if (battery_label_) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%s%d%%", charging ? "CHG " : "", level);
        lv_label_set_text(battery_label_, buf);
        if (level <= 20) {
            lv_obj_set_style_text_color(battery_label_, lv_color_hex(0xEF476F), 0);
        } else {
            lv_obj_set_style_text_color(battery_label_, lv_color_hex(0x52B788), 0);
        }
    }
}

void BuddyHomeScreen::SetWifiStatus(bool connected, int rssi) {
    if (wifi_label_) {
        lv_label_set_text(wifi_label_, connected ? "Wi-Fi" : "No Wi-Fi");
        lv_obj_set_style_text_color(wifi_label_, connected ? lv_color_hex(0x48CAE4) : lv_color_hex(0xEF476F), 0);
    }
}
