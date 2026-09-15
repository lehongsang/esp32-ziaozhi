#include "savings_screen.h"
#include "screen_manager.h"
#include "assets/buddy_assets.h"
#include "settings.h"

#include <esp_log.h>
#include <material_symbols.h>
#include <cstdio>
#include <cmath>

#define TAG "SavingsScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

static std::string FormatAmount(int64_t amount, const std::string& unit) {
    std::string s = std::to_string(std::abs(amount));
    int n = s.length();
    std::string formatted = "";
    int count = 0;
    for (int i = n - 1; i >= 0; --i) {
        formatted = s[i] + formatted;
        count++;
        if (count % 3 == 0 && i > 0) {
            formatted = "," + formatted;
        }
    }
    if (amount < 0) formatted = "-" + formatted;
    return formatted + unit;
}

SavingsScreen::SavingsScreen() {
    goal_definitions_ = {
        {GoalType::kBike, "My New Bike", 3000000, &goal_art_bike},
        {GoalType::kRobot, "Smart Robot", 2000000, &goal_art_robot},
        {GoalType::kLego, "LEGO Castle", 1500000, &goal_art_lego},
        {GoalType::kSkateboard, "Pro Skateboard", 800000, nullptr},
        {GoalType::kRollerSkates, "Roller Skates", 1200000, nullptr},
        {GoalType::kGameConsole, "Game Console", 3500000, nullptr},
        {GoalType::kTeddyBear, "Giant Teddy Bear", 600000, nullptr},
        {GoalType::kBookshelf, "Science Bookshelf", 500000, nullptr},
        {GoalType::kTelescope, "Space Telescope", 2500000, nullptr},
        {GoalType::kGuitar, "Acoustic Guitar", 1800000, nullptr}
    };
}

SavingsScreen::~SavingsScreen() {}

const GoalItemDef* SavingsScreen::FindGoalDef(GoalType type) const {
    for (const auto& g : goal_definitions_) {
        if (g.type == type) return &g;
    }
    return nullptr;
}

void SavingsScreen::LoadFromNVS() {
    Settings settings("savings", false);
    has_active_goal_ = settings.GetBool("has_goal", false);
    current_goal_type_ = (GoalType)settings.GetInt("goal_type", 0);
    current_goal_name_ = settings.GetString("goal_name", "");
    target_amount_ = settings.GetInt("target_amt", 0);
    current_amount_ = settings.GetInt("current_amt", 0);
    currency_ = settings.GetString("currency", "d");

    if (current_goal_type_ == GoalType::kNone || target_amount_ <= 0) {
        has_active_goal_ = false;
    }
}

void SavingsScreen::SaveToNVS() {
    Settings settings("savings", true);
    settings.SetBool("has_goal", has_active_goal_);
    settings.SetInt("goal_type", (int32_t)current_goal_type_);
    settings.SetString("goal_name", current_goal_name_);
    settings.SetInt("target_amt", target_amount_);
    settings.SetInt("current_amt", current_amount_);
    settings.SetString("currency", currency_);
}

void SavingsScreen::Create(lv_obj_t* parent) {
    LoadFromNVS();

    // Root Container
    root_container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(root_container_);
    lv_obj_set_size(root_container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(root_container_, lv_color_hex(0x0C101A), 0);
    lv_obj_set_style_bg_opa(root_container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root_container_, LV_OBJ_FLAG_SCROLLABLE);

    // Build 4 Sub-Views
    BuildEmptyView();
    BuildActiveGoalView();
    BuildGoalSelectionView();
    BuildPriceInputView();

    // Show initial view based on state
    if (has_active_goal_) {
        ShowActiveGoalView();
    } else {
        ShowEmptyView();
    }
}

void SavingsScreen::BuildEmptyView() {
    view_empty_ = lv_obj_create(root_container_);
    lv_obj_remove_style_all(view_empty_);
    lv_obj_set_size(view_empty_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view_empty_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(view_empty_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(view_empty_, 24, 0);
    lv_obj_set_style_pad_bottom(view_empty_, 12, 0);
    lv_obj_set_style_pad_left(view_empty_, 8, 0);
    lv_obj_set_style_pad_right(view_empty_, 8, 0);
    lv_obj_clear_flag(view_empty_, LV_OBJ_FLAG_SCROLLABLE);

    // Header Title (Centered)
    lv_obj_t* title = lv_label_create(view_empty_);
    lv_label_set_text(title, "Dream Goal");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_bottom(title, 4, 0);

    // 3D Dream Piggy Bank Artwork (76x76)
    lv_obj_t* center_card = lv_button_create(view_empty_);
    lv_obj_remove_style_all(center_card);
    lv_obj_set_size(center_card, 76, 76);
    lv_obj_set_style_pad_all(center_card, 0, 0);
    lv_obj_add_event_cb(center_card, OnCreateGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* img_piggy = lv_image_create(center_card);
    lv_image_set_src(img_piggy, &goal_art_piggy);
    lv_obj_set_size(img_piggy, 76, 76);
    lv_obj_center(img_piggy);

    // Subtitle & Hint (100% English & Centered)
    lv_obj_t* sub = lv_label_create(view_empty_);
    lv_label_set_text(sub, "No Dream Goal Set");
    lv_obj_set_style_text_color(sub, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_top(sub, 4, 0);
    lv_obj_set_style_margin_bottom(sub, 6, 0);

    // Action Button: Create Goal (Centered)
    lv_obj_t* btn_create = lv_button_create(view_empty_);
    lv_obj_set_size(btn_create, 140, 26);
    lv_obj_set_style_radius(btn_create, 13, 0);
    lv_obj_set_style_bg_color(btn_create, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_pad_all(btn_create, 0, 0);
    lv_obj_add_event_cb(btn_create, OnCreateGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* c_lbl = lv_label_create(btn_create);
    lv_label_set_text(c_lbl, "+ Set Dream Goal");
    lv_obj_set_style_text_color(c_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(c_lbl);
}

void SavingsScreen::BuildActiveGoalView() {
    view_active_goal_ = lv_obj_create(root_container_);
    lv_obj_remove_style_all(view_active_goal_);
    lv_obj_set_size(view_active_goal_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view_active_goal_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(view_active_goal_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(view_active_goal_, 22, 0);
    lv_obj_set_style_pad_bottom(view_active_goal_, 12, 0);
    lv_obj_set_style_pad_left(view_active_goal_, 8, 0);
    lv_obj_set_style_pad_right(view_active_goal_, 8, 0);
    lv_obj_clear_flag(view_active_goal_, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Goal Title (Centered)
    goal_title_label_ = lv_label_create(view_active_goal_);
    lv_label_set_text(goal_title_label_, current_goal_name_.c_str());
    lv_obj_set_style_text_color(goal_title_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(goal_title_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_bottom(goal_title_label_, 2, 0);

    // 2. Clickable Center 3D Image Component (76x76)
    goal_card_btn_ = lv_button_create(view_active_goal_);
    lv_obj_remove_style_all(goal_card_btn_);
    lv_obj_set_size(goal_card_btn_, 76, 76);
    lv_obj_set_style_pad_all(goal_card_btn_, 0, 0);
    lv_obj_set_style_margin_bottom(goal_card_btn_, 2, 0);
    lv_obj_add_event_cb(goal_card_btn_, OnChangeGoalClicked, LV_EVENT_CLICKED, this);

    goal_image_ = lv_image_create(goal_card_btn_);
    lv_image_set_src(goal_image_, &goal_art_bike);
    lv_obj_set_size(goal_image_, 76, 76);
    lv_obj_center(goal_image_);

    // 3. Percent Label (Centered)
    percent_label_ = lv_label_create(view_active_goal_);
    lv_label_set_text(percent_label_, "0%");
    lv_obj_set_style_text_color(percent_label_, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_text_align(percent_label_, LV_TEXT_ALIGN_CENTER, 0);

    // 4. Amount Balance Label (Centered)
    amount_label_ = lv_label_create(view_active_goal_);
    lv_label_set_text(amount_label_, "0 / 3,000,000d");
    lv_obj_set_style_text_color(amount_label_, lv_color_hex(0xE2E8F0), 0);
    lv_obj_set_style_text_align(amount_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_bottom(amount_label_, 2, 0);

    // 5. Capsule Progress Bar
    bar_progress_ = lv_bar_create(view_active_goal_);
    lv_obj_set_size(bar_progress_, 120, 6);
    lv_obj_set_style_radius(bar_progress_, 3, 0);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x222638), 0);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x22C55E), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_progress_, 3, LV_PART_INDICATOR);
    lv_bar_set_value(bar_progress_, 0, LV_ANIM_OFF);
    lv_obj_set_style_margin_bottom(bar_progress_, 4, 0);

    // 6. Action Buttons Row (Compact 24px)
    lv_obj_t* btn_row = lv_obj_create(view_active_goal_);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_size(btn_row, lv_pct(100), 24);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn_row, 8, 0);

    // Button 1: Feed Piggy (+50k)
    btn_feed_piggy_ = lv_button_create(btn_row);
    lv_obj_set_size(btn_feed_piggy_, 90, 22);
    lv_obj_set_style_radius(btn_feed_piggy_, 11, 0);
    lv_obj_set_style_bg_color(btn_feed_piggy_, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_pad_all(btn_feed_piggy_, 0, 0);
    lv_obj_add_event_cb(btn_feed_piggy_, OnFeedPiggyClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* feed_label = lv_label_create(btn_feed_piggy_);
    lv_label_set_text(feed_label, "+50k Feed Piggy");
    lv_obj_set_style_text_color(feed_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(feed_label);

    // Button 2: Change Goal
    btn_change_goal_ = lv_button_create(btn_row);
    lv_obj_set_size(btn_change_goal_, 90, 22);
    lv_obj_set_style_radius(btn_change_goal_, 11, 0);
    lv_obj_set_style_bg_color(btn_change_goal_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_color(btn_change_goal_, lv_color_hex(0x3B82F6), 0);
    lv_obj_set_style_border_width(btn_change_goal_, 1, 0);
    lv_obj_set_style_pad_all(btn_change_goal_, 0, 0);
    lv_obj_add_event_cb(btn_change_goal_, OnChangeGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* change_label = lv_label_create(btn_change_goal_);
    lv_label_set_text(change_label, "Change Goal");
    lv_obj_set_style_text_color(change_label, lv_color_hex(0x60A5FA), 0);
    lv_obj_center(change_label);
}

void SavingsScreen::BuildGoalSelectionView() {
    view_select_goal_ = lv_obj_create(root_container_);
    lv_obj_remove_style_all(view_select_goal_);
    lv_obj_set_size(view_select_goal_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view_select_goal_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(view_select_goal_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(view_select_goal_, 28, 0); // Moved down below status bar
    lv_obj_set_style_pad_bottom(view_select_goal_, 10, 0);
    lv_obj_set_style_pad_left(view_select_goal_, 8, 0);
    lv_obj_set_style_pad_right(view_select_goal_, 8, 0);
    lv_obj_clear_flag(view_select_goal_, LV_OBJ_FLAG_SCROLLABLE);

    // Header with Back Button (Larger button, exactly centered title)
    lv_obj_t* header_row = lv_obj_create(view_select_goal_);
    lv_obj_remove_style_all(header_row);
    lv_obj_set_size(header_row, lv_pct(100), 26);
    lv_obj_clear_flag(header_row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_margin_bottom(header_row, 3, 0);

    lv_obj_t* btn_back = lv_button_create(header_row);
    lv_obj_set_size(btn_back, 32, 26);
    lv_obj_set_style_radius(btn_back, 6, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btn_back, OnBackToPreviousClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0x94A3B8), 0);
    lv_obj_center(back_icon);

    lv_obj_t* title = lv_label_create(header_row);
    lv_label_set_text(title, "Select Dream Goal");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFB703), 0);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // Scrollable 10 Goals List (Narrow cards centered horizontally)
    goal_list_container_ = lv_obj_create(view_select_goal_);
    lv_obj_remove_style_all(goal_list_container_);
    lv_obj_set_size(goal_list_container_, lv_pct(100), 154);
    lv_obj_set_flex_flow(goal_list_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(goal_list_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(goal_list_container_, 5, 0);
    lv_obj_set_scrollbar_mode(goal_list_container_, LV_SCROLLBAR_MODE_OFF);

    for (const auto& g : goal_definitions_) {
        lv_obj_t* card = lv_button_create(goal_list_container_);
        lv_obj_set_size(card, 180, 26);
        lv_obj_set_style_radius(card, 6, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x161C28), 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0x2A324B), 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_pad_all(card, 0, 0);

        lv_obj_add_event_cb(card, OnGoalSelected, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(card, (void*)(uintptr_t)g.type);

        // Goal Title (Centered)
        lv_obj_t* name_lbl = lv_label_create(card);
        lv_label_set_text(name_lbl, g.name.c_str());
        lv_obj_set_style_text_color(name_lbl, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_align(name_lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(name_lbl);
    }
}

void SavingsScreen::BuildPriceInputView() {
    view_input_price_ = lv_obj_create(root_container_);
    lv_obj_remove_style_all(view_input_price_);
    lv_obj_set_size(view_input_price_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(view_input_price_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(view_input_price_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(view_input_price_, 28, 0); // Moved down below status bar
    lv_obj_set_style_pad_bottom(view_input_price_, 6, 0);
    lv_obj_set_style_pad_left(view_input_price_, 10, 0);
    lv_obj_set_style_pad_right(view_input_price_, 10, 0);
    lv_obj_clear_flag(view_input_price_, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Header (Back Button + Centered Title)
    lv_obj_t* header_row = lv_obj_create(view_input_price_);
    lv_obj_remove_style_all(header_row);
    lv_obj_set_size(header_row, lv_pct(100), 26);
    lv_obj_clear_flag(header_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* btn_back = lv_button_create(header_row);
    lv_obj_set_size(btn_back, 32, 26);
    lv_obj_set_style_radius(btn_back, 6, 0);
    lv_obj_set_style_bg_color(btn_back, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_pad_all(btn_back, 0, 0);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btn_back, OnChangeGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* back_icon = lv_label_create(btn_back);
    lv_label_set_text(back_icon, MATERIAL_SYMBOLS_ARROW_BACK);
    lv_obj_set_style_text_font(back_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(back_icon, lv_color_hex(0x94A3B8), 0);
    lv_obj_center(back_icon);

    input_goal_name_label_ = lv_label_create(header_row);
    lv_label_set_text(input_goal_name_label_, "My New Bike");
    lv_obj_set_style_text_color(input_goal_name_label_, lv_color_hex(0xFFB703), 0);
    lv_obj_align(input_goal_name_label_, LV_ALIGN_CENTER, 0, 0);

    // 2. Realtime Amount Display Box
    lv_obj_t* disp_box = lv_obj_create(view_input_price_);
    lv_obj_remove_style_all(disp_box);
    lv_obj_set_size(disp_box, lv_pct(100), 28);
    lv_obj_set_style_bg_color(disp_box, lv_color_hex(0x161C28), 0);
    lv_obj_set_style_border_color(disp_box, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_width(disp_box, 1, 0);
    lv_obj_set_style_radius(disp_box, 6, 0);
    lv_obj_set_flex_flow(disp_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(disp_box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_margin_top(disp_box, 2, 0);
    lv_obj_set_style_margin_bottom(disp_box, 4, 0);

    input_amount_display_label_ = lv_label_create(disp_box);
    lv_label_set_text(input_amount_display_label_, "3,000,000 d");
    lv_obj_set_style_text_color(input_amount_display_label_, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_text_align(input_amount_display_label_, LV_TEXT_ALIGN_CENTER, 0);

    // 3. Clean 3x4 Touch Keypad (Standard Calculator Layout: 4 separate rows of 3 buttons)
    lv_obj_t* keypad_container = lv_obj_create(view_input_price_);
    lv_obj_remove_style_all(keypad_container);
    lv_obj_set_size(keypad_container, lv_pct(100), 104);
    lv_obj_set_flex_flow(keypad_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(keypad_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(keypad_container, 3, 0);

    const char* row_keys[4][3] = {
        {"1", "2", "3"},
        {"4", "5", "6"},
        {"7", "8", "9"},
        {"C", "0", "000"}
    };

    for (int r = 0; r < 4; ++r) {
        lv_obj_t* row = lv_obj_create(keypad_container);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, lv_pct(100), 23);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        for (int c = 0; c < 3; ++c) {
            const char* key = row_keys[r][c];
            lv_obj_t* kbtn = lv_button_create(row);
            lv_obj_set_size(kbtn, 68, 23);
            lv_obj_set_style_radius(kbtn, 5, 0);
            lv_obj_set_style_bg_color(kbtn, lv_color_hex(0x1E293B), 0);
            lv_obj_set_style_pad_all(kbtn, 0, 0);
            lv_obj_add_event_cb(kbtn, OnKeypadDigitClicked, LV_EVENT_CLICKED, this);
            lv_obj_set_user_data(kbtn, (void*)key);

            lv_obj_t* klbl = lv_label_create(kbtn);
            lv_label_set_text(klbl, key);
            lv_obj_set_style_text_color(klbl, (strcmp(key, "C") == 0) ? lv_color_hex(0xEF4444) : lv_color_hex(0xFFFFFF), 0);
            lv_obj_center(klbl);
        }
    }

    // 4. Start Button (Centered)
    lv_obj_t* btn_start = lv_button_create(view_input_price_);
    lv_obj_set_size(btn_start, lv_pct(100), 28);
    lv_obj_set_style_radius(btn_start, 14, 0);
    lv_obj_set_style_bg_color(btn_start, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_pad_all(btn_start, 0, 0);
    lv_obj_set_style_margin_top(btn_start, 5, 0);
    lv_obj_add_event_cb(btn_start, OnConfirmPriceClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* s_lbl = lv_label_create(btn_start);
    lv_label_set_text(s_lbl, "Start");
    lv_obj_set_style_text_color(s_lbl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(s_lbl);
}

void SavingsScreen::ShowEmptyView() {
    if (view_empty_) lv_obj_clear_flag(view_empty_, LV_OBJ_FLAG_HIDDEN);
    if (view_active_goal_) lv_obj_add_flag(view_active_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_select_goal_) lv_obj_add_flag(view_select_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_input_price_) lv_obj_add_flag(view_input_price_, LV_OBJ_FLAG_HIDDEN);

    BuddyScreenManager::GetInstance().SetTileviewScrollable(true);
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(true);
}

void SavingsScreen::ShowActiveGoalView() {
    if (view_empty_) lv_obj_add_flag(view_empty_, LV_OBJ_FLAG_HIDDEN);
    if (view_active_goal_) lv_obj_clear_flag(view_active_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_select_goal_) lv_obj_add_flag(view_select_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_input_price_) lv_obj_add_flag(view_input_price_, LV_OBJ_FLAG_HIDDEN);

    BuddyScreenManager::GetInstance().SetTileviewScrollable(true);
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(true);

    UpdateActiveGoalUI(true);
}

void SavingsScreen::ShowGoalSelectionView() {
    if (view_empty_) lv_obj_add_flag(view_empty_, LV_OBJ_FLAG_HIDDEN);
    if (view_active_goal_) lv_obj_add_flag(view_active_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_select_goal_) lv_obj_clear_flag(view_select_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_input_price_) lv_obj_add_flag(view_input_price_, LV_OBJ_FLAG_HIDDEN);

    BuddyScreenManager::GetInstance().SetTileviewScrollable(false);
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(false);
}

void SavingsScreen::ShowPriceInputView(GoalType selected_type) {
    pending_goal_type_ = selected_type;
    const auto* def = FindGoalDef(selected_type);
    if (def) {
        if (input_goal_name_label_) {
            lv_label_set_text(input_goal_name_label_, def->name.c_str());
        }
        input_buffer_ = std::to_string(def->default_price);
        UpdatePriceInputDisplay();
    }

    if (view_empty_) lv_obj_add_flag(view_empty_, LV_OBJ_FLAG_HIDDEN);
    if (view_active_goal_) lv_obj_add_flag(view_active_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_select_goal_) lv_obj_add_flag(view_select_goal_, LV_OBJ_FLAG_HIDDEN);
    if (view_input_price_) lv_obj_clear_flag(view_input_price_, LV_OBJ_FLAG_HIDDEN);

    BuddyScreenManager::GetInstance().SetTileviewScrollable(false);
    BuddyScreenManager::GetInstance().SetIndicatorsVisible(false);
}

void SavingsScreen::UpdatePriceInputDisplay() {
    if (!input_amount_display_label_) return;
    int64_t val = 0;
    try {
        if (!input_buffer_.empty()) {
            val = std::stoll(input_buffer_);
        }
    } catch (...) {
        val = 0;
    }
    std::string text = FormatAmount(val, " d");
    lv_label_set_text(input_amount_display_label_, text.c_str());
}

void SavingsScreen::UpdateActiveGoalUI(bool animate) {
    if (goal_title_label_) {
        lv_label_set_text(goal_title_label_, current_goal_name_.c_str());
    }

    // Set 3D Image corresponding to current GoalType enum
    if (goal_image_) {
        if (current_goal_type_ == GoalType::kRobot) {
            lv_image_set_src(goal_image_, &goal_art_robot);
        } else if (current_goal_type_ == GoalType::kLego) {
            lv_image_set_src(goal_image_, &goal_art_lego);
        } else {
            lv_image_set_src(goal_image_, &goal_art_bike);
        }
    }

    int pct = 0;
    if (target_amount_ > 0) {
        pct = (int)((current_amount_ * 100LL) / target_amount_);
        if (pct > 100) pct = 100;
    }

    if (percent_label_) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", pct);
        lv_label_set_text(percent_label_, buf);
    }

    if (amount_label_) {
        std::string cur_str = FormatAmount(current_amount_, "");
        std::string tgt_str = FormatAmount(target_amount_, currency_);
        std::string full_amt = cur_str + " / " + tgt_str;
        lv_label_set_text(amount_label_, full_amt.c_str());
    }

    if (bar_progress_) {
        lv_bar_set_value(bar_progress_, pct, animate ? LV_ANIM_ON : LV_ANIM_OFF);
    }
}

void SavingsScreen::SetGoal(GoalType type, const std::string& name, int32_t current_amount, int32_t target_amount, const std::string& currency) {
    has_active_goal_ = true;
    current_goal_type_ = type;
    current_goal_name_ = name;
    current_amount_ = current_amount;
    target_amount_ = target_amount;
    currency_ = currency;
    SaveToNVS();
    ShowActiveGoalView();
}

void SavingsScreen::AddSavings(int32_t amount) {
    if (!has_active_goal_) return;
    current_amount_ += amount;
    SaveToNVS();
    UpdateActiveGoalUI(true);
}

void SavingsScreen::ResetGoal() {
    has_active_goal_ = false;
    current_goal_type_ = GoalType::kNone;
    current_goal_name_ = "";
    current_amount_ = 0;
    target_amount_ = 0;
    SaveToNVS();
    ShowEmptyView();
}

void SavingsScreen::OnCreateGoalClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Create goal clicked -> opening goal selector");
    self->ShowGoalSelectionView();
}

void SavingsScreen::OnFeedPiggyClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Feed piggy +50,000 clicked!");
    self->AddSavings(50000);
}

void SavingsScreen::OnChangeGoalClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Change goal clicked -> opening goal selector");
    self->ShowGoalSelectionView();
}

void SavingsScreen::OnBackToPreviousClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    if (self->has_active_goal_) {
        self->ShowActiveGoalView();
    } else {
        self->ShowEmptyView();
    }
}

void SavingsScreen::OnGoalSelected(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    lv_obj_t* card = (lv_obj_t*)lv_event_get_target(e);
    GoalType selected_type = (GoalType)(uintptr_t)lv_obj_get_user_data(card);

    ESP_LOGI(TAG, "Goal type selected: %d -> opening price keypad", (int)selected_type);
    self->ShowPriceInputView(selected_type);
}

void SavingsScreen::OnKeypadDigitClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    const char* key = (const char*)lv_obj_get_user_data(btn);
    if (!key) return;

    if (strcmp(key, "C") == 0) {
        if (!self->input_buffer_.empty()) {
            self->input_buffer_.pop_back();
        }
        if (self->input_buffer_.empty()) {
            self->input_buffer_ = "0";
        }
    } else if (strcmp(key, "000") == 0) {
        if (self->input_buffer_ != "0" && self->input_buffer_.length() + 3 <= 10) {
            self->input_buffer_ += "000";
        }
    } else {
        if (self->input_buffer_ == "0") {
            self->input_buffer_ = key;
        } else if (self->input_buffer_.length() < 10) {
            self->input_buffer_ += key;
        }
    }
    self->UpdatePriceInputDisplay();
}

void SavingsScreen::OnQuickAddAmountClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int32_t val = (int32_t)(intptr_t)lv_obj_get_user_data(btn);

    int64_t current = 0;
    try {
        if (!self->input_buffer_.empty()) {
            current = std::stoll(self->input_buffer_);
        }
    } catch (...) {
        current = 0;
    }
    current += val;
    if (current > 999999999) current = 999999999;
    self->input_buffer_ = std::to_string(current);
    self->UpdatePriceInputDisplay();
}

void SavingsScreen::OnConfirmPriceClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    int64_t price = 0;
    try {
        if (!self->input_buffer_.empty()) {
            price = std::stoll(self->input_buffer_);
        }
    } catch (...) {
        price = 1000000;
    }
    if (price <= 0) price = 1000000;

    const auto* def = self->FindGoalDef(self->pending_goal_type_);
    if (def) {
        self->has_active_goal_ = true;
        self->current_goal_type_ = def->type;
        self->current_goal_name_ = def->name;
        self->target_amount_ = (int32_t)price;
        self->current_amount_ = 0;
        ESP_LOGI(TAG, "Goal created: %s, target: %ld d", self->current_goal_name_.c_str(), (long)price);
        self->SaveToNVS();
        self->ShowActiveGoalView();
    }
}
