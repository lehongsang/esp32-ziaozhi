#include "ai_tutor_screen.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"

#include <esp_log.h>
#include <material_symbols.h>

#define TAG "AiTutorScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

AiTutorScreen::AiTutorScreen() {}
AiTutorScreen::~AiTutorScreen() {}

void AiTutorScreen::Create(lv_obj_t* parent) {
    // 1. Root Container
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 3D Robot AI Background (320x240 RGB565)
    bg_img_ = lv_image_create(container_);
    lv_image_set_src(bg_img_, &buddy_bg_tutor);
    lv_obj_set_size(bg_img_, 320, 240);
    lv_obj_align(bg_img_, LV_ALIGN_TOP_LEFT, 0, 0);

    // 3. Top Status HUD Bar - Centered Title Pill
    header_pill_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_pill_);
    lv_obj_set_size(header_pill_, 110, 24);
    lv_obj_align(header_pill_, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(header_pill_, lv_color_hex(0x0A0E1A), 0);
    lv_obj_set_style_bg_opa(header_pill_, LV_OPA_60, 0);
    lv_obj_set_style_radius(header_pill_, 12, 0);
    lv_obj_set_style_border_color(header_pill_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_opa(header_pill_, LV_OPA_20, 0);
    lv_obj_set_style_border_width(header_pill_, 1, 0);
    lv_obj_set_flex_flow(header_pill_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header_pill_, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(header_pill_);
    lv_label_set_text(title_label_, "AI Tutor");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), 0);

    // 4. Interactive Floating Speech Text Area (Centered inside the Cloud Thought Bubble)
    speech_bubble_ = lv_obj_create(container_);
    lv_obj_remove_style_all(speech_bubble_);
    lv_obj_set_size(speech_bubble_, 152, 84);
    lv_obj_set_pos(speech_bubble_, 12, 42);
    lv_obj_set_style_bg_opa(speech_bubble_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(speech_bubble_, 0, 0);
    lv_obj_set_style_pad_all(speech_bubble_, 0, 0);
    lv_obj_set_flex_flow(speech_bubble_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(speech_bubble_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(speech_bubble_, LV_OBJ_FLAG_SCROLLABLE);

    speech_label_ = lv_label_create(speech_bubble_);
    lv_obj_set_width(speech_label_, 144);
    lv_label_set_long_mode(speech_label_, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(speech_label_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_text_align(speech_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(speech_label_, "How can I\nhelp you today?");

    // 5. Bottom 3 Interactive Action Buttons (Vector Material Symbols)
    lv_obj_t* btn_bar = lv_obj_create(container_);
    lv_obj_remove_style_all(btn_bar);
    lv_obj_set_size(btn_bar, 158, 52);
    lv_obj_set_pos(btn_bar, 8, 172);
    lv_obj_set_flex_flow(btn_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_bar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(btn_bar, LV_OBJ_FLAG_SCROLLABLE);

    // 5.1 Microphone Button (Purple Circular)
    btn_mic_ = lv_btn_create(btn_bar);
    lv_obj_set_size(btn_mic_, 44, 44);
    lv_obj_set_style_radius(btn_mic_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_mic_, lv_color_hex(0x7B2CBF), 0);
    lv_obj_set_style_bg_opa(btn_mic_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_mic_, lv_color_hex(0xC77DFF), 0);
    lv_obj_set_style_border_width(btn_mic_, 2, 0);
    lv_obj_add_event_cb(btn_mic_, OnMicBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* mic_icon = lv_label_create(btn_mic_);
    lv_label_set_text(mic_icon, MATERIAL_SYMBOLS_MIC);
    lv_obj_set_style_text_font(mic_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(mic_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(mic_icon);

    // 5.2 Camera Button (Emerald Green Circular)
    btn_cam_ = lv_btn_create(btn_bar);
    lv_obj_set_size(btn_cam_, 44, 44);
    lv_obj_set_style_radius(btn_cam_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_cam_, lv_color_hex(0x06D6A0), 0);
    lv_obj_set_style_bg_opa(btn_cam_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_cam_, lv_color_hex(0x70E000), 0);
    lv_obj_set_style_border_width(btn_cam_, 2, 0);
    lv_obj_add_event_cb(btn_cam_, OnCamBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* cam_icon = lv_label_create(btn_cam_);
    lv_label_set_text(cam_icon, MATERIAL_SYMBOLS_PHOTO_CAMERA);
    lv_obj_set_style_text_font(cam_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(cam_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(cam_icon);

    // 5.3 Quiz / Keyboard Button (Amber Orange Circular)
    btn_quiz_ = lv_btn_create(btn_bar);
    lv_obj_set_size(btn_quiz_, 44, 44);
    lv_obj_set_style_radius(btn_quiz_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_quiz_, lv_color_hex(0xFFB703), 0);
    lv_obj_set_style_bg_opa(btn_quiz_, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn_quiz_, lv_color_hex(0xFFD166), 0);
    lv_obj_set_style_border_width(btn_quiz_, 2, 0);
    lv_obj_add_event_cb(btn_quiz_, OnQuizBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* quiz_icon = lv_label_create(btn_quiz_);
    lv_label_set_text(quiz_icon, MATERIAL_SYMBOLS_EDIT_SQUARE);
    lv_obj_set_style_text_font(quiz_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(quiz_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(quiz_icon);

    ESP_LOGI(TAG, "AiTutorScreen created with 3D robot background and vector action buttons");
}

void AiTutorScreen::OnMicBtnCb(lv_event_t* e) {
    auto* self = static_cast<AiTutorScreen*>(lv_event_get_user_data(e));
    if (self) {
        if (self->speech_label_) {
            lv_label_set_text(self->speech_label_, "Listening to you\nspeak...");
        }
        self->SetListeningState(true);
        if (self->on_mic_click_) {
            self->on_mic_click_();
        }
    }
}

void AiTutorScreen::OnCamBtnCb(lv_event_t* e) {
    auto* self = static_cast<AiTutorScreen*>(lv_event_get_user_data(e));
    if (self) {
        if (self->speech_label_) {
            lv_label_set_text(self->speech_label_, "Capturing homework\nphoto...");
        }
        if (self->on_cam_click_) {
            self->on_cam_click_();
        }
    }
}

void AiTutorScreen::OnQuizBtnCb(lv_event_t* e) {
    auto* self = static_cast<AiTutorScreen*>(lv_event_get_user_data(e));
    if (self) {
        if (self->speech_label_) {
            lv_label_set_text(self->speech_label_, "Starting interactive\nquiz mode!");
        }
        if (self->on_quiz_click_) {
            self->on_quiz_click_();
        }
    }
}

void AiTutorScreen::SetSpeechText(const std::string& text) {
    if (speech_label_) {
        lv_label_set_text(speech_label_, text.c_str());
    }
}

void AiTutorScreen::SetListeningState(bool listening) {
    if (btn_mic_) {
        lv_obj_set_style_bg_color(btn_mic_, listening ? lv_color_hex(0xFF006E) : lv_color_hex(0x7B2CBF), 0);
        lv_obj_set_style_border_color(btn_mic_, listening ? lv_color_hex(0xFF70A6) : lv_color_hex(0xC77DFF), 0);
    }
}

void AiTutorScreen::OnMicClicked(std::function<void()> callback) {
    on_mic_click_ = callback;
}

void AiTutorScreen::OnCamClicked(std::function<void()> callback) {
    on_cam_click_ = callback;
}

void AiTutorScreen::OnQuizClicked(std::function<void()> callback) {
    on_quiz_click_ = callback;
}
