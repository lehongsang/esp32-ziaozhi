#pragma once

#include <string>
#include <vector>
#include <functional>
#include <lvgl.h>

#include "quiz_repository.h"
#include "story_repository.h"

enum class QuestSubflowState {
    kHidden,
    kViewDetails,
    kReadStory,
    kDoQuiz,
    kMidCheer,
    kMovementTimer,
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

    // Story Session Data
    StoryItem current_story_;
    size_t current_story_page_ = 0;
    size_t current_reading_story_idx_ = 1;
    size_t total_reading_stories_ = 2;

    // Quiz Session Data
    std::vector<QuizQuestion> session_questions_;
    size_t current_q_idx_ = 0;
    bool is_answering_locked_ = false;

    // Movement Timer Session Data
    uint32_t movement_total_seconds_ = 20 * 60;
    uint32_t movement_remaining_seconds_ = 20 * 60;
    bool is_movement_running_ = false;
    lv_timer_t* movement_timer_ = nullptr;
    lv_obj_t* movement_timer_lbl_ = nullptr;
    lv_obj_t* movement_arc_ = nullptr;
    lv_obj_t* movement_play_pause_icon_ = nullptr;
    lv_obj_t* movement_tip_lbl_ = nullptr;
    size_t movement_tip_idx_ = 0;

    // Callbacks
    std::function<void(const std::string& quest_id)> on_completed_;
    std::function<void()> on_close_;

    // State Renderers
    void SwitchState(QuestSubflowState state);
    void RenderViewDetails();
    void RenderReadStory();
    void RenderDoQuiz();
    void RenderMidCheer();
    void RenderMovementTimer();
    void RenderCompleted();
    void RenderReward();

    // Helpers
    bool IsReadingQuest() const {
        return current_quest_id_ == "q2" || current_quest_id_ == "q_read" || current_quest_id_.find("read") != std::string::npos;
    }
    bool IsMovementQuest() const {
        return current_quest_id_ == "q3" || current_quest_id_ == "q_move" || current_quest_id_.find("move") != std::string::npos;
    }
    void HandleOptionSelected(int selected_index, lv_obj_t* btn);
    void AdvanceNextQuestion();
    void UpdateMovementTimerDisplay();
    void CleanupMovementTimer();

    // Event Handlers
    static void OnStartBtnCb(lv_event_t* e);
    static void OnStoryNextBtnCb(lv_event_t* e);
    static void OnOptionBtnCb(lv_event_t* e);
    static void OnContinueCheerBtnCb(lv_event_t* e);
    static void OnClaimRewardBtnCb(lv_event_t* e);
    static void OnBackBtnCb(lv_event_t* e);
    static void OnNextQuestionTimerCb(lv_timer_t* timer);
    static void OnMovementTimerTickCb(lv_timer_t* timer);
    static void OnMovementPlayPauseBtnCb(lv_event_t* e);
    static void OnMovementFinishBtnCb(lv_event_t* e);
};


