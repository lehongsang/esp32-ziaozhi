#include "buddy_call_screen.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"
#include "buddy_toast_overlay.h"
#include "buddy_call_overlay.h"
#include "application.h"
#include "assets/lang_config.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "BuddyCallScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

BuddyCallScreen::BuddyCallScreen() {}
BuddyCallScreen::~BuddyCallScreen() {}

void BuddyCallScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Deep Slate Velvet Background)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x080C16), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Top Header Floating Glass Pill (Centered, Pos: 40, 4, Size: 240x28)
    header_pill_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_pill_);
    lv_obj_set_size(header_pill_, 240, 28);
    lv_obj_align(header_pill_, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(header_pill_, lv_color_hex(0x131D31), 0);
    lv_obj_set_style_bg_opa(header_pill_, LV_OPA_90, 0);
    lv_obj_set_style_radius(header_pill_, 14, 0);
    lv_obj_set_style_border_color(header_pill_, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_opa(header_pill_, LV_OPA_60, 0);
    lv_obj_set_style_border_width(header_pill_, 1, 0);
    lv_obj_set_flex_flow(header_pill_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header_pill_, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(header_pill_);
    lv_obj_set_style_text_font(title_label_, GetBuddyFont(), 0);
    lv_label_set_text(title_label_, "📞 Gọi Cho Bố Mẹ");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xF8FAFC), 0);

    // 3. Main Family Card (Pos: 10, 36, Size: 300x198)
    main_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(main_card_);
    lv_obj_set_size(main_card_, 300, 198);
    lv_obj_set_pos(main_card_, 10, 36);
    lv_obj_set_style_bg_color(main_card_, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(main_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(main_card_, 18, 0);
    lv_obj_set_style_border_color(main_card_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(main_card_, 1, 0);
    lv_obj_set_flex_flow(main_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(main_card_, 10, 0);
    lv_obj_clear_flag(main_card_, LV_OBJ_FLAG_SCROLLABLE);

    // 3.1 Top Content Row: Left 3D Family Artwork + Right Status & Text
    lv_obj_t* top_row = lv_obj_create(main_card_);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_size(top_row, 280, 128);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top_row, LV_OBJ_FLAG_SCROLLABLE);

    // Left: 3D AI Family Artwork (140x110)
    family_img_ = lv_image_create(top_row);
    lv_image_set_src(family_img_, &buddy_family_call);
    lv_obj_set_size(family_img_, 140, 110);
    lv_obj_set_style_radius(family_img_, 14, 0);
    lv_obj_set_style_clip_corner(family_img_, true, 0);
    lv_obj_set_style_border_color(family_img_, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_opa(family_img_, LV_OPA_40, 0);
    lv_obj_set_style_border_width(family_img_, 1, 0);

    // Right: Info Column (132x120)
    lv_obj_t* info_col = lv_obj_create(top_row);
    lv_obj_remove_style_all(info_col);
    lv_obj_set_size(info_col, 132, 120);
    lv_obj_set_flex_flow(info_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_col, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_hor(info_col, 4, 0);
    lv_obj_clear_flag(info_col, LV_OBJ_FLAG_SCROLLABLE);

    // Status Pill
    status_badge_ = lv_obj_create(info_col);
    lv_obj_remove_style_all(status_badge_);
    lv_obj_set_size(status_badge_, 124, 24);
    lv_obj_set_style_bg_color(status_badge_, lv_color_hex(0x064E3B), 0);
    lv_obj_set_style_bg_opa(status_badge_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(status_badge_, 12, 0);
    lv_obj_set_style_border_color(status_badge_, lv_color_hex(0x10B981), 0);
    lv_obj_set_style_border_width(status_badge_, 1, 0);
    lv_obj_set_flex_flow(status_badge_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_badge_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(status_badge_, 6, 0);
    lv_obj_clear_flag(status_badge_, LV_OBJ_FLAG_SCROLLABLE);

    status_dot_ = lv_obj_create(status_badge_);
    lv_obj_remove_style_all(status_dot_);
    lv_obj_set_size(status_dot_, 8, 8);
    lv_obj_set_style_radius(status_dot_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(status_dot_, lv_color_hex(0x34D399), 0);
    lv_obj_set_style_bg_opa(status_dot_, LV_OPA_COVER, 0);

    status_text_ = lv_label_create(status_badge_);
    lv_obj_set_style_text_font(status_text_, GetBuddyFont(), 0);
    lv_label_set_text(status_text_, "Bố Mẹ Sẵn Sàng");
    lv_obj_set_style_text_color(status_text_, lv_color_hex(0x6EE7B7), 0);

    // Warm friendly caption
    desc_label_ = lv_label_create(info_col);
    lv_obj_set_style_text_font(desc_label_, GetBuddyFont(), 0);
    lv_label_set_long_mode(desc_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(desc_label_, 124);
    lv_label_set_text(desc_label_, "Bố Mẹ luôn bên con!\nNhấn nút để gọi nhé ❤️");
    lv_obj_set_style_text_color(desc_label_, lv_color_hex(0xCBD5E1), 0);

    // 3.2 Bottom Action Row: 2 Big Voice Call Buttons (Gọi Mẹ & Gọi Bố)
    lv_obj_t* btn_row = lv_obj_create(main_card_);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_size(btn_row, 280, 42);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn_row, 8, 0);
    lv_obj_clear_flag(btn_row, LV_OBJ_FLAG_SCROLLABLE);

    // Button 1: Gọi Mẹ (Emerald / Pink Rose #EC4899)
    btn_call_mom_ = lv_button_create(btn_row);
    lv_obj_set_size(btn_call_mom_, 134, 40);
    lv_obj_set_style_radius(btn_call_mom_, 20, 0);
    lv_obj_set_style_bg_color(btn_call_mom_, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_color(btn_call_mom_, lv_color_hex(0xF472B6), 0);
    lv_obj_set_style_border_width(btn_call_mom_, 2, 0);
    lv_obj_set_style_pad_all(btn_call_mom_, 0, 0);
    lv_obj_add_event_cb(btn_call_mom_, OnCallMomBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* mom_row = lv_obj_create(btn_call_mom_);
    lv_obj_remove_style_all(mom_row);
    lv_obj_set_size(mom_row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(mom_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mom_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(mom_row, 6, 0);
    lv_obj_clear_flag(mom_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* mom_lbl = lv_label_create(mom_row);
    lv_obj_set_style_text_font(mom_lbl, GetBuddyFont(), 0);
    lv_label_set_text(mom_lbl, "👩 Gọi Mẹ");
    lv_obj_set_style_text_color(mom_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(mom_lbl);

    // Button 2: Gọi Bố (Royal Blue #2563EB)
    btn_call_dad_ = lv_button_create(btn_row);
    lv_obj_set_size(btn_call_dad_, 134, 40);
    lv_obj_set_style_radius(btn_call_dad_, 20, 0);
    lv_obj_set_style_bg_color(btn_call_dad_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_border_color(btn_call_dad_, lv_color_hex(0x60A5FA), 0);
    lv_obj_set_style_border_width(btn_call_dad_, 2, 0);
    lv_obj_set_style_pad_all(btn_call_dad_, 0, 0);
    lv_obj_add_event_cb(btn_call_dad_, OnCallDadBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* dad_row = lv_obj_create(btn_call_dad_);
    lv_obj_remove_style_all(dad_row);
    lv_obj_set_size(dad_row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(dad_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dad_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(dad_row, 6, 0);
    lv_obj_clear_flag(dad_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* dad_lbl = lv_label_create(dad_row);
    lv_obj_set_style_text_font(dad_lbl, GetBuddyFont(), 0);
    lv_label_set_text(dad_lbl, "👨 Gọi Bố");
    lv_obj_set_style_text_color(dad_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(dad_lbl);

    ESP_LOGI(TAG, "BuddyCallScreen created with AI 3D Family Artwork and Voice Call buttons");
}

void BuddyCallScreen::SetParentStatus(bool online) {
    is_online_ = online;
    if (!status_badge_ || !status_dot_ || !status_text_) return;

    if (online) {
        lv_obj_set_style_bg_color(status_badge_, lv_color_hex(0x064E3B), 0);
        lv_obj_set_style_border_color(status_badge_, lv_color_hex(0x10B981), 0);
        lv_obj_set_style_bg_color(status_dot_, lv_color_hex(0x34D399), 0);
        lv_label_set_text(status_text_, "Bố Mẹ Sẵn Sàng");
        lv_obj_set_style_text_color(status_text_, lv_color_hex(0x6EE7B7), 0);
    } else {
        lv_obj_set_style_bg_color(status_badge_, lv_color_hex(0x334155), 0);
        lv_obj_set_style_border_color(status_badge_, lv_color_hex(0x64748B), 0);
        lv_obj_set_style_bg_color(status_dot_, lv_color_hex(0x94A3B8), 0);
        lv_label_set_text(status_text_, "Ngoại Tuyến");
        lv_obj_set_style_text_color(status_text_, lv_color_hex(0xCBD5E1), 0);
    }
}

void BuddyCallScreen::SetOnCallParent(std::function<void(const std::string& parent_role)> callback) {
    on_call_parent_cb_ = callback;
}

void BuddyCallScreen::OnCallMomBtnCb(lv_event_t* e) {
    auto* self = static_cast<BuddyCallScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Child requested call to Mom");
    Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);

    if (self->on_call_parent_cb_) {
        self->on_call_parent_cb_("Mẹ");
    }
}

void BuddyCallScreen::OnCallDadBtnCb(lv_event_t* e) {
    auto* self = static_cast<BuddyCallScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Child requested call to Dad");
    Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);

    if (self->on_call_parent_cb_) {
        self->on_call_parent_cb_("Bố");
    }
}
