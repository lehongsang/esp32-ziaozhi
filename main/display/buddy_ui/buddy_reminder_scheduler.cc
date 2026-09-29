#include "buddy_reminder_scheduler.h"
#include <esp_log.h>
#include <ctime>
#include "application.h"
#include "buddy_toast_overlay.h"

#define TAG "BuddyReminderScheduler"

BuddyReminderScheduler& BuddyReminderScheduler::GetInstance() {
    static BuddyReminderScheduler instance;
    return instance;
}

BuddyReminderScheduler::BuddyReminderScheduler() {}
BuddyReminderScheduler::~BuddyReminderScheduler() {
    if (task_handle_) {
        vTaskDelete(task_handle_);
        task_handle_ = nullptr;
    }
}

void BuddyReminderScheduler::Initialize() {
    if (!task_handle_) {
        xTaskCreate(SchedulerTask, "buddy_reminder", 4096, this, 2, &task_handle_);
        ESP_LOGI(TAG, "BuddyReminderScheduler task started");
    }
}

void BuddyReminderScheduler::UpdateQuests(const std::vector<QuestItemData>& quests) {
    quests_ = quests;
    ESP_LOGI(TAG, "Updated %d quests for reminder scheduler", (int)quests_.size());
}

int BuddyReminderScheduler::ParseTimeToMinutes(const std::string& time_str) {
    if (time_str.empty()) return -1;

    // Supports "17:00" or "17:00 - 17:20"
    int h = 0, m = 0;
    if (sscanf(time_str.c_str(), "%d:%d", &h, &m) == 2) {
        return h * 60 + m;
    }
    return -1;
}

void BuddyReminderScheduler::SchedulerTask(void* arg) {
    auto* self = static_cast<BuddyReminderScheduler*>(arg);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(30000)); // Check every 30 seconds
        self->CheckReminders();
    }
}

void BuddyReminderScheduler::CheckReminders() {
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    if (!tm_info) return;

    int current_day = tm_info->tm_yday;
    if (current_day != last_checked_day_) {
        // Reset triggered cache on day change
        triggered_reminders_.clear();
        last_checked_day_ = current_day;
    }

    int current_min = tm_info->tm_hour * 60 + tm_info->tm_min;

    for (const auto& q : quests_) {
        if (q.completed) continue; // Skip completed quests

        int start_min = ParseTimeToMinutes(!q.start_time.empty() ? q.start_time : q.scheduled_time);
        if (start_min < 0) continue;

        int diff = start_min - current_min;
        int trigger_slot = -1;

        if (diff == 60) {
            trigger_slot = 60;
        } else if (diff == 30) {
            trigger_slot = 30;
        } else if (diff == 15) {
            trigger_slot = 15;
        } else if (diff == 0) {
            trigger_slot = 0;
        }

        if (trigger_slot >= 0) {
            std::string key = q.id + "_" + std::to_string(trigger_slot);
            if (triggered_reminders_.find(key) == triggered_reminders_.end()) {
                triggered_reminders_.insert(key);
                ESP_LOGI(TAG, "Triggering reminder for '%s' (%s) at %d min before", q.title.c_str(), q.id.c_str(), trigger_slot);

                Application::GetInstance().Schedule([this, q, trigger_slot]() {
                    if (trigger_slot == 0) {
                        std::string title = "Đến giờ rồi!";
                        std::string body = "Đến giờ " + q.title + " (" + q.scheduled_time + ")";
                        BuddyToastOverlay::GetInstance().Show(title, body, ToastType::kReminder, 8000);
                    } else {
                        std::string title = "Nhắc việc sắp tới";
                        std::string body = "Còn " + std::to_string(trigger_slot) + " phút nữa là đến giờ " + q.title;
                        BuddyToastOverlay::GetInstance().Show(title, body, ToastType::kReminder, 5000);
                    }

                    if (on_trigger_reminder_) {
                        on_trigger_reminder_(q, trigger_slot);
                    }
                });
            }
        }
    }
}
