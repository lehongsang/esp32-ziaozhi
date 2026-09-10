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
    bool completed;
};

class TodayQuestScreen {
public:
    TodayQuestScreen();
    ~TodayQuestScreen();

    void Create(lv_obj_t* parent);
    void SetQuests(const std::vector<QuestItemData>& quests);
    void OnQuestSelected(std::function<void(const std::string& quest_id)> callback);

    lv_obj_t* GetContainer() const { return container_; }

private:
    lv_obj_t* container_ = nullptr;
    lv_obj_t* title_label_ = nullptr;
    lv_obj_t* quest_list_ = nullptr;
    lv_obj_t* more_arrow_ = nullptr;

    QuestSubflowScreen subflow_screen_;
    std::vector<QuestItemData> current_quests_;
    std::function<void(const std::string& quest_id)> on_quest_selected_;

    void RenderItem(const QuestItemData& item);
    static void OnCardClickedCb(lv_event_t* e);
};

#endif // TODAY_QUEST_SCREEN_H
