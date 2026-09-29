#ifndef AI_TUTOR_SCREEN_H
#define AI_TUTOR_SCREEN_H

#include <string>
#include <functional>
#include <cstdint>

#include <lvgl.h>

class AiTutorScreen {
public:
    AiTutorScreen();
    ~AiTutorScreen();

    void Create(lv_obj_t* parent);
    void SetSpeechText(const std::string& text);
    void SetListeningState(bool listening);
    void OnMicClicked(std::function<void()> callback);
    void OnCamClicked(std::function<void()> callback);
    void OnQuizClicked(std::function<void()> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnMicBtnCb(lv_event_t* e);
    static void OnCamBtnCb(lv_event_t* e);
    static void OnQuizBtnCb(lv_event_t* e);

    void StartListeningPulse();
    void StopListeningPulse();

    lv_obj_t* container_ = nullptr;
    lv_obj_t* bg_img_ = nullptr;
    lv_obj_t* header_pill_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* speech_bubble_ = nullptr;
    lv_obj_t* speech_label_ = nullptr;
    lv_obj_t* btn_mic_ = nullptr;
    lv_obj_t* btn_cam_ = nullptr;
    lv_obj_t* btn_quiz_ = nullptr;

    bool is_listening_ = false;
    std::function<void()> on_mic_click_;
    std::function<void()> on_cam_click_;
    std::function<void()> on_quiz_click_;
};

#endif // AI_TUTOR_SCREEN_H
