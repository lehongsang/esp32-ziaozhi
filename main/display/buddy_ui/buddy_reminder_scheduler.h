#ifndef BUDDY_REMINDER_SCHEDULER_H
#define BUDDY_REMINDER_SCHEDULER_H

#include <vector>
#include <string>
#include <set>
#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "today_quest_screen.h"

class BuddyReminderScheduler {
public:
    static BuddyReminderScheduler& GetInstance();

    void Initialize();
    void UpdateQuests(const std::vector<QuestItemData>& quests);
    void CheckReminders();

    void SetOnTriggerReminder(std::function<void(const QuestItemData& quest, int minutes_left)> cb) {
        on_trigger_reminder_ = cb;
    }

private:
    BuddyReminderScheduler();
    ~BuddyReminderScheduler();

    std::vector<QuestItemData> quests_;
    std::set<std::string> triggered_reminders_;
    std::function<void(const QuestItemData& quest, int minutes_left)> on_trigger_reminder_;
    TaskHandle_t task_handle_ = nullptr;
    int last_checked_day_ = -1;

    static void SchedulerTask(void* arg);
    int ParseTimeToMinutes(const std::string& time_str);
};

#endif // BUDDY_REMINDER_SCHEDULER_H
