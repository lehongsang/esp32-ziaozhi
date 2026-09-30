#include "today_quest_screen.h"
#include "buddy_sync_service.h"
#include "buddy_font_helper.h"
#include "settings.h"
#include <material_symbols.h>
#include <esp_log.h>

#define TAG "TodayQuestScreen"

LV_FONT_DECLARE(font_material_symbols_20_4);
LV_FONT_DECLARE(font_material_symbols_30_4);

TodayQuestScreen::TodayQuestScreen() {}
TodayQuestScreen::~TodayQuestScreen() {}

void TodayQuestScreen::Create(lv_obj_t* parent) {
    // 1. Root Container (Deep Pure Black Theme)
    container_ = lv_obj_create(parent);
    lv_obj_remove_style_all(container_);
    lv_obj_set_size(container_, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Top Header Progress Row (Clean horizontal bar, Pos: 14, 10, Size: 292x24)
    header_progress_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(header_progress_card_);
    lv_obj_set_size(header_progress_card_, 292, 24);
    lv_obj_set_pos(header_progress_card_, 14, 10);
    lv_obj_set_flex_flow(header_progress_card_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header_progress_card_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(header_progress_card_, 10, 0);
    lv_obj_clear_flag(header_progress_card_, LV_OBJ_FLAG_SCROLLABLE);

    progress_ratio_label_ = lv_label_create(header_progress_card_);
    lv_obj_set_style_text_font(progress_ratio_label_, GetBuddyFont(), 0);
    lv_label_set_text(progress_ratio_label_, "0/0");
    lv_obj_set_style_text_color(progress_ratio_label_, lv_color_hex(0xFFFFFF), 0);

    progress_bar_ = lv_bar_create(header_progress_card_);
    lv_obj_set_size(progress_bar_, 170, 7);
    lv_bar_set_range(progress_bar_, 0, 100);
    lv_bar_set_value(progress_bar_, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress_bar_, lv_color_hex(0x27272A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_bar_, lv_color_hex(0x38BDF8), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progress_bar_, 4, LV_PART_MAIN);
    lv_obj_set_style_radius(progress_bar_, 4, LV_PART_INDICATOR);
    lv_obj_set_flex_grow(progress_bar_, 1);

    star_icon_ = lv_label_create(header_progress_card_);
    lv_label_set_text(star_icon_, MATERIAL_SYMBOLS_STAR);
    lv_obj_set_style_text_font(star_icon_, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(star_icon_, lv_color_hex(0xFACC15), 0);

    // 3. Main Stacked Pastel Task Card (Pos: 14, 38, Size: 292x142, Radius: 20px)
    quest_list_ = lv_obj_create(container_);
    lv_obj_remove_style_all(quest_list_);
    lv_obj_set_size(quest_list_, 292, 142);
    lv_obj_set_pos(quest_list_, 14, 38);
    lv_obj_set_style_radius(quest_list_, 20, 0);
    lv_obj_set_style_clip_corner(quest_list_, true, 0);
    lv_obj_set_style_shadow_width(quest_list_, 14, 0);
    lv_obj_set_style_shadow_color(quest_list_, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(quest_list_, LV_OPA_60, 0);
    lv_obj_set_flex_flow(quest_list_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(quest_list_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(quest_list_, 0, 0);
    lv_obj_set_style_pad_gap(quest_list_, 0, 0);
    lv_obj_add_flag(quest_list_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(quest_list_, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(quest_list_, LV_SCROLLBAR_MODE_AUTO);

    // 4. Bottom Encouragement Section (Spacious layout, Pos: 14, 186, Size: 292x46)
    footer_card_ = lv_obj_create(container_);
    lv_obj_remove_style_all(footer_card_);
    lv_obj_set_size(footer_card_, 292, 46);
    lv_obj_set_pos(footer_card_, 14, 186);
    lv_obj_set_flex_flow(footer_card_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer_card_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(footer_card_, 14, 0);
    lv_obj_clear_flag(footer_card_, LV_OBJ_FLAG_SCROLLABLE);

    footer_label_ = lv_label_create(footer_card_);
    lv_obj_set_style_text_font(footer_label_, GetBuddyFont(), 0);
    lv_label_set_text(footer_label_, "Sẵn sàng cho ngày mới\ncùng Buddy! ✨");
    lv_obj_set_style_text_color(footer_label_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(footer_label_, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_t* footer_star = lv_label_create(footer_card_);
    lv_label_set_text(footer_star, MATERIAL_SYMBOLS_STAR);
    lv_obj_set_style_text_font(footer_star, &font_material_symbols_30_4, 0);
    lv_obj_set_style_text_color(footer_star, lv_color_hex(0xFACC15), 0); // Warm Golden Star

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

    // 6. Load saved quests from NVS (No hardcoded mock defaults)
    LoadQuestsFromNVS();
    SetQuests(current_quests_);

    ESP_LOGI(TAG, "TodayQuestScreen initialized with dynamic backend & NVS quest sync (Loaded: %d)", (int)current_quests_.size());
}

void TodayQuestScreen::SetQuests(const std::vector<QuestItemData>& quests) {
    current_quests_ = quests;
    if (!quest_list_) return;
    lv_obj_clean(quest_list_);

    if (current_quests_.empty()) {
        RenderEmptyState();
    } else {
        for (size_t i = 0; i < current_quests_.size(); ++i) {
            RenderItem(current_quests_[i], i);
        }
    }

    UpdateSummaryHeader();
    SaveQuestsToNVS();
}

void TodayQuestScreen::RenderEmptyState() {
    lv_obj_t* empty_card = lv_obj_create(quest_list_);
    lv_obj_remove_style_all(empty_card);
    lv_obj_set_size(empty_card, 292, 142);
    lv_obj_set_style_bg_color(empty_card, lv_color_hex(0x18181B), 0);
    lv_obj_set_style_bg_opa(empty_card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(empty_card, 16, 0);
    lv_obj_set_flex_flow(empty_card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(empty_card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(empty_card, 8, 0);
    lv_obj_clear_flag(empty_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* icon_circle = lv_obj_create(empty_card);
    lv_obj_remove_style_all(icon_circle);
    lv_obj_set_size(icon_circle, 42, 42);
    lv_obj_set_style_bg_color(icon_circle, lv_color_hex(0x0284C7), 0);
    lv_obj_set_style_bg_opa(icon_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(icon_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(icon_circle, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* icon = lv_label_create(icon_circle);
    lv_label_set_text(icon, MATERIAL_SYMBOLS_CHECK_CIRCLE);
    lv_obj_set_style_text_font(icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(icon);

    lv_obj_t* title = lv_label_create(empty_card);
    lv_obj_set_style_text_font(title, GetBuddyFont(), 0);
    lv_label_set_text(title, "Chưa có nhiệm vụ hôm nay");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* sub = lv_label_create(empty_card);
    lv_obj_set_style_text_font(sub, GetBuddyFont(), 0);
    lv_label_set_text(sub, "Bố Mẹ hãy giao nhiệm vụ từ\nứng dụng để cùng bé thực hiện! ✨");
    lv_obj_set_style_text_color(sub, lv_color_hex(0x94A3B8), 0);
    lv_obj_set_style_text_align(sub, LV_TEXT_ALIGN_CENTER, 0);
}

void TodayQuestScreen::UpdateSummaryHeader() {
    int total = current_quests_.size();
    int completed = 0;
    for (const auto& q : current_quests_) {
        if (q.completed) completed++;
    }

    if (progress_ratio_label_) {
        std::string ratio = std::to_string(completed) + "/" + std::to_string(total);
        lv_label_set_text(progress_ratio_label_, ratio.c_str());
    }

    if (progress_bar_ && total > 0) {
        int percent = (completed * 100) / total;
        lv_bar_set_value(progress_bar_, percent, LV_ANIM_ON);
    }

    if (footer_label_) {
        int remaining = total - completed;
        if (remaining == 0) {
            lv_label_set_text(footer_label_, "Tuyệt vời!\nĐã xong hết việc hôm nay!");
            lv_obj_set_style_text_color(footer_label_, lv_color_hex(0x4ADE80), 0);
        } else {
            std::string text = "Còn " + std::to_string(remaining) + " việc nữa là\ntrọn vẹn hôm nay!";
            lv_label_set_text(footer_label_, text.c_str());
            lv_obj_set_style_text_color(footer_label_, lv_color_hex(0xFFFFFF), 0);
        }
    }

    if (on_quests_changed_) {
        on_quests_changed_(total, completed);
    }
}

void TodayQuestScreen::RenderItem(const QuestItemData& item, size_t index) {
    lv_obj_t* card = lv_btn_create(quest_list_);
    lv_obj_remove_style_all(card);
    lv_obj_set_size(card, 292, 47);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(card, 14, 0);
    lv_obj_set_style_pad_gap(card, 12, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Apply exact Pastel Color scheme per category
    if (index == 0 || item.category == "math" || item.id.find("math") != std::string::npos) {
        lv_obj_set_style_bg_color(card, lv_color_hex(0xE0F2FE), 0); // Pastel Sky Blue
        lv_obj_set_style_border_side(card, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0xBAE6FD), 0);
        lv_obj_set_style_border_width(card, 1, 0);
    } else if (index == 1 || item.category == "read" || item.id.find("read") != std::string::npos) {
        lv_obj_set_style_bg_color(card, lv_color_hex(0xDCFCE7), 0); // Pastel Mint Green
        lv_obj_set_style_border_side(card, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0xBBF7D0), 0);
        lv_obj_set_style_border_width(card, 1, 0);
    } else {
        lv_obj_set_style_bg_color(card, lv_color_hex(0xFFEDD5), 0); // Pastel Warm Peach
        lv_obj_set_style_border_width(card, 0, 0);
    }
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);

    // Click handler on card
    lv_obj_add_event_cb(card, OnCardClickedCb, LV_EVENT_CLICKED, this);

    // 1. Left Rounded Square Icon Badge (Vibrant Solid OPA COVER + subtle glow)
    lv_obj_t* badge = lv_obj_create(card);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 34, 34);
    lv_obj_set_style_radius(badge, 10, 0);
    lv_obj_set_style_bg_opa(badge, LV_OPA_COVER, 0);
    lv_obj_clear_flag(badge, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* badge_icon = lv_label_create(badge);
    lv_obj_set_style_text_font(badge_icon, &font_material_symbols_20_4, 0);
    lv_obj_set_style_text_color(badge_icon, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(badge_icon);

    if (index == 0 || item.category == "math" || item.id.find("math") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0x2563EB), 0); // Vibrant Royal Blue
        lv_obj_set_style_shadow_width(badge, 6, 0);
        lv_obj_set_style_shadow_color(badge, lv_color_hex(0x2563EB), 0);
        lv_obj_set_style_shadow_opa(badge, LV_OPA_40, 0);
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_CALCULATE);
    } else if (index == 1 || item.category == "read" || item.id.find("read") != std::string::npos) {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0x16A34A), 0); // Vibrant Emerald Green
        lv_obj_set_style_shadow_width(badge, 6, 0);
        lv_obj_set_style_shadow_color(badge, lv_color_hex(0x16A34A), 0);
        lv_obj_set_style_shadow_opa(badge, LV_OPA_40, 0);
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_EDIT_SQUARE);
    } else {
        lv_obj_set_style_bg_color(badge, lv_color_hex(0xEA580C), 0); // Vibrant Orange
        lv_obj_set_style_shadow_width(badge, 6, 0);
        lv_obj_set_style_shadow_color(badge, lv_color_hex(0xEA580C), 0);
        lv_obj_set_style_shadow_opa(badge, LV_OPA_40, 0);
        lv_label_set_text(badge_icon, MATERIAL_SYMBOLS_CHECK);
    }

    // 2. Title Text (Bold Dark Navy text, No hours text!)
    lv_obj_t* title = lv_label_create(card);
    lv_obj_set_style_text_font(title, GetBuddyFont(), 0);
    lv_label_set_text(title, SanitizeVietnamese(item.title).c_str());
    lv_obj_set_style_text_color(title, lv_color_hex(0x0A0F1D), 0); // High contrast dark navy text on pastel
    lv_obj_set_flex_grow(title, 1);

    // 3. Right Status (Solid Green Checkmark Badge or Orange Circle Ring)
    lv_obj_t* check_badge = lv_obj_create(card);
    lv_obj_remove_style_all(check_badge);
    lv_obj_set_size(check_badge, 28, 28);
    lv_obj_set_style_radius(check_badge, LV_RADIUS_CIRCLE, 0);
    lv_obj_clear_flag(check_badge, LV_OBJ_FLAG_SCROLLABLE);

    if (item.completed) {
        lv_obj_set_style_bg_color(check_badge, lv_color_hex(0x22C55E), 0); // Solid Green
        lv_obj_set_style_bg_opa(check_badge, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(check_badge, 0, 0);

        lv_obj_t* check_icon = lv_label_create(check_badge);
        lv_label_set_text(check_icon, MATERIAL_SYMBOLS_CHECK);
        lv_obj_set_style_text_font(check_icon, &font_material_symbols_20_4, 0);
        lv_obj_set_style_text_color(check_icon, lv_color_hex(0xFFFFFF), 0);
        lv_obj_center(check_icon);
    } else {
        lv_obj_set_style_bg_opa(check_badge, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(check_badge, lv_color_hex(0xEA580C), 0); // Vibrant Orange Ring
        lv_obj_set_style_border_width(check_badge, 2, 0);
    }
}

void TodayQuestScreen::OnCardClickedCb(lv_event_t* e) {
    auto* self = static_cast<TodayQuestScreen*>(lv_event_get_user_data(e));
    lv_obj_t* card = (lv_obj_t*)lv_event_get_current_target(e);
    if (!self || !card) return;

    uint32_t child_idx = lv_obj_get_index(card);
    if (child_idx < self->current_quests_.size()) {
        auto& q = self->current_quests_[child_idx];
        ESP_LOGI(TAG, "Quest card clicked: %s (%s)", q.title.c_str(), q.id.c_str());
        
        // Toggle or start subflow
        if (!q.completed) {
            self->subflow_screen_.StartQuest(q.id, q.title);
        } else {
            // Already completed
        }
    }
}

void TodayQuestScreen::OnQuestSelected(std::function<void(const std::string& quest_id)> callback) {
    on_quest_selected_ = callback;
}

void TodayQuestScreen::SaveQuestsToNVS() {
    Settings settings("buddy_quests", true);
    settings.SetInt("count", (int)current_quests_.size());
    for (size_t i = 0; i < current_quests_.size(); ++i) {
        std::string prefix = "q_" + std::to_string(i) + "_";
        settings.SetString(prefix + "id", current_quests_[i].id);
        settings.SetString(prefix + "title", current_quests_[i].title);
        settings.SetString(prefix + "prog", current_quests_[i].progress_text);
        settings.SetString(prefix + "sched", current_quests_[i].scheduled_time);
        settings.SetString(prefix + "start", current_quests_[i].start_time);
        settings.SetInt(prefix + "dur", current_quests_[i].duration);
        settings.SetInt(prefix + "stars", current_quests_[i].reward_stars);
        settings.SetString(prefix + "cat", current_quests_[i].category);
        settings.SetInt(prefix + "remind", current_quests_[i].remind_before);
        settings.SetInt(prefix + "comp", current_quests_[i].completed ? 1 : 0);
    }
}

void TodayQuestScreen::LoadQuestsFromNVS() {
    Settings settings("buddy_quests", false);
    int count = settings.GetInt("count", 0);
    if (count <= 0) {
        current_quests_.clear();
        return;
    }

    current_quests_.clear();
    for (int i = 0; i < count; ++i) {
        std::string prefix = "q_" + std::to_string(i) + "_";
        QuestItemData q;
        q.id = settings.GetString(prefix + "id", "q_" + std::to_string(i));
        q.title = settings.GetString(prefix + "title", "");
        if (q.title.empty()) continue;
        q.progress_text = settings.GetString(prefix + "prog", "");
        q.scheduled_time = settings.GetString(prefix + "sched", "");
        q.start_time = settings.GetString(prefix + "start", "");
        q.duration = settings.GetInt(prefix + "dur", 20);
        q.reward_stars = settings.GetInt(prefix + "stars", 1);
        q.category = settings.GetString(prefix + "cat", "habit");
        q.remind_before = settings.GetInt(prefix + "remind", 30);
        q.completed = settings.GetInt(prefix + "comp", 0) == 1;
        current_quests_.push_back(std::move(q));
    }
}
