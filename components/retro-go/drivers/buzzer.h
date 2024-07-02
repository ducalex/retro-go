#pragma once

void setup_buzzer(int sampleRate);
void stop_buzzer();
void rg_audio_submit_buzzer(const rg_audio_frame_t *frames, size_t count);
void rg_audio_set_mute_buzzer(bool mute);
