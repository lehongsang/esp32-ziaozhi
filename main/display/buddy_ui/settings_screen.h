#ifndef BUDDY_SETTINGS_SCREEN_H
#define BUDDY_SETTINGS_SCREEN_H

#include <lvgl.h>
#include <string>

class SettingsScreen {
public:
    SettingsScreen();
    ~SettingsScreen();

    void Create(lv_obj_t* parent);
    void Open();
    void Close();
    void Toggle();
    bool IsOpen() const { return is_open_; }

    void UpdateWifiStatus(bool connected, const std::string& ssid, const std::string& ip);
    void ShowWifiConfigPrompt(bool active, const std::string& ap_name, const std::string& ip);
    void SetVolume(int volume);
    void SetBrightness(int brightness);
    void UpdateBattery(int level, bool charging);

private:
    static void WifiConfigBtnClickedCb(lv_event_t* e);
    static void OnCloseBtnClickedCb(lv_event_t* e);
    static void OnVolumeSliderChangedCb(lv_event_t* e);
    static void OnVolumeMuteClickedCb(lv_event_t* e);
    static void OnBrightnessSliderChangedCb(lv_event_t* e);
    static void OnRootGestureCb(lv_event_t* e);

    lv_obj_t* root_ = nullptr;
    bool is_open_ = false;

    // Header
    lv_obj_t* header_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* btn_close_ = nullptr;

    // Content Grid
    lv_obj_t* content_grid_ = nullptr;

    // Card 1: Wi-Fi
    lv_obj_t* wifi_card_ = nullptr;
    lv_obj_t* wifi_status_label_ = nullptr;
    lv_obj_t* wifi_info_label_ = nullptr;
    lv_obj_t* btn_wifi_config_ = nullptr;
    lv_obj_t* btn_config_label_ = nullptr;
    lv_obj_t* ap_guide_box_ = nullptr;
    lv_obj_t* ap_guide_label_ = nullptr;

    // Card 2: Audio & Volume
    lv_obj_t* volume_card_ = nullptr;
    lv_obj_t* btn_mute_ = nullptr;
    lv_obj_t* mute_icon_ = nullptr;
    lv_obj_t* volume_title_ = nullptr;
    lv_obj_t* volume_label_ = nullptr;
    lv_obj_t* volume_slider_ = nullptr;
    int current_volume_ = 80;
    int prev_volume_before_mute_ = 80;
    bool is_muted_ = false;

    // Card 3: Brightness
    lv_obj_t* brightness_card_ = nullptr;
    lv_obj_t* brightness_title_ = nullptr;
    lv_obj_t* brightness_label_ = nullptr;
    lv_obj_t* brightness_slider_ = nullptr;
    int current_brightness_ = 100;

    // Card 4: Battery & Device Info
    lv_obj_t* info_card_ = nullptr;
    lv_obj_t* battery_label_ = nullptr;
    lv_obj_t* ver_label_ = nullptr;
    lv_obj_t* mode_label_ = nullptr;

    // Bottom Pull / Handle Bar
    lv_obj_t* handle_pill_ = nullptr;
};

#endif // BUDDY_SETTINGS_SCREEN_H
