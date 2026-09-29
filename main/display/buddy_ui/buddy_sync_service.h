#ifndef BUDDY_SYNC_SERVICE_H
#define BUDDY_SYNC_SERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <mqtt.h>
#include <cJSON.h>

#include "buddy_home_screen.h"
#include "today_quest_screen.h"
#include "savings_screen.h"
#include "family_moment_screen.h"

class BuddySyncService {
public:
    static BuddySyncService& GetInstance();

    void Initialize(BuddyHomeScreen* home_screen, TodayQuestScreen* quest_screen, SavingsScreen* savings_screen, FamilyMomentScreen* family_screen);
    void Start(const std::string& broker_host = "", int broker_port = 0);
    void Stop();

    // Outbound notifications from Device -> Server
    void NotifyQuestCompleted(const std::string& quest_id);
    void NotifyFamilyLove();
    void NotifyGoalChanged(int goal_type, const std::string& goal_name, int32_t target_amount);
    void SendHeartbeat(int battery, int level, int xp);

    bool IsConnected() const { return is_connected_; }

private:
    BuddySyncService();
    ~BuddySyncService();
    BuddySyncService(const BuddySyncService&) = delete;
    BuddySyncService& operator=(const BuddySyncService&) = delete;

    void ConnectMqtt();
    void HandleIncomingMqtt(const std::string& topic, const std::string& payload);
    void HandleQuestsPayload(cJSON* root);
    void HandleSavingsPayload(cJSON* root);
    void HandleFamilyMessagePayload(cJSON* root);

    BuddyHomeScreen* home_screen_ = nullptr;
    TodayQuestScreen* quest_screen_ = nullptr;
    SavingsScreen* savings_screen_ = nullptr;
    FamilyMomentScreen* family_screen_ = nullptr;

    std::unique_ptr<Mqtt> mqtt_;
    std::string device_id_ = "default";
    std::string broker_host_ = "";
    int broker_port_ = 1883;
    std::atomic<bool> is_connected_{false};
    std::mutex mutex_;
};

#endif // BUDDY_SYNC_SERVICE_H
