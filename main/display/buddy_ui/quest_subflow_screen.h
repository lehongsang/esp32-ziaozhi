#pragma once

#include <string>
#include <vector>
#include <functional>
#include <lvgl.h>

#include "quiz_repository.h"

enum class QuestSubflowState {
    kHidden,
    kViewDetails,
    kDoQuiz,
    kMidCheer,
    kCompleted,
    kReward
};

class QuestSubflowScreen {
public:
    QuestSubflowScreen();
    ~QuestSubflowScreen();

    void Create(lv_obj_t* parent);
    void StartQuest(const std::string& quest_id, const std::string& quest_title);
    void Close();

    void SetOnCompleted(std::function<void(const std::string& quest_id)> cb) { on_completed_ = cb; }
    void SetOnClose(std::function<void()> cb) { on_close_ = cb; }

    bool IsActive() const { return current_state_ != QuestSubflowState::kHidden; }

private:
    lv_obj_t* root_ = nullptr;
    QuestSubflowState current_state_ = QuestSubflowState::kHidden;
    std::string current_quest_id_;
    std::string current_quest_title_;

    // Quiz Session Data
    std::vector<QuizQuestion> session_questions_;
    size_t current_q_idx_ = 0;
    bool is_answering_locked_ = false;

    // Callbacks
    std::function<void(const std::string& quest_id)> on_completed_;
    std::function<void()> on_close_;

    // State Renderers
    void SwitchState(QuestSubflowState state);
    void RenderViewDetails();
    void RenderDoQuiz();
    void RenderMidCheer();
    void RenderCompleted();
    void RenderReward();

    // Event Handlers
    static void OnStartBtnCb(lv_event_t* e);
    static void OnOptionBtnCb(lv_event_t* e);
    static void OnContinueCheerBtnCb(lv_event_t* e);
    static void OnClaimRewardBtnCb(lv_event_t* e);
    static void OnBackBtnCb(lv_event_t* e);
    static void OnNextQuestionTimerCb(lv_timer_t* timer);

    void HandleOptionSelected(int selected_index, lv_obj_t* btn);
    void AdvanceNextQuestion();
};
