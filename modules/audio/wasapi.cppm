module;

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <audiopolicy.h>

export module nfwk.audio:wasapi;

import std.core;
import std.threading;
import nfwk.core;
import :interfaces;

struct IMMDevice;
struct IAudioClient;
struct IAudioRenderClient;
struct IAudioSessionManager;
struct ISimpleAudioVolume;
struct IAudioClockAdjustment;
struct tWAVEFORMATEX;
using WAVEFORMATEX = tWAVEFORMATEX;

namespace nfwk::wasapi::device_enumerator {

static IMMDeviceEnumerator* device_enumerator{ nullptr };

static void create() {
	if (device_enumerator) {
		return;
	}
	const HRESULT result{ CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&device_enumerator) };
	if (result != S_OK) {
		warning("audio", "Failed to create device enumerator. Error: {0:x}", result);
	}
}

static void release() {
	if (device_enumerator) {
		device_enumerator->Release();
		device_enumerator = nullptr;
	}
}

static auto get() {
	return device_enumerator;
}

}

namespace nfwk::wasapi {

static pcm_format wave_format_to_audio_data_format(const WAVEFORMATEX* wave_format) {
	const auto* extended_format = reinterpret_cast<const WAVEFORMATEXTENSIBLE*>(wave_format);
	switch (wave_format->wFormatTag) {
	case WAVE_FORMAT_IEEE_FLOAT:
		return pcm_format::float_32;
	case WAVE_FORMAT_PCM:
		return pcm_format::int_16;
	case WAVE_FORMAT_EXTENSIBLE:
		if (memcmp(&extended_format->SubFormat, &KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, sizeof(GUID)) == 0) {
			return pcm_format::float_32;
		} else if (memcmp(&extended_format->SubFormat, &KSDATAFORMAT_SUBTYPE_PCM, sizeof(GUID)) == 0) {
			return pcm_format::int_16;
		} else {
			return pcm_format::unknown;
		}
	default:
		warning("audio", "Unknown wave format: {}", wave_format->wFormatTag);
		return pcm_format::unknown;
	}
}

}

export namespace nfwk::wasapi {

class audio_client : public audio_player {
public:

	audio_client(IMMDevice* device) : playing_audio_stream{ nullptr } {
		if (const auto result = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&client); result != S_OK) {
			warning("audio", "Failed to activate audio client. Error: {}", result);
			return;
		}
		if (const auto result = device->Activate(__uuidof(IAudioSessionManager), CLSCTX_ALL, nullptr, (void**)&session_manager); result != S_OK) {
			warning("audio", "Failed to activate session manager. Error: {}", result);
			return;
		}
		if (const auto result = session_manager->GetSimpleAudioVolume(nullptr, FALSE, &audio_volume); result != S_OK || !audio_volume) {
			warning("audio", "Failed to get simple audio volume interface. Error: {}", result);
			return;
		}
		if (const auto result = client->GetMixFormat(&wave_format); result != S_OK) {
			warning("audio", "Failed to get mix format. Error: {}", result);
			return;
		}
		if (const auto result = client->GetDevicePeriod(&default_device_period, nullptr); result != S_OK) {
			warning("audio", "Failed to get device period. Error: {}", result);
			return;
		}
		const DWORD stream_flags{ AUDCLNT_STREAMFLAGS_RATEADJUST /*| AUDCLNT_STREAMFLAGS_EVENTCALLBACK*/ };
		if (const auto result = client->Initialize(AUDCLNT_SHAREMODE_SHARED, stream_flags, default_device_period, 0, wave_format, nullptr); result != S_OK) {
			warning("audio", "Failed to initialize the audio client. Error: {}", result);
			return;
		}
		if (const auto result = client->GetBufferSize(&buffer_frame_count); result != S_OK) {
			warning("audio", "Failed to get buffer size. Error: {}", result);
			return;
		}
		if (const auto result = client->GetService(__uuidof(IAudioRenderClient), (void**)&render_client); result != S_OK) {
			warning("audio", "Failed to get audio render client. Error: {}", result);
			return;
		}
		if (const auto result = client->GetService(__uuidof(IAudioClockAdjustment), (void**)&clock_adjustment); result != S_OK) {
			warning("audio", "Failed to get audio clock adjustment. Audio might play at wrong rate. Error: {}", result);
		}
		info("audio", "[b]WASAPI Audio Client[/b]\n[b]Device Period:[/b] {}\n[b]Buffer Frame Count:[/b] {}", default_device_period, buffer_frame_count);
	}

	audio_client(const audio_client&) = delete;
	audio_client(audio_client&&) = delete;

	~audio_client() override {
		stop();
		CoTaskMemFree(wave_format);
		if (client) {
			client->Release();
		}
		if (render_client) {
			render_client->Release();
		}
		if (audio_volume) {
			audio_volume->Release();
		}
		if (clock_adjustment) {
			clock_adjustment->Release();
		}
	}

	audio_client& operator=(const audio_client&) = delete;
	audio_client& operator=(audio_client&&) = delete;

	void play(const pcm_stream& new_audio_stream) override {
		stop();
		playing = true;
		playing_audio_stream = new_audio_stream;
		thread = std::thread{ &audio_client::stream, this };
	}

	void play(audio_source* source) override {
		play(pcm_stream{ source });
	}

	void pause() override {
		paused = true;
	}

	void resume() override {
		paused = false;
	}

	void stop() override {
		playing = false;
		if (thread.joinable()) {
			thread.join();
		}
		playing_audio_stream.reset();
		if (const auto result = client->Reset(); result != S_OK) {
			warning("audio", "Failed to reset the audio client. Error: {}", result);
		}
		paused = false;
	}

	void loop() override {
		looping = true;
	}

	void once() override {
		looping = false;
	}

	void set_volume(float volume) override {
		if (audio_volume) {
			audio_volume->SetMasterVolume(volume, nullptr);
		}
	}

	bool is_playing() const override {
		return playing;
	}

	bool is_paused() const override {
		return paused;
	}

	bool is_looping() const override {
		return looping;
	}

private:

	void stream() {
		if (playing_audio_stream.is_empty()) {
			warning("audio", "Audio source is empty.");
			return;
		}
		upload(buffer_frame_count);
		if (const HRESULT result{ client->Start() }; result != S_OK) {
			warning("audio", "Failed to play audio. Error: {}", result);
			return;
		}
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
		clock_adjustment->SetSampleRate(static_cast<float>(playing_audio_stream.sample_rate()));
		const int one_millisecond{ 10000 };
		const int sleep_period_ms{ static_cast<int>(default_device_period) / one_millisecond / 2 };
		bool has_been_stopped{ false };
		while (true) {
			if (!playing) {
				client->Stop();
				return;
			}
			if (playing_audio_stream.is_empty()) {
				if (looping) {
					playing_audio_stream.reset();
					client->Reset();
					client->Start();
					upload(buffer_frame_count);
				} else {
					break;
				}
			}
			sleep(sleep_period_ms);
			if (paused) {
				if (!has_been_stopped) {
					has_been_stopped = true;
					client->Stop();
				}
				continue;
			}
			if (has_been_stopped) {
				has_been_stopped = false;
				client->Start();
			}
			UINT32 padding_frames{ 0 };
			if (const HRESULT result{ client->GetCurrentPadding(&padding_frames) }; result != S_OK) {
				warning("audio", "Failed to get padding. Error: {}", result);
				break;
			}
			const int frames_available{ static_cast<int>(buffer_frame_count) - static_cast<int>(padding_frames) };
			upload(frames_available);
		}
		sleep(sleep_period_ms);
		client->Stop();
		playing = false;
	}

	void upload(unsigned int frames) {
		BYTE* buffer{ nullptr };
		if (const HRESULT result{ render_client->GetBuffer(frames, &buffer) }; result != S_OK) {
			warning("audio", "Failed to get buffer. Error: {}", result);
			return;
		}
		const auto format = wave_format_to_audio_data_format(wave_format);
		const std::size_t size{ frames * wave_format->nBlockAlign };
		const std::size_t channels{ wave_format->nChannels };
		playing_audio_stream.stream(format, buffer, size, channels);
		if (const HRESULT result{ render_client->ReleaseBuffer(frames, 0) }; result != S_OK) {
			warning("audio", "Failed to release buffer. Error: {}", result);
		}
	}

	std::thread thread;
	pcm_stream playing_audio_stream;

	std::atomic<bool> playing{ false };
	std::atomic<bool> paused{ false };
	std::atomic<bool> looping{ false };

	IAudioClient* client{ nullptr };
	IAudioRenderClient* render_client{ nullptr };
	IAudioSessionManager* session_manager{ nullptr };
	ISimpleAudioVolume* audio_volume{ nullptr };
	IAudioClockAdjustment* clock_adjustment{ nullptr };
	long long default_device_period{ 0 };
	unsigned int buffer_frame_count{ 0 };
	WAVEFORMATEX* wave_format{ nullptr };

};

class audio_device : public audio_endpoint {
public:

	audio_device() {
		message("audio", "Initializing audio device");
		device_enumerator::create();
		if (const HRESULT result{ device_enumerator::get()->GetDefaultAudioEndpoint(eRender, eConsole, &device) }; result != S_OK) {
			warning("audio", "Failed to get default audio endpoint. Error: {}", result);
		}
	}
	~audio_device() override {
		clear_players();
		if (device) {
			device->Release();
		}
		device_enumerator::release();
	}

	audio_player* add_player() override {
		message("audio", "Adding audio player");
		return players.emplace_back(new audio_client{ device });
	}

	void stop_all_players() override {
		message("audio", "Stopping all audio players");
		for (auto& player : players) {
			player->stop();
		}
	}

	void clear_players() override {
		message("audio", "Clearing all audio players");
		for (auto& player : players) {
			delete player;
		}
		players.clear();
	}

private:

	IMMDevice* device{ nullptr };
	std::vector<audio_client*> players;

};

}
