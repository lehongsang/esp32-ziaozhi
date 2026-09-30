#include "buddy_reminder_scheduler.h"
#include <esp_log.h>
#include <ctime>
#include "application.h"
#include "buddy_toast_overlay.h"
#include "assets/lang_config.h"

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
    if (!tm_info || tm_info->tm_year < (2025 - 1900)) return;

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

        if (diff == q.remind_before && q.remind_before > 0) {
            trigger_slot = q.remind_before;
        } else if (diff == 0) {
            trigger_slot = 0;
        }

        if (trigger_slot >= 0) {
            std::string key = q.id + "_" + std::to_string(trigger_slot);
            if (triggered_reminders_.find(key) == triggered_reminders_.end()) {
                triggered_reminders_.insert(key);
                ESP_LOGI(TAG, "Triggering alarm reminder for '%s' (%s) at %d min before deadline (%s)",
                         q.title.c_str(), q.id.c_str(), trigger_slot, q.scheduled_time.c_str());

                Application::GetInstance().Schedule([this, q, trigger_slot]() {
                    // Play alarm audio chime
                    Application::GetInstance().PlaySound(Lang::Sounds::OGG_POPUP);

                    if (trigger_slot == 0) {
                        std::string title = "⏰ Đã Đến Hạn Làm Bài!";
                        std::string body = "Đến hạn " + q.title + " rồi con ơi! Bấm để làm ngay! ✨";
                        BuddyToastOverlay::GetInstance().Show(title, body, ToastType::kReminder, 10000);
                    } else {
                        std::string title = "⏰ Sắp Đến Giờ Làm Bài!";
                        std::string body = "Con nhớ làm " + q.title + " trước " + q.scheduled_time + " nhé! (Còn " + std::to_string(trigger_slot) + "p)";
                        BuddyToastOverlay::GetInstance().Show(title, body, ToastType::kReminder, 8000);
                    }

                    if (on_trigger_reminder_) {
                        on_trigger_reminder_(q, trigger_slot);
                    }
                });
            }
        }
    }
}
