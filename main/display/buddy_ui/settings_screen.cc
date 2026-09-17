#include "settings_screen.h"
#include "screen_manager.h"
#include <esp_log.h>
#include "board.h"
#include "wifi_board.h"
#include "application.h"

#define TAG "SettingsScreen"

SettingsScreen::SettingsScreen() {}
SettingsScreen::~SettingsScreen() {}

void SettingsScreen::Create(lv_obj_t* parent) {
    // 1. Root Container: Full Display Size 320x240, Starts Hidden Above (y = -240)
    root_ = lv_obj_create(parent);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, 320, 240);
    lv_obj_set_pos(root_, 0, -240);
    lv_obj_set_style_bg_color(root_, lv_color_hex(0x0C0E17), 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0); // 100% OPAQUE - ZERO BLEED FROM BACKGROUND
    lv_obj_set_style_pad_all(root_, 0, 0);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(root_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(root_, OnRootGestureCb, LV_EVENT_GESTURE, this);

    // Read initial volume from audio codec if available
    auto* codec = Board::GetInstance().GetAudioCodec();
    if (codec) {
        current_volume_ = codec->output_volume();
        if (current_volume_ <= 0) current_volume_ = 80;
        prev_volume_before_mute_ = current_volume_;
    }

    // Read initial brightness from backlight if available
    auto* bl = Board::GetInstance().GetBacklight();
    if (bl) {
        current_brightness_ = bl->brightness();
        if (current_brightness_ <= 0) current_brightness_ = 100;
    }

    // 2. Top Header Bar (Height: 28px)
    header_ = lv_obj_create(root_);
    lv_obj_remove_style_all(header_);
    lv_obj_set_size(header_, 320, 28);
    lv_obj_set_pos(header_, 0, 0);
    lv_obj_set_style_bg_color(header_, lv_color_hex(0x131722), 0);
    lv_obj_set_style_bg_opa(header_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(header_, LV_OBJ_FLAG_SCROLLABLE);

    title_label_ = lv_label_create(header_);
    lv_label_set_text(title_label_, "⚙️ Control Center");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xF8FAFC), 0);
    lv_obj_align(title_label_, LV_ALIGN_LEFT_MID, 12, 0);

    btn_close_ = lv_button_create(header_);
    lv_obj_set_size(btn_close_, 28, 22);
    lv_obj_set_style_radius(btn_close_, 6, 0);
    lv_obj_set_style_bg_color(btn_close_, lv_color_hex(0x273043), 0);
    lv_obj_set_style_pad_all(btn_close_, 0, 0);
    lv_obj_align(btn_close_, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_add_event_cb(btn_close_, OnCloseBtnClickedCb, LV_EVENT_CLICKED, this);

    lv_obj_t* close_lbl = lv_label_create(btn_close_);
    lv_label_set_text(close_lbl, "✕");
    lv_obj_set_style_text_color(close_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(close_lbl);

    // 3. Main Content Container (Height: 180px)
    lv_obj_t* content = lv_obj_create(root_);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, 304, 180);
    lv_obj_align(content, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(content, 6, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    // 3.1 Row 1: Wi-Fi Card & Volume Card
    lv_obj_t* row1 = lv_obj_create(content);
    lv_obj_remove_style_all(row1);
    lv_obj_set_size(row1, 304, 84);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row1, LV_OBJ_FLAG_SCROLLABLE);

    // Card 1: Wi-Fi Tile (148 x 84px)
    wifi_card_ = lv_obj_create(row1);
    lv_obj_remove_style_all(wifi_card_);
    lv_obj_set_size(wifi_card_, 148, 84);
    lv_obj_set_style_bg_color(wifi_card_, lv_color_hex(0x13192B), 0);
    lv_obj_set_style_bg_opa(wifi_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(wifi_card_, 10, 0);
    lv_obj_set_style_border_color(wifi_card_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_border_opa(wifi_card_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(wifi_card_, 1, 0);
    lv_obj_set_style_pad_left(wifi_card_, 8, 0);
    lv_obj_set_style_pad_right(wifi_card_, 8, 0);
    lv_obj_set_style_pad_top(wifi_card_, 6, 0);
    lv_obj_set_style_pad_bottom(wifi_card_, 6, 0);
    lv_obj_set_flex_flow(wifi_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(wifi_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(wifi_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* wifi_head = lv_obj_create(wifi_card_);
    lv_obj_remove_style_all(wifi_head);
    lv_obj_set_size(wifi_head, lv_pct(100), 16);
    lv_obj_set_flex_flow(wifi_head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wifi_head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(wifi_head, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* w_title = lv_label_create(wifi_head);
    lv_label_set_text(w_title, "📶 Wi-Fi");
    lv_obj_set_style_text_color(w_title, lv_color_hex(0x60A5FA), 0);

    wifi_status_label_ = lv_label_create(wifi_head);
    lv_label_set_text(wifi_status_label_, "Online");
    lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0x4ADE80), 0);

    wifi_info_label_ = lv_label_create(wifi_card_);
    lv_label_set_text(wifi_info_label_, "SSID: Connected");
    lv_obj_set_style_text_color(wifi_info_label_, lv_color_hex(0x94A3B8), 0);

    btn_wifi_config_ = lv_button_create(wifi_card_);
    lv_obj_set_size(btn_wifi_config_, 132, 22);
    lv_obj_set_style_radius(btn_wifi_config_, 6, 0);
    lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_pad_all(btn_wifi_config_, 0, 0);
    lv_obj_add_event_cb(btn_wifi_config_, WifiConfigBtnClickedCb, LV_EVENT_CLICKED, this);

    btn_config_label_ = lv_label_create(btn_wifi_config_);
    lv_label_set_text(btn_config_label_, "⚡ Setup AP");
    lv_obj_set_style_text_color(btn_config_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_config_label_);

    // Card 2: Audio & Volume Tile (148 x 84px)
    volume_card_ = lv_obj_create(row1);
    lv_obj_remove_style_all(volume_card_);
    lv_obj_set_size(volume_card_, 148, 84);
    lv_obj_set_style_bg_color(volume_card_, lv_color_hex(0x11211A), 0);
    lv_obj_set_style_bg_opa(volume_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(volume_card_, 10, 0);
    lv_obj_set_style_border_color(volume_card_, lv_color_hex(0x10B981), 0);
    lv_obj_set_style_border_opa(volume_card_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(volume_card_, 1, 0);
    lv_obj_set_style_pad_left(volume_card_, 8, 0);
    lv_obj_set_style_pad_right(volume_card_, 8, 0);
    lv_obj_set_style_pad_top(volume_card_, 6, 0);
    lv_obj_set_style_pad_bottom(volume_card_, 6, 0);
    lv_obj_set_flex_flow(volume_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(volume_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(volume_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* vol_head = lv_obj_create(volume_card_);
    lv_obj_remove_style_all(vol_head);
    lv_obj_set_size(vol_head, lv_pct(100), 16);
    lv_obj_set_flex_flow(vol_head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(vol_head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(vol_head, LV_OBJ_FLAG_SCROLLABLE);

    volume_title_ = lv_label_create(vol_head);
    lv_label_set_text(volume_title_, "🔊 Sound");
    lv_obj_set_style_text_color(volume_title_, lv_color_hex(0x34D399), 0);

    volume_label_ = lv_label_create(vol_head);
    lv_label_set_text(volume_label_, (std::to_string(current_volume_) + "%").c_str());
    lv_obj_set_style_text_color(volume_label_, lv_color_hex(0xFFFFFF), 0);

    volume_slider_ = lv_slider_create(volume_card_);
    lv_obj_set_size(volume_slider_, 132, 12);
    lv_slider_set_range(volume_slider_, 0, 100);
    lv_slider_set_value(volume_slider_, current_volume_, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(volume_slider_, lv_color_hex(0x1F2937), 0);
    lv_obj_set_style_bg_color(volume_slider_, lv_color_hex(0x10B981), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(volume_slider_, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_pad_all(volume_slider_, 2, LV_PART_KNOB);
    lv_obj_add_event_cb(volume_slider_, OnVolumeSliderChangedCb, LV_EVENT_VALUE_CHANGED, this);

    btn_mute_ = lv_button_create(volume_card_);
    lv_obj_set_size(btn_mute_, 132, 22);
    lv_obj_set_style_radius(btn_mute_, 6, 0);
    lv_obj_set_style_bg_color(btn_mute_, lv_color_hex(0x1F2937), 0);
    lv_obj_set_style_pad_all(btn_mute_, 0, 0);
    lv_obj_add_event_cb(btn_mute_, OnVolumeMuteClickedCb, LV_EVENT_CLICKED, this);

    mute_icon_ = lv_label_create(btn_mute_);
    lv_label_set_text(mute_icon_, "🔇 Mute");
    lv_obj_set_style_text_color(mute_icon_, lv_color_hex(0x94A3B8), 0);
    lv_obj_center(mute_icon_);

    // 3.2 Row 2: Brightness Card & Battery Info Card
    lv_obj_t* row2 = lv_obj_create(content);
    lv_obj_remove_style_all(row2);
    lv_obj_set_size(row2, 304, 84);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(row2, LV_OBJ_FLAG_SCROLLABLE);

    // Card 3: Brightness Tile (148 x 84px)
    brightness_card_ = lv_obj_create(row2);
    lv_obj_remove_style_all(brightness_card_);
    lv_obj_set_size(brightness_card_, 148, 84);
    lv_obj_set_style_bg_color(brightness_card_, lv_color_hex(0x23180E), 0);
    lv_obj_set_style_bg_opa(brightness_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(brightness_card_, 10, 0);
    lv_obj_set_style_border_color(brightness_card_, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_opa(brightness_card_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(brightness_card_, 1, 0);
    lv_obj_set_style_pad_left(brightness_card_, 8, 0);
    lv_obj_set_style_pad_right(brightness_card_, 8, 0);
    lv_obj_set_style_pad_top(brightness_card_, 6, 0);
    lv_obj_set_style_pad_bottom(brightness_card_, 6, 0);
    lv_obj_set_flex_flow(brightness_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(brightness_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(brightness_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* br_head = lv_obj_create(brightness_card_);
    lv_obj_remove_style_all(br_head);
    lv_obj_set_size(br_head, lv_pct(100), 16);
    lv_obj_set_flex_flow(br_head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(br_head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(br_head, LV_OBJ_FLAG_SCROLLABLE);

    brightness_title_ = lv_label_create(br_head);
    lv_label_set_text(brightness_title_, "☀️ Brightness");
    lv_obj_set_style_text_color(brightness_title_, lv_color_hex(0xFBBF24), 0);

    brightness_label_ = lv_label_create(br_head);
    lv_label_set_text(brightness_label_, (std::to_string(current_brightness_) + "%").c_str());
    lv_obj_set_style_text_color(brightness_label_, lv_color_hex(0xFFFFFF), 0);

    brightness_slider_ = lv_slider_create(brightness_card_);
    lv_obj_set_size(brightness_slider_, 132, 12);
    lv_slider_set_range(brightness_slider_, 10, 100);
    lv_slider_set_value(brightness_slider_, current_brightness_, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(brightness_slider_, lv_color_hex(0x1F2937), 0);
    lv_obj_set_style_bg_color(brightness_slider_, lv_color_hex(0xF59E0B), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(brightness_slider_, lv_color_hex(0xFFFFFF), LV_PART_KNOB);
    lv_obj_set_style_pad_all(brightness_slider_, 2, LV_PART_KNOB);
    lv_obj_add_event_cb(brightness_slider_, OnBrightnessSliderChangedCb, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t* br_sub = lv_label_create(brightness_card_);
    lv_label_set_text(br_sub, "Display Backlight");
    lv_obj_set_style_text_color(br_sub, lv_color_hex(0x78716C), 0);

    // Card 4: Device & Battery Tile (148 x 84px)
    info_card_ = lv_obj_create(row2);
    lv_obj_remove_style_all(info_card_);
    lv_obj_set_size(info_card_, 148, 84);
    lv_obj_set_style_bg_color(info_card_, lv_color_hex(0x1B162A), 0);
    lv_obj_set_style_bg_opa(info_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(info_card_, 10, 0);
    lv_obj_set_style_border_color(info_card_, lv_color_hex(0x8B5CF6), 0);
    lv_obj_set_style_border_opa(info_card_, LV_OPA_50, 0);
    lv_obj_set_style_border_width(info_card_, 1, 0);
    lv_obj_set_style_pad_left(info_card_, 8, 0);
    lv_obj_set_style_pad_right(info_card_, 8, 0);
    lv_obj_set_style_pad_top(info_card_, 6, 0);
    lv_obj_set_style_pad_bottom(info_card_, 6, 0);
    lv_obj_set_flex_flow(info_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(info_card_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(info_card_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* bat_head = lv_obj_create(info_card_);
    lv_obj_remove_style_all(bat_head);
    lv_obj_set_size(bat_head, lv_pct(100), 16);
    lv_obj_set_flex_flow(bat_head, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bat_head, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bat_head, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* bat_title = lv_label_create(bat_head);
    lv_label_set_text(bat_title, "🔋 Battery");
    lv_obj_set_style_text_color(bat_title, lv_color_hex(0xA78BFA), 0);

    battery_label_ = lv_label_create(bat_head);
    int bat_lvl = 100;
    bool charging = false, discharging = false;
    Board::GetInstance().GetBatteryLevel(bat_lvl, charging, discharging);
    std::string bat_str = std::to_string(bat_lvl) + "%";
    lv_label_set_text(battery_label_, bat_str.c_str());
    lv_obj_set_style_text_color(battery_label_, lv_color_hex(0x34D399), 0);

    ver_label_ = lv_label_create(info_card_);
    lv_label_set_text(ver_label_, "MB BUDDY v1.0.0");
    lv_obj_set_style_text_color(ver_label_, lv_color_hex(0xE2E8F0), 0);

    mode_label_ = lv_label_create(info_card_);
    lv_label_set_text(mode_label_, "● AI Voice Ready");
    lv_obj_set_style_text_color(mode_label_, lv_color_hex(0x38BDF8), 0);

    // 4. Bottom Handle Bar (Close Touch Zone)
    handle_pill_ = lv_obj_create(root_);
    lv_obj_remove_style_all(handle_pill_);
    lv_obj_set_size(handle_pill_, 160, 24);
    lv_obj_align(handle_pill_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(handle_pill_, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(handle_pill_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(handle_pill_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(handle_pill_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(handle_pill_, OnCloseBtnClickedCb, LV_EVENT_CLICKED, this);

    lv_obj_t* handle_bar = lv_obj_create(handle_pill_);
    lv_obj_remove_style_all(handle_bar);
    lv_obj_set_size(handle_bar, 40, 4);
    lv_obj_set_style_bg_color(handle_bar, lv_color_hex(0x475569), 0);
    lv_obj_set_style_radius(handle_bar, 2, 0);
}

void SettingsScreen::Open() {
    if (!root_) return;
    is_open_ = true;
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(root_);

    // Freeze background tileview so dragging inside Settings never causes background scrolling
    BuddyScreenManager::GetInstance().SetTileviewScrollable(false);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, root_);
    lv_anim_set_values(&a, lv_obj_get_y(root_), 0);
    lv_anim_set_duration(&a, 200);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, [](void* var, int32_t v) {
        lv_obj_set_y((lv_obj_t*)var, v);
    });
    lv_anim_start(&a);
}

void SettingsScreen::Close() {
    if (!root_) return;
    is_open_ = false;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, root_);
    lv_anim_set_values(&a, lv_obj_get_y(root_), -240);
    lv_anim_set_duration(&a, 180);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, [](void* var, int32_t v) {
        lv_obj_set_y((lv_obj_t*)var, v);
    });
    lv_anim_set_completed_cb(&a, [](lv_anim_t* anim) {
        lv_obj_add_flag((lv_obj_t*)anim->var, LV_OBJ_FLAG_HIDDEN);
        // Restore background tileview scrolling
        BuddyScreenManager::GetInstance().SetTileviewScrollable(true);
    });
    lv_anim_start(&a);
}

void SettingsScreen::Toggle() {
    if (is_open_) {
        Close();
    } else {
        Open();
    }
}

void SettingsScreen::OnCloseBtnClickedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    if (self) self->Close();
}

void SettingsScreen::OnRootGestureCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_TOP) {
        self->Close();
    }
}

void SettingsScreen::OnVolumeSliderChangedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    if (!self || !self->volume_slider_) return;

    int32_t val = lv_slider_get_value(self->volume_slider_);
    self->current_volume_ = val;
    if (self->volume_label_) {
        lv_label_set_text(self->volume_label_, (std::to_string(val) + "%").c_str());
    }

    if (val > 0) {
        self->is_muted_ = false;
        if (self->mute_icon_) lv_label_set_text(self->mute_icon_, "🔊");
    } else {
        self->is_muted_ = true;
        if (self->mute_icon_) lv_label_set_text(self->mute_icon_, "🔇");
    }

    auto* codec = Board::GetInstance().GetAudioCodec();
    if (codec) {
        codec->SetOutputVolume(val);
    }
}

void SettingsScreen::OnVolumeMuteClickedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    if (!self || !self->volume_slider_) return;

    self->is_muted_ = !self->is_muted_;
    if (self->is_muted_) {
        self->prev_volume_before_mute_ = self->current_volume_;
        self->current_volume_ = 0;
        lv_slider_set_value(self->volume_slider_, 0, LV_ANIM_ON);
        if (self->volume_label_) lv_label_set_text(self->volume_label_, "0%");
        if (self->mute_icon_) lv_label_set_text(self->mute_icon_, "🔇");
    } else {
        self->current_volume_ = self->prev_volume_before_mute_ > 0 ? self->prev_volume_before_mute_ : 80;
        lv_slider_set_value(self->volume_slider_, self->current_volume_, LV_ANIM_ON);
        if (self->volume_label_) lv_label_set_text(self->volume_label_, (std::to_string(self->current_volume_) + "%").c_str());
        if (self->mute_icon_) lv_label_set_text(self->mute_icon_, "🔊");
    }

    auto* codec = Board::GetInstance().GetAudioCodec();
    if (codec) {
        codec->SetOutputVolume(self->current_volume_);
    }
}

void SettingsScreen::OnBrightnessSliderChangedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    if (!self || !self->brightness_slider_) return;

    int32_t val = lv_slider_get_value(self->brightness_slider_);
    self->current_brightness_ = val;
    if (self->brightness_label_) {
        lv_label_set_text(self->brightness_label_, (std::to_string(val) + "%").c_str());
    }

    auto* bl = Board::GetInstance().GetBacklight();
    if (bl) {
        bl->SetBrightness(val);
    }
}

void SettingsScreen::UpdateWifiStatus(bool connected, const std::string& ssid, const std::string& ip) {
    if (!wifi_status_label_ || !wifi_info_label_) return;
    if (connected) {
        lv_label_set_text(wifi_status_label_, "Connected");
        lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0x4ADE80), 0);
        std::string info = "SSID: " + ssid + "\nIP: " + ip;
        lv_label_set_text(wifi_info_label_, info.c_str());
    } else {
        lv_label_set_text(wifi_status_label_, "Disconnected");
        lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0xF87171), 0);
        lv_label_set_text(wifi_info_label_, "Not connected");
    }
}

void SettingsScreen::ShowWifiConfigPrompt(bool active, const std::string& ap_name, const std::string& ip) {
    if (!btn_config_label_ || !btn_wifi_config_) return;
    if (active) {
        lv_label_set_text(btn_config_label_, "AP Broadcasting...");
        lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x10B981), 0);
    } else {
        lv_label_set_text(btn_config_label_, "Setup Wi-Fi AP");
        lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x2563EB), 0);
    }
}

void SettingsScreen::SetVolume(int volume) {
    current_volume_ = volume;
    if (volume_slider_) lv_slider_set_value(volume_slider_, volume, LV_ANIM_OFF);
    if (volume_label_) lv_label_set_text(volume_label_, (std::to_string(volume) + "%").c_str());
}

void SettingsScreen::SetBrightness(int brightness) {
    current_brightness_ = brightness;
    if (brightness_slider_) lv_slider_set_value(brightness_slider_, brightness, LV_ANIM_OFF);
    if (brightness_label_) lv_label_set_text(brightness_label_, (std::to_string(brightness) + "%").c_str());
}

void SettingsScreen::UpdateBattery(int level, bool charging) {
    if (!battery_label_) return;
    std::string str = std::string(charging ? "⚡ Battery: " : "🔋 Battery: ") + std::to_string(level) + "%";
    lv_label_set_text(battery_label_, str.c_str());
}

void SettingsScreen::WifiConfigBtnClickedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    ESP_LOGI(TAG, "Entering Wi-Fi config mode from Settings Control Center");
    auto* wifi_board = dynamic_cast<WifiBoard*>(&Board::GetInstance());
    if (wifi_board) {
        wifi_board->EnterWifiConfigMode();
    }
    self->ShowWifiConfigPrompt(true, "XiaoZhi-ESP32", "192.168.4.1");
}

