#include "savings_screen.h"
#include <cstdio>

SavingsScreen::SavingsScreen() {}
SavingsScreen::~SavingsScreen() {}

void SavingsScreen::Create(lv_obj_t* parent) {
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(container_, 8, 0);
    lv_obj_set_style_pad_bottom(container_, 12, 0);

    // Goal Title
    goal_title_ = lv_label_create(container_);
    lv_label_set_text(goal_title_, "Goal: New Smartphone");
    lv_obj_set_style_text_color(goal_title_, lv_color_hex(0xFFFFFF), 0);

    // Goal Icon Card
    goal_card_ = lv_obj_create(container_);
    lv_obj_set_size(goal_card_, 80, 80);
    lv_obj_set_style_radius(goal_card_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(goal_card_, lv_color_hex(0x3D348B), 0);
    lv_obj_set_style_border_color(goal_card_, lv_color_hex(0xF72585), 0);
    lv_obj_set_style_border_width(goal_card_, 2, 0);
    lv_obj_set_style_margin_top(goal_card_, 4, 0);
    lv_obj_set_style_margin_bottom(goal_card_, 4, 0);

    goal_icon_ = lv_label_create(goal_card_);
    lv_label_set_text(goal_icon_, "📱");
    lv_obj_center(goal_icon_);

    // Percent Text
    percent_label_ = lv_label_create(container_);
    lv_label_set_text(percent_label_, "62%");
    lv_obj_set_style_text_color(percent_label_, lv_color_hex(0x06D6A0), 0);

    // Amount Text
    amount_label_ = lv_label_create(container_);
    lv_label_set_text(amount_label_, "$62 / $100");
    lv_obj_set_style_text_color(amount_label_, lv_color_hex(0xDDDDDD), 0);

    // Progress Bar
    bar_progress_ = lv_bar_create(container_);
    lv_obj_set_size(bar_progress_, lv_pct(70), 8);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x2B2D42), 0);
    lv_obj_set_style_bg_color(bar_progress_, lv_color_hex(0x06D6A0), LV_PART_INDICATOR);
    lv_bar_set_value(bar_progress_, 62, LV_ANIM_OFF);
    lv_obj_set_style_margin_top(bar_progress_, 4, 0);
}

void SavingsScreen::SetGoal(const std::string& name, int current_amount, int target_amount) {
    if (goal_title_) {
        lv_label_set_text(goal_title_, name.c_str());
    }
    if (target_amount > 0) {
        int pct = (current_amount * 100) / target_amount;
        if (percent_label_) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d%%", pct);
            lv_label_set_text(percent_label_, buf);
        }
        if (bar_progress_) {
            lv_bar_set_value(bar_progress_, pct, LV_ANIM_ON);
        }
    }
    if (amount_label_) {
        char buf[64];
        snprintf(buf, sizeof(buf), "$%d / $%d", current_amount, target_amount);
        lv_label_set_text(amount_label_, buf);
    }
}
