#ifndef TODAY_QUEST_SCREEN_H
#define TODAY_QUEST_SCREEN_H

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

#include <lvgl.h>
#include "screen_types.h"
#include "quest_subflow_screen.h"

struct QuestItemData {
    std::string id;
    std::string title;
    std::string progress_text;
    std::string scheduled_time;
    std::string start_time;
    int duration = 20;
    int reward_stars = 1;
    std::string category = "habit";
    bool completed = false;
};

class TodayQuestScreen {
public:
    TodayQuestScreen();
    ~TodayQuestScreen();

    void Create(lv_obj_t* parent);
    void SetQuests(const std::vector<QuestItemData>& quests);
    const std::vector<QuestItemData>& GetQuests() const { return current_quests_; }
    void OnQuestSelected(std::function<void(const std::string& quest_id)> callback);
    void SetOnQuestsChanged(std::function<void(int total, int completed)> callback) {
        on_quests_changed_ = callback;
    }

    lv_obj_t* GetContainer() const { return container_; }

private:
    lv_obj_t* container_ = nullptr;
    
    // Top Progress Header (2/3 + Bar + Star)
    lv_obj_t* header_progress_card_ = nullptr;
    lv_obj_t* progress_ratio_label_ = nullptr;
    lv_obj_t* progress_bar_ = nullptr;
    lv_obj_t* star_icon_ = nullptr;

    // Quests Scroll List
    lv_obj_t* quest_list_ = nullptr;

    // Encouragement Footer
    lv_obj_t* footer_card_ = nullptr;
    lv_obj_t* footer_label_ = nullptr;

    QuestSubflowScreen subflow_screen_;
    std::vector<QuestItemData> current_quests_;
    std::function<void(const std::string& quest_id)> on_quest_selected_;
    std::function<void(int total, int completed)> on_quests_changed_;

    void RenderItem(const QuestItemData& item, size_t index);
    void RenderEmptyState();
    void UpdateSummaryHeader();
    void LoadQuestsFromNVS();
    void SaveQuestsToNVS();
    static void OnCardClickedCb(lv_event_t* e);
};

#endif // TODAY_QUEST_SCREEN_H
