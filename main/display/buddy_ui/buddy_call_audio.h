#ifndef BUDDY_CALL_AUDIO_H
#define BUDDY_CALL_AUDIO_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <functional>
#include <atomic>

class BuddyCallAudioService {
public:
    static BuddyCallAudioService& GetInstance();

    using SendCallback = std::function<void(const std::string& topic, const std::string& payload)>;

    void StartCallAudio(SendCallback send_fn, const std::string& device_id);
    void StopCallAudio();
    bool IsInCall() const { return is_running_.load(); }
    void HandleIncomingAudio(const char* data, size_t len);

private:
    BuddyCallAudioService();
    ~BuddyCallAudioService();

    void MicTask();

    SendCallback send_fn_;
    TaskHandle_t mic_task_handle_ = nullptr;
    std::atomic<bool> is_running_{false};
    std::string device_id_;
    std::string up_topic_;
};

#endif // BUDDY_CALL_AUDIO_H
