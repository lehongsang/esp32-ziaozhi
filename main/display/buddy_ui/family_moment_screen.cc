#include "family_moment_screen.h"

FamilyMomentScreen::FamilyMomentScreen() {}
FamilyMomentScreen::~FamilyMomentScreen() {}

void FamilyMomentScreen::Create(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(container_, 8, 0);
    lv_obj_set_style_pad_bottom(container_, 12, 0);

    // Title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, "Family Moment");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), 0);

    // Message Card Container
    msg_card_ = lv_obj_create(container_);
    lv_obj_set_size(msg_card_, lv_pct(88), 130);
    lv_obj_set_style_bg_color(msg_card_, lv_color_hex(0x2B2D42), 0);
    lv_obj_set_style_radius(msg_card_, 16, 0);
    lv_obj_set_style_border_color(msg_card_, lv_color_hex(0xFF70A6), 0);
    lv_obj_set_style_border_width(msg_card_, 1, 0);
    lv_obj_set_flex_flow(msg_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(msg_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(msg_card_, 10, 0);
    lv_obj_set_style_margin_top(msg_card_, 8, 0);

    // Sender Row (Avatar + Name)
    lv_obj_t* sender_row = lv_obj_create(msg_card_);
    lv_obj_remove_style_all(sender_row);
    lv_obj_set_flex_flow(sender_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sender_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(sender_row, 6, 0);

    lv_obj_t* avatar = lv_label_create(sender_row);
    lv_label_set_text(avatar, "👩");

    sender_label_ = lv_label_create(sender_row);
    lv_label_set_text(sender_label_, "Mom");
    lv_obj_set_style_text_color(sender_label_, lv_color_hex(0xFFE484), 0);

    // Message Content
    msg_label_ = lv_label_create(msg_card_);
    lv_label_set_text(msg_label_, "So proud of you!\nKeep up the great work! ❤️");
    lv_obj_set_style_text_color(msg_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(msg_label_, LV_TEXT_ALIGN_CENTER, 0);

    // Bottom Action Row
    lv_obj_t* action_row = lv_obj_create(container_);
    lv_obj_remove_style_all(action_row);
    lv_obj_set_size(action_row, lv_pct(80), 40);
    lv_obj_set_flex_flow(action_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(action_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_top(action_row, 6, 0);

    // Like Button
    btn_like_ = lv_btn_create(action_row);
    lv_obj_set_size(btn_like_, 36, 36);
    lv_obj_set_style_radius(btn_like_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_like_, lv_color_hex(0xFF006E), 0);
    like_icon_ = lv_label_create(btn_like_);
    lv_label_set_text(like_icon_, "❤️");
    lv_obj_center(like_icon_);
}

void FamilyMomentScreen::SetMessage(const std::string& sender, const std::string& message) {
    if (sender_label_) {
        lv_label_set_text(sender_label_, sender.c_str());
    }
    if (msg_label_) {
        lv_label_set_text(msg_label_, message.c_str());
    }
}

void FamilyMomentScreen::ClearMessage() {
    if (sender_label_) {
        lv_label_set_text(sender_label_, "Family");
    }
    if (msg_label_) {
        lv_label_set_text(msg_label_, "Waiting for loving messages\nfrom Mom and Dad...");
    }
}

void FamilyMomentScreen::OnLikeClicked(std::function<void(bool liked)> callback) {
    on_like_click_ = callback;
}
