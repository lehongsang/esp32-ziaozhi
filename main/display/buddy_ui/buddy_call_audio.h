#ifndef BUDDY_CALL_AUDIO_H
#define BUDDY_CALL_AUDIO_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <memory>
#include <atomic>
#include <web_socket.h>

class BuddyCallAudioService {
public:
    static BuddyCallAudioService& GetInstance();

    void StartCallAudio(const std::string& host, int port, const std::string& device_id);
    void StopCallAudio();
    bool IsInCall() const { return is_running_.load(); }

private:
    BuddyCallAudioService();
    ~BuddyCallAudioService();

    void MicTask();
    void HandleIncomingAudio(const char* data, size_t len);

    std::unique_ptr<WebSocket> websocket_;
    TaskHandle_t mic_task_handle_ = nullptr;
    std::atomic<bool> is_running_{false};
    std::string device_id_;
    std::string host_;
    int port_ = 0;
};

#endif // BUDDY_CALL_AUDIO_H
