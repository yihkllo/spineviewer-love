#ifndef SLPRO_COMMON_MODULE_AUDIO_H_
#define SLPRO_COMMON_MODULE_AUDIO_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct IXAudio2;
struct IXAudio2MasteringVoice;
struct IXAudio2SourceVoice;

namespace slaudio
{

	struct pcm_clip
	{
		std::uint32_t sample_rate = 0;
		std::uint16_t channels = 0;
		std::uint16_t bits = 0;
		std::uint16_t block_align = 0;
		std::vector<std::uint8_t> samples;

		bool empty() const noexcept { return samples.empty(); }
	};

	class audio_deck
	{
	public:
		audio_deck();
		~audio_deck();

		audio_deck(const audio_deck&) = delete;
		audio_deck& operator=(const audio_deck&) = delete;

		bool boot();
		void shut();
		bool ready() const noexcept { return m_x2 != nullptr; }

		bool bgm_start(const std::wstring& file_path, float volume);
		void bgm_stop();
		void bgm_set_volume(float v);
		bool bgm_is_active() const;

		bool voice_start(const std::wstring& file_path, float volume, bool loop);
		void voice_stop();
		void voice_set_volume(float v);
		bool voice_is_active() const;
		bool voice_is_ended() const;
		std::uint64_t voice_elapsed_ms() const;
		float voice_level() const;

		bool se_fire(const std::wstring& file_path, float volume);

		void se_stop_all();

		using voice_end_callback = std::function<void()>;
		void on_voice_end(voice_end_callback cb) { m_voice_end_cb = std::move(cb); }

	private:
		struct voice_sink;
		struct se_slot;

		std::shared_ptr<const pcm_clip> decode_or_cache(const std::wstring& path);

		void release_bgm_voice();
		void release_voice_voice();

		IXAudio2* m_x2 = nullptr;
		IXAudio2MasteringVoice* m_master = nullptr;
		bool m_mf_started = false;

		IXAudio2SourceVoice* m_bgm_voice = nullptr;
		std::shared_ptr<const pcm_clip> m_bgm_clip;
		std::unique_ptr<voice_sink> m_bgm_sink;
		float m_bgm_volume = 0.5f;
		std::atomic<bool> m_bgm_active{ false };

		IXAudio2SourceVoice* m_voice_voice = nullptr;
		std::shared_ptr<const pcm_clip> m_voice_clip;
		std::unique_ptr<voice_sink> m_voice_sink;
		float m_voice_volume = 1.0f;
		std::atomic<bool> m_voice_active{ false };
		std::atomic<bool> m_voice_ended{ false };
		std::uint64_t m_voice_start_tick = 0;

		std::vector<std::unique_ptr<se_slot>> m_se_slots;

		mutable std::mutex m_cache_mutex;
		std::unordered_map<std::wstring, std::shared_ptr<const pcm_clip>> m_cache;

		voice_end_callback m_voice_end_cb;
	};
}

#endif
