#include "quest_subflow_screen.h"
#include "assets/buddy_assets.h"

#include <esp_log.h>
#include <material_symbols.h>

#define TAG "QuestSubflowScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

QuestSubflowScreen::QuestSubflowScreen() {}
QuestSubflowScreen::~QuestSubflowScreen() {}

void QuestSubflowScreen::Create(lv_obj_t* parent) {
    root_ = lv_obj_create(parent);
    lv_obj_remove_style_all(root_);
    lv_obj_set_size(root_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(root_, lv_color_hex(0x0C101A), 0);
    lv_obj_set_style_bg_opa(root_, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void QuestSubflowScreen::StartQuest(const std::string& quest_id, const std::string& quest_title) {
    current_quest_id_ = quest_id;
    current_quest_title_ = quest_title;
    
    // Sample 10 randomized questions from repository
    session_questions_ = QuizRepository::GetInstance().GetRandomSessionQuestions(10);
    current_q_idx_ = 0;
    is_answering_locked_ = false;

    if (root_) {
        lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root_);
    }
    SwitchState(QuestSubflowState::kViewDetails);
}

void QuestSubflowScreen::Close() {
    current_state_ = QuestSubflowState::kHidden;
    if (root_) {
        lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clean(root_);
    }
    if (on_close_) {
        on_close_();
    }
}

void QuestSubflowScreen::SwitchState(QuestSubflowState state) {
    current_state_ = state;
    if (!root_) return;
    lv_obj_clean(root_);

    switch (state) {
        case QuestSubflowState::kViewDetails:
            RenderViewDetails();
            break;
        case QuestSubflowState::kDoQuiz:
            RenderDoQuiz();
            break;
        case QuestSubflowState::kMidCheer:
            RenderMidCheer();
            break;
        case QuestSubflowState::kCompleted:
            RenderCompleted();
            break;
        case QuestSubflowState::kReward:
            RenderReward();
            break;
        default:
            break;
    }
}

void QuestSubflowScreen::RenderViewDetails() {
    // 1. Top Bar with Back Button & Title
    lv_obj_t* top_bar = lv_obj_create(root_);
    lv_obj_remove_style_all(top_bar);
    lv_obj_set_size(top_bar, lv_pct(94), 32);
    lv_obj_set_style_margin_top(top_bar, 6, 0);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_back = lv_btn_create(top_bar);
    lv_obj_set_size(btn_back, 30, 30);
    lv_obj_set_style_radius(btn_back, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E2738), 0);
    lv_obj_add_event_cb(btn_back, OnBackBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(back_icon);

    lv_obj_t* title_lbl = lv_label_create(top_bar);
    lv_label_set_text(title_lbl, "Mission Details");
    lv_obj_set_style_text_color(title_lbl, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* dummy_spacer = lv_obj_create(top_bar);
    lv_obj_remove_style_all(dummy_spacer);
    lv_obj_set_size(dummy_spacer, 30, 30);

    // 2. Large Central Icon
    lv_obj_t* badge = lv_obj_create(root_);
    lv_obj_set_size(badge, 52, 52);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(badge, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_width(badge, 0, 0);
    lv_obj_set_style_margin_top(badge, 10, 0);

    lv_obj_t* badge_icon = lv_label_create(badge);
    lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_EDIT_SQUARE);
    lv_obj_set_style_text_font(badge_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(badge_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(badge_icon);

    // 3. Quest Title & Sub-info
    lv_obj_t* name_lbl = lv_label_create(root_);
    lv_label_set_text(name_lbl, current_quest_title_.c_str());
    lv_obj_set_style_text_color(name_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(name_lbl, 8, 0);

    lv_obj_t* desc_lbl = lv_label_create(root_);
    lv_label_set_text(desc_lbl, "Solve 10 fun math exercises");
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_margin_top(desc_lbl, 2, 0);

    // 4. Progress Text
    lv_obj_t* prog_lbl = lv_label_create(root_);
    lv_label_set_text(prog_lbl, "Progress: 0 / 10");
    lv_obj_set_style_text_color(prog_lbl, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_margin_top(prog_lbl, 6, 0);

    // 5. Start Now Button
    lv_obj_t* btn_start = lv_btn_create(root_);
    lv_obj_set_size(btn_start, 180, 42);
    lv_obj_set_style_radius(btn_start, 21, 0);
    lv_obj_set_style_bg_color(btn_start, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_margin_top(btn_start, 14, 0);
    lv_obj_add_event_cb(btn_start, OnStartBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_start);
    lv_label_set_text(btn_txt, "Start now  ▶");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

void QuestSubflowScreen::RenderDoQuiz() {
    if (session_questions_.empty() || current_q_idx_ >= session_questions_.size()) {
        SwitchState(QuestSubflowState::kCompleted);
        return;
    }

    const auto& q = session_questions_[current_q_idx_];
    is_answering_locked_ = false;

    // 1. Top HUD: Title + Counter (e.g. "1 / 10")
    lv_obj_t* top_row = lv_obj_create(root_);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_size(top_row, lv_pct(92), 26);
    lv_obj_set_style_margin_top(top_row, 4, 0);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* quest_name = lv_label_create(top_row);
    lv_label_set_text(quest_name, "Math homework");
    lv_obj_set_style_text_color(quest_name, lv_color_hex(0x94A3B8), 0);

    lv_obj_t* counter = lv_label_create(top_row);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d / %d", (int)(current_q_idx_ + 1), (int)session_questions_.size());
    lv_label_set_text(counter, buf);
    lv_obj_set_style_text_color(counter, lv_color_hex(0x22C55E), 0);

    // 2. Question Text Box
    lv_obj_t* q_box = lv_obj_create(root_);
    lv_obj_remove_style_all(q_box);
    lv_obj_set_size(q_box, lv_pct(92), 48);
    lv_obj_set_style_margin_top(q_box, 4, 0);
    lv_obj_set_flex_flow(q_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(q_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* q_text = lv_label_create(q_box);
    lv_label_set_text(q_text, q.question.c_str());
    lv_obj_set_style_text_color(q_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(q_text, LV_TEXT_ALIGN_CENTER, 0);

    // 3. 2x2 Answer Grid
    lv_obj_t* grid_container = lv_obj_create(root_);
    lv_obj_remove_style_all(grid_container);
    lv_obj_set_size(grid_container, 290, 130);
    lv_obj_set_flex_flow(grid_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(grid_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(grid_container, 8, 0);

    for (int row = 0; row < 2; ++row) {
        lv_obj_t* row_box = lv_obj_create(grid_container);
        lv_obj_remove_style_all(row_box);
        lv_obj_set_size(row_box, 290, 56);
        lv_obj_set_flex_flow(row_box, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        for (int col = 0; col < 2; ++col) {
            int opt_idx = row * 2 + col;
            if (opt_idx >= (int)q.options.size()) continue;

            lv_obj_t* btn = lv_btn_create(row_box);
            lv_obj_set_size(btn, 138, 52);
            lv_obj_set_style_radius(btn, 14, 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E2738), 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x2E384D), 0);
            lv_obj_set_style_border_width(btn, 1, 0);

            // Pass self & option index in user data
            lv_obj_set_user_data(btn, (void*)(uintptr_t)opt_idx);
            lv_obj_add_event_cb(btn, OnOptionBtnCb, LV_EVENT_CLICKED, this);

            lv_obj_t* opt_lbl = lv_label_create(btn);
            lv_label_set_text(opt_lbl, q.options[opt_idx].c_str());
            lv_obj_set_style_text_color(opt_lbl, lv_color_hex(0xFFFFFF), 0);
            lv_obj_center(opt_lbl);
        }
    }
}

void QuestSubflowScreen::HandleOptionSelected(int selected_index, lv_obj_t* btn) {
    if (is_answering_locked_ || session_questions_.empty() || current_q_idx_ >= session_questions_.size()) {
        return;
    }

    const auto& q = session_questions_[current_q_idx_];

    if (selected_index == q.correct_index) {
        // Correct answer! Highlight green
        is_answering_locked_ = true;
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x22C55E), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x4ADE80), 0);

        // Schedule next question after 350ms
        lv_timer_create(OnNextQuestionTimerCb, 350, this);
    } else {
        // Wrong answer! Highlight red feedback
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xEF4444), 0);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xF87171), 0);
    }
}

void QuestSubflowScreen::AdvanceNextQuestion() {
    current_q_idx_++;
    
    // Check if halfway milestone reached (5 of 10)
    if (current_q_idx_ == 5) {
        SwitchState(QuestSubflowState::kMidCheer);
    } else if (current_q_idx_ >= session_questions_.size()) {
        SwitchState(QuestSubflowState::kCompleted);
    } else {
        SwitchState(QuestSubflowState::kDoQuiz);
    }
}

void QuestSubflowScreen::RenderMidCheer() {
    // 1. Header
    lv_obj_t* top_lbl = lv_label_create(root_);
    lv_label_set_text(top_lbl, "5 / 10");
    lv_obj_set_style_text_color(top_lbl, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_margin_top(top_lbl, 4, 0);

    lv_obj_t* cheer_lbl = lv_label_create(root_);
    lv_label_set_text(cheer_lbl, "Great job! Keep it up! 🌟");
    lv_obj_set_style_text_color(cheer_lbl, lv_color_hex(0xFFD166), 0);
    lv_obj_set_style_margin_top(cheer_lbl, 2, 0);

    // 2. 3D Cheering Piggy AI Image (130x130)
    lv_obj_t* img = lv_image_create(root_);
    lv_image_set_src(img, &buddy_piggy_cheer);
    lv_obj_set_size(img, 130, 130);
    lv_obj_set_style_margin_top(img, 4, 0);

    // 3. Continue Button
    lv_obj_t* btn_cont = lv_btn_create(root_);
    lv_obj_set_size(btn_cont, 180, 38);
    lv_obj_set_style_radius(btn_cont, 19, 0);
    lv_obj_set_style_bg_color(btn_cont, lv_color_hex(0x3B82F6), 0);
    lv_obj_set_style_margin_top(btn_cont, 6, 0);
    lv_obj_add_event_cb(btn_cont, OnContinueCheerBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_cont);
    lv_label_set_text(btn_txt, "Continue (6/10) ▶");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

void QuestSubflowScreen::RenderCompleted() {
    // 1. Title
    lv_obj_t* top_lbl = lv_label_create(root_);
    lv_label_set_text(top_lbl, "Math homework");
    lv_obj_set_style_text_color(top_lbl, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_margin_top(top_lbl, 8, 0);

    // 2. Big Green Victory Circle
    lv_obj_t* check_circle = lv_obj_create(root_);
    lv_obj_set_size(check_circle, 64, 64);
    lv_obj_set_style_radius(check_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(check_circle, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_color(check_circle, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_border_width(check_circle, 3, 0);
    lv_obj_set_style_margin_top(check_circle, 14, 0);

    lv_obj_t* check_icon = lv_label_create(check_circle);
    lv_label_set_text(check_icon, MATERIAL_SYMBOLS_CHECK);
    lv_obj_set_style_text_font(check_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(check_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(check_icon);

    // 3. Victory Text
    lv_obj_t* comp_lbl = lv_label_create(root_);
    lv_label_set_text(comp_lbl, "Completed!");
    lv_obj_set_style_text_color(comp_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(comp_lbl, 10, 0);

    lv_obj_t* score_lbl = lv_label_create(root_);
    lv_label_set_text(score_lbl, "10 / 10 - You did amazing! ✨");
    lv_obj_set_style_text_color(score_lbl, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_margin_top(score_lbl, 4, 0);

    // 4. Claim Reward Button
    lv_obj_t* btn_claim = lv_btn_create(root_);
    lv_obj_set_size(btn_claim, 180, 40);
    lv_obj_set_style_radius(btn_claim, 20, 0);
    lv_obj_set_style_bg_color(btn_claim, lv_color_hex(0xFFB703), 0);
    lv_obj_set_style_margin_top(btn_claim, 16, 0);
    lv_obj_add_event_cb(btn_claim, OnClaimRewardBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_claim);
    lv_label_set_text(btn_txt, "Get Reward 🎁");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0x0F172A), 0);
    lv_obj_center(btn_txt);
}

void QuestSubflowScreen::RenderReward() {
    // 1. Title
    lv_obj_t* top_lbl = lv_label_create(root_);
    lv_label_set_text(top_lbl, "You earned");
    lv_obj_set_style_text_color(top_lbl, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_margin_top(top_lbl, 4, 0);

    lv_obj_t* reward_lbl = lv_label_create(root_);
    lv_label_set_text(reward_lbl, "+2 Corn! 🌽");
    lv_obj_set_style_text_color(reward_lbl, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_margin_top(reward_lbl, 2, 0);

    // 2. 3D Golden Sweet Corn AI Image (130x130)
    lv_obj_t* img = lv_image_create(root_);
    lv_image_set_src(img, &buddy_reward_corn);
    lv_obj_set_size(img, 130, 130);
    lv_obj_set_style_margin_top(img, 4, 0);

    // 3. Finish & Feed Piggy Button
    lv_obj_t* btn_done = lv_btn_create(root_);
    lv_obj_set_size(btn_done, 190, 38);
    lv_obj_set_style_radius(btn_done, 19, 0);
    lv_obj_set_style_bg_color(btn_done, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_margin_top(btn_done, 6, 0);

    lv_obj_add_event_cb(btn_done, [](lv_event_t* e) {
        auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
        if (self) {
            if (self->on_completed_) {
                self->on_completed_(self->current_quest_id_);
            }
            self->Close();
        }
    }, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_done);
    lv_label_set_text(btn_txt, "Claim & Back to Quest ✨");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

// Event callbacks
void QuestSubflowScreen::OnStartBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->SwitchState(QuestSubflowState::kDoQuiz);
    }
}

void QuestSubflowScreen::OnOptionBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    lv_obj_t* target = (lv_obj_t*)lv_event_get_target(e);
    if (self && target) {
        int opt_idx = (int)(uintptr_t)lv_obj_get_user_data(target);
        self->HandleOptionSelected(opt_idx, target);
    }
}

void QuestSubflowScreen::OnContinueCheerBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->SwitchState(QuestSubflowState::kDoQuiz);
    }
}

void QuestSubflowScreen::OnClaimRewardBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->SwitchState(QuestSubflowState::kReward);
    }
}

void QuestSubflowScreen::OnBackBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (self) {
        self->Close();
    }
}

void QuestSubflowScreen::OnNextQuestionTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_timer_get_user_data(timer));
    lv_timer_delete(timer);
    if (self) {
        self->AdvanceNextQuestion();
    }
}
