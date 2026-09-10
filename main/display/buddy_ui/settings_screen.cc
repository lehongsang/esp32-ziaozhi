#include "settings_screen.h"
#include <esp_log.h>
#include "board.h"
#include "wifi_board.h"
#include "application.h"

#define TAG "SettingsScreen"

SettingsScreen::SettingsScreen() {}
SettingsScreen::~SettingsScreen() {}

void SettingsScreen::Create(lv_obj_t* parent) {
    root_ = lv_obj_create(parent);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(root_, 12, 0);
    lv_obj_set_style_pad_row(root_, 10, 0);
    lv_obj_set_style_pad_top(root_, 12, 0);
    lv_obj_set_scrollbar_mode(root_, LV_SCROLLBAR_MODE_AUTO);

    // 1. Title Header
    lv_obj_t* title_label = lv_label_create(root_);
    lv_label_set_text(title_label, "Settings & Wi-Fi");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);

    // 2. Wi-Fi Configuration Card
    wifi_card_ = lv_obj_create(root_);
    lv_obj_remove_style_all(wifi_card_);
    lv_obj_set_size(wifi_card_, 216, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(wifi_card_, lv_color_hex(0x232438), 0);
    lv_obj_set_style_bg_opa(wifi_card_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(wifi_card_, 14, 0);
    lv_obj_set_style_pad_all(wifi_card_, 12, 0);
    lv_obj_set_flex_flow(wifi_card_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(wifi_card_, 8, 0);

    // Header in card
    lv_obj_t* card_header = lv_label_create(wifi_card_);
    lv_label_set_text(card_header, "Wireless Network (Wi-Fi)");
    lv_obj_set_style_text_color(card_header, lv_color_hex(0xFFB74D), 0);

    // Status label
    wifi_status_label_ = lv_label_create(wifi_card_);
    lv_label_set_text(wifi_status_label_, "Status: Disconnected");
    lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0xCCCCCC), 0);

    // Info label (SSID / IP)
    wifi_info_label_ = lv_label_create(wifi_card_);
    lv_label_set_text(wifi_info_label_, "No Wi-Fi configured");
    lv_obj_set_style_text_color(wifi_info_label_, lv_color_hex(0x8E8EA0), 0);

    // Wi-Fi Config Button
    btn_wifi_config_ = lv_btn_create(wifi_card_);
    lv_obj_set_size(btn_wifi_config_, 192, 36);
    lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x3B82F6), 0);
    lv_obj_set_style_radius(btn_wifi_config_, 10, 0);
    lv_obj_add_event_cb(btn_wifi_config_, WifiConfigBtnClickedCb, LV_EVENT_CLICKED, this);

    btn_config_label_ = lv_label_create(btn_wifi_config_);
    lv_label_set_text(btn_config_label_, "Setup Wi-Fi (Web AP)");
    lv_obj_set_style_text_color(btn_config_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_config_label_);

    // AP Guide Box (hidden by default)
    ap_guide_box_ = lv_obj_create(wifi_card_);
    lv_obj_remove_style_all(ap_guide_box_);
    lv_obj_set_size(ap_guide_box_, 192, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(ap_guide_box_, lv_color_hex(0x181926), 0);
    lv_obj_set_style_bg_opa(ap_guide_box_, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(ap_guide_box_, 8, 0);
    lv_obj_set_style_pad_all(ap_guide_box_, 8, 0);
    lv_obj_add_flag(ap_guide_box_, LV_OBJ_FLAG_HIDDEN);

    ap_guide_label_ = lv_label_create(ap_guide_box_);
    lv_label_set_text(ap_guide_label_, "Connect to device Wi-Fi and open:\nhttp://192.168.4.1");
    lv_obj_set_style_text_color(ap_guide_label_, lv_color_hex(0x4ADE80), 0);

    // 3. Device Info Card
    lv_obj_t* info_card = lv_obj_create(root_);
    lv_obj_remove_style_all(info_card);
    lv_obj_set_size(info_card, 216, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(info_card, lv_color_hex(0x232438), 0);
    lv_obj_set_style_bg_opa(info_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(info_card, 14, 0);
    lv_obj_set_style_pad_all(info_card, 12, 0);
    lv_obj_set_flex_flow(info_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(info_card, 4, 0);

    lv_obj_t* dev_title = lv_label_create(info_card);
    lv_label_set_text(dev_title, "MB BUDDY Device");
    lv_obj_set_style_text_color(dev_title, lv_color_hex(0xEC4899), 0);

    lv_obj_t* ver_label = lv_label_create(info_card);
    lv_label_set_text(ver_label, "Version: v1.0.0 (EN)");
    lv_obj_set_style_text_color(ver_label, lv_color_hex(0xCCCCCC), 0);

    lv_obj_t* mode_label = lv_label_create(info_card);
    lv_label_set_text(mode_label, "Mode: Offline & AI Ready");
    lv_obj_set_style_text_color(mode_label, lv_color_hex(0x8E8EA0), 0);
}

void SettingsScreen::UpdateWifiStatus(bool connected, const std::string& ssid, const std::string& ip) {
    if (!wifi_status_label_ || !wifi_info_label_) return;
    if (connected) {
        lv_label_set_text(wifi_status_label_, "Status: Connected");
        lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0x4ADE80), 0);
        std::string info = "Wi-Fi: " + ssid + "\nIP: " + ip;
        lv_label_set_text(wifi_info_label_, info.c_str());
    } else {
        lv_label_set_text(wifi_status_label_, "Status: Disconnected");
        lv_obj_set_style_text_color(wifi_status_label_, lv_color_hex(0xF87171), 0);
        lv_label_set_text(wifi_info_label_, "Not connected");
    }
}

void SettingsScreen::ShowWifiConfigPrompt(bool active, const std::string& ap_name, const std::string& ip) {
    if (!ap_guide_box_ || !ap_guide_label_ || !btn_config_label_) return;
    if (active) {
        lv_obj_clear_flag(ap_guide_box_, LV_OBJ_FLAG_HIDDEN);
        std::string guide = "Connect to Wi-Fi: " + ap_name + "\nOpen: http://" + ip;
        lv_label_set_text(ap_guide_label_, guide.c_str());
        lv_label_set_text(btn_config_label_, "AP Broadcasting...");
        lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x10B981), 0);
    } else {
        lv_obj_add_flag(ap_guide_box_, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(btn_config_label_, "Setup Wi-Fi (Web AP)");
        lv_obj_set_style_bg_color(btn_wifi_config_, lv_color_hex(0x3B82F6), 0);
    }
}

void SettingsScreen::WifiConfigBtnClickedCb(lv_event_t* e) {
    auto* self = static_cast<SettingsScreen*>(lv_event_get_user_data(e));
    ESP_LOGI(TAG, "Entering Wi-Fi config mode from Settings screen");
    auto* wifi_board = dynamic_cast<WifiBoard*>(&Board::GetInstance());
    if (wifi_board) {
        wifi_board->EnterWifiConfigMode();
    }
    self->ShowWifiConfigPrompt(true, "XiaoZhi-ESP32", "192.168.4.1");
}
