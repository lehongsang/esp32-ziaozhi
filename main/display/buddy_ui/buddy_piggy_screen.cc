#include "buddy_piggy_screen.h"
#include "assets/buddy_assets.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "BuddyPiggyScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

BuddyPiggyScreen::BuddyPiggyScreen() {}
BuddyPiggyScreen::~BuddyPiggyScreen() {}

void BuddyPiggyScreen::Create(lv_obj_t* parent) {
    // 1. Root Container
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x0C101A), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);

    // 2. Animated / Ambient Background
    bg_img_ = lv_image_create(container_);
    lv_image_set_src(bg_img_, &buddy_bg_day);
    lv_obj_set_size(bg_img_, 320, 240);
    lv_obj_align(bg_img_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_opa(bg_img_, LV_OPA_COVER, 0);

    // 3. Top Clock (Centered, clean without wifi/battery)
    time_label_ = lv_label_create(container_);
    lv_label_set_text(time_label_, "16:30");
    lv_obj_align(time_label_, LV_ALIGN_TOP_MID, 0, 8);
    lv_obj_set_style_text_color(time_label_, lv_color_hex(0xFFFFFF), 0);

    // 4. Speech Bubble / Greeting Message Card (Yellow Theme, Placed at bottom / waist level)
    speech_bubble_ = lv_obj_create(container_);
    lv_obj_remove_style_all(speech_bubble_);
    lv_obj_set_size(speech_bubble_, 288, 64);
    lv_obj_align(speech_bubble_, LV_ALIGN_BOTTOM_MID, 0, -16);
    lv_obj_set_style_bg_color(speech_bubble_, lv_color_hex(0xFACC15), 0); // Warm Yellow
    lv_obj_set_style_bg_opa(speech_bubble_, LV_OPA_90, 0);
    lv_obj_set_style_radius(speech_bubble_, 18, 0);
    lv_obj_set_style_border_color(speech_bubble_, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_width(speech_bubble_, 2, 0);
    lv_obj_set_style_shadow_width(speech_bubble_, 10, 0);
    lv_obj_set_style_shadow_color(speech_bubble_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(speech_bubble_, LV_OPA_40, 0);
    lv_obj_set_style_pad_hor(speech_bubble_, 12, 0);
    lv_obj_set_style_pad_ver(speech_bubble_, 8, 0);
    lv_obj_set_flex_flow(speech_bubble_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(speech_bubble_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(speech_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    speech_label_ = lv_label_create(speech_bubble_);
    lv_label_set_text(speech_label_, "Chào Minh!\nChúc con một ngày mới tràn đầy niềm vui!");
    lv_obj_set_style_text_color(speech_label_, lv_color_hex(0x451A03), 0); // Dark Amber Brown for high contrast on yellow
    lv_obj_set_style_text_align(speech_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(speech_label_, 264);
    lv_label_set_long_mode(speech_label_, LV_LABEL_LONG_WRAP);

    ESP_LOGI(TAG, "BuddyPiggyScreen created (Clock centered, yellow speech bubble at bottom)");
}

void BuddyPiggyScreen::OnPiggyTouchCb(lv_event_t* e) {
    auto* self = static_cast<BuddyPiggyScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->HandlePiggyTouch();
    }
}

void BuddyPiggyScreen::HandlePiggyTouch() {
    touch_count_++;
    if (speech_label_) {
        const char* quotes[] = {
            "Chào Minh!\nChúc con một ngày mới tràn đầy niềm vui!",
            "Chào con!\nChúc con một buổi sáng thật tốt lành!",
            "Cố lên nhé!\nHôm nay con sẽ làm thật xuất sắc!",
            "Tớ luôn ở đây\nđồng hành cùng con mỗi ngày!"
        };
        lv_label_set_text(speech_label_, quotes[touch_count_ % 4]);
    }
}

void BuddyPiggyScreen::UpdateTime(const std::string& time_str) {
    if (time_label_) {
        lv_label_set_text(time_label_, time_str.c_str());
    }
}

void BuddyPiggyScreen::SetGreeting(const std::string& child_name) {
    child_name_ = child_name;
    if (speech_label_) {
        std::string msg = "Chào " + child_name_ + "! Chúc con một ngày mới tràn đầy niềm vui!";
        lv_label_set_text(speech_label_, msg.c_str());
    }
}

void BuddyPiggyScreen::SetSpeechText(const std::string& text) {
    if (speech_label_) {
        lv_label_set_text(speech_label_, text.c_str());
    }
}

void BuddyPiggyScreen::SetBatteryLevel(int level, bool charging) {
    if (battery_label_) {
        if (charging) {
            lv_label_set_text(battery_label_, MATERIAL_SYMBOLS_BATTERY_ANDROID_FRAME_BOLT);
            lv_obj_set_style_text_color(battery_label_, lv_color_hex(0xFACC15), 0);
        } else if (level <= 20) {
            lv_label_set_text(battery_label_, MATERIAL_SYMBOLS_BATTERY_ANDROID_FRAME_ALERT);
            lv_obj_set_style_text_color(battery_label_, lv_color_hex(0xEF4444), 0);
        } else {
            lv_label_set_text(battery_label_, MATERIAL_SYMBOLS_BATTERY_ANDROID_FRAME_FULL);
            lv_obj_set_style_text_color(battery_label_, lv_color_hex(0x4ADE80), 0);
        }
    }
}

void BuddyPiggyScreen::SetWifiStatus(bool connected, int rssi) {
    if (wifi_label_) {
        lv_label_set_text(wifi_label_, connected ? MATERIAL_SYMBOLS_WIFI : MATERIAL_SYMBOLS_WIFI_OFF);
        lv_obj_set_style_text_color(wifi_label_, connected ? lv_color_hex(0x38BDF8) : lv_color_hex(0xEF4444), 0);
    }
}

void BuddyPiggyScreen::SetDayMode(bool is_day) {
    is_day_mode_ = is_day;
    if (bg_img_) {
        lv_image_set_src(bg_img_, is_day_mode_ ? &buddy_bg_day : &buddy_bg_night);
    }
}
