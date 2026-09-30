#include "family_moment_screen.h"
#include "buddy_font_helper.h"
#include "buddy_toast_overlay.h"
#include "application.h"
#include "settings.h"
#include "assets/lang_config.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "FamilyMomentScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_material_symbols_30_4);

FamilyMomentScreen::FamilyMomentScreen() {}
FamilyMomentScreen::~FamilyMomentScreen() {}

void FamilyMomentScreen::Create(lv_obj_t* parent) {
    // 0. Load persisted message from NVS
    Settings settings("family_msg", false);
    sender_name_ = settings.GetString("sender", "Mẹ");
    if (sender_name_.empty()) sender_name_ = "Mẹ";
    message_body_ = settings.GetString("message", "");
    timestamp_ = settings.GetString("time", "Hôm nay");
    is_liked_ = settings.GetInt("liked", 0) == 1;
    has_message_ = !message_body_.empty();

    // 1. Root Container (Deep Midnight Velvet #0B0817)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x0B0817), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Top Header Floating Glass Pill (Centered, Pos: 45, 4, Size: 230x28)
    header_pill_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_pill_);
    lv_obj_set_size(header_pill_, 230, 28);
    lv_obj_align(header_pill_, LV_ALIGN_TOP_MID, 0, 4);
    lv_obj_set_style_bg_color(header_pill_, lv_color_hex(0x241238), 0);
    lv_obj_set_style_bg_opa(header_pill_, LV_OPA_80, 0);
    lv_obj_set_style_radius(header_pill_, 14, 0);
    lv_obj_set_style_border_color(header_pill_, lv_color_hex(0xEC4899), 0); // Rose Gold Pink
    lv_obj_set_style_border_opa(header_pill_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(header_pill_, 1, 0);
    lv_obj_set_flex_flow(header_pill_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header_pill_, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(header_pill_);
    lv_obj_set_style_text_font(title_label_, GetBuddyFont(), 0);
    lv_label_set_text(title_label_, "💌 Lời Nhắn Yêu Thương");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), 0);

    // 3. View: Placeholder / Empty Card (When no message received)
    placeholder_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(placeholder_card_);
    lv_obj_set_size(placeholder_card_, 298, 198);
    lv_obj_set_pos(placeholder_card_, 11, 36);
    lv_obj_set_style_bg_color(placeholder_card_, lv_color_hex(0x18112E), 0);
    lv_obj_set_style_bg_opa(placeholder_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(placeholder_card_, 18, 0);
    lv_obj_set_style_border_color(placeholder_card_, lv_color_hex(0x8B5CF6), 0);
    lv_obj_set_style_border_opa(placeholder_card_, LV_OPA_30, 0);
    lv_obj_set_style_border_width(placeholder_card_, 1, 0);
    lv_obj_set_flex_flow(placeholder_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(placeholder_card_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(placeholder_card_, 8, 0);
    lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* empty_icon_circle = lv_obj_create(placeholder_card_);
    lv_obj_remove_style_all(empty_icon_circle);
    lv_obj_set_size(empty_icon_circle, 48, 48);
    lv_obj_set_style_bg_color(empty_icon_circle, lv_color_hex(0x2D1538), 0);
    lv_obj_set_style_bg_opa(empty_icon_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(empty_icon_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_color(empty_icon_circle, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_border_width(empty_icon_circle, 2, 0);
    lv_obj_clear_flag(empty_icon_circle, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* mail_ico = lv_label_create(empty_icon_circle);
    lv_label_set_text(mail_ico, MATERIAL_SYMBOLS_MAIL);
    lv_obj_set_style_text_font(mail_ico, &font_material_symbols_30_4, 0);
    lv_obj_set_style_text_color(mail_ico, lv_color_hex(0xF472B6), 0);
    lv_obj_center(mail_ico);

    lv_obj_t* empty_title = lv_label_create(placeholder_card_);
    lv_obj_set_style_text_font(empty_title, GetBuddyFont(), 0);
    lv_label_set_text(empty_title, "Chưa có lời nhắn mới");
    lv_obj_set_style_text_color(empty_title, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* empty_sub = lv_label_create(placeholder_card_);
    lv_obj_set_style_text_font(empty_sub, GetBuddyFont(), 0);
    lv_label_set_text(empty_sub, "Lời nhắn yêu thương từ Bố Mẹ\nsẽ xuất hiện ở đây nhé! ✨");
    lv_obj_set_style_text_color(empty_sub, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_align(empty_sub, LV_TEXT_ALIGN_CENTER, 0);

    // 4. View: Active Message Card (Spacious Luxury Love Postcard)
    msg_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(msg_card_);
    lv_obj_set_size(msg_card_, 298, 198);
    lv_obj_set_pos(msg_card_, 11, 36);
    lv_obj_set_style_bg_color(msg_card_, lv_color_hex(0x18112E), 0); // Warm Deep Amethyst
    lv_obj_set_style_bg_opa(msg_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(msg_card_, 18, 0);
    lv_obj_set_style_border_color(msg_card_, lv_color_hex(0xEC4899), 0); // Glowing Rose Pink Border
    lv_obj_set_style_border_width(msg_card_, 2, 0);
    lv_obj_set_flex_flow(msg_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(msg_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(msg_card_, 10, 0);
    lv_obj_set_style_pad_ver(msg_card_, 8, 0);
    lv_obj_clear_flag(msg_card_, LV_OBJ_FLAG_SCROLLABLE);

    // 4.1 Sender Header Row (Single Horizontal Row: Avatar + Name on left, Time on right)
    lv_obj_t* sender_row = lv_obj_create(msg_card_);
    lv_obj_remove_style_all(sender_row);
    lv_obj_set_size(sender_row, lv_pct(100), 34);
    lv_obj_set_flex_flow(sender_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sender_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(sender_row, LV_OBJ_FLAG_SCROLLABLE);

    // Left container: Avatar + Sender Name
    lv_obj_t* sender_left = lv_obj_create(sender_row);
    lv_obj_remove_style_all(sender_left);
    lv_obj_set_size(sender_left, LV_SIZE_CONTENT, 34);
    lv_obj_set_flex_flow(sender_left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sender_left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(sender_left, 8, 0);
    lv_obj_clear_flag(sender_left, LV_OBJ_FLAG_SCROLLABLE);

    // Round Glowing Avatar Badge (Pink Circle with Heart Icon)
    sender_badge_ = lv_obj_create(sender_left);
    lv_obj_remove_style_all(sender_badge_);
    lv_obj_set_size(sender_badge_, 32, 32);
    lv_obj_set_style_radius(sender_badge_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(sender_badge_, lv_color_hex(0xEC4899), 0);
    lv_obj_set_style_bg_opa(sender_badge_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(sender_badge_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* badge_heart = lv_label_create(sender_badge_);
    lv_label_set_text(badge_heart, MATERIAL_SYMBOLS_FAVORITE);
    lv_obj_set_style_text_font(badge_heart, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(badge_heart, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(badge_heart);

    sender_label_ = lv_label_create(sender_left);
    lv_obj_set_style_text_font(sender_label_, GetBuddyFont(), 0);
    std::string initial_sender = sender_name_.empty() ? "Mẹ Yêu" : sender_name_;
    if (initial_sender == "Mom" || initial_sender == "Mẹ") initial_sender = "Mẹ Yêu";
    else if (initial_sender == "Dad" || initial_sender == "Bố") initial_sender = "Bố Yêu";
    else if (initial_sender == "Family" || initial_sender == "Gia đình") initial_sender = "Gia Đình";
    lv_label_set_text(sender_label_, initial_sender.c_str());
    lv_obj_set_style_text_color(sender_label_, lv_color_hex(0xFDE047), 0); // Warm Golden Yellow

    // Right container: Timestamp
    time_label_ = lv_label_create(sender_row);
    lv_obj_set_style_text_font(time_label_, GetBuddyFont(), 0);
    lv_label_set_text(time_label_, timestamp_.c_str());
    lv_obj_set_style_text_color(time_label_, lv_color_hex(0x94A3B8), 0); // Rose Silver

    // 4.2 Message Body Box (Spacious Letter Parchment Bubble)
    lv_obj_t* msg_bubble = lv_obj_create(msg_card_);
    lv_obj_remove_style_all(msg_bubble);
    lv_obj_set_size(msg_bubble, 278, 86);
    lv_obj_set_style_bg_color(msg_bubble, lv_color_hex(0x251A45), 0); // Cozy Parchment #251A45
    lv_obj_set_style_bg_opa(msg_bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(msg_bubble, 12, 0);
    lv_obj_set_style_border_color(msg_bubble, lv_color_hex(0x8B5CF6), 0);
    lv_obj_set_style_border_opa(msg_bubble, LV_OPA_40, 0);
    lv_obj_set_style_border_width(msg_bubble, 1, 0);
    lv_obj_set_style_pad_hor(msg_bubble, 8, 0);
    lv_obj_set_style_pad_ver(msg_bubble, 6, 0);
    lv_obj_set_flex_flow(msg_bubble, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(msg_bubble, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(msg_bubble, LV_OBJ_FLAG_SCROLLABLE);

    msg_label_ = lv_label_create(msg_bubble);
    lv_obj_set_style_text_font(msg_label_, GetBuddyFont(), 0);
    lv_label_set_long_mode(msg_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(msg_label_, 260);
    lv_label_set_text(msg_label_, message_body_.c_str());
    lv_obj_set_style_text_color(msg_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(msg_label_, LV_TEXT_ALIGN_CENTER, 0);

    // 4.3 Reaction Button (Vibrant Glowing Rose Button)
    btn_like_ = lv_button_create(msg_card_);
    lv_obj_set_size(btn_like_, 210, 36);
    lv_obj_set_style_radius(btn_like_, 18, 0);
    lv_obj_set_style_pad_all(btn_like_, 0, 0);
    lv_obj_add_event_cb(btn_like_, OnLikeButtonEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t* like_row = lv_obj_create(btn_like_);
    lv_obj_remove_style_all(like_row);
    lv_obj_set_size(like_row, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(like_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(like_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(like_row, 6, 0);
    lv_obj_clear_flag(like_row, LV_OBJ_FLAG_SCROLLABLE);

    like_text_ = lv_label_create(like_row);
    lv_obj_set_style_text_font(like_text_, GetBuddyFont(), 0);
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

    ESP_LOGI(TAG, "FamilyMomentScreen created with luxury postcard aesthetic");
}

void FamilyMomentScreen::UpdateLikeButtonState() {
    if (!btn_like_ || !like_text_) return;

    if (is_liked_) {
        lv_obj_set_style_bg_color(btn_like_, lv_color_hex(0x059669), 0); // Emerald Green
        lv_obj_set_style_border_color(btn_like_, lv_color_hex(0x34D399), 0);
        lv_obj_set_style_border_width(btn_like_, 2, 0);
        lv_label_set_text(like_text_, "💖 Đã gửi ngàn trái tim!");
        lv_obj_set_style_text_color(like_text_, lv_color_hex(0xFFFFFF), 0);
    } else {
        lv_obj_set_style_bg_color(btn_like_, lv_color_hex(0xEC4899), 0); // Hot Pink
        lv_obj_set_style_border_color(btn_like_, lv_color_hex(0xF472B6), 0);
        lv_obj_set_style_border_width(btn_like_, 2, 0);
        lv_label_set_text(like_text_, "❤️ Thả tim cho Bố Mẹ");
        lv_obj_set_style_text_color(like_text_, lv_color_hex(0xFFFFFF), 0);
    }
}

void FamilyMomentScreen::OnLikeButtonEventCb(lv_event_t* e) {
    auto* self = static_cast<FamilyMomentScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    self->is_liked_ = !self->is_liked_;
    ESP_LOGI(TAG, "Family reaction like toggled: %d", self->is_liked_);
    self->UpdateLikeButtonState();

    // Play cheerful audio chime when child loves
    if (self->is_liked_) {
        Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);
        BuddyToastOverlay::GetInstance().Show("Trái Tim Yêu Thương", "Con đã gửi ngàn trái tim đến Bố Mẹ!", ToastType::kReward, 3500);
    }

    // Persist liked state to NVS
    Settings settings("family_msg", true);
    settings.SetInt("liked", self->is_liked_ ? 1 : 0);

    if (self->on_like_click_) {
        self->on_like_click_(self->is_liked_);
    }
}

void FamilyMomentScreen::SetMessage(const std::string& sender, const std::string& message, const std::string& timestamp) {
    has_message_ = !message.empty();
    sender_name_ = sender.empty() ? "Mẹ" : sender;
    message_body_ = message;
    timestamp_ = timestamp.empty() ? "Hôm nay" : timestamp;
    is_liked_ = false;

    // Persist to NVS
    Settings settings("family_msg", true);
    settings.SetString("sender", sender_name_);
    settings.SetString("message", message);
    settings.SetString("time", timestamp_);
    settings.SetInt("liked", 0);

    if (sender_label_) {
        std::string display_sender = sender_name_;
        if (sender_name_ == "Mom" || sender_name_ == "Mẹ") display_sender = "Mẹ Yêu";
        else if (sender_name_ == "Dad" || sender_name_ == "Bố") display_sender = "Bố Yêu";
        else if (sender_name_ == "Family" || sender_name_ == "Gia đình") display_sender = "Gia Đình";

        lv_obj_set_style_text_font(sender_label_, GetBuddyFont(), 0);
        lv_label_set_text(sender_label_, display_sender.c_str());
    }
    if (msg_label_) {
        lv_obj_set_style_text_font(msg_label_, GetBuddyFont(), 0);
        lv_label_set_text(msg_label_, message.c_str());
    }
    if (time_label_) {
        lv_obj_set_style_text_font(time_label_, GetBuddyFont(), 0);
        lv_label_set_text(time_label_, timestamp_.c_str());
    }

    UpdateLikeButtonState();

    if (has_message_) {
        if (msg_card_) lv_obj_clear_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
        if (placeholder_card_) lv_obj_add_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (msg_card_) lv_obj_add_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
        if (placeholder_card_) lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
    }
}

void FamilyMomentScreen::ClearMessage() {
    has_message_ = false;
    is_liked_ = false;

    Settings settings("family_msg", true);
    settings.EraseKey("message");
    settings.EraseKey("sender");

    if (msg_card_) lv_obj_add_flag(msg_card_, LV_OBJ_FLAG_HIDDEN);
    if (placeholder_card_) lv_obj_clear_flag(placeholder_card_, LV_OBJ_FLAG_HIDDEN);
}

void FamilyMomentScreen::OnLikeClicked(std::function<void(bool liked)> callback) {
    on_like_click_ = callback;
}





