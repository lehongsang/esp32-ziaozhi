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

void BuddyCallAudioService::StartCallAudio(SendCallback send_fn, const std::string& device_id) {
    if (is_running_.load()) {
        ESP_LOGW(TAG, "Call Audio is already active");
        return;
    }
    is_running_.store(true);
    send_fn_ = send_fn;
    device_id_ = device_id;
    up_topic_ = "buddy/" + device_id_ + "/call/audio/up";

    ESP_LOGI(TAG, "Starting Audio-over-MQTT for device %s (Topic: %s)...", device_id_.c_str(), up_topic_.c_str());

    auto codec = Board::GetInstance().GetAudioCodec();
    if (codec) {
        codec->SetOutputVolume(95);
        if (!codec->output_enabled()) {
            codec->EnableOutput(true);
        }
    }

    // 1. Temporarily disable wake word detection and voice processing to prevent I2S conflict
    Application::GetInstance().GetAudioService().EnableWakeWordDetection(false);
    Application::GetInstance().GetAudioService().EnableVoiceProcessing(false);

    // 2. Start Microphone Streaming Task (lightweight, zero TLS memory allocated)
    xTaskCreate([](void* arg) {
        auto* self = static_cast<BuddyCallAudioService*>(arg);
        self->MicTask();
        vTaskDelete(NULL);
    }, "call_mic", 3584, this, 5, &mic_task_handle_);
}

void BuddyCallAudioService::StopCallAudio() {
    if (!is_running_.load()) {
        return;
    }
    is_running_.store(false);

    // Give Mic task time to exit cleanly
    vTaskDelay(pdMS_TO_TICKS(50));
    mic_task_handle_ = nullptr;
    send_fn_ = nullptr;

    // Restore idle state for wake word detection
    Application::GetInstance().GetAudioService().EnableWakeWordDetection(true);

    ESP_LOGI(TAG, "Audio-over-MQTT stopped and restored idle audio pipeline");
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

    int sample_rate = codec->input_sample_rate();
    if (sample_rate <= 0) sample_rate = 16000;
    // 40ms chunk = sample_rate / 25 samples (optimal for MQTT throughput and low latency)
    const int chunk_samples = sample_rate / 25;
    int in_channels = codec->input_channels();
    std::vector<int16_t> input_buf(chunk_samples * in_channels);

    ESP_LOGI(TAG, "Call Mic Task started (channels: %d, rate: %d, chunk: %d)", in_channels, sample_rate, chunk_samples);

    while (is_running_.load()) {
        if (codec->InputData(input_buf)) {
            if (send_fn_) {
                if (in_channels == 2) {
                    // Extract left channel for mono & apply 2.0x mic boost
                    std::vector<int16_t> mono_buf(chunk_samples);
                    for (int i = 0; i < chunk_samples; ++i) {
                        int32_t val = static_cast<int32_t>(input_buf[i * 2]) * 2;
                        if (val > 32767) val = 32767;
                        if (val < -32768) val = -32768;
                        mono_buf[i] = static_cast<int16_t>(val);
                    }
                    send_fn_(up_topic_, std::string(reinterpret_cast<const char*>(mono_buf.data()), mono_buf.size() * sizeof(int16_t)));
                } else {
                    std::vector<int16_t> mono_buf(chunk_samples);
                    for (int i = 0; i < chunk_samples; ++i) {
                        int32_t val = static_cast<int32_t>(input_buf[i]) * 2;
                        if (val > 32767) val = 32767;
                        if (val < -32768) val = -32768;
                        mono_buf[i] = static_cast<int16_t>(val);
                    }
                    send_fn_(up_topic_, std::string(reinterpret_cast<const char*>(mono_buf.data()), mono_buf.size() * sizeof(int16_t)));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
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
        // Expand Mono to Stereo for DAC/I2S 2-channel output
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
