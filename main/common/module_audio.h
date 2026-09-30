#ifndef SLPRO_COMMON_MODULE_AUDIO_H_
#define SLPRO_COMMON_MODULE_AUDIO_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

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
		bool ready() const noexcept { return m_engine != nullptr; }

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
		struct engine;
		struct state;

		std::shared_ptr<engine> m_engine;
		std::shared_ptr<state> m_state;
		voice_end_callback m_voice_end_cb;
	};
}

#endif
