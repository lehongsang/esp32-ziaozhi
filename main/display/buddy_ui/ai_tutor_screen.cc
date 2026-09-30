#include "ai_tutor_screen.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"
#include "buddy_toast_overlay.h"
#include "board.h"
#include "application.h"
#include "assets/lang_config.h"

#include <esp_log.h>
#include <material_symbols.h>
#include <cmath>

#define TAG "AiTutorScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_material_symbols_30_4);

AiTutorScreen::AiTutorScreen() {}
AiTutorScreen::~AiTutorScreen() {
    if (waveform_timer_) {
        lv_timer_delete(waveform_timer_);
        waveform_timer_ = nullptr;
    }
}

void AiTutorScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Deep Pure Black Theme matching Mockup)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Top Row: 3D Robot Avatar (Left) & User Speech Bubble (Right)
    // 2.1 3D Robot Avatar (Pos: 12, 6, Size: 52x52)
    robot_avatar_img_ = lv_image_create(container_);
    lv_image_set_src(robot_avatar_img_, &buddy_robot_avatar);
    lv_obj_set_size(robot_avatar_img_, 52, 52);
    lv_obj_set_pos(robot_avatar_img_, 12, 6);
    lv_obj_set_style_radius(robot_avatar_img_, 14, 0);
    lv_obj_set_style_shadow_width(robot_avatar_img_, 10, 0);
    lv_obj_set_style_shadow_color(robot_avatar_img_, lv_color_hex(0x38BDF8), 0); // Subtle Cyan Glow
    lv_obj_set_style_shadow_opa(robot_avatar_img_, LV_OPA_40, 0);

    // 2.2 User Speech Bubble (Pos: 70, 6, Size: 238x52, Vibrant Royal Blue)
    user_bubble_ = lv_obj_create(container_);
    lv_obj_remove_style_all(user_bubble_);
    lv_obj_set_size(user_bubble_, 238, 52);
    lv_obj_set_pos(user_bubble_, 70, 6);
    lv_obj_set_style_bg_color(user_bubble_, lv_color_hex(0x2563EB), 0); // Vibrant Blue #2563EB
    lv_obj_set_style_bg_opa(user_bubble_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(user_bubble_, 16, 0);
    lv_obj_set_style_pad_hor(user_bubble_, 12, 0);
    lv_obj_set_style_pad_ver(user_bubble_, 4, 0);
    lv_obj_set_flex_flow(user_bubble_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(user_bubble_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(user_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    user_label_ = lv_label_create(user_bubble_);
    lv_obj_set_style_text_font(user_label_, GetBuddyFont(), 0);
    lv_obj_set_width(user_label_, 214);
    lv_label_set_long_mode(user_label_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(user_label_, "Buddy ơi,\nngày mai mình học gì?");
    lv_obj_set_style_text_color(user_label_, lv_color_hex(0xFFFFFF), 0);

    // 3. Middle Row: AI Response Bubble (Pos: 12, 64, Size: 296x72, Pure Crisp White)
    assistant_bubble_ = lv_obj_create(container_);
    lv_obj_remove_style_all(assistant_bubble_);
    lv_obj_set_size(assistant_bubble_, 296, 72);
    lv_obj_set_pos(assistant_bubble_, 12, 64);
    lv_obj_set_style_bg_color(assistant_bubble_, lv_color_hex(0xFFFFFF), 0); // Clean White #FFFFFF
    lv_obj_set_style_bg_opa(assistant_bubble_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(assistant_bubble_, 18, 0);
    lv_obj_set_style_shadow_width(assistant_bubble_, 10, 0);
    lv_obj_set_style_shadow_color(assistant_bubble_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(assistant_bubble_, LV_OPA_40, 0);
    lv_obj_set_style_pad_hor(assistant_bubble_, 14, 0);
    lv_obj_set_style_pad_ver(assistant_bubble_, 6, 0);
    lv_obj_set_flex_flow(assistant_bubble_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(assistant_bubble_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(assistant_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    assistant_label_ = lv_label_create(assistant_bubble_);
    lv_obj_set_style_text_font(assistant_label_, GetBuddyFont(), 0);
    lv_obj_set_width(assistant_label_, 268);
    lv_label_set_long_mode(assistant_label_, LV_LABEL_LONG_WRAP);
    lv_label_set_text(assistant_label_, "Ngày mai con có\nToán, Tiếng Việt\nvà Mỹ thuật.");
    lv_obj_set_style_text_color(assistant_label_, lv_color_hex(0x0F172A), 0); // Bold dark navy text

    // 4. Bottom Action Section (Pos: 12, 142, Size: 296x48)
    // 4.1 Audio Waveform Container (Glowing Cyan Waveform Bars)
    waveform_container_ = lv_obj_create(container_);
    lv_obj_remove_style_all(waveform_container_);
    lv_obj_set_size(waveform_container_, 226, 42);
    lv_obj_set_pos(waveform_container_, 12, 144);
    lv_obj_set_flex_flow(waveform_container_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(waveform_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(waveform_container_, 5, 0);
    lv_obj_clear_flag(waveform_container_, LV_OBJ_FLAG_SCROLLABLE);

    const int kNumBars = 16;
    waveform_bars_.clear();
    for (int i = 0; i < kNumBars; ++i) {
        lv_obj_t* bar = lv_obj_create(waveform_container_);
        lv_obj_remove_style_all(bar);
        lv_obj_set_size(bar, 4, 10);
        lv_obj_set_style_radius(bar, 2, 0);
        lv_obj_set_style_bg_color(bar, lv_color_hex(0x38BDF8), 0); // Glowing Cyan #38BDF8
        lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        waveform_bars_.push_back(bar);
    }

    // 4.2 Floating Round White Mic Button (Pos: 252, 140, Size: 50x50)
    btn_mic_ = lv_btn_create(container_);
    lv_obj_remove_style_all(btn_mic_);
    lv_obj_set_size(btn_mic_, 50, 50);
    lv_obj_set_pos(btn_mic_, 252, 140);
    lv_obj_set_style_radius(btn_mic_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_mic_, lv_color_hex(0xFFFFFF), 0); // Crisp White Button
    lv_obj_set_style_bg_opa(btn_mic_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_mic_, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_style_border_width(btn_mic_, 2, 0);
    lv_obj_set_style_shadow_width(btn_mic_, 12, 0);
    lv_obj_set_style_shadow_color(btn_mic_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(btn_mic_, LV_OPA_50, 0);
    lv_obj_add_event_cb(btn_mic_, OnMicBtnCb, LV_EVENT_CLICKED, this);

    mic_icon_ = lv_label_create(btn_mic_);
    lv_label_set_text(mic_icon_, MATERIAL_SYMBOLS_MIC);
    lv_obj_set_style_text_font(mic_icon_, &font_material_symbols_30_4, 0);
    lv_obj_set_style_text_color(mic_icon_, lv_color_hex(0x10B981), 0); // Emerald Green Microphone
    lv_obj_center(mic_icon_);

    // 5. Bottom Status Footer Label (Pos: 12, 196, Width: 296)
    status_label_ = lv_label_create(container_);
    lv_obj_set_style_text_font(status_label_, GetBuddyFont(), 0);
    lv_obj_set_size(status_label_, 296, 20);
    lv_obj_set_pos(status_label_, 12, 196);
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(status_label_, "Luôn sẵn sàng khi con cần");
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0x94A3B8), 0);

    // 6. Start Waveform Animation Timer (Updates dynamic equalizer effect)
    StartWaveformAnimation();

    ESP_LOGI(TAG, "AiTutorScreen created with 3D Robot Avatar, real-time speech bubbles and dynamic equalizer");
}

void AiTutorScreen::SetUserMessage(const std::string& text) {
    if (user_label_ && !text.empty()) {
        lv_label_set_text(user_label_, text.c_str());
    }
}

void AiTutorScreen::SetAssistantMessage(const std::string& text) {
    if (assistant_label_ && !text.empty()) {
        lv_label_set_text(assistant_label_, text.c_str());
    }
}

void AiTutorScreen::SetSpeechText(const std::string& text) {
    // Fallback updates assistant message
    SetAssistantMessage(text);
}

void AiTutorScreen::SetListeningState(bool listening) {
    is_listening_ = listening;
    is_speaking_ = false;

    if (mic_icon_) {
        lv_obj_set_style_text_color(mic_icon_, listening ? lv_color_hex(0xEF4444) : lv_color_hex(0x10B981), 0);
    }
    if (status_label_) {
        lv_label_set_text(status_label_, listening ? "Đang lắng nghe con nói..." : "Luôn sẵn sàng khi con cần");
        lv_obj_set_style_text_color(status_label_, listening ? lv_color_hex(0x38BDF8) : lv_color_hex(0x94A3B8), 0);
    }
}

void AiTutorScreen::SetSpeakingState(bool speaking) {
    is_speaking_ = speaking;
    is_listening_ = false;

    if (mic_icon_) {
        lv_obj_set_style_text_color(mic_icon_, lv_color_hex(0x3B82F6), 0);
    }
    if (status_label_) {
        lv_label_set_text(status_label_, speaking ? "Buddy đang trả lời..." : "Luôn sẵn sàng khi con cần");
        lv_obj_set_style_text_color(status_label_, speaking ? lv_color_hex(0x34D399) : lv_color_hex(0x94A3B8), 0);
    }
}

void AiTutorScreen::SetIdleState() {
    is_listening_ = false;
    is_speaking_ = false;

    if (mic_icon_) {
        lv_obj_set_style_text_color(mic_icon_, lv_color_hex(0x10B981), 0);
    }
    if (status_label_) {
        lv_label_set_text(status_label_, "Luôn sẵn sàng khi con cần");
        lv_obj_set_style_text_color(status_label_, lv_color_hex(0x94A3B8), 0);
    }
}

void AiTutorScreen::StartWaveformAnimation() {
    if (!waveform_timer_) {
        waveform_timer_ = lv_timer_create(WaveformTimerCb, 80, this);
    }
}

void AiTutorScreen::StopWaveformAnimation() {
    if (waveform_timer_) {
        lv_timer_delete(waveform_timer_);
        waveform_timer_ = nullptr;
    }
}

void AiTutorScreen::WaveformTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<AiTutorScreen*>(lv_timer_get_user_data(timer));
    if (!self || self->waveform_bars_.empty()) return;

    self->anim_step_ = (self->anim_step_ + 1) % 360;
    float phase = self->anim_step_ * 0.2f;

    for (size_t i = 0; i < self->waveform_bars_.size(); ++i) {
        lv_obj_t* bar = self->waveform_bars_[i];
        if (!bar) continue;

        int32_t height = 8;
        if (self->is_listening_) {
            // Dynamic energetic wave when listening
            float wave = std::sin(phase + i * 0.45f) * 0.5f + 0.5f;
            float wave2 = std::cos(phase * 0.7f + i * 0.3f) * 0.5f + 0.5f;
            height = 8 + static_cast<int32_t>((wave * 0.6f + wave2 * 0.4f) * 28);
            lv_obj_set_style_bg_color(bar, lv_color_hex(0x38BDF8), 0); // Vibrant Cyan
        } else if (self->is_speaking_) {
            // Rhythmic pulse when speaking
            float wave = std::sin(phase * 1.2f + i * 0.5f) * 0.5f + 0.5f;
            height = 8 + static_cast<int32_t>(wave * 24);
            lv_obj_set_style_bg_color(bar, lv_color_hex(0x34D399), 0); // Emerald Cyan
        } else {
            // Gentle resting wave when idle
            float wave = std::sin(phase * 0.3f + i * 0.3f) * 0.5f + 0.5f;
            height = 6 + static_cast<int32_t>(wave * 10);
            lv_obj_set_style_bg_color(bar, lv_color_hex(0x0284C7), 0); // Muted Sky Blue
        }
        lv_obj_set_height(bar, height);
    }
}

void AiTutorScreen::OnMicBtnCb(lv_event_t* e) {
    auto* self = static_cast<AiTutorScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    auto dev_state = Application::GetInstance().GetDeviceState();
    if (dev_state == kDeviceStateWifiConfiguring || dev_state == kDeviceStateStarting) {
        Application::GetInstance().PlaySound(Lang::Sounds::OGG_EXCLAMATION);
        BuddyToastOverlay::GetInstance().Show("Chưa có Wi-Fi!", "Hãy vuốt trên xuống để cài đặt Wi-Fi.", ToastType::kWarning, 3500);
        return;
    }

    // Toggle Chat State (Start listening or stop chatting)
    Application::GetInstance().ToggleChatState();

    if (self->on_mic_click_) {
        self->on_mic_click_();
    }
}

void AiTutorScreen::OnMicClicked(std::function<void()> callback) {
    on_mic_click_ = callback;
}
