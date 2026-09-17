#include "today_quest_screen.h"
#include "buddy_sync_service.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "TodayQuestScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);

TodayQuestScreen::TodayQuestScreen() {}
TodayQuestScreen::~TodayQuestScreen() {}

void TodayQuestScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Full Screen Dark Theme)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x0C101A), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(container_, 10, 0);
    lv_obj_set_style_pad_bottom(container_, 2, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Header Title
    title_label_ = lv_label_create(container_);
    lv_label_set_text(title_label_, "Today's Quest");
    lv_obj_set_style_text_color(title_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_margin_bottom(title_label_, 4, 0);

    // 3. Quest List Container (Scrollable)
    quest_list_ = lv_obj_create(container_);
    lv_obj_remove_style_all(quest_list_);
    lv_obj_set_size(quest_list_, lv_pct(94), 146);
    lv_obj_set_flex_flow(quest_list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(quest_list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(quest_list_, 6, 0);
    lv_obj_set_scrollbar_mode(quest_list_, LV_SCROLLBAR_MODE_OFF);

    // 4. Down Arrow Scroll Indicator (Visible when more items exist)
    more_arrow_ = lv_label_create(container_);
    lv_label_set_text(more_arrow_, MATERIAL_SYMBOLS_KEYBOARD_ARROW_DOWN);
    lv_obj_set_style_text_font(more_arrow_, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(more_arrow_, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_margin_top(more_arrow_, 0, 0);
    lv_obj_add_flag(more_arrow_, LV_OBJ_FLAG_HIDDEN); // Hidden by default unless > 3 items

    // 5. Initialize Subflow Screen Overlay (Created on parent tile to cover full screen)
    subflow_screen_.Create(parent);
    subflow_screen_.SetOnCompleted([this](const std::string& completed_id) {
        for (auto& q : current_quests_) {
            if (q.id == completed_id) {
                q.completed = true;
                q.progress_text = "";
                break;
            }
        }
        SetQuests(current_quests_);
        BuddySyncService::GetInstance().NotifyQuestCompleted(completed_id);
        if (on_quest_selected_) {
            on_quest_selected_(completed_id);
        }
    });

    // Default items
    current_quests_ = {
        {"q_feed", "Feed Piggy", "0/3", false},
        {"q_math", "Math homework", "0/10", false},
        {"q_read", "English reading", "0/2", false},
        {"q_move", "Do exercise", "20 min", false},
        {"q_parent", "Parent's quest", "0/1", false}
    };
    SetQuests(current_quests_);

    ESP_LOGI(TAG, "TodayQuestScreen initialized with English layout, scroll indicator & Quiz Subflow");
}

void TodayQuestScreen::SetQuests(const std::vector<QuestItemData>& quests) {
    current_quests_ = quests;
    if (!quest_list_) return;
    lv_obj_clean(quest_list_);

    for (size_t i = 0; i < current_quests_.size(); ++i) {
        RenderItem(current_quests_[i]);
    }

    if (more_arrow_) {
        if (current_quests_.size() > 3) {
            lv_obj_clear_flag(more_arrow_, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(more_arrow_, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

void TodayQuestScreen::RenderItem(const QuestItemData& item) {
    lv_obj_t* card = lv_btn_create(quest_list_);
    lv_obj_set_size(card, lv_pct(100), 40);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x161C28), 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x232D3E), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 20, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, item.completed ? lv_color_hex(0x22C55E) : lv_color_hex(0x2A354A), 0);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 8, 0);
    lv_obj_set_style_pad_gap(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Click handler on card
    lv_obj_add_event_cb(card, OnCardClickedCb, LV_EVENT_CLICKED, this);

    // 1. Left Circular Badge
    lv_obj_t* badge = lv_obj_create(card);
    lv_obj_set_size(badge, 28, 28);
    lv_obj_set_style_radius(badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(badge, 0, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* badge_icon = lv_label_create(badge);
    lv_obj_set_style_text_font(badge_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(badge_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(badge_icon);

    if (item.id.find("feed") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0xEC4899), 0); // Rose Pink
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_FAVORITE);
    } else if (item.id.find("math") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0x22C55E), 0); // Green
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_EDIT_SQUARE);
    } else if (item.id.find("read") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0x06D6A0), 0); // Teal
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_SCHEDULE);
    } else if (item.id.find("move") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0x3A86FF), 0); // Blue
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_SPORTS_ESPORTS);
    } else {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0xFFB703), 0); // Amber
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_PERSON);
    }

    // 2. Title Text (Auto expands to fill middle space)
    lv_obj_t* title = lv_label_create(card);
    lv_label_set_text(title, item.title.c_str());
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_flex_grow(title, 1);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);

    // 3. Right Status (Green Checkmark Badge or Progress Text)
    if (item.completed) {
        lv_obj_t* check_badge = lv_obj_create(card);
        lv_obj_set_size(check_badge, 24, 24);
        lv_obj_set_style_radius(check_badge, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(check_badge, lv_color_hex(0x22C55E), 0);
        lv_obj_set_style_border_width(check_badge, 0, 0);
        lv_obj_clear_flag(check_badge, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* check_icon = lv_label_create(check_badge);
        lv_label_set_text(check_icon, MATERIAL_SYMBOLS_CHECK);
        lv_obj_set_style_text_font(check_icon, &font_material_symbols_20_4, 0);
        lv_obj_set_style_text_color(check_icon, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(check_icon);
    } else {
        lv_obj_t* progress = lv_label_create(card);
        lv_label_set_text(progress, item.progress_text.c_str());
        lv_obj_set_style_text_color(progress, lv_color_hex(0xE2E8F0), 0);
        lv_obj_set_style_pad_right(progress, 4, 0);
    }
}

void TodayQuestScreen::OnCardClickedCb(lv_event_t* e) {
    auto* self = static_cast<TodayQuestScreen*>(lv_event_get_user_data(e));
    lv_obj_t* card = (lv_obj_t*)lv_event_get_current_target(e);
    if (!self || !card) return;

    uint32_t child_idx = lv_obj_get_index(card);
    if (child_idx < self->current_quests_.size()) {
        const auto& q = self->current_quests_[child_idx];
        ESP_LOGI(TAG, "Quest card clicked: %s (%s)", q.title.c_str(), q.id.c_str());
        self->subflow_screen_.StartQuest(q.id, q.title);
    }
}

void TodayQuestScreen::OnQuestSelected(std::function<void(const std::string& quest_id)> callback) {
    on_quest_selected_ = callback;
}
