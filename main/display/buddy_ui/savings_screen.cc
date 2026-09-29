#include "savings_screen.h"
#include "screen_manager.h"
#include "buddy_sync_service.h"
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

bool SavingsScreen::IsGoalConfigured(GoalType type) const {
    Settings settings("savings", false);
    return settings.GetBool(("cfg_" + std::to_string((int)type)).c_str(), false);
}

void SavingsScreen::SetGoalConfigured(GoalType type, bool configured) {
    Settings settings("savings", true);
    settings.SetBool(("cfg_" + std::to_string((int)type)).c_str(), configured);
}

int32_t SavingsScreen::GetSavedGoalPrice(GoalType type) const {
    Settings settings("savings", false);
    int32_t p = settings.GetInt(("gp_" + std::to_string((int)type)).c_str(), 0);
    if (p > 0) return p;
    if (type == current_goal_type_ && target_amount_ > 0) {
        return target_amount_;
    }
    const auto* def = FindGoalDef(type);
    return def ? def->default_price : 2000000;
}

void SavingsScreen::SetSavedGoalPrice(GoalType type, int32_t price) {
    Settings settings("savings", true);
    settings.SetInt(("gp_" + std::to_string((int)type)).c_str(), price);
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
    } else {
        SetGoalConfigured(current_goal_type_, true);
        SetSavedGoalPrice(current_goal_type_, target_amount_);
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
    lv_obj_clear_flag(view_empty_, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Center 160x160 Dream Piggy 3D Artwork (Zoomed & Huge)
    lv_obj_t* img_piggy = lv_image_create(view_empty_);
    lv_image_set_src(img_piggy, &goal_art_piggy);
    lv_obj_set_size(img_piggy, 160, 160);
    lv_obj_align(img_piggy, LV_ALIGN_CENTER, 0, -12);
    lv_obj_add_flag(img_piggy, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img_piggy, OnCreateGoalClicked, LV_EVENT_CLICKED, this);

    // 2. Top Floating Glass Pill Header
    lv_obj_t* top_pill = lv_obj_create(view_empty_);
    lv_obj_remove_style_all(top_pill);
    lv_obj_set_size(top_pill, 140, 22);
    lv_obj_align(top_pill, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(top_pill, lv_color_hex(0x161328), 0);
    lv_obj_set_style_bg_opa(top_pill, LV_OPA_80, 0);
    lv_obj_set_style_radius(top_pill, 11, 0);
    lv_obj_set_style_border_color(top_pill, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_opa(top_pill, LV_OPA_30, 0);
    lv_obj_set_style_border_width(top_pill, 1, 0);
    lv_obj_set_flex_flow(top_pill, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_pill, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top_pill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(top_pill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(top_pill, OnCreateGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* title = lv_label_create(top_pill);
    lv_label_set_text(title, "Mục tiêu ước mơ");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFD166), 0);

    // 3. Bottom Floating Glass Action Card (Golden Sparkle Capsule Pill)
    lv_obj_t* bottom_card = lv_obj_create(view_empty_);
    lv_obj_remove_style_all(bottom_card);
    lv_obj_set_size(bottom_card, 180, 30);
    lv_obj_align(bottom_card, LV_ALIGN_BOTTOM_MID, 0, -22);
    lv_obj_set_style_bg_color(bottom_card, lv_color_hex(0x1D142F), 0);
    lv_obj_set_style_bg_opa(bottom_card, LV_OPA_90, 0);
    lv_obj_set_style_radius(bottom_card, 15, 0);
    lv_obj_set_style_border_color(bottom_card, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_opa(bottom_card, LV_OPA_80, 0);
    lv_obj_set_style_border_width(bottom_card, 1, 0);
    lv_obj_set_flex_flow(bottom_card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bottom_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(bottom_card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bottom_card, OnCreateGoalClicked, LV_EVENT_CLICKED, this);

    lv_obj_t* c_lbl = lv_label_create(bottom_card);
    lv_label_set_text(c_lbl, "Đặt mục tiêu");
    lv_obj_set_style_text_color(c_lbl, lv_color_hex(0xFFE082), 0);
}

void SavingsScreen::BuildActiveGoalView() {
    view_active_goal_ = lv_obj_create(root_container_);
    lv_obj_remove_style_all(view_active_goal_);
    lv_obj_set_size(view_active_goal_, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(view_active_goal_, LV_OBJ_FLAG_SCROLLABLE);

    // 1. Center 160x160 3D Artwork Layer (Zoomed & Huge)
    goal_image_ = lv_image_create(view_active_goal_);
    lv_image_set_src(goal_image_, &goal_art_bike);
    lv_obj_set_size(goal_image_, 160, 160);
    lv_obj_align(goal_image_, LV_ALIGN_CENTER, 0, -12);
    lv_obj_add_flag(goal_image_, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(goal_image_, OnChangeGoalClicked, LV_EVENT_CLICKED, this);

    // 2. Top Floating Glass Pill Header (Centered)
    lv_obj_t* top_pill = lv_obj_create(view_active_goal_);
    lv_obj_remove_style_all(top_pill);
    lv_obj_set_size(top_pill, 160, 22);
    lv_obj_align(top_pill, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_bg_color(top_pill, lv_color_hex(0x161328), 0);
    lv_obj_set_style_bg_opa(top_pill, LV_OPA_80, 0);
    lv_obj_set_style_radius(top_pill, 11, 0);
    lv_obj_set_style_border_color(top_pill, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_opa(top_pill, LV_OPA_30, 0);
    lv_obj_set_style_border_width(top_pill, 1, 0);
    lv_obj_set_flex_flow(top_pill, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_pill, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(top_pill, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(top_pill, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(top_pill, OnChangeGoalClicked, LV_EVENT_CLICKED, this);

    goal_title_label_ = lv_label_create(top_pill);
    lv_label_set_text(goal_title_label_, current_goal_name_.c_str());
    lv_obj_set_style_text_color(goal_title_label_, lv_color_hex(0xFFD166), 0);

    // 3. Bottom Floating Glass Status HUD Card (Tap to Edit Target Price)
    lv_obj_t* bottom_card = lv_obj_create(view_active_goal_);
    lv_obj_remove_style_all(bottom_card);
    lv_obj_set_size(bottom_card, 170, 36);
    lv_obj_align(bottom_card, LV_ALIGN_BOTTOM_MID, 0, -22);
    lv_obj_set_style_bg_color(bottom_card, lv_color_hex(0x161328), 0);
    lv_obj_set_style_bg_opa(bottom_card, LV_OPA_90, 0);
    lv_obj_set_style_radius(bottom_card, 18, 0);
    lv_obj_set_style_border_color(bottom_card, lv_color_hex(0xF59E0B), 0);
    lv_obj_set_style_border_opa(bottom_card, LV_OPA_30, 0);
    lv_obj_set_style_border_width(bottom_card, 1, 0);
    lv_obj_set_flex_flow(bottom_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(bottom_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(bottom_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(bottom_card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(bottom_card, OnEditPriceClicked, LV_EVENT_CLICKED, this);

    amount_label_ = lv_label_create(bottom_card);
    lv_label_set_text(amount_label_, "0 / 2,000,000d");
    lv_obj_set_style_text_color(amount_label_, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_text_align(amount_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_margin_bottom(amount_label_, 2, 0);

    bar_progress_ = lv_bar_create(bottom_card);
    lv_obj_set_size(bar_progress_, 140, 4);
    lv_obj_set_style_radius(bar_progress_, 2, 0);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x22C55E), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar_progress_, 2, LV_PART_INDICATOR);
    lv_bar_set_value(bar_progress_, 0, LV_ANIM_OFF);
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

    // Scrollable 10 Goals List
    goal_list_container_ = lv_obj_create(view_select_goal_);
    lv_obj_remove_style_all(goal_list_container_);
    lv_obj_set_size(goal_list_container_, lv_pct(100), 154);
    lv_obj_set_flex_flow(goal_list_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(goal_list_container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(goal_list_container_, 5, 0);
    lv_obj_set_scrollbar_mode(goal_list_container_, LV_SCROLLBAR_MODE_OFF);

    RefreshGoalSelectionList();
}

void SavingsScreen::RefreshGoalSelectionList() {
    if (!goal_list_container_) return;
    lv_obj_clean(goal_list_container_);

    for (const auto& g : goal_definitions_) {
        bool is_active = (g.type == current_goal_type_ && has_active_goal_);

        lv_obj_t* card = lv_button_create(goal_list_container_);
        lv_obj_set_size(card, 200, 28);
        lv_obj_set_style_radius(card, 8, 0);
        lv_obj_set_style_bg_color(card, is_active ? lv_color_hex(0x231A38) : lv_color_hex(0x161C28), 0);
        lv_obj_set_style_border_color(card, is_active ? lv_color_hex(0xF59E0B) : lv_color_hex(0x2A324B), 0);
        lv_obj_set_style_border_width(card, is_active ? 2 : 1, 0);
        lv_obj_set_style_pad_all(card, 0, 0);

        lv_obj_add_event_cb(card, OnGoalSelected, LV_EVENT_CLICKED, this);
        lv_obj_set_user_data(card, (void*)(uintptr_t)g.type);

        // Centered Goal Name (+ Star if Active)
        lv_obj_t* name_lbl = lv_label_create(card);
        std::string title_str = (is_active ? "[*] " : "") + g.name;
        lv_label_set_text(name_lbl, title_str.c_str());
        lv_obj_set_style_text_color(name_lbl, is_active ? lv_color_hex(0xFFD166) : lv_color_hex(0xFFFFFF), 0);
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
    RefreshGoalSelectionList();

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
        int32_t saved_price = GetSavedGoalPrice(selected_type);
        input_buffer_ = std::to_string(saved_price);
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

void SavingsScreen::OnChangeGoalClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Change goal clicked -> opening goal selector");
    self->ShowGoalSelectionView();
}

void SavingsScreen::OnEditPriceClicked(lv_event_t* e) {
    auto* self = static_cast<SavingsScreen*>(lv_event_get_user_data(e));
    if (!self) return;

    ESP_LOGI(TAG, "Edit price clicked -> opening keypad for current goal");
    self->ShowPriceInputView(self->current_goal_type_);
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
    const auto* def = self->FindGoalDef(selected_type);
    if (!def) return;

    if (self->IsGoalConfigured(selected_type)) {
        // Cờ = 1: Đã setup tiền trước đó -> Kích hoạt ngay với số tiền đã lưu, không cần nhập lại!
        self->has_active_goal_ = true;
        self->current_goal_type_ = def->type;
        self->current_goal_name_ = def->name;
        self->target_amount_ = self->GetSavedGoalPrice(selected_type);
        if (self->target_amount_ <= 0) self->target_amount_ = 2000000;
        self->SaveToNVS();
        ESP_LOGI(TAG, "Goal %s already configured (flag=1, target=%ld d) -> activating directly",
                 def->name.c_str(), (long)self->target_amount_);
        self->ShowActiveGoalView();
    } else {
        // Cờ = 0: Chưa từng setup tiền -> Mở bàn phím để nhập số tiền lần đầu
        ESP_LOGI(TAG, "Goal %s not configured yet (flag=0) -> opening price keypad", def->name.c_str());
        self->ShowPriceInputView(selected_type);
    }
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
        price = 2000000;
    }
    if (price <= 0) price = 2000000;

    const auto* def = self->FindGoalDef(self->pending_goal_type_);
    if (def) {
        self->SetSavedGoalPrice(self->pending_goal_type_, (int32_t)price);
        self->SetGoalConfigured(self->pending_goal_type_, true); // Đánh cờ -> 1 (đã setup tiền)
        self->has_active_goal_ = true;
        self->current_goal_type_ = def->type;
        self->current_goal_name_ = def->name;
        self->target_amount_ = (int32_t)price;
        ESP_LOGI(TAG, "Goal confirmed & flag marked: %s, target: %ld d", self->current_goal_name_.c_str(), (long)price);
        self->SaveToNVS();
        self->ShowActiveGoalView();
        BuddySyncService::GetInstance().NotifyGoalChanged(static_cast<int>(def->type), def->name, (int32_t)price);
    }
}
