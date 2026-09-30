#include "buddy_home_screen.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"
#include <material_symbols.h>
#include <esp_log.h>
#include <ctime>

#define TAG "BuddyHomeScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_material_symbols_30_4);

BuddyHomeScreen::BuddyHomeScreen() {}
BuddyHomeScreen::~BuddyHomeScreen() {}

void BuddyHomeScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Deep Space Black #080C15)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x080C15), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);

    // 2. Left Column: 3D Mascot Hero Card (Pos: 12, 32)
    avatar_box_ = lv_obj_create(container_);
    lv_obj_remove_style_all(avatar_box_);
    lv_obj_set_size(avatar_box_, 90, 90);
    lv_obj_set_pos(avatar_box_, 12, 32);
    lv_obj_set_style_bg_color(avatar_box_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(avatar_box_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(avatar_box_, 22, 0);
    lv_obj_set_style_border_color(avatar_box_, lv_color_hex(0xF59E0B), 0); // Amber Gold Border
    lv_obj_set_style_border_width(avatar_box_, 2, 0);
    lv_obj_set_style_shadow_width(avatar_box_, 12, 0);
    lv_obj_set_style_shadow_color(avatar_box_, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_shadow_opa(avatar_box_, LV_OPA_40, 0);
    lv_obj_clear_flag(avatar_box_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(avatar_box_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(avatar_box_, OnAvatarTouchCb, LV_EVENT_CLICKED, this);

    lv_obj_t* avatar_img = lv_image_create(avatar_box_);
    lv_image_set_src(avatar_img, &buddy_bear_mascot);
    lv_obj_center(avatar_img);

    // Level Tag Pill under Mascot (y=130)
    lv_obj_t* level_pill = lv_obj_create(container_);
    lv_obj_remove_style_all(level_pill);
    lv_obj_set_size(level_pill, 90, 24);
    lv_obj_set_pos(level_pill, 12, 130);
    lv_obj_set_style_bg_color(level_pill, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_radius(level_pill, 12, 0);
    lv_obj_set_style_border_color(level_pill, lv_color_hex(0x334155), 0);
    lv_obj_set_style_border_width(level_pill, 1, 0);
    lv_obj_set_flex_flow(level_pill, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(level_pill, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(level_pill, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* level_txt = lv_label_create(level_pill);
    lv_label_set_text(level_txt, "Level 1");
    lv_obj_set_style_text_color(level_txt, lv_color_hex(0xFBBF24), 0);

    // 3. Right Column - Top Greeting Card (Spacious 196x76, Pos: 112, 32, Align START)
    header_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_card_);
    lv_obj_set_size(header_card_, 196, 76);
    lv_obj_set_pos(header_card_, 112, 32);
    lv_obj_set_style_bg_color(header_card_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(header_card_, LV_OPA_90, 0);
    lv_obj_set_style_radius(header_card_, 16, 0);
    lv_obj_set_style_border_color(header_card_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(header_card_, 1, 0);
    lv_obj_set_style_pad_hor(header_card_, 10, 0);
    lv_obj_set_style_pad_ver(header_card_, 8, 0);
    lv_obj_set_flex_flow(header_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(header_card_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_gap(header_card_, 4, 0);
    lv_obj_clear_flag(header_card_, LV_OBJ_FLAG_SCROLLABLE);

    greeting_title_ = lv_label_create(header_card_);
    lv_label_set_text(greeting_title_, "Chào Minh!");
    lv_obj_set_style_text_color(greeting_title_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_width(greeting_title_, 176);

    greeting_sub_ = lv_label_create(header_card_);
    lv_label_set_text(greeting_sub_, "Hôm nay con có 3 việc");
    lv_obj_set_style_text_color(greeting_sub_, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_width(greeting_sub_, 176);

    // 4. Right Column - 2 Action Buttons [XEM VIỆC] & [BUDDY ƠI] (Pos: 112, 116, Size: 196x88)
    btn_container_ = lv_obj_create(container_);
    lv_obj_remove_style_all(btn_container_);
    lv_obj_set_size(btn_container_, 196, 88);
    lv_obj_set_pos(btn_container_, 112, 116);
    lv_obj_set_flex_flow(btn_container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_container_, LV_OBJ_FLAG_SCROLLABLE);

    // 4.1 Nút 1: [XEM VIỆC] (Royal Blue Glow)
    btn_view_quests_ = lv_btn_create(btn_container_);
    lv_obj_remove_style_all(btn_view_quests_);
    lv_obj_set_size(btn_view_quests_, 94, 86);
    lv_obj_set_style_bg_color(btn_view_quests_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_bg_opa(btn_view_quests_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btn_view_quests_, lv_color_hex(0x1D4ED8), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_view_quests_, 18, 0);
    lv_obj_set_style_shadow_width(btn_view_quests_, 10, 0);
    lv_obj_set_style_shadow_color(btn_view_quests_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_shadow_opa(btn_view_quests_, LV_OPA_40, 0);
    lv_obj_set_flex_flow(btn_view_quests_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_view_quests_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn_view_quests_, 4, 0);
    lv_obj_clear_flag(btn_view_quests_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_view_quests_, OnViewQuestsClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* icon_view = lv_label_create(btn_view_quests_);
    lv_label_set_text(icon_view, MATERIAL_SYMBOLS_EDIT_SQUARE);
    lv_obj_set_style_text_font(icon_view, &font_material_symbols_30_4, 0);
    lv_obj_set_style_text_color(icon_view, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* label_view = lv_label_create(btn_view_quests_);
    lv_label_set_text(label_view, "Xem việc");
    lv_obj_set_style_text_color(label_view, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(label_view, 3, 0);

    // 4.2 Nút 2: [BUDDY ƠI] (Emerald Green Glow)
    btn_talk_buddy_ = lv_btn_create(btn_container_);
    lv_obj_remove_style_all(btn_talk_buddy_);
    lv_obj_set_size(btn_talk_buddy_, 94, 86);
    lv_obj_set_style_bg_color(btn_talk_buddy_, lv_color_hex(0x16A34A), 0);
    lv_obj_set_style_bg_opa(btn_talk_buddy_, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btn_talk_buddy_, lv_color_hex(0x15803D), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_talk_buddy_, 18, 0);
    lv_obj_set_style_shadow_width(btn_talk_buddy_, 10, 0);
    lv_obj_set_style_shadow_color(btn_talk_buddy_, lv_color_hex(0x16A34A), 0);
    lv_obj_set_style_shadow_opa(btn_talk_buddy_, LV_OPA_40, 0);
    lv_obj_set_flex_flow(btn_talk_buddy_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_talk_buddy_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn_talk_buddy_, 4, 0);
    lv_obj_clear_flag(btn_talk_buddy_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_talk_buddy_, OnTalkBuddyClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* icon_talk = lv_label_create(btn_talk_buddy_);
    lv_label_set_text(icon_talk, MATERIAL_SYMBOLS_ROBOT_2);
    lv_obj_set_style_text_font(icon_talk, &font_material_symbols_30_4, 0);
    lv_obj_set_style_text_color(icon_talk, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* label_talk = lv_label_create(btn_talk_buddy_);
    lv_label_set_text(label_talk, "Buddy ơi");
    lv_obj_set_style_text_color(label_talk, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(label_talk, 3, 0);

    ESP_LOGI(TAG, "BuddyHomeScreen created: Stable clean layout inheriting theme font");
}

void BuddyHomeScreen::OnViewQuestsClicked(lv_event_t* e) {
    auto* self = static_cast<BuddyHomeScreen*>(lv_event_get_user_data(e));
    if (self && self->on_view_quests_) {
        self->on_view_quests_();
    }
}

void BuddyHomeScreen::OnTalkBuddyClicked(lv_event_t* e) {
    auto* self = static_cast<BuddyHomeScreen*>(lv_event_get_user_data(e));
    if (self && self->on_talk_buddy_) {
        self->on_talk_buddy_();
    }
}

void BuddyHomeScreen::OnAvatarTouchCb(lv_event_t* e) {
    auto* self = static_cast<BuddyHomeScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->HandleAvatarTouch();
    }
}

void BuddyHomeScreen::HandleAvatarTouch() {
    // Little jump/pulse animation on mascot
    if (avatar_box_) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, avatar_box_);
        lv_anim_set_time(&a, 150);
        lv_anim_set_playback_time(&a, 150);
        lv_anim_set_values(&a, 36, 28);
        lv_anim_set_custom_exec_cb(&a, [](lv_anim_t* anim, int32_t val) {
            auto* obj = static_cast<lv_obj_t*>(anim->var);
            if (obj) {
                lv_obj_set_pos(obj, 16, val);
            }
        });
        lv_anim_start(&a);
    }
}

void BuddyHomeScreen::UpdateTime(const std::string& time_str) {
    if (time_label_) {
        lv_label_set_text(time_label_, time_str.c_str());
    }
}

void BuddyHomeScreen::SetGreeting(const std::string& child_name, int total_quests) {
    child_name_ = child_name;
    total_quests_ = total_quests;
    if (greeting_title_) {
        std::string title = "Chào " + child_name_ + "!";
        lv_label_set_text(greeting_title_, SanitizeVietnamese(title).c_str());
    }
    if (greeting_sub_) {
        std::string sub = "Hôm nay con có " + std::to_string(total_quests_) + " việc";
        lv_label_set_text(greeting_sub_, SanitizeVietnamese(sub).c_str());
    }
}

void BuddyHomeScreen::SetQuestSummary(int total_quests, int completed_quests, const std::string& child_name) {
    total_quests_ = total_quests;
    completed_quests_ = completed_quests;
    if (!child_name.empty()) {
        child_name_ = child_name;
    }

    SetGreeting(child_name_, total_quests_);

    if (progress_ratio_label_) {
        std::string ratio = std::to_string(completed_quests_) + "/" + std::to_string(total_quests_);
        lv_label_set_text(progress_ratio_label_, ratio.c_str());
    }

    if (progress_bar_ && total_quests_ > 0) {
        int percent = (completed_quests_ * 100) / total_quests_;
        lv_bar_set_value(progress_bar_, percent, LV_ANIM_ON);
    }
}

void BuddyHomeScreen::SetSpeechText(const std::string& text) {
    if (greeting_sub_) {
        lv_label_set_text(greeting_sub_, text.c_str());
    }
}

void BuddyHomeScreen::SetBatteryLevel(int level, bool charging) {
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

void BuddyHomeScreen::SetWifiStatus(bool connected, int rssi) {
    if (wifi_label_) {
        lv_label_set_text(wifi_label_, connected ? MATERIAL_SYMBOLS_WIFI : MATERIAL_SYMBOLS_WIFI_OFF);
        lv_obj_set_style_text_color(wifi_label_, connected ? lv_color_hex(0x38BDF8) : lv_color_hex(0xEF4444), 0);
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
}
