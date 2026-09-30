#include "buddy_toast_overlay.h"
#include "buddy_font_helper.h"
#include "application.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "BuddyToastOverlay"

LV_FONT_DECLARE(font_material_symbols_20_4);

BuddyToastOverlay& BuddyToastOverlay::GetInstance() {
    static BuddyToastOverlay instance;
    return instance;
}

BuddyToastOverlay::BuddyToastOverlay() {}
BuddyToastOverlay::~BuddyToastOverlay() {}

void BuddyToastOverlay::Initialize(lv_obj_t* root_layer) {
    if (!root_layer) {
        root_layer = lv_layer_top();
    }

    container_ = lv_obj_create(root_layer);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, 296, 54);
    lv_obj_align(container_, LV_ALIGN_TOP_MID, 0, -60); // Hidden offscreen initially
    lv_obj_set_style_bg_color(container_, lv_color_hex(0xE0F2FE), 0); // Vibrant Light Sky Blue #E0F2FE
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0); // Solid opaque
    lv_obj_set_style_radius(container_, 16, 0);
    lv_obj_set_style_border_color(container_, lv_color_hex(0x0284C7), 0); // Ocean Blue Border
    lv_obj_set_style_border_width(container_, 2, 0);
    lv_obj_set_style_shadow_width(container_, 16, 0);
    lv_obj_set_style_shadow_color(container_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(container_, LV_OPA_60, 0);
    lv_obj_set_style_pad_hor(container_, 10, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container_, 10, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(container_, OnToastClickedCb, LV_EVENT_CLICKED, this);

    // Left Icon Box
    lv_obj_t* icon_box = lv_obj_create(container_);
    lv_obj_remove_style_all(icon_box);
    lv_obj_set_size(icon_box, 36, 36);
    lv_obj_set_style_radius(icon_box, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(icon_box, lv_color_hex(0x0284C7), 0);
    lv_obj_set_style_bg_opa(icon_box, LV_OPA_COVER, 0);
    lv_obj_clear_flag(icon_box, LV_OBJ_FLAG_SCROLLABLE);

    icon_label_ = lv_label_create(icon_box);
    lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_NOTIFICATIONS);
    lv_obj_set_style_text_font(icon_label_, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(icon_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(icon_label_);

    // Text Content Column
    lv_obj_t* text_col = lv_obj_create(container_);
    lv_obj_remove_style_all(text_col);
    lv_obj_set_flex_flow(text_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(text_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_flex_grow(text_col, 1);
    lv_obj_set_style_pad_gap(text_col, 1, 0);
    lv_obj_clear_flag(text_col, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(text_col);
    lv_obj_set_style_text_font(title_label_, GetBuddyFont(), 0);
    lv_label_set_text(title_label_, "Tin nhắn mới");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0x0369A1), 0); // High contrast dark blue title

    body_label_ = lv_label_create(text_col);
    lv_obj_set_style_text_font(body_label_, GetBuddyFont(), 0);
    lv_obj_set_width(body_label_, 226);
    lv_label_set_long_mode(body_label_, LV_LABEL_LONG_DOT);
    lv_label_set_text(body_label_, "Nội dung tin nhắn");
    lv_obj_set_style_text_color(body_label_, lv_color_hex(0x0F172A), 0); // Bold dark navy text for max readability

    ESP_LOGI(TAG, "BuddyToastOverlay initialized with vibrant Light Sky Blue theme");
}

void BuddyToastOverlay::Show(const std::string& title, const std::string& body, ToastType type, uint32_t duration_ms) {
    if (!container_) return;

    // Play pleasant popup audio chime
    Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);

    lv_obj_move_foreground(container_);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);

    if (title_label_) lv_label_set_text(title_label_, title.c_str());
    if (body_label_) lv_label_set_text(body_label_, body.c_str());

    lv_obj_t* icon_box = lv_obj_get_parent(icon_label_);

    switch (type) {
        case ToastType::kMessage:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_CHAT_BUBBLE);
            lv_obj_set_style_border_color(container_, lv_color_hex(0xEC4899), 0); // Pink
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0xDB2777), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0xBE185D), 0);
            break;
        case ToastType::kNewQuest:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_STAR);
            lv_obj_set_style_border_color(container_, lv_color_hex(0x0284C7), 0); // Ocean Blue
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0x0284C7), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0x0369A1), 0);
            break;
        case ToastType::kReminder:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_ALARM);
            lv_obj_set_style_border_color(container_, lv_color_hex(0xF59E0B), 0); // Amber
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0xD97706), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0xB45309), 0);
            break;
        case ToastType::kReward:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_STAR);
            lv_obj_set_style_border_color(container_, lv_color_hex(0xF59E0B), 0); // Golden Amber
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0xD97706), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0xB45309), 0);
            break;
        case ToastType::kWarning:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_WARNING);
            lv_obj_set_style_border_color(container_, lv_color_hex(0xEF4444), 0); // Red
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0xDC2626), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0xB91C1C), 0);
            break;
        case ToastType::kInfo:
        default:
            if (icon_label_) lv_label_set_text(icon_label_, MATERIAL_SYMBOLS_INFO);
            lv_obj_set_style_border_color(container_, lv_color_hex(0x0284C7), 0); // Ocean Blue
            if (icon_box) lv_obj_set_style_bg_color(icon_box, lv_color_hex(0x0284C7), 0);
            if (title_label_) lv_obj_set_style_text_color(title_label_, lv_color_hex(0x0369A1), 0);
            break;
    }

    // Slide down animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, container_);
    lv_anim_set_time(&a, 350);
    lv_anim_set_values(&a, -60, 6);
    lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
    lv_anim_set_custom_exec_cb(&a, [](lv_anim_t* anim, int32_t val) {
        auto* obj = static_cast<lv_obj_t*>(anim->var);
        if (obj) {
            lv_obj_set_y(obj, val);
            lv_obj_invalidate(obj);
        }
    });
    lv_anim_start(&a);

    is_visible_ = true;

    // Reset or start timer
    if (hide_timer_) {
        lv_timer_delete(hide_timer_);
        hide_timer_ = nullptr;
    }
    if (duration_ms > 0) {
        hide_timer_ = lv_timer_create(AutoHideTimerCb, duration_ms, this);
        lv_timer_set_repeat_count(hide_timer_, 1);
    }
}

void BuddyToastOverlay::Hide() {
    if (!is_visible_ || !container_) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, container_);
    lv_anim_set_time(&a, 250);
    lv_anim_set_values(&a, lv_obj_get_y(container_), -60);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_custom_exec_cb(&a, [](lv_anim_t* anim, int32_t val) {
        auto* obj = static_cast<lv_obj_t*>(anim->var);
        if (obj) {
            lv_obj_set_y(obj, val);
            lv_obj_invalidate(obj);
        }
    });
    lv_anim_start(&a);

    is_visible_ = false;
}

void BuddyToastOverlay::OnToastClickedCb(lv_event_t* e) {
    auto* self = static_cast<BuddyToastOverlay*>(lv_event_get_user_data(e));
    if (self) {
        self->Hide();
    }
}

void BuddyToastOverlay::AutoHideTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<BuddyToastOverlay*>(lv_timer_get_user_data(timer));
    if (self) {
        self->Hide();
        self->hide_timer_ = nullptr;
    }
}
