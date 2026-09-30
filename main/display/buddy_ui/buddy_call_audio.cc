#include "buddy_call_audio.h"
#include "application.h"
#include "board.h"
#include "audio_codec.h"
#include <esp_log.h>
#include <vector>
#include <cstring>

#define TAG "BuddyCallAudio"

BuddyCallAudioService& BuddyCallAudioService::GetInstance() {
    static BuddyCallAudioService instance;
    return instance;
}

BuddyCallAudioService::BuddyCallAudioService() {}

BuddyCallAudioService::~BuddyCallAudioService() {
    StopCallAudio();
}

void BuddyCallAudioService::StartCallAudio(const std::string& host, int port, const std::string& device_id) {
    if (is_running_.load()) {
        ESP_LOGW(TAG, "Call Audio is already active");
        return;
    }
    is_running_.store(true);
    host_ = host;
    port_ = port;
    device_id_ = device_id;

    ESP_LOGI(TAG, "Starting Call Audio for device %s...", device_id_.c_str());

    // 1. Temporarily disable wake word detection and voice processing to prevent I2S conflict
    Application::GetInstance().GetAudioService().EnableWakeWordDetection(false);
    Application::GetInstance().GetAudioService().EnableVoiceProcessing(false);

    // 2. Prepare WebSocket URL
    std::string scheme = (port == 443) ? "wss://" : "ws://";
    std::string url;
    if (port == 80 || port == 443) {
        url = scheme + host + "/call?type=device&deviceId=" + device_id;
    } else {
        url = scheme + host + ":" + std::to_string(port) + "/call?type=device&deviceId=" + device_id;
    }

    auto network = Board::GetInstance().GetNetwork();
    if (!network) {
        ESP_LOGE(TAG, "Network not ready, cannot create Call WebSocket");
        is_running_.store(false);
        return;
    }

    websocket_ = network->CreateWebSocket(1);
    if (!websocket_) {
        ESP_LOGE(TAG, "Failed to create WebSocket instance for Call Audio");
        is_running_.store(false);
        return;
    }

    websocket_->OnConnected([]() {
        ESP_LOGI(TAG, "📞 Call Audio WebSocket Connected to Relay!");
    });

    websocket_->OnData([this](const char* data, size_t len, bool binary) {
        if (!is_running_.load()) return;
        if (binary) {
            HandleIncomingAudio(data, len);
        }
    });

    websocket_->OnDisconnected([]() {
        ESP_LOGW(TAG, "Call Audio WebSocket Disconnected");
    });

    ESP_LOGI(TAG, "Connecting Call Audio WS to %s...", url.c_str());
    if (!websocket_->Connect(url.c_str())) {
        ESP_LOGE(TAG, "Failed to connect to Call Audio WebSocket: %s", url.c_str());
    }

    // 3. Start Microphone Streaming Task
    xTaskCreate([](void* arg) {
        auto* self = static_cast<BuddyCallAudioService*>(arg);
        self->MicTask();
        vTaskDelete(NULL);
    }, "call_mic", 4096, this, 6, &mic_task_handle_);
}

void BuddyCallAudioService::StopCallAudio() {
    if (!is_running_.load()) {
        return;
    }
    is_running_.store(false);

    if (websocket_) {
        websocket_->Close();
        websocket_.reset();
    }

    // Give Mic task time to exit cleanly
    vTaskDelay(pdMS_TO_TICKS(60));
    mic_task_handle_ = nullptr;

    // Restore idle state for wake word detection
    Application::GetInstance().GetAudioService().EnableWakeWordDetection(true);

    ESP_LOGI(TAG, "Call Audio stopped and restored idle audio pipeline");
}

void BuddyCallAudioService::MicTask() {
    auto codec = Board::GetInstance().GetAudioCodec();
    if (!codec) {
        ESP_LOGE(TAG, "No audio codec found!");
        return;
    }

    if (!codec->input_enabled()) {
        codec->EnableInput(true);
    }

    // 16kHz Mono: 320 samples = 20ms of audio (640 bytes)
    const int chunk_samples = 320;
    int in_channels = codec->input_channels();
    std::vector<int16_t> input_buf(chunk_samples * in_channels);

    ESP_LOGI(TAG, "Call Mic Task started (channels: %d, rate: %d)", in_channels, codec->input_sample_rate());

    while (is_running_.load()) {
        if (codec->InputData(input_buf)) {
            if (websocket_ && websocket_->IsConnected()) {
                if (in_channels == 2) {
                    // Extract left channel for mono
                    std::vector<int16_t> mono_buf(chunk_samples);
                    for (int i = 0; i < chunk_samples; ++i) {
                        mono_buf[i] = input_buf[i * 2];
                    }
                    websocket_->Send(mono_buf.data(), mono_buf.size() * sizeof(int16_t), true);
                } else {
                    websocket_->Send(input_buf.data(), input_buf.size() * sizeof(int16_t), true);
                }
            }
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    ESP_LOGI(TAG, "Call Mic Task exited");
}

void BuddyCallAudioService::HandleIncomingAudio(const char* data, size_t len) {
    if (!data || len < sizeof(int16_t)) return;

    auto codec = Board::GetInstance().GetAudioCodec();
    if (!codec) return;

    if (!codec->output_enabled()) {
        codec->EnableOutput(true);
    }

    size_t sample_count = len / sizeof(int16_t);
    const int16_t* pcm_in = reinterpret_cast<const int16_t*>(data);

    if (codec->output_channels() == 2) {
        // Expand Mono from parent browser to Stereo for 2-channel DAC/I2S
        std::vector<int16_t> stereo_buf(sample_count * 2);
        for (size_t i = 0; i < sample_count; ++i) {
            stereo_buf[i * 2] = pcm_in[i];
            stereo_buf[i * 2 + 1] = pcm_in[i];
        }
        codec->OutputData(stereo_buf);
    } else {
        std::vector<int16_t> mono_buf(pcm_in, pcm_in + sample_count);
        codec->OutputData(mono_buf);
    }
}
