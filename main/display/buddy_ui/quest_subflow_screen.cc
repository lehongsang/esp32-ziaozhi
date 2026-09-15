#include "quest_subflow_screen.h"
#include "screen_manager.h"
#include "assets/buddy_assets.h"
#include "buddy_font_helper.h"

#include <esp_log.h>
#include <material_symbols.h>

#define TAG "QuestSubflowScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_noto_sans_basic_16_4);

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
    lv_obj_set_style_pad_top(root_, 18, 0);
    lv_obj_set_style_pad_bottom(root_, 6, 0);
    lv_obj_set_style_pad_left(root_, 6, 0);
    lv_obj_set_style_pad_right(root_, 6, 0);
    lv_obj_clear_flag(root_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(root_, LV_OBJ_FLAG_HIDDEN);
}

void QuestSubflowScreen::StartQuest(const std::string& quest_id, const std::string& quest_title) {
    current_quest_id_ = quest_id;
    current_quest_title_ = quest_title;
    
    // Hide bottom page indicators & disable tileview swipe while inside quest sub-flow
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(false);
    BuddyScreenManager::GetInstance().SetTileviewScrollable(false);

    session_questions_.clear();
    current_q_idx_ = 0;
    is_answering_locked_ = false;

    if (IsReadingQuest()) {
        // English Reading Quest: Sample 1st story & its comprehension questions
        current_reading_story_idx_ = 1;
        total_reading_stories_ = 2;
        current_story_ = StoryRepository::GetInstance().GetRandomStory();
        current_story_page_ = 0;
        for (size_t qi = 0; qi < current_story_.questions.size(); ++qi) {
            const auto& sq = current_story_.questions[qi];
            session_questions_.push_back({"sq_" + std::to_string(qi), sq.question, sq.options, sq.correct_index});
        }
    } else if (IsMovementQuest()) {
        movement_total_seconds_ = 20 * 60;
        movement_remaining_seconds_ = 20 * 60;
        is_movement_running_ = true;
        movement_tip_idx_ = 0;
    } else if (quest_id == "q1" || quest_id == "q_math" || quest_id.find("math") != std::string::npos) {
        // Math Homework Quest: Sample 10 math questions
        session_questions_ = QuizRepository::GetInstance().GetRandomSessionQuestions(10);
    } else {
        // Generic / Habit Quest: Sample 5 questions
        session_questions_ = QuizRepository::GetInstance().GetRandomSessionQuestions(5);
    }

    if (root_) {
        lv_obj_clear_flag(root_, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root_);
    }
    SwitchState(QuestSubflowState::kViewDetails);
}

void QuestSubflowScreen::Close() {
    CleanupMovementTimer();
    // Restore bottom page indicators & tileview swipe
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(true);
    BuddyScreenManager::GetInstance().SetTileviewScrollable(true);

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
    CleanupMovementTimer();
    current_state_ = state;
    if (!root_) return;
    lv_obj_clean(root_);

    // Always enforce indicators hidden and swipe locked while inside subflow
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(false);
    BuddyScreenManager::GetInstance().SetTileviewScrollable(false);

    // 28px top padding ensures all subflow content is placed safely below the top WiFi/Battery status bar
    lv_obj_set_style_pad_top(root_, (state == QuestSubflowState::kMovementTimer) ? 16 : 28, 0);

    switch (state) {
        case QuestSubflowState::kViewDetails:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderViewDetails();
            break;
        case QuestSubflowState::kReadStory:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderReadStory();
            break;
        case QuestSubflowState::kDoQuiz:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderDoQuiz();
            break;
        case QuestSubflowState::kMidCheer:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderMidCheer();
            break;
        case QuestSubflowState::kMovementTimer:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderMovementTimer();
            break;
        case QuestSubflowState::kCompleted:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            RenderCompleted();
            break;
        case QuestSubflowState::kReward:
            lv_obj_set_flex_align(root_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
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
    lv_obj_set_size(top_bar, lv_pct(96), 28);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_back = lv_btn_create(top_bar);
    lv_obj_remove_style_all(btn_back);
    lv_obj_set_size(btn_back, 46, 28);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E2738), 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x3B82F6), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_add_flag(btn_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn_back, 12);
    lv_obj_add_event_cb(btn_back, OnBackBtnCb, LV_EVENT_CLICKED, this);
    lv_obj_move_foreground(btn_back);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_remove_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(back_icon);

    lv_obj_t* title_lbl = lv_label_create(top_bar);
    lv_label_set_text(title_lbl, "Mission Details");
    lv_obj_set_style_text_font(title_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(title_lbl, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* dummy_spacer = lv_obj_create(top_bar);
    lv_obj_remove_style_all(dummy_spacer);
    lv_obj_set_size(dummy_spacer, 46, 28);

    // 2. Central Icon Badge
    lv_obj_t* badge = lv_obj_create(root_);
    lv_obj_set_size(badge, 32, 32);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    
    lv_color_t badge_color = lv_color_hex(0x22C55E); // Green default
    const char* badge_icon_str = MATERIAL_SYMBOLS_EDIT_SQUARE;

    if (IsReadingQuest()) {
        badge_color = lv_color_hex(0x06D6A0);
        badge_icon_str = MATERIAL_SYMBOLS_SCHEDULE;
    } else if (current_quest_id_.find("move") != std::string::npos) {
        badge_color = lv_color_hex(0x3A86FF);
        badge_icon_str = MATERIAL_SYMBOLS_SPORTS_ESPORTS;
    } else {
        badge_color = lv_color_hex(0xFFB703);
        badge_icon_str = MATERIAL_SYMBOLS_PERSON;
    }
    lv_obj_set_style_bg_color(badge, badge_color, 0);
    lv_obj_set_style_border_width(badge, 0, 0);
    lv_obj_set_style_margin_top(badge, 3, 0);

    lv_obj_t* badge_icon = lv_label_create(badge);
    lv_label_set_text(badge_icon, badge_icon_str);
    lv_obj_set_style_text_font(badge_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(badge_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(badge_icon);

    // 3. Quest Title & Sub-info
    lv_obj_t* name_lbl = lv_label_create(root_);
    lv_label_set_text(name_lbl, current_quest_title_.c_str());
    lv_obj_set_style_text_color(name_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(name_lbl, 2, 0);

    lv_obj_t* desc_lbl = lv_label_create(root_);
    if (IsReadingQuest()) {
        std::string story_desc = "Story: " + current_story_.title;
        lv_label_set_text(desc_lbl, story_desc.c_str());
    } else if (current_quest_id_.find("math") != std::string::npos || current_quest_id_ == "q1") {
        lv_label_set_text(desc_lbl, "Solve 10 fun math exercises");
    } else if (IsMovementQuest()) {
        lv_label_set_text(desc_lbl, "Daily 20-min active workout & stretch");
    } else {
        lv_label_set_text(desc_lbl, "Daily task assigned by parents via app");
    }
    lv_obj_set_style_text_font(desc_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(desc_lbl, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_margin_top(desc_lbl, 1, 0);

    // 4. Progress Text
    lv_obj_t* prog_lbl = lv_label_create(root_);
    if (IsReadingQuest()) {
        lv_label_set_text(prog_lbl, "Goal: 2 Stories + Comprehension Quizzes");
    } else if (current_quest_id_.find("math") != std::string::npos || current_quest_id_ == "q1") {
        lv_label_set_text(prog_lbl, "Progress: 0 / 10");
    } else if (IsMovementQuest()) {
        lv_label_set_text(prog_lbl, "Goal: 20 Minutes Exercise");
    } else {
        lv_label_set_text(prog_lbl, "Goal: Complete Parent's Mission");
    }
    lv_obj_set_style_text_font(prog_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(prog_lbl, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_margin_top(prog_lbl, 2, 0);

    // 5. Start Now Button (Elevated, completely clear from bottom)
    lv_obj_t* btn_start = lv_btn_create(root_);
    lv_obj_set_size(btn_start, 140, 30);
    lv_obj_set_style_radius(btn_start, 15, 0);
    lv_obj_set_style_bg_color(btn_start, badge_color, 0);
    lv_obj_set_style_margin_top(btn_start, 6, 0);
    lv_obj_add_event_cb(btn_start, OnStartBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_start);
    lv_label_set_text(btn_txt, IsReadingQuest() ? "Read story  ▶" : (IsMovementQuest() ? "Start Exercise  ▶" : "Start now  ▶"));
    lv_obj_set_style_text_font(btn_txt, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

void QuestSubflowScreen::RenderReadStory() {
    if (current_story_.pages.empty()) {
        SwitchState(QuestSubflowState::kDoQuiz);
        return;
    }

    // 1. Top HUD Bar with Back Button on Left and Centered Title (Below status bar)
    lv_obj_t* top_row = lv_obj_create(root_);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_size(top_row, lv_pct(96), 28);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_back = lv_btn_create(top_row);
    lv_obj_remove_style_all(btn_back);
    lv_obj_set_size(btn_back, 46, 28);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E2738), 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x3B82F6), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_add_flag(btn_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn_back, 12);
    lv_obj_add_event_cb(btn_back, OnBackBtnCb, LV_EVENT_CLICKED, this);
    lv_obj_move_foreground(btn_back);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_remove_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(back_icon);

    lv_obj_t* story_hud_lbl = lv_label_create(top_row);
    char hud_buf[48];
    snprintf(hud_buf, sizeof(hud_buf), "Story (%u/%u)", (unsigned)current_reading_story_idx_, (unsigned)total_reading_stories_);
    lv_label_set_text(story_hud_lbl, hud_buf);
    lv_obj_set_style_text_font(story_hud_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(story_hud_lbl, lv_color_hex(0xFFD166), 0);
    lv_obj_set_flex_grow(story_hud_lbl, 1);
    lv_obj_set_style_text_align(story_hud_lbl, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t* dummy_spacer = lv_obj_create(top_row);
    lv_obj_remove_style_all(dummy_spacer);
    lv_obj_set_size(dummy_spacer, 46, 28);

    // 2. Scrollable Story Container Box
    lv_obj_t* scroll_card = lv_obj_create(root_);
    lv_obj_remove_style_all(scroll_card);
    lv_obj_set_size(scroll_card, 300, 168);
    lv_obj_set_style_radius(scroll_card, 12, 0);
    lv_obj_set_style_bg_color(scroll_card, lv_color_hex(0x131A2A), 0);
    lv_obj_set_style_bg_opa(scroll_card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(scroll_card, lv_color_hex(0x27354D), 0);
    lv_obj_set_style_border_width(scroll_card, 1, 0);
    lv_obj_set_style_pad_hor(scroll_card, 10, 0);
    lv_obj_set_style_pad_top(scroll_card, 8, 0);
    lv_obj_set_style_pad_bottom(scroll_card, 12, 0);
    lv_obj_set_style_margin_top(scroll_card, 4, 0);
    lv_obj_set_flex_flow(scroll_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(scroll_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(scroll_card, LV_SCROLLBAR_MODE_AUTO);

    // Title inside card (Full width available)
    lv_obj_t* card_title = lv_label_create(scroll_card);
    lv_label_set_long_mode(card_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(card_title, 276);
    std::string title_with_icon = "📖 " + current_story_.title;
    lv_label_set_text(card_title, title_with_icon.c_str());
    lv_obj_set_style_text_font(card_title, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(card_title, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_style_text_align(card_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_bottom(card_title, 6, 0);

    // Combine all pages into one continuous story text
    std::string full_story;
    for (size_t i = 0; i < current_story_.pages.size(); ++i) {
        if (i > 0) full_story += "\n\n";
        full_story += current_story_.pages[i];
    }

    lv_obj_t* story_text = lv_label_create(scroll_card);
    lv_label_set_long_mode(story_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(story_text, 276);
    lv_label_set_text(story_text, full_story.c_str());
    lv_obj_set_style_text_font(story_text, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(story_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_line_space(story_text, 3, 0);

    // 3. Take Quiz Button (Located at the bottom of the scroll container)
    lv_obj_t* btn_quiz = lv_btn_create(scroll_card);
    lv_obj_set_size(btn_quiz, 170, 30);
    lv_obj_set_style_radius(btn_quiz, 15, 0);
    lv_obj_set_style_bg_color(btn_quiz, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_margin_top(btn_quiz, 12, 0);
    lv_obj_set_style_margin_bottom(btn_quiz, 4, 0);
    lv_obj_add_event_cb(btn_quiz, OnStoryNextBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_quiz);
    lv_label_set_text(btn_txt, "Take Quiz (3 Qs)  ✍️");
    lv_obj_set_style_text_font(btn_txt, &font_noto_sans_basic_16_4, 0);
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

    // 1. Top HUD Bar with Back Button on Left and Centered Question Counter
    lv_obj_t* top_row = lv_obj_create(root_);
    lv_obj_remove_style_all(top_row);
    lv_obj_set_size(top_row, lv_pct(96), 34);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_back = lv_btn_create(top_row);
    lv_obj_remove_style_all(btn_back);
    lv_obj_set_size(btn_back, 52, 34);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E2738), 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x3B82F6), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_add_flag(btn_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn_back, 12);
    lv_obj_add_event_cb(btn_back, OnBackBtnCb, LV_EVENT_CLICKED, this);
    lv_obj_move_foreground(btn_back);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_remove_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(back_icon);

    lv_obj_t* counter = lv_label_create(top_row);
    char counter_buf[64];
    if (IsReadingQuest()) {
        snprintf(counter_buf, sizeof(counter_buf), "Story %u/%u • Q %u/%u", (unsigned)current_reading_story_idx_, (unsigned)total_reading_stories_, (unsigned)(current_q_idx_ + 1), (unsigned)session_questions_.size());
    } else {
        snprintf(counter_buf, sizeof(counter_buf), "Question %u / %u", (unsigned)(current_q_idx_ + 1), (unsigned)session_questions_.size());
    }
    lv_label_set_text(counter, counter_buf);
    lv_obj_set_style_text_font(counter, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(counter, lv_color_hex(0x22C55E), 0);
    lv_obj_set_flex_grow(counter, 1);
    lv_obj_set_style_text_align(counter, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t* dummy_spacer = lv_obj_create(top_row);
    lv_obj_remove_style_all(dummy_spacer);
    lv_obj_set_size(dummy_spacer, 52, 34);

    // 2. Question Text Box
    lv_obj_t* q_box = lv_obj_create(root_);
    lv_obj_remove_style_all(q_box);
    lv_obj_set_size(q_box, lv_pct(94), 36);
    lv_obj_set_style_margin_top(q_box, 4, 0);
    lv_obj_set_flex_flow(q_box, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(q_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* q_text = lv_label_create(q_box);
    lv_label_set_text(q_text, q.question.c_str());
    lv_obj_set_style_text_font(q_text, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(q_text, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(q_text, LV_TEXT_ALIGN_CENTER, 0);

    // 3. 2x2 Answer Grid
    lv_obj_t* grid_container = lv_obj_create(root_);
    lv_obj_remove_style_all(grid_container);
    lv_obj_set_size(grid_container, 284, 96);
    lv_obj_set_flex_flow(grid_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(grid_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(grid_container, 6, 0);
    lv_obj_set_style_margin_top(grid_container, 6, 0);

    for (int row = 0; row < 2; ++row) {
        lv_obj_t* row_box = lv_obj_create(grid_container);
        lv_obj_remove_style_all(row_box);
        lv_obj_set_size(row_box, 284, 44);
        lv_obj_set_flex_flow(row_box, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        for (int col = 0; col < 2; ++col) {
            int opt_idx = row * 2 + col;
            if (opt_idx >= (int)q.options.size()) continue;

            lv_obj_t* btn = lv_btn_create(row_box);
            lv_obj_set_size(btn, 136, 42);
            lv_obj_set_style_radius(btn, 10, 0);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E2738), 0);
            lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x2E384D), 0);
            lv_obj_set_style_border_width(btn, 1, 0);

            // Pass option index in user data
            lv_obj_set_user_data(btn, (void*)(uintptr_t)opt_idx);
            lv_obj_add_event_cb(btn, OnOptionBtnCb, LV_EVENT_CLICKED, this);

            lv_obj_t* opt_lbl = lv_label_create(btn);
            lv_label_set_text(opt_lbl, q.options[opt_idx].c_str());
            lv_obj_set_style_text_font(opt_lbl, &font_noto_sans_basic_16_4, 0);
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
    
    if (IsReadingQuest()) {
        if (current_q_idx_ >= session_questions_.size()) {
            if (current_reading_story_idx_ < total_reading_stories_) {
                // Completed Story 1 -> Show Mid-Story Cheer screen to proceed to Story 2
                SwitchState(QuestSubflowState::kMidCheer);
            } else {
                // Completed all stories -> Completed
                SwitchState(QuestSubflowState::kCompleted);
            }
        } else {
            SwitchState(QuestSubflowState::kDoQuiz);
        }
    } else {
        // Math / Generic: Check if halfway milestone reached (5 of 10)
        if (session_questions_.size() == 10 && current_q_idx_ == 5) {
            SwitchState(QuestSubflowState::kMidCheer);
        } else if (current_q_idx_ >= session_questions_.size()) {
            SwitchState(QuestSubflowState::kCompleted);
        } else {
            SwitchState(QuestSubflowState::kDoQuiz);
        }
    }
}

void QuestSubflowScreen::RenderMidCheer() {
    // 1. Header
    lv_obj_t* top_lbl = lv_label_create(root_);
    if (IsReadingQuest()) {
        char buf[48];
        snprintf(buf, sizeof(buf), "🌟 Story %u/%u Done!", (unsigned)current_reading_story_idx_, (unsigned)total_reading_stories_);
        lv_label_set_text(top_lbl, buf);
    } else {
        lv_label_set_text(top_lbl, "🌟 Halfway There! (5/10)");
    }
    lv_obj_set_style_text_color(top_lbl, lv_color_hex(0x22C55E), 0);

    lv_obj_t* cheer_lbl = lv_label_create(root_);
    if (IsReadingQuest()) {
        lv_label_set_text(cheer_lbl, "Great reading! Ready for Story 2? ✨");
    } else {
        lv_label_set_text(cheer_lbl, "Great job! Keep it up! ✨");
    }
    lv_obj_set_style_text_color(cheer_lbl, lv_color_hex(0xFFD166), 0);
    lv_obj_set_style_margin_top(cheer_lbl, 1, 0);

    // 2. 3D Cheering Piggy AI Image (100x100 zoomed close-up with seamless background)
    lv_obj_t* img = lv_image_create(root_);
    lv_image_set_src(img, &buddy_piggy_cheer);
    lv_obj_set_size(img, 100, 100);
    lv_obj_set_style_margin_top(img, 2, 0);

    // 3. Continue Button (Ample 190px width, elevated safely above bottom)
    lv_obj_t* btn_cont = lv_btn_create(root_);
    lv_obj_set_size(btn_cont, 190, 32);
    lv_obj_set_style_radius(btn_cont, 16, 0);
    lv_obj_set_style_bg_color(btn_cont, lv_color_hex(0x3B82F6), 0);
    lv_obj_set_style_margin_top(btn_cont, 4, 0);
    lv_obj_add_event_cb(btn_cont, OnContinueCheerBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_cont);
    if (IsReadingQuest()) {
        lv_label_set_text(btn_txt, "Read Story 2  ▶");
    } else {
        lv_label_set_text(btn_txt, "Continue (6/10) ▶");
    }
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

void QuestSubflowScreen::RenderCompleted() {
    // 1. Title
    lv_obj_t* top_lbl = lv_label_create(root_);
    lv_label_set_text(top_lbl, "Quest Completed! 🎉");
    lv_obj_set_style_text_color(top_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_top(top_lbl, 2, 0);

    // 2. Big Green Victory Circle
    lv_obj_t* check_circle = lv_obj_create(root_);
    lv_obj_set_size(check_circle, 38, 38);
    lv_obj_set_style_radius(check_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(check_circle, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_color(check_circle, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_border_width(check_circle, 2, 0);
    lv_obj_set_style_margin_top(check_circle, 4, 0);

    lv_obj_t* check_icon = lv_label_create(check_circle);
    lv_label_set_text(check_icon, MATERIAL_SYMBOLS_CHECK);
    lv_obj_set_style_text_font(check_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(check_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(check_icon);

    // 3. Victory Text
    lv_obj_t* score_lbl = lv_label_create(root_);
    char buf[64];
    if (IsReadingQuest()) {
        snprintf(buf, sizeof(buf), "2/2 Stories completed - Super Reader! 📚");
    } else if (IsMovementQuest()) {
        snprintf(buf, sizeof(buf), "20 min done - Super energetic! 🔥");
    } else {
        snprintf(buf, sizeof(buf), "%u / %u - Amazing work!", (unsigned)session_questions_.size(), (unsigned)session_questions_.size());
    }
    lv_label_set_text(score_lbl, buf);
    lv_obj_set_style_text_color(score_lbl, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_margin_top(score_lbl, 3, 0);

    // 4. Claim Reward Button
    lv_obj_t* btn_claim = lv_btn_create(root_);
    lv_obj_set_size(btn_claim, 150, 30);
    lv_obj_set_style_radius(btn_claim, 15, 0);
    lv_obj_set_style_bg_color(btn_claim, lv_color_hex(0xFFB703), 0);
    lv_obj_set_style_margin_top(btn_claim, 6, 0);
    lv_obj_add_event_cb(btn_claim, OnClaimRewardBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* btn_txt = lv_label_create(btn_claim);
    lv_label_set_text(btn_txt, "Get Reward 🎁");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0x0F172A), 0);
    lv_obj_center(btn_txt);
}

static const char* kExerciseTips[] = {
    "Jumping Jacks 🏃",
    "Stretch & Reach 🧘",
    "High Knees Run 🤸",
    "Dance to the Beat 💃",
    "Squats & Lunges 🦵",
    "Deep Breaths & Relax 🌬️"
};
static const size_t kExerciseTipsCount = sizeof(kExerciseTips) / sizeof(kExerciseTips[0]);

void QuestSubflowScreen::RenderMovementTimer() {
    // 1. Top HUD Bar with Back Button & Title
    lv_obj_t* top_bar = lv_obj_create(root_);
    lv_obj_remove_style_all(top_bar);
    lv_obj_set_size(top_bar, lv_pct(96), 28);
    lv_obj_set_flex_flow(top_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* btn_back = lv_btn_create(top_bar);
    lv_obj_remove_style_all(btn_back);
    lv_obj_set_size(btn_back, 46, 28);
    lv_obj_set_style_radius(btn_back, 8, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E2738), 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x3B82F6), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(btn_back, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(btn_back, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_add_flag(btn_back, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(btn_back, 12);
    lv_obj_add_event_cb(btn_back, OnBackBtnCb, LV_EVENT_CLICKED, this);
    lv_obj_move_foreground(btn_back);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_remove_flag(back_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_center(back_icon);

    lv_obj_t* title_lbl = lv_label_create(top_bar);
    lv_label_set_text(title_lbl, "Do Exercise");
    lv_obj_set_style_text_font(title_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(title_lbl, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* dummy_spacer = lv_obj_create(top_bar);
    lv_obj_remove_style_all(dummy_spacer);
    lv_obj_set_size(dummy_spacer, 46, 28);

    // 2. Circular Arc & Timer Center (90x90)
    lv_obj_t* timer_wrapper = lv_obj_create(root_);
    lv_obj_remove_style_all(timer_wrapper);
    lv_obj_set_size(timer_wrapper, 92, 92);
    lv_obj_set_style_margin_top(timer_wrapper, 2, 0);

    movement_arc_ = lv_arc_create(timer_wrapper);
    lv_obj_set_size(movement_arc_, 90, 90);
    lv_arc_set_rotation(movement_arc_, 270);
    lv_arc_set_bg_angles(movement_arc_, 0, 360);
    lv_arc_set_range(movement_arc_, 0, movement_total_seconds_);
    lv_obj_remove_style(movement_arc_, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(movement_arc_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(movement_arc_, 7, LV_PART_MAIN);
    lv_obj_set_style_arc_color(movement_arc_, lv_color_hex(0x1E2738), LV_PART_MAIN);
    lv_obj_set_style_arc_width(movement_arc_, 7, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(movement_arc_, lv_color_hex(0x3A86FF), LV_PART_INDICATOR);
    lv_obj_center(movement_arc_);

    movement_timer_lbl_ = lv_label_create(timer_wrapper);
    lv_obj_set_style_text_font(movement_timer_lbl_, GetBuddyFont(), 0);
    lv_obj_set_style_text_color(movement_timer_lbl_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(movement_timer_lbl_);

    // 3. Current Exercise Tip Card (Compact height 26px)
    lv_obj_t* tip_card = lv_obj_create(root_);
    lv_obj_remove_style_all(tip_card);
    lv_obj_set_size(tip_card, lv_pct(92), 26);
    lv_obj_set_style_bg_color(tip_card, lv_color_hex(0x161C28), 0);
    lv_obj_set_style_bg_opa(tip_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(tip_card, 8, 0);
    lv_obj_set_style_border_width(tip_card, 1, 0);
    lv_obj_set_style_border_color(tip_card, lv_color_hex(0x2A354A), 0);
    lv_obj_set_flex_flow(tip_card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tip_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_top(tip_card, 4, 0);

    movement_tip_lbl_ = lv_label_create(tip_card);
    lv_obj_set_style_text_font(movement_tip_lbl_, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(movement_tip_lbl_, lv_color_hex(0x38BDF8), 0);
    lv_label_set_text(movement_tip_lbl_, kExerciseTips[movement_tip_idx_ % kExerciseTipsCount]);

    // 4. Control Buttons (Play/Pause + Finish) (Compact height 30px)
    lv_obj_t* ctrl_row = lv_obj_create(root_);
    lv_obj_remove_style_all(ctrl_row);
    lv_obj_set_size(ctrl_row, lv_pct(94), 30);
    lv_obj_set_flex_flow(ctrl_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(ctrl_row, 12, 0);
    lv_obj_set_style_margin_top(ctrl_row, 4, 0);

    // Play/Pause Button
    lv_obj_t* btn_play_pause = lv_btn_create(ctrl_row);
    lv_obj_remove_style_all(btn_play_pause);
    lv_obj_set_size(btn_play_pause, 50, 30);
    lv_obj_set_style_radius(btn_play_pause, 15, 0);
    lv_obj_set_style_bg_color(btn_play_pause, lv_color_hex(0x3A86FF), 0);
    lv_obj_set_style_bg_opa(btn_play_pause, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(btn_play_pause, OnMovementPlayPauseBtnCb, LV_EVENT_CLICKED, this);

    movement_play_pause_icon_ = lv_label_create(btn_play_pause);
    lv_label_set_text(movement_play_pause_icon_, is_movement_running_ ? MATERIAL_SYMBOLS_PAUSE : MATERIAL_SYMBOLS_PLAY_ARROW);
    lv_obj_set_style_text_font(movement_play_pause_icon_, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(movement_play_pause_icon_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(movement_play_pause_icon_);

    // Finish Button
    lv_obj_t* btn_finish = lv_btn_create(ctrl_row);
    lv_obj_remove_style_all(btn_finish);
    lv_obj_set_size(btn_finish, 100, 30);
    lv_obj_set_style_radius(btn_finish, 15, 0);
    lv_obj_set_style_bg_color(btn_finish, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_bg_opa(btn_finish, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(btn_finish, OnMovementFinishBtnCb, LV_EVENT_CLICKED, this);

    lv_obj_t* finish_lbl = lv_label_create(btn_finish);
    lv_label_set_text(finish_lbl, "Finish  ⭐");
    lv_obj_set_style_text_font(finish_lbl, &font_noto_sans_basic_16_4, 0);
    lv_obj_set_style_text_color(finish_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(finish_lbl);

    UpdateMovementTimerDisplay();

    // Start 1s tick timer
    movement_timer_ = lv_timer_create(OnMovementTimerTickCb, 1000, this);
}

void QuestSubflowScreen::UpdateMovementTimerDisplay() {
    if (movement_timer_lbl_) {
        uint32_t mins = movement_remaining_seconds_ / 60;
        uint32_t secs = movement_remaining_seconds_ % 60;
        char buf[16];
        snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned int)mins, (unsigned int)secs);
        lv_label_set_text(movement_timer_lbl_, buf);
    }
    if (movement_arc_) {
        uint32_t elapsed = movement_total_seconds_ >= movement_remaining_seconds_ ? (movement_total_seconds_ - movement_remaining_seconds_) : 0;
        lv_arc_set_value(movement_arc_, elapsed);
    }
}

void QuestSubflowScreen::CleanupMovementTimer() {
    if (movement_timer_) {
        lv_timer_delete(movement_timer_);
        movement_timer_ = nullptr;
    }
    movement_timer_lbl_ = nullptr;
    movement_arc_ = nullptr;
    movement_play_pause_icon_ = nullptr;
    movement_tip_lbl_ = nullptr;
}

void QuestSubflowScreen::OnMovementTimerTickCb(lv_timer_t* timer) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_timer_get_user_data(timer));
    if (!self || !self->is_movement_running_) return;

    if (self->movement_remaining_seconds_ > 0) {
        self->movement_remaining_seconds_--;
        self->UpdateMovementTimerDisplay();

        // Rotate exercise tips every 30 seconds
        if ((self->movement_remaining_seconds_ % 30) == 0 && self->movement_tip_lbl_) {
            self->movement_tip_idx_++;
            lv_label_set_text(self->movement_tip_lbl_, kExerciseTips[self->movement_tip_idx_ % kExerciseTipsCount]);
        }
    } else {
        self->CleanupMovementTimer();
        self->SwitchState(QuestSubflowState::kCompleted);
    }
}

void QuestSubflowScreen::OnMovementPlayPauseBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    self->is_movement_running_ = !self->is_movement_running_;
    if (self->movement_play_pause_icon_) {
        lv_label_set_text(self->movement_play_pause_icon_, self->is_movement_running_ ? MATERIAL_SYMBOLS_PAUSE : MATERIAL_SYMBOLS_PLAY_ARROW);
    }
}

void QuestSubflowScreen::OnMovementFinishBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    self->CleanupMovementTimer();
    self->SwitchState(QuestSubflowState::kCompleted);
}

void QuestSubflowScreen::RenderReward() {
    // 1. Title
    lv_obj_t* reward_lbl = lv_label_create(root_);
    lv_label_set_text(reward_lbl, "You earned +2 Corn! 🌽");
    lv_obj_set_style_text_color(reward_lbl, lv_color_hex(0x4ADE80), 0);
    lv_obj_set_style_margin_top(reward_lbl, 2, 0);

    // 2. 3D Cute Piggy with Corn, Apple & Potatoes (100x100 zoomed with seamless background)
    lv_obj_t* img = lv_image_create(root_);
    lv_image_set_src(img, &buddy_reward_corn);
    lv_obj_set_size(img, 100, 100);
    lv_obj_set_style_margin_top(img, 2, 0);

    // 3. Finish & Claim Reward Button (Ample 190px width)
    lv_obj_t* btn_done = lv_btn_create(root_);
    lv_obj_set_size(btn_done, 190, 32);
    lv_obj_set_style_radius(btn_done, 16, 0);
    lv_obj_set_style_bg_color(btn_done, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_margin_top(btn_done, 4, 0);

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
    lv_label_set_text(btn_txt, "Claim Reward  ✨");
    lv_obj_set_style_text_color(btn_txt, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(btn_txt);
}

// Event callbacks
void QuestSubflowScreen::OnStartBtnCb(lv_event_t* e) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_event_get_user_data(e));
    if (self) {
        if (self->IsReadingQuest()) {
            self->SwitchState(QuestSubflowState::kReadStory);
        } else if (self->IsMovementQuest()) {
            self->SwitchState(QuestSubflowState::kMovementTimer);
        } else {
            self->SwitchState(QuestSubflowState::kDoQuiz);
        }
    }
}

void QuestSubflowScreen::OnStoryNextBtnCb(lv_event_t* e) {
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
        if (self->IsReadingQuest() && self->current_reading_story_idx_ < self->total_reading_stories_) {
            self->current_reading_story_idx_++;
            self->current_story_ = StoryRepository::GetInstance().GetRandomStoryExcluding(self->current_story_.id);
            self->current_story_page_ = 0;
            self->session_questions_.clear();
            for (size_t qi = 0; qi < self->current_story_.questions.size(); ++qi) {
                const auto& sq = self->current_story_.questions[qi];
                self->session_questions_.push_back({"sq_" + std::to_string(qi), sq.question, sq.options, sq.correct_index});
            }
            self->current_q_idx_ = 0;
            self->SwitchState(QuestSubflowState::kReadStory);
        } else {
            self->SwitchState(QuestSubflowState::kDoQuiz);
        }
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
        ESP_LOGI(TAG, "Back button pressed, current state: %d", (int)self->current_state_);
        self->CleanupMovementTimer();
        if (self->current_state_ == QuestSubflowState::kReadStory || self->current_state_ == QuestSubflowState::kMovementTimer) {
            self->SwitchState(QuestSubflowState::kViewDetails);
        } else if (self->current_state_ == QuestSubflowState::kDoQuiz) {
            if (self->IsReadingQuest()) {
                self->SwitchState(QuestSubflowState::kReadStory);
            } else {
                self->SwitchState(QuestSubflowState::kViewDetails);
            }
        } else {
            self->Close();
        }
    }
}

void QuestSubflowScreen::OnNextQuestionTimerCb(lv_timer_t* timer) {
    auto* self = static_cast<QuestSubflowScreen*>(lv_timer_get_user_data(timer));
    lv_timer_delete(timer);
    if (self) {
        self->AdvanceNextQuestion();
    }
}

