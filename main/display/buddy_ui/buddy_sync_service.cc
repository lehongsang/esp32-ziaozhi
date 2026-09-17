#include "buddy_sync_service.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "application.h"
#include "board.h"
#include "settings.h"

#define TAG "BuddySyncService"

BuddySyncService& BuddySyncService::GetInstance() {
    static BuddySyncService instance;
    return instance;
}

BuddySyncService::BuddySyncService() {
    Settings settings("buddy_sync", false);
    device_id_ = settings.GetString("device_id", "default");
    broker_host_ = settings.GetString("broker_host", "192.168.2.19");
    broker_port_ = settings.GetInt("broker_port", 1883);
}

BuddySyncService::~BuddySyncService() {
    Stop();
}

void BuddySyncService::Initialize(TodayQuestScreen* quest_screen, SavingsScreen* savings_screen, FamilyMomentScreen* family_screen) {
    std::lock_guard<std::mutex> lock(mutex_);
    quest_screen_ = quest_screen;
    savings_screen_ = savings_screen;
    family_screen_ = family_screen;
    ESP_LOGI(TAG, "BuddySyncService initialized for device_id: %s", device_id_.c_str());
}

void BuddySyncService::Start(const std::string& broker_host, int broker_port) {
    if (!broker_host.empty()) {
        broker_host_ = broker_host;
    }
    if (broker_port > 0) {
        broker_port_ = broker_port;
    }

    xTaskCreate([](void* arg) {
        auto* self = static_cast<BuddySyncService*>(arg);
        self->ConnectMqtt();
        vTaskDelete(NULL);
    }, "buddy_sync", 4096 * 2, this, 3, nullptr);
}

void BuddySyncService::Stop() {
    if (mqtt_ && is_connected_) {
        mqtt_->Disconnect();
    }
    is_connected_ = false;
}

void BuddySyncService::ConnectMqtt() {
    auto network = Board::GetInstance().GetNetwork();
    if (!network) {
        ESP_LOGE(TAG, "Network not ready, cannot start MQTT");
        return;
    }

    mqtt_ = network->CreateMqtt(1);
    if (!mqtt_) {
        ESP_LOGE(TAG, "Failed to create MQTT client instance");
        return;
    }

    mqtt_->SetKeepAlive(60);

    mqtt_->OnConnected([this]() {
        ESP_LOGI(TAG, "Connected to Buddy Backend MQTT Broker (%s:%d)", broker_host_.c_str(), broker_port_);
        is_connected_ = true;

        std::string prefix = "buddy/" + device_id_ + "/";
        mqtt_->Subscribe(prefix + "quests/set", 0);
        mqtt_->Subscribe(prefix + "savings/set", 0);
        mqtt_->Subscribe(prefix + "family/message", 0);

        // Also subscribe to wildcard default
        if (device_id_ != "default") {
            mqtt_->Subscribe("buddy/default/quests/set", 0);
            mqtt_->Subscribe("buddy/default/savings/set", 0);
            mqtt_->Subscribe("buddy/default/family/message", 0);
        }

        // Send initial online status
        SendHeartbeat(100, 2, 350);
    });

    mqtt_->OnDisconnected([this]() {
        ESP_LOGW(TAG, "Disconnected from Buddy Backend MQTT Broker");
        is_connected_ = false;
    });

    mqtt_->OnMessage([this](const std::string& topic, const std::string& payload) {
        HandleIncomingMqtt(topic, payload);
    });

    ESP_LOGI(TAG, "Connecting to MQTT broker at %s:%d (Client ID: buddy_%s)...",
             broker_host_.c_str(), broker_port_, device_id_.c_str());

    std::string client_id = "buddy_" + device_id_;
    if (!mqtt_->Connect(broker_host_, broker_port_, client_id, "", "")) {
        ESP_LOGE(TAG, "Failed to connect to MQTT broker %s:%d, code=%d",
                 broker_host_.c_str(), broker_port_, mqtt_->GetLastError());
    }
}

void BuddySyncService::HandleIncomingMqtt(const std::string& topic, const std::string& payload) {
    ESP_LOGI(TAG, "Incoming MQTT [%s]: %s", topic.c_str(), payload.c_str());

    cJSON* root = cJSON_Parse(payload.c_str());
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON payload");
        return;
    }

    if (topic.find("/quests/set") != std::string::npos) {
        HandleQuestsPayload(root);
    } else if (topic.find("/savings/set") != std::string::npos) {
        HandleSavingsPayload(root);
    } else if (topic.find("/family/message") != std::string::npos) {
        HandleFamilyMessagePayload(root);
    }

    cJSON_Delete(root);
}

void BuddySyncService::HandleQuestsPayload(cJSON* root) {
    cJSON* quests_array = nullptr;
    if (cJSON_IsArray(root)) {
        quests_array = root;
    } else if (cJSON_IsObject(root)) {
        quests_array = cJSON_GetObjectItem(root, "quests");
    }

    if (!quests_array || !cJSON_IsArray(quests_array)) {
        ESP_LOGE(TAG, "Invalid quests payload");
        return;
    }

    std::vector<QuestItemData> quests;
    int size = cJSON_GetArraySize(quests_array);
    for (int i = 0; i < size; ++i) {
        cJSON* item = cJSON_GetArrayItem(quests_array, i);
        if (!item || !cJSON_IsObject(item)) continue;

        QuestItemData q;
        cJSON* id = cJSON_GetObjectItem(item, "id");
        cJSON* title = cJSON_GetObjectItem(item, "title");
        cJSON* progress = cJSON_GetObjectItem(item, "progress_text");
        cJSON* completed = cJSON_GetObjectItem(item, "completed");

        q.id = (id && cJSON_IsString(id)) ? id->valuestring : ("quest_" + std::to_string(i));
        q.title = (title && cJSON_IsString(title)) ? title->valuestring : "";
        q.progress_text = (progress && cJSON_IsString(progress)) ? progress->valuestring : "";
        q.completed = (completed && cJSON_IsTrue(completed));

        quests.push_back(std::move(q));
    }

    if (quest_screen_ && !quests.empty()) {
        Application::GetInstance().Schedule([this, quests]() {
            if (quest_screen_) {
                quest_screen_->SetQuests(quests);
            }
        });
    }
}

void BuddySyncService::HandleSavingsPayload(cJSON* root) {
    if (!cJSON_IsObject(root)) return;

    cJSON* type_item = cJSON_GetObjectItem(root, "goal_type");
    cJSON* name_item = cJSON_GetObjectItem(root, "goal_name");
    cJSON* current_item = cJSON_GetObjectItem(root, "current_amount");
    cJSON* target_item = cJSON_GetObjectItem(root, "target_amount");
    cJSON* currency_item = cJSON_GetObjectItem(root, "currency");

    int goal_type_int = (type_item && cJSON_IsNumber(type_item)) ? type_item->valueint : 1;
    std::string goal_name = (name_item && cJSON_IsString(name_item)) ? name_item->valuestring : "My New Bike";
    int32_t current_amount = (current_item && cJSON_IsNumber(current_item)) ? (int32_t)current_item->valuedouble : 0;
    int32_t target_amount = (target_item && cJSON_IsNumber(target_item)) ? (int32_t)target_item->valuedouble : 2000000;
    std::string currency = (currency_item && cJSON_IsString(currency_item)) ? currency_item->valuestring : "d";

    GoalType goal_type = static_cast<GoalType>(goal_type_int);

    if (savings_screen_) {
        Application::GetInstance().Schedule([this, goal_type, goal_name, current_amount, target_amount, currency]() {
            if (savings_screen_) {
                savings_screen_->SetGoal(goal_type, goal_name, current_amount, target_amount, currency);
            }
        });
    }
}

void BuddySyncService::HandleFamilyMessagePayload(cJSON* root) {
    if (!cJSON_IsObject(root)) return;

    cJSON* sender_item = cJSON_GetObjectItem(root, "sender");
    cJSON* msg_item = cJSON_GetObjectItem(root, "message");
    cJSON* time_item = cJSON_GetObjectItem(root, "timestamp");

    std::string sender = (sender_item && cJSON_IsString(sender_item)) ? sender_item->valuestring : "Mom";
    std::string message = (msg_item && cJSON_IsString(msg_item)) ? msg_item->valuestring : "";
    std::string timestamp = (time_item && cJSON_IsString(time_item)) ? time_item->valuestring : "Today";

    if (family_screen_) {
        Application::GetInstance().Schedule([this, sender, message, timestamp]() {
            if (family_screen_) {
                family_screen_->SetMessage(sender, message, timestamp);
            }
        });
    }
}

void BuddySyncService::NotifyQuestCompleted(const std::string& quest_id) {
    if (!mqtt_ || !is_connected_) return;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "quest_id", quest_id.c_str());
    cJSON_AddStringToObject(root, "device_id", device_id_.c_str());
    cJSON_AddBoolToObject(root, "completed", true);

    char* json_str = cJSON_PrintUnformatted(root);
    std::string topic = "buddy/" + device_id_ + "/quests/completed";
    mqtt_->Publish(topic, json_str, 0);

    cJSON_free(json_str);
    cJSON_Delete(root);
}

void BuddySyncService::NotifyFamilyLove() {
    if (!mqtt_ || !is_connected_) return;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", device_id_.c_str());
    cJSON_AddBoolToObject(root, "liked", true);

    char* json_str = cJSON_PrintUnformatted(root);
    std::string topic = "buddy/" + device_id_ + "/family/like";
    mqtt_->Publish(topic, json_str, 0);

    cJSON_free(json_str);
    cJSON_Delete(root);
}

void BuddySyncService::NotifyGoalChanged(int goal_type, const std::string& goal_name, int32_t target_amount) {
    if (!mqtt_ || !is_connected_) return;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", device_id_.c_str());
    cJSON_AddNumberToObject(root, "goal_type", goal_type);
    cJSON_AddStringToObject(root, "goal_name", goal_name.c_str());
    cJSON_AddNumberToObject(root, "target_amount", target_amount);

    char* json_str = cJSON_PrintUnformatted(root);
    std::string topic = "buddy/" + device_id_ + "/savings/goal_changed";
    mqtt_->Publish(topic, json_str, 0);

    cJSON_free(json_str);
    cJSON_Delete(root);
}

void BuddySyncService::SendHeartbeat(int battery, int level, int xp) {
    if (!mqtt_ || !is_connected_) return;

    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", device_id_.c_str());
    cJSON_AddStringToObject(root, "status", "online");
    cJSON_AddNumberToObject(root, "battery", battery);
    cJSON_AddNumberToObject(root, "level", level);
    cJSON_AddNumberToObject(root, "xp", xp);

    char* json_str = cJSON_PrintUnformatted(root);
    std::string topic = "buddy/" + device_id_ + "/status";
    mqtt_->Publish(topic, json_str, 0);

    cJSON_free(json_str);
    cJSON_Delete(root);
}
