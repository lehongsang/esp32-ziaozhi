#include "family_moment_screen.h"
#include <esp_log.h>

#define TAG "FamilyMomentScreen"

FamilyMomentScreen::FamilyMomentScreen() {}
FamilyMomentScreen::~FamilyMomentScreen() {}

void FamilyMomentScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Deep Loving Dark Blue-Purple)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x0C0B18), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Top Header Floating Glass Pill (Centered, y = 6px)
    header_pill_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_pill_);
    lv_obj_set_size(header_pill_, 160, 22);
    lv_obj_align(header_pill_, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(header_pill_, lv_color_hex(0x181328), 0);
    lv_obj_set_style_bg_opa(header_pill_, LV_OPA_80, 0);
    lv_obj_set_style_radius(header_pill_, 11, 0);
    lv_obj_set_style_border_color(header_pill_, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_opa(header_pill_, LV_OPA_30, 0);
    lv_obj_set_style_border_width(header_pill_, 1, 0);
    lv_obj_set_flex_flow(header_pill_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header_pill_, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(header_pill_);
    lv_label_set_text(title_label_, "💌 Family Moment");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), 0);

    // 3. View: Placeholder / Empty Card (When no message received)
    placeholder_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(placeholder_card_);
    lv_obj_set_size(placeholder_card_, 260, 156);
    lv_obj_align(placeholder_card_, LV_ALIGN_CENTER, 0, -8);
    lv_obj_set_style_bg_color(placeholder_card_, lv_color_hex(0x151228), 0);
    lv_obj_set_style_bg_opa(placeholder_card_, LV_OPA_90, 0);
    lv_obj_set_style_radius(placeholder_card_, 16, 0);
    lv_obj_set_style_border_color(placeholder_card_, lv_color_hex(0x8B5CF6), 0);
    lv_obj_set_style_border_opa(placeholder_card_, LV_OPA_30, 0);
    lv_obj_set_style_border_width(placeholder_card_, 1, 0);
    lv_obj_set_flex_flow(placeholder_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(placeholder_card_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* icon_circle = lv_obj_create(placeholder_card_);
    lv_obj_remove_style_all(icon_circle);
    lv_obj_set_size(icon_circle, 44, 44);
    lv_obj_set_style_bg_color(icon_circle, lv_color_hex(0x2D1538), 0);
    lv_obj_set_style_bg_opa(icon_circle, LV_OPA_90, 0);
    lv_obj_set_style_radius(icon_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(icon_circle, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_opa(icon_circle, LV_OPA_40, 0);
    lv_obj_set_style_border_width(icon_circle, 1, 0);

    lv_obj_t* mail_ico = lv_label_create(icon_circle);
    lv_label_set_text(mail_ico, "💌");
    lv_obj_center(mail_ico);

    lv_obj_t* empty_title = lv_label_create(placeholder_card_);
    lv_label_set_text(empty_title, "No Messages Yet");
    lv_obj_set_style_text_color(empty_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(empty_title, 6, 0);

    lv_obj_t* empty_sub = lv_label_create(placeholder_card_);
    lv_label_set_text(empty_sub, "Loving notes from Mom & Dad\nwill appear here ✨");
    lv_obj_set_style_text_color(empty_sub, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_align(empty_sub, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_top(empty_sub, 4, 0);

    // 4. View: Active Message Card (Postcard Style)
    msg_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(msg_card_);
    lv_obj_set_size(msg_card_, 270, 156);
    lv_obj_align(msg_card_, LV_ALIGN_CENTER, 0, -8);
    lv_obj_set_style_bg_color(msg_card_, lv_color_hex(0x16132C), 0);
    lv_obj_set_style_bg_opa(msg_card_, LV_OPA_90, 0);
    lv_obj_set_style_radius(msg_card_, 16, 0);
    lv_obj_set_style_border_color(msg_card_, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_opa(msg_card_, LV_OPA_40, 0);
    lv_obj_set_style_border_width(msg_card_, 1, 0);
    lv_obj_set_flex_flow(msg_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(msg_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(msg_card_, 12, 0);
    lv_obj_set_style_pad_right(msg_card_, 12, 0);
    lv_obj_set_style_pad_top(msg_card_, 8, 0);
    lv_obj_set_style_pad_bottom(msg_card_, 8, 0);
    lv_obj_clear_flag(msg_card_, LV_OBJ_FLAG_SCROLLABLE);

    // 4.1 Sender Header Row (Badge + Timestamp)
    lv_obj_t* sender_row = lv_obj_create(msg_card_);
    lv_obj_remove_style_all(sender_row);
    lv_obj_set_size(sender_row, lv_pct(100), 22);
    lv_obj_set_flex_flow(sender_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sender_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(sender_row, LV_OBJ_FLAG_SCROLLABLE);

    sender_badge_ = lv_obj_create(sender_row);
    lv_obj_remove_style_all(sender_badge_);
    lv_obj_set_style_bg_color(sender_badge_, lv_color_hex(0x2E143B), 0);
    lv_obj_set_style_bg_opa(sender_badge_, LV_OPA_80, 0);
    lv_obj_set_style_radius(sender_badge_, 10, 0);
    lv_obj_set_style_border_color(sender_badge_, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_opa(sender_badge_, LV_OPA_30, 0);
    lv_obj_set_style_border_width(sender_badge_, 1, 0);
    lv_obj_set_style_pad_left(sender_badge_, 8, 0);
    lv_obj_set_style_pad_right(sender_badge_, 8, 0);
    lv_obj_set_style_pad_top(sender_badge_, 2, 0);
    lv_obj_set_style_pad_bottom(sender_badge_, 2, 0);

    sender_label_ = lv_label_create(sender_badge_);
    lv_label_set_text(sender_label_, (sender_name_ == "Mom" ? "👩 Mom" : (sender_name_ == "Dad" ? "👨 Dad" : ("🏡 " + sender_name_).c_str())));
    lv_obj_set_style_text_color(sender_label_, lv_color_hex(0xFDE047), 0);

    time_label_ = lv_label_create(sender_row);
    lv_label_set_text(time_label_, timestamp_.c_str());
    lv_obj_set_style_text_color(time_label_, lv_color_hex(0x94A3B8), 0);

    // 4.2 Message Body Box
    msg_label_ = lv_label_create(msg_card_);
    lv_label_set_long_mode(msg_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg_label_, 246);
    lv_label_set_text(msg_label_, message_body_.c_str());
    lv_obj_set_style_text_color(msg_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(msg_label_, LV_TEXT_ALIGN_CENTER, 0);

    // 4.3 Reaction Button (Send Love ❤️)
    btn_like_ = lv_button_create(msg_card_);
    lv_obj_set_size(btn_like_, 130, 26);
    lv_obj_set_style_radius(btn_like_, 13, 0);
    lv_obj_set_style_pad_all(btn_like_, 0, 0);
    lv_obj_add_event_cb(btn_like_, OnLikeButtonEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t* like_row = lv_obj_create(btn_like_);
    lv_obj_remove_style_all(like_row);
    lv_obj_set_size(like_row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(like_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(like_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(like_row, 4, 0);

    like_text_ = lv_label_create(like_row);
    lv_obj_center(like_text_);

    UpdateLikeButtonState();

    // Toggle initial view
    if (has_message_) {
        lv_obj_clear_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
    }
}

void FamilyMomentScreen::UpdateLikeButtonState() {
    if (!btn_like_ || !like_text_) return;

    if (is_liked_) {
        lv_obj_set_style_bg_color(btn_like_, lv_color_hex(0xEC4899), 0);
        lv_obj_set_style_border_color(btn_like_, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_border_opa(btn_like_, LV_OPA_40, 0);
        lv_obj_set_style_border_width(btn_like_, 1, 0);
        lv_label_set_text(like_text_, "Loved! 💖");
        lv_obj_set_style_text_color(like_text_, lv_color_hex(0xFFFFFF), 0);
    } else {
        lv_obj_set_style_bg_color(btn_like_, lv_color_hex(0x2A1535), 0);
        lv_obj_set_style_border_color(btn_like_, lv_color_hex(0xEC4899), 0);
        lv_obj_set_style_border_opa(btn_like_, LV_OPA_60, 0);
        lv_obj_set_style_border_width(btn_like_, 1, 0);
        lv_label_set_text(like_text_, "Send Love ❤️");
        lv_obj_set_style_text_color(like_text_, lv_color_hex(0xF472B6), 0);
    }
}

void FamilyMomentScreen::OnLikeButtonEventCb(lv_event_t* e) {
    auto* self = static_cast<FamilyMomentScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    self->is_liked_ = !self->is_liked_;
    ESP_LOGI(TAG, "Family reaction like toggled: %d", self->is_liked_);
    self->UpdateLikeButtonState();

    if (self->on_like_click_) {
        self->on_like_click_(self->is_liked_);
    }
}

void FamilyMomentScreen::SetMessage(const std::string& sender, const std::string& message, const std::string& timestamp) {
    has_message_ = true;
    sender_name_ = sender;
    message_body_ = message;
    timestamp_ = timestamp;
    is_liked_ = false;

    if (sender_label_) {
        lv_label_set_text(sender_label_, (sender == "Mom" ? "👩 Mom" : (sender == "Dad" ? "👨 Dad" : ("🏡 " + sender).c_str())));
    }
    if (msg_label_) {
        lv_label_set_text(msg_label_, message.c_str());
    }
    if (time_label_) {
        lv_label_set_text(time_label_, timestamp.c_str());
    }

    UpdateLikeButtonState();

    if (msg_card_) lv_obj_clear_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
    if (placeholder_card_) lv_obj_add_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
}

void FamilyMomentScreen::ClearMessage() {
    has_message_ = false;
    is_liked_ = false;

    if (msg_card_) lv_obj_add_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
    if (placeholder_card_) lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
}

void FamilyMomentScreen::OnLikeClicked(std::function<void(bool liked)> callback) {
    on_like_click_ = callback;
}


