#ifndef AI_TUTOR_SCREEN_H
#define AI_TUTOR_SCREEN_H

#include <string>
#include <functional>
#include <vector>
#include <cstdint>

#include <lvgl.h>

class AiTutorScreen {
public:
    AiTutorScreen();
    ~AiTutorScreen();

    void Create(lv_obj_t* parent);
    void SetUserMessage(const std::string& text);
    void SetAssistantMessage(const std::string& text);
    void SetSpeechText(const std::string& text); // Generic fallback
    void SetListeningState(bool listening);
    void SetSpeakingState(bool speaking);
    void SetIdleState();
    
    void OnMicClicked(std::function<void()> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    static void OnMicBtnCb(lv_event_t* e);
    static void WaveformTimerCb(lv_timer_t* timer);

    void StartWaveformAnimation();
    void StopWaveformAnimation();

    lv_obj_t* container_ = nullptr;
    
    // Top Row: Robot Avatar & User Speech Bubble
    lv_obj_t* top_row_ = nullptr;
    lv_obj_t* robot_avatar_img_ = nullptr;
    lv_obj_t* user_bubble_ = nullptr;
    lv_obj_t* user_label_ = nullptr;

    // Middle Row: AI Response Bubble
    lv_obj_t* assistant_bubble_ = nullptr;
    lv_obj_t* assistant_label_ = nullptr;

    // Bottom Action Row: Audio Waveform & Floating Round Mic Button
    lv_obj_t* bottom_row_ = nullptr;
    lv_obj_t* waveform_container_ = nullptr;
    std::vector<lv_obj_t*> waveform_bars_;
    lv_timer_t* waveform_timer_ = nullptr;

    lv_obj_t* btn_mic_ = nullptr;
    lv_obj_t* mic_icon_ = nullptr;

    // Bottom Status Footer Label
    lv_obj_t* status_label_ = nullptr;

    bool is_listening_ = false;
    bool is_speaking_ = false;
    int anim_step_ = 0;
    std::function<void()> on_mic_click_;
};

#endif // AI_TUTOR_SCREEN_H
