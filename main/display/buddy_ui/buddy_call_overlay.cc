#include "buddy_call_overlay.h"
#include "buddy_font_helper.h"
#include "application.h"
#include "assets/lang_config.h"
#include <material_symbols.h>
#include <esp_log.h>
#include <cmath>

#define TAG "BuddyCallOverlay"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_material_symbols_30_4);

BuddyCallOverlay& BuddyCallOverlay::GetInstance() {
    static BuddyCallOverlay instance;
    return instance;
}

BuddyCallOverlay::BuddyCallOverlay() {}
BuddyCallOverlay::~BuddyCallOverlay() {
    if (waveform_timer_) {
        lv_timer_delete(waveform_timer_);
        waveform_timer_ = nullptr;
    }
    if (duration_timer_) {
        lv_timer_delete(duration_timer_);
        duration_timer_ = nullptr;
    }
}

void BuddyCallOverlay::Initialize(lv_obj_t* root_layer) {
    if (!root_layer) {
        root_layer = lv_layer_top();
    }

    // 1. Full Screen Backdrop (Translucent Dimmer)
    backdrop_ = lv_obj_create(root_layer);
    lv_obj_remove_style_all(backdrop_);
    lv_obj_set_size(backdrop_, 320, 240);
    lv_obj_set_style_bg_color(backdrop_, lv_color_hex(0x050810), 0);
    lv_obj_set_style_bg_opa(backdrop_, LV_OPA_80, 0);
    lv_obj_clear_flag(backdrop_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(backdrop_, LV_OBJ_FLAG_HIDDEN);

    // 2. Central Modal Card (280x200px)
    modal_card_ = lv_obj_create(backdrop_);
    lv_obj_remove_style_all(modal_card_);
    lv_obj_set_size(modal_card_, 280, 204);
    lv_obj_center(modal_card_);
    lv_obj_set_style_bg_color(modal_card_, lv_color_hex(0x131A2A), 0);
    lv_obj_set_style_bg_opa(modal_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(modal_card_, 20, 0);
    lv_obj_set_style_border_color(modal_card_, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_border_width(modal_card_, 2, 0);
    lv_obj_set_flex_flow(modal_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(modal_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(modal_card_, 12, 0);
    lv_obj_clear_flag(modal_card_, LV_OBJ_FLAG_SCROLLABLE);

    // 2.1 Top Section: Avatar & Name
    lv_obj_t* top_box = lv_obj_create(modal_card_);
    lv_obj_remove_style_all(top_box);
    lv_obj_set_size(top_box, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(top_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(top_box, 4, 0);
    lv_obj_clear_flag(top_box, LV_OBJ_FLAG_SCROLLABLE);

    // Round Icon Circle (44x44)
    avatar_circle_ = lv_obj_create(top_box);
    lv_obj_remove_style_all(avatar_circle_);
    lv_obj_set_size(avatar_circle_, 44, 44);
    lv_obj_set_style_radius(avatar_circle_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(avatar_circle_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_bg_opa(avatar_circle_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(avatar_circle_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* call_icon = lv_label_create(avatar_circle_);
    lv_label_set_text(call_icon, MATERIAL_SYMBOLS_PHONE);
    lv_obj_set_style_text_font(call_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(call_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(call_icon);

    title_label_ = lv_label_create(top_box);
    lv_obj_set_style_text_font(title_label_, GetBuddyFont(), 0);
    lv_label_set_text(title_label_, "Mẹ Yêu");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xF8FAFC), 0);

    status_label_ = lv_label_create(top_box);
    lv_obj_set_style_text_font(status_label_, GetBuddyFont(), 0);
    lv_label_set_text(status_label_, "Đang đổ chuông...");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x38BDF8), 0);

    timer_label_ = lv_label_create(top_box);
    lv_obj_set_style_text_font(timer_label_, GetBuddyFont(), 0);
    lv_label_set_text(timer_label_, "00:00");
    lv_obj_set_style_text_color(timer_label_, lv_color_hex(0x34D399), 0);
    lv_obj_add_flag(timer_label_, LV_OBJ_FLAG_HIDDEN);

    // 2.2 Audio Waveform Equalizer (10 bars)
    waveform_container_ = lv_obj_create(modal_card_);
    lv_obj_remove_style_all(waveform_container_);
    lv_obj_set_size(waveform_container_, 180, 24);
    lv_obj_set_flex_flow(waveform_container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(waveform_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(waveform_container_, 4, 0);
    lv_obj_clear_flag(waveform_container_, LV_OBJ_FLAG_SCROLLABLE);

    waveform_bars_.clear();
    for (int i = 0; i < 10; ++i) {
        lv_obj_t* bar = lv_obj_create(waveform_container_);
        lv_obj_remove_style_all(bar);
        lv_obj_set_size(bar, 4, 8);
        lv_obj_set_style_radius(bar, 2, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x38BDF8), 0);
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        waveform_bars_.push_back(bar);
    }

    // 2.3 Bottom Actions: Incoming Actions (Accept / Reject)
    incoming_actions_ = lv_obj_create(modal_card_);
    lv_obj_remove_style_all(incoming_actions_);
    lv_obj_set_size(incoming_actions_, lv_pct(100), 44);
    lv_obj_set_flex_flow(incoming_actions_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(incoming_actions_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(incoming_actions_, 24, 0);
    lv_obj_clear_flag(incoming_actions_, LV_OBJ_FLAG_SCROLLABLE);

    // Accept Button (Green Circle)
    btn_accept_ = lv_button_create(incoming_actions_);
    lv_obj_set_size(btn_accept_, 44, 44);
    lv_obj_set_style_radius(btn_accept_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_accept_, lv_color_hex(0x10B981), 0);
    lv_obj_add_event_cb(btn_accept_, OnAcceptBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* accept_ico = lv_label_create(btn_accept_);
    lv_label_set_text(accept_ico, MATERIAL_SYMBOLS_PHONE);
    lv_obj_set_style_text_font(accept_ico, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(accept_ico, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(accept_ico);

    // Reject Button (Red Circle)
    btn_reject_ = lv_button_create(incoming_actions_);
    lv_obj_set_size(btn_reject_, 44, 44);
    lv_obj_set_style_radius(btn_reject_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_reject_, lv_color_hex(0xEF4444), 0);
    lv_obj_add_event_cb(btn_reject_, OnRejectBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* reject_ico = lv_label_create(btn_reject_);
    lv_label_set_text(reject_ico, MATERIAL_SYMBOLS_CLOSE);
    lv_obj_set_style_text_font(reject_ico, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(reject_ico, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(reject_ico);

    // 2.4 Bottom Actions: Active / Outgoing Actions (Hangup)
    active_actions_ = lv_obj_create(modal_card_);
    lv_obj_remove_style_all(active_actions_);
    lv_obj_set_size(active_actions_, lv_pct(100), 44);
    lv_obj_set_flex_flow(active_actions_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(active_actions_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(active_actions_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(active_actions_, LV_OBJ_FLAG_HIDDEN);

    btn_hangup_ = lv_button_create(active_actions_);
    lv_obj_set_size(btn_hangup_, 44, 44);
    lv_obj_set_style_radius(btn_hangup_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_hangup_, lv_color_hex(0xEF4444), 0);
    lv_obj_add_event_cb(btn_hangup_, OnHangupBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* hangup_ico = lv_label_create(btn_hangup_);
    lv_label_set_text(hangup_ico, MATERIAL_SYMBOLS_CLOSE);
    lv_obj_set_style_text_font(hangup_ico, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(hangup_ico, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(hangup_ico);

    ESP_LOGI(TAG, "BuddyCallOverlay initialized on top layer");
}

void BuddyCallOverlay::ShowIncomingCall(const std::string& caller_name, std::function<void()> on_accept, std::function<void()> on_reject) {
    if (!backdrop_) return;

    state_ = CallOverlayState::kIncoming;
    on_accept_cb_ = on_accept;
    on_reject_cb_ = on_reject;
    on_hangup_cb_ = on_reject;

    lv_label_set_text(title_label_, caller_name.c_str());
    lv_label_set_text(status_label_, "Cuộc gọi đến...");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x38BDF8), 0);

    lv_obj_add_flag(timer_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(incoming_actions_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(active_actions_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(backdrop_, LV_OBJ_FLAG_HIDDEN);

    // Play ringing tone
    Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);

    if (!waveform_timer_) {
        waveform_timer_ = lv_timer_create(WaveformTimerCb, 80, this);
    }
}

void BuddyCallOverlay::ShowOutgoingCall(const std::string& callee_name, std::function<void()> on_cancel) {
    if (!backdrop_) return;

    state_ = CallOverlayState::kOutgoing;
    on_hangup_cb_ = on_cancel;

    lv_label_set_text(title_label_, callee_name.c_str());
    lv_label_set_text(status_label_, "Đang gọi cho Bố Mẹ...");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0xFBBF24), 0);

    lv_obj_add_flag(timer_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(incoming_actions_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(active_actions_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(backdrop_, LV_OBJ_FLAG_HIDDEN);

    if (!waveform_timer_) {
        waveform_timer_ = lv_timer_create(WaveformTimerCb, 80, this);
    }
}

void BuddyCallOverlay::SetCallActive() {
    if (!backdrop_) return;

    state_ = CallOverlayState::kActive;
    call_start_ticks_ = esp_timer_get_time() / 1000000;

    lv_label_set_text(status_label_, "Đang đàm thoại 2 chiều");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x34D399), 0);

    lv_label_set_text(timer_label_, "00:00");
    lv_obj_clear_flag(timer_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(incoming_actions_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(active_actions_, LV_OBJ_FLAG_HIDDEN);

    if (!duration_timer_) {
        duration_timer_ = lv_timer_create(CallDurationTimerCb, 1000, this);
    }
}

void BuddyCallOverlay::EndCall(const std::string& reason) {
    if (!backdrop_) return;

    state_ = CallOverlayState::kIdle;
    lv_obj_add_flag(backdrop_, LV_OBJ_FLAG_HIDDEN);

    if (waveform_timer_) {
        lv_timer_delete(waveform_timer_);
        waveform_timer_ = nullptr;
    }
    if (duration_timer_) {
        lv_timer_delete(duration_timer_);
        duration_timer_ = nullptr;
    }
}

void BuddyCallOverlay::OnAcceptBtnCb(lv_event_t* e) {
    auto* self = static_cast<BuddyCallOverlay*>(lv_event_get_user_data(e));
    if (!self) return;

    self->SetCallActive();
    if (self->on_accept_cb_) {
        self->on_accept_cb_();
    }
}

void BuddyCallOverlay::OnRejectBtnCb(lv_event_t* e) {
    auto* self = static_cast<BuddyCallOverlay*>(lv_event_get_user_data(e));
    if (!self) return;

    self->EndCall();
    if (self->on_reject_cb_) {
        self->on_reject_cb_();
    }
}

void BuddyCallOverlay::OnHangupBtnCb(lv_event_t* e) {
    auto* self = static_cast<BuddyCallOverlay*>(lv_event_get_user_data(e));
    if (!self) return;

    self->EndCall();
    if (self->on_hangup_cb_) {
        self->on_hangup_cb_();
    }
}

void BuddyCallOverlay::WaveformTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<BuddyCallOverlay*>(lv_timer_get_user_data(timer));
    if (!self || self->waveform_bars_.empty()) return;

    self->anim_step_ = (self->anim_step_ + 1) % 360;
    float phase = self->anim_step_ * 0.25f;

    for (size_t i = 0; i < self->waveform_bars_.size(); ++i) {
        lv_obj_t* bar = self->waveform_bars_[i];
        if (!bar) continue;

        float wave = std::sin(phase + i * 0.5f) * 0.5f + 0.5f;
        int32_t h = (self->state_ == CallOverlayState::kActive) ? (6 + static_cast<int32_t>(wave * 18)) : 6;
        lv_obj_set_height(bar, h);
    }
}

void BuddyCallOverlay::CallDurationTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<BuddyCallOverlay*>(lv_timer_get_user_data(timer));
    if (!self || self->state_ != CallOverlayState::kActive || !self->timer_label_) return;

    uint32_t now = esp_timer_get_time() / 1000000;
    uint32_t elapsed = (now >= self->call_start_ticks_) ? (now - self->call_start_ticks_) : 0;
    uint32_t mins = elapsed / 60;
    uint32_t secs = elapsed % 60;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu", (unsigned long)mins, (unsigned long)secs);
    lv_label_set_text(self->timer_label_, buf);
}
