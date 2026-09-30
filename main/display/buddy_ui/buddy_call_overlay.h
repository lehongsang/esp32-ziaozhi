#ifndef BUDDY_CALL_OVERLAY_H
#define BUDDY_CALL_OVERLAY_H

#include <string>
#include <functional>
#include <lvgl.h>

enum class CallOverlayState {
    kIdle,
    kIncoming,
    kOutgoing,
    kActive
};

class BuddyCallOverlay {
public:
    static BuddyCallOverlay& GetInstance();

    void Initialize(lv_obj_t* root_layer);
    void ShowIncomingCall(const std::string& caller_name, std::function<void()> on_accept, std::function<void()> on_reject);
    void ShowOutgoingCall(const std::string& callee_name, std::function<void()> on_cancel);
    void SetCallActive();
    void EndCall(const std::string& reason = "");
    bool IsInCall() const { return state_ != CallOverlayState::kIdle; }

private:
    BuddyCallOverlay();
    ~BuddyCallOverlay();

    static void OnAcceptBtnCb(lv_event_t* e);
    static void OnRejectBtnCb(lv_event_t* e);
    static void OnHangupBtnCb(lv_event_t* e);
    static void WaveformTimerCb(lv_timer_t* timer);
    static void CallDurationTimerCb(lv_timer_t* timer);

    lv_obj_t* backdrop_ = nullptr;
    lv_obj_t* modal_card_ = nullptr;
    lv_obj_t* avatar_circle_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
    lv_obj_t* timer_label_ = nullptr;
    lv_obj_t* waveform_container_ = nullptr;
    std::vector<lv_obj_t*> waveform_bars_;

    lv_obj_t* incoming_actions_ = nullptr;
    lv_obj_t* btn_accept_ = nullptr;
    lv_obj_t* btn_reject_ = nullptr;

    lv_obj_t* active_actions_ = nullptr;
    lv_obj_t* btn_hangup_ = nullptr;

    lv_timer_t* waveform_timer_ = nullptr;
    lv_timer_t* duration_timer_ = nullptr;
    uint32_t call_start_ticks_ = 0;
    int anim_step_ = 0;

    CallOverlayState state_ = CallOverlayState::kIdle;
    std::function<void()> on_accept_cb_ = nullptr;
    std::function<void()> on_reject_cb_ = nullptr;
    std::function<void()> on_hangup_cb_ = nullptr;
};

#endif // BUDDY_CALL_OVERLAY_H
