#ifndef BUDDY_SETTINGS_SCREEN_H
#define BUDDY_SETTINGS_SCREEN_H

#include <lvgl.h>
#include <string>

class SettingsScreen {
public:
    SettingsScreen();
    ~SettingsScreen();

    void Create(lv_obj_t* parent);
    void UpdateWifiStatus(bool connected, const std::string& ssid, const std::string& ip);
    void ShowWifiConfigPrompt(bool active, const std::string& ap_name, const std::string& ip);

private:
    lv_obj_t* root_ = nullptr;
    lv_obj_t* wifi_card_ = nullptr;
    lv_obj_t* wifi_status_label_ = nullptr;
    lv_obj_t* wifi_info_label_ = nullptr;
    lv_obj_t* btn_wifi_config_ = nullptr;
    lv_obj_t* btn_config_label_ = nullptr;
    lv_obj_t* ap_guide_box_ = nullptr;
    lv_obj_t* ap_guide_label_ = nullptr;

    static void WifiConfigBtnClickedCb(lv_event_t* e);
};

#endif // BUDDY_SETTINGS_SCREEN_H
