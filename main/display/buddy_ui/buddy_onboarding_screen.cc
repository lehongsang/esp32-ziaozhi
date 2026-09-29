#include "buddy_onboarding_screen.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "BuddyOnboarding"

LV_FONT_DECLARE(font_material_symbols_20_4);

BuddyOnboardingScreen::BuddyOnboardingScreen() {}
BuddyOnboardingScreen::~BuddyOnboardingScreen() {}

void BuddyOnboardingScreen::Create(lv_obj_t* parent) {
    // 1. Root Container Fullscreen
    root_ = lv_obj_create(parent);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, 320, 240);
    lv_obj_set_pos(root_, 0, 0);
    lv_obj_set_style_bg_color(root_, lv_color_hex(0x0C101A), 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN); // Hidden by default until checked

    // 2. 3D Background
    bg_img_ = lv_image_create(root_);
    lv_image_set_src(bg_img_, &buddy_bg_day);
    lv_obj_set_size(bg_img_, 320, 240);
    lv_obj_align(bg_img_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_opa(bg_img_, LV_OPA_50, 0);

    // 3. 3D Piggy Mascot Character
    piggy_img_ = lv_image_create(root_);
    lv_image_set_src(piggy_img_, &buddy_piggy_happy);
    lv_obj_align(piggy_img_, LV_ALIGN_TOP_LEFT, 12, 16);

    // 4. Speech Bubble from Piggy
    speech_bubble_ = lv_obj_create(root_);
    lv_obj_remove_style_all(speech_bubble_);
    lv_obj_set_size(speech_bubble_, 180, 54);
    lv_obj_align(speech_bubble_, LV_ALIGN_TOP_RIGHT, -12, 16);
    lv_obj_set_style_bg_color(speech_bubble_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_bg_opa(speech_bubble_, LV_OPA_90, 0);
    lv_obj_set_style_radius(speech_bubble_, 14, 0);
    lv_obj_set_style_border_color(speech_bubble_, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_width(speech_bubble_, 1, 0);
    lv_obj_set_style_pad_all(speech_bubble_, 8, 0);
    lv_obj_clear_flag(speech_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    speech_label_ = lv_label_create(speech_bubble_);
    lv_obj_set_style_text_font(speech_label_, GetBuddyFont(), 0);
    lv_label_set_text(speech_label_, "Chào bạn nhỏ!\nTên của con là gì nè?");
    lv_obj_set_style_text_color(speech_label_, lv_color_hex(0xFDE047), 0);
    lv_obj_center(speech_label_);

    // 5. Name Display Card
    name_card_ = lv_obj_create(root_);
    lv_obj_remove_style_all(name_card_);
    lv_obj_set_size(name_card_, 296, 40);
    lv_obj_align(name_card_, LV_ALIGN_TOP_MID, 0, 78);
    lv_obj_set_style_bg_color(name_card_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(name_card_, LV_OPA_90, 0);
    lv_obj_set_style_radius(name_card_, 12, 0);
    lv_obj_set_style_border_color(name_card_, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_width(name_card_, 2, 0);
    lv_obj_set_flex_flow(name_card_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(name_card_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(name_card_, LV_OBJ_FLAG_SCROLLABLE);

    name_label_ = lv_label_create(name_card_);
    lv_obj_set_style_text_font(name_label_, GetBuddyFont(), 0);
    lv_label_set_text(name_label_, "Tên bé: Minh");
    lv_obj_set_style_text_color(name_label_, lv_color_hex(0xFFFFFF), 0);

    // 6. Quick Name Suggestion Pills (1-tap selection for kids)
    quick_names_box_ = lv_obj_create(root_);
    lv_obj_remove_style_all(quick_names_box_);
    lv_obj_set_size(quick_names_box_, 296, 32);
    lv_obj_align(quick_names_box_, LV_ALIGN_TOP_MID, 0, 126);
    lv_obj_set_flex_flow(quick_names_box_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(quick_names_box_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(quick_names_box_, LV_OBJ_FLAG_SCROLLABLE);

    const char* quick_names[] = {"Minh", "Bảo An", "Alex", "Tít"};
    for (int i = 0; i < 4; ++i) {
        lv_obj_t* btn = lv_btn_create(quick_names_box_);
        lv_obj_set_size(btn, 68, 30);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E293B), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0284C7), LV_STATE_PRESSED);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x334155), 0);
        lv_obj_set_style_border_width(btn, 1, 0);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(btn, OnQuickNameClicked, LV_EVENT_CLICKED, this);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(lbl, GetBuddyFont(), 0);
        lv_label_set_text(lbl, quick_names[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xE2E8F0), 0);
        lv_obj_center(lbl);
    }

    // 7. Confirm Button: [BẮT ĐẦU CÙNG BUDDY 🚀]
    btn_confirm_ = lv_btn_create(root_);
    lv_obj_set_size(btn_confirm_, 296, 48);
    lv_obj_align(btn_confirm_, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_bg_color(btn_confirm_, lv_color_hex(0xEA580C), 0); // Vibrant Orange
    lv_obj_set_style_bg_color(btn_confirm_, lv_color_hex(0xC2410C), LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_confirm_, 16, 0);
    lv_obj_set_style_shadow_width(btn_confirm_, 12, 0);
    lv_obj_set_style_shadow_color(btn_confirm_, lv_color_hex(0xEA580C), 0);
    lv_obj_set_style_shadow_opa(btn_confirm_, LV_OPA_50, 0);
    lv_obj_set_flex_flow(btn_confirm_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_confirm_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn_confirm_, 8, 0);
    lv_obj_clear_flag(btn_confirm_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_confirm_, OnConfirmClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* rocket_icon = lv_label_create(btn_confirm_);
    lv_label_set_text(rocket_icon, MATERIAL_SYMBOLS_PLAY_ARROW);
    lv_obj_set_style_text_font(rocket_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(rocket_icon, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* confirm_label = lv_label_create(btn_confirm_);
    lv_obj_set_style_text_font(confirm_label, GetBuddyFont(), 0);
    lv_label_set_text(confirm_label, "BẮT ĐẦU CÙNG BUDDY");
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xFFFFFF), 0);

    ESP_LOGI(TAG, "BuddyOnboardingScreen created with crash-free name selection card");
}

void BuddyOnboardingScreen::Show() {
    if (!root_) return;
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
    is_visible_ = true;
    ESP_LOGI(TAG, "BuddyOnboardingScreen displayed");
}

void BuddyOnboardingScreen::Hide() {
    if (!root_) return;
    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
    is_visible_ = false;
    ESP_LOGI(TAG, "BuddyOnboardingScreen hidden");
}

void BuddyOnboardingScreen::OnQuickNameClicked(lv_event_t* e) {
    auto* self = static_cast<BuddyOnboardingScreen*>(lv_event_get_user_data(e));
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_current_target(e);
    if (!self || !btn || !self->name_label_) return;

    lv_obj_t* lbl = lv_obj_get_child(btn, 0);
    if (lbl) {
        const char* name = lv_label_get_text(lbl);
        self->selected_name_ = name ? name : "Minh";
        std::string txt = "Tên bé: " + self->selected_name_;
        lv_label_set_text(self->name_label_, txt.c_str());
        ESP_LOGI(TAG, "Selected quick name: %s", self->selected_name_.c_str());
    }
}

void BuddyOnboardingScreen::OnConfirmClicked(lv_event_t* e) {
    auto* self = static_cast<BuddyOnboardingScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Child confirmed name: %s", self->selected_name_.c_str());

    if (self->on_name_confirmed_) {
        self->on_name_confirmed_(self->selected_name_);
    }
}
