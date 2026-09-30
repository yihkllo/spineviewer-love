#include "module_audio.h"

#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSink>
#include <QCoreApplication>
#include <QIODevice>
#include <QMediaDevices>
#include <QThread>
#include <QUrl>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cwctype>
#include <deque>
#include <unordered_map>

namespace slaudio
{

	namespace
	{
		constexpr std::size_t kMaxEffects = 8;

		std::wstring lc(std::wstring s)
		{
			for (wchar_t& c : s) c = static_cast<wchar_t>(std::towlower(c));
			return s;
		}

		std::int64_t now_ms()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}

		void append_int16(const QAudioBuffer& buffer, std::vector<std::uint8_t>& out)
		{
			const QAudioFormat format = buffer.format();
			const qsizetype count = buffer.sampleCount();
			const std::size_t base = out.size();
			out.resize(base + static_cast<std::size_t>(count) * 2);
			auto* target = reinterpret_cast<std::int16_t*>(out.data() + base);
			switch (format.sampleFormat())
			{
			case QAudioFormat::Int16:
				std::memcpy(target, buffer.constData<std::int16_t>(), static_cast<std::size_t>(count) * 2);
				break;
			case QAudioFormat::UInt8:
			{
				const auto* source = buffer.constData<std::uint8_t>();
				for (qsizetype i = 0; i < count; ++i) target[i] = static_cast<std::int16_t>((static_cast<int>(source[i]) - 128) << 8);
				break;
			}
			case QAudioFormat::Int32:
			{
				const auto* source = buffer.constData<std::int32_t>();
				for (qsizetype i = 0; i < count; ++i) target[i] = static_cast<std::int16_t>(source[i] >> 16);
				break;
			}
			case QAudioFormat::Float:
			{
				const auto* source = buffer.constData<float>();
				for (qsizetype i = 0; i < count; ++i)
					target[i] = static_cast<std::int16_t>(std::lround(std::clamp(source[i], -1.0f, 1.0f) * 32767.0f));
				break;
			}
			default:
				out.resize(base);
				break;
			}
		}

		class clip_device final : public QIODevice
		{
		public:
			clip_device(std::shared_ptr<const pcm_clip> clip, bool loop) : m_clip(std::move(clip)), m_loop(loop) { open(QIODevice::ReadOnly); }
			bool isSequential() const override { return true; }
			bool finished() const noexcept { return !m_loop && m_position >= m_clip->samples.size(); }
			qint64 bytesAvailable() const override
			{
				const qint64 left = m_loop ? qint64(1) << 30 : static_cast<qint64>(m_clip->samples.size() - m_position);
				return left + QIODevice::bytesAvailable();
			}

		protected:
			qint64 readData(char* data, qint64 maximum) override
			{
				const auto& samples = m_clip->samples;
				qint64 done = 0;
				while (done < maximum)
				{
					if (m_position >= samples.size())
					{
						if (!m_loop || samples.empty()) break;
						m_position = 0;
					}
					const qint64 count = (std::min)(maximum - done, static_cast<qint64>(samples.size() - m_position));
					std::memcpy(data + done, samples.data() + m_position, static_cast<std::size_t>(count));
					m_position += static_cast<std::size_t>(count);
					done += count;
				}
				return done;
			}
			qint64 writeData(const char*, qint64) override { return -1; }

		private:
			std::shared_ptr<const pcm_clip> m_clip;
			bool m_loop = false;
			std::size_t m_position = 0;
		};

		struct player
		{
			std::unique_ptr<clip_device> device;
			std::unique_ptr<QAudioSink> sink;
		};

		using clip_ref = std::shared_ptr<const pcm_clip>;
		using clip_waiter = std::function<void(const clip_ref&)>;

		struct decode_job
		{
			std::unique_ptr<QAudioDecoder> decoder;
			std::shared_ptr<pcm_clip> clip;
			std::vector<clip_waiter> waiters;
			bool fallback = false;
		};
	}

	struct audio_deck::state
	{
		std::atomic<bool> bgm_active{ false };
		std::atomic<bool> voice_active{ false };
		std::atomic<bool> voice_ended{ false };
		std::atomic<std::uint64_t> bgm_token{ 0 };
		std::atomic<std::uint64_t> voice_token{ 0 };
		std::atomic<std::int64_t> voice_start{ 0 };
		mutable std::mutex mutex;
		clip_ref voice_clip;
		std::atomic<float> bgm_volume{ 0.5f };
		std::atomic<float> voice_volume{ 1.0f };
	};

	struct audio_deck::engine
	{
		QThread thread;
		QObject* context = nullptr;
		std::unordered_map<std::wstring, clip_ref> cache;
		std::unordered_map<std::wstring, std::unique_ptr<decode_job>> jobs;
		std::unique_ptr<player> bgm;
		std::unique_ptr<player> voice;
		std::deque<std::unique_ptr<player>> effects;

		template <class F>
		void post(F&& function) { QMetaObject::invokeMethod(context, std::forward<F>(function), Qt::QueuedConnection); }

		void start_decode(const std::wstring& key, decode_job& job, const std::wstring& path)
		{
			job.decoder = std::make_unique<QAudioDecoder>();
			job.clip = std::make_shared<pcm_clip>();
			if (!job.fallback)
			{
				QAudioFormat format = QMediaDevices::defaultAudioOutput().preferredFormat();
				format.setSampleFormat(QAudioFormat::Int16);
				if (format.isValid()) job.decoder->setAudioFormat(format);
			}
			QAudioDecoder* decoder = job.decoder.get();
			QObject::connect(decoder, &QAudioDecoder::bufferReady, context, [this, key]() {
				const auto found = jobs.find(key);
				if (found == jobs.end()) return;
				decode_job& current = *found->second;
				const QAudioBuffer buffer = current.decoder->read();
				if (!buffer.isValid()) return;
				const QAudioFormat format = buffer.format();
				if (current.clip->sample_rate == 0)
				{
					current.clip->sample_rate = static_cast<std::uint32_t>(format.sampleRate());
					current.clip->channels = static_cast<std::uint16_t>(format.channelCount());
					current.clip->bits = 16;
					current.clip->block_align = static_cast<std::uint16_t>(format.channelCount() * 2);
				}
				append_int16(buffer, current.clip->samples);
			});
			QObject::connect(decoder, &QAudioDecoder::finished, context, [this, key]() { finish_decode(key, true); });
			QObject::connect(decoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), context, [this, key, path](QAudioDecoder::Error) {
				const auto found = jobs.find(key);
				if (found == jobs.end()) return;
				decode_job& current = *found->second;
				if (!current.fallback && current.clip->samples.empty())
				{
					current.fallback = true;
					auto retired = std::move(current.decoder);
					retired->disconnect(context);
					retired.release()->deleteLater();
					start_decode(key, current, path);
					return;
				}
				finish_decode(key, false);
			});
			decoder->setSource(QUrl::fromLocalFile(QString::fromStdWString(path)));
			decoder->start();
		}

		void finish_decode(const std::wstring& key, bool success)
		{
			const auto found = jobs.find(key);
			if (found == jobs.end()) return;
			std::unique_ptr<decode_job> job = std::move(found->second);
			jobs.erase(found);
			job->decoder->disconnect(context);
			job->decoder.release()->deleteLater();
			clip_ref clip;
			if (success && !job->clip->empty() && job->clip->sample_rate != 0 && job->clip->channels != 0)
			{
				clip = job->clip;
				cache.emplace(key, clip);
			}
			for (const auto& waiter : job->waiters) waiter(clip);
		}

		void with_clip(const std::wstring& path, clip_waiter waiter)
		{
			const std::wstring key = lc(path);
			const auto cached = cache.find(key);
			if (cached != cache.end()) { waiter(cached->second); return; }
			auto& job = jobs[key];
			if (job) { job->waiters.push_back(std::move(waiter)); return; }
			job = std::make_unique<decode_job>();
			job->waiters.push_back(std::move(waiter));
			start_decode(key, *job, path);
		}

		std::unique_ptr<player> play(const clip_ref& clip, bool loop, float volume)
		{
			QAudioFormat format;
			format.setSampleRate(static_cast<int>(clip->sample_rate));
			format.setChannelCount(clip->channels);
			format.setSampleFormat(QAudioFormat::Int16);
			const QAudioDevice device = QMediaDevices::defaultAudioOutput();
			if (device.isNull()) return nullptr;
			auto result = std::make_unique<player>();
			result->device = std::make_unique<clip_device>(clip, loop);
			result->sink = std::make_unique<QAudioSink>(device, format);
			result->sink->setVolume(std::clamp(volume, 0.0f, 1.0f));
			result->sink->start(result->device.get());
			if (result->sink->error() != QAudio::NoError) return nullptr;
			return result;
		}

		static void stop(std::unique_ptr<player>& target)
		{
			if (!target) return;
			target->sink->disconnect();
			target->sink->stop();
			target.reset();
		}

		void clear()
		{
			stop(bgm);
			stop(voice);
			for (auto& effect : effects) stop(effect);
			effects.clear();
			for (auto& [key, job] : jobs)
			{
				job->decoder->disconnect(context);
				job->decoder->stop();
			}
			jobs.clear();
			cache.clear();
		}
	};

	audio_deck::audio_deck() : m_state(std::make_shared<state>()) {}
	audio_deck::~audio_deck() { shut(); }

	bool audio_deck::boot()
	{
		if (m_engine) return true;
		if (QCoreApplication::instance() == nullptr) return false;
		auto created = std::make_shared<engine>();
		created->thread.setObjectName(QStringLiteral("audio"));
		created->context = new QObject;
		created->context->moveToThread(&created->thread);
		created->thread.start();
		if (!created->thread.isRunning())
		{
			delete created->context;
			return false;
		}
		m_engine = std::move(created);
		return true;
	}

	void audio_deck::shut()
	{
		if (!m_engine) return;
		std::shared_ptr<engine> current = std::move(m_engine);
		m_state->bgm_token.fetch_add(1);
		m_state->voice_token.fetch_add(1);
		QMetaObject::invokeMethod(current->context, [current]() {
			current->clear();
			current->context->deleteLater();
		}, Qt::BlockingQueuedConnection);
		current->thread.quit();
		current->thread.wait();
		m_state->bgm_active.store(false);
		m_state->voice_active.store(false);
		m_state->voice_ended.store(false);
		std::lock_guard<std::mutex> guard(m_state->mutex);
		m_state->voice_clip.reset();
	}

	bool audio_deck::bgm_start(const std::wstring& file_path, float volume)
	{
		if (!ready()) return false;
		m_state->bgm_volume.store(std::clamp(volume, 0.f, 1.f));
		const std::uint64_t token = m_state->bgm_token.fetch_add(1) + 1;
		m_state->bgm_active.store(false);
		engine* e = m_engine.get();
		auto shared = m_state;
		e->post([e, shared, token, file_path]() {
			engine::stop(e->bgm);
			e->with_clip(file_path, [e, shared, token](const clip_ref& clip) {
				if (!clip || shared->bgm_token.load() != token) return;
				engine::stop(e->bgm);
				e->bgm = e->play(clip, true, shared->bgm_volume.load());
				shared->bgm_active.store(e->bgm != nullptr);
			});
		});
		return true;
	}

	void audio_deck::bgm_stop()
	{
		m_state->bgm_token.fetch_add(1);
		m_state->bgm_active.store(false);
		if (!ready()) return;
		engine* e = m_engine.get();
		e->post([e]() { engine::stop(e->bgm); });
	}

	void audio_deck::bgm_set_volume(float v)
	{
		m_state->bgm_volume.store(std::clamp(v, 0.f, 1.f));
		if (!ready()) return;
		engine* e = m_engine.get();
		auto shared = m_state;
		e->post([e, shared]() { if (e->bgm) e->bgm->sink->setVolume(shared->bgm_volume.load()); });
	}

	bool audio_deck::bgm_is_active() const
	{
		return m_state->bgm_active.load(std::memory_order_acquire);
	}

	bool audio_deck::voice_start(const std::wstring& file_path, float volume, bool loop)
	{
		if (!ready()) return false;
		m_state->voice_volume.store(std::clamp(volume, 0.f, 1.f));
		const std::uint64_t token = m_state->voice_token.fetch_add(1) + 1;
		m_state->voice_active.store(false);
		m_state->voice_ended.store(false);
		engine* e = m_engine.get();
		auto shared = m_state;
		auto ended = m_voice_end_cb;
		e->post([e, shared, token, file_path, loop, ended]() {
			engine::stop(e->voice);
			e->with_clip(file_path, [e, shared, token, loop, ended](const clip_ref& clip) {
				if (!clip || shared->voice_token.load() != token) return;
				engine::stop(e->voice);
				e->voice = e->play(clip, loop, shared->voice_volume.load());
				if (!e->voice) return;
				{
					std::lock_guard<std::mutex> guard(shared->mutex);
					shared->voice_clip = clip;
				}
				shared->voice_start.store(now_ms());
				shared->voice_active.store(true, std::memory_order_release);
				auto* device = e->voice->device.get();
				QObject::connect(e->voice->sink.get(), &QAudioSink::stateChanged, e->context, [e, shared, token, device, ended](QAudio::State now) {
					if (shared->voice_token.load() != token || !e->voice || e->voice->device.get() != device) return;
					const bool failed = now == QAudio::StoppedState && e->voice->sink->error() != QAudio::NoError;
					if (!failed && !(now == QAudio::IdleState && device->finished())) return;
					shared->voice_active.store(false, std::memory_order_release);
					shared->voice_ended.store(true, std::memory_order_release);
					e->post([e, shared, token, device]() {
						if (shared->voice_token.load() == token && e->voice && e->voice->device.get() == device) engine::stop(e->voice);
					});
					if (ended) ended();
				});
			});
		});
		return true;
	}

	void audio_deck::voice_stop()
	{
		m_state->voice_token.fetch_add(1);
		m_state->voice_active.store(false, std::memory_order_release);
		m_state->voice_ended.store(false, std::memory_order_release);
		{
			std::lock_guard<std::mutex> guard(m_state->mutex);
			m_state->voice_clip.reset();
		}
		if (!ready()) return;
		engine* e = m_engine.get();
		e->post([e]() { engine::stop(e->voice); });
	}

	void audio_deck::voice_set_volume(float v)
	{
		m_state->voice_volume.store(std::clamp(v, 0.f, 1.f));
		if (!ready()) return;
		engine* e = m_engine.get();
		auto shared = m_state;
		e->post([e, shared]() { if (e->voice) e->voice->sink->setVolume(shared->voice_volume.load()); });
	}

	bool audio_deck::voice_is_active() const
	{
		return m_state->voice_active.load(std::memory_order_acquire);
	}

	bool audio_deck::voice_is_ended() const
	{
		return m_state->voice_ended.load(std::memory_order_acquire);
	}

	std::uint64_t audio_deck::voice_elapsed_ms() const
	{
		const std::int64_t start = m_state->voice_start.load();
		if (start == 0) return 0;
		const std::int64_t now = now_ms();
		return now > start ? static_cast<std::uint64_t>(now - start) : 0;
	}

	float audio_deck::voice_level() const
	{
		if (!m_state->voice_active.load(std::memory_order_acquire))
			return 0.0f;
		clip_ref clip;
		{
			std::lock_guard<std::mutex> guard(m_state->mutex);
			clip = m_state->voice_clip;
		}
		if (!clip || clip->empty() || clip->sample_rate == 0 ||
			clip->block_align == 0 || clip->bits != 16)
		{
			return 0.0f;
		}

		const std::uint64_t frameCount = clip->samples.size() / clip->block_align;
		if (frameCount == 0)
			return 0.0f;
		const std::uint64_t positionFrame =
			(voice_elapsed_ms() * clip->sample_rate / 1000) % frameCount;

		const std::uint64_t windowFrames = (std::max<std::uint64_t>)(1, clip->sample_rate / 20);
		const std::uint64_t beginFrame = positionFrame > windowFrames / 2 ? positionFrame - windowFrames / 2 : 0;
		const std::uint64_t endFrame = (std::min<std::uint64_t>)(frameCount, beginFrame + windowFrames);

		const std::int16_t* samples = reinterpret_cast<const std::int16_t*>(clip->samples.data());
		const std::uint32_t stride = clip->block_align / 2;
		double accumulated = 0.0;
		std::uint64_t counted = 0;
		for (std::uint64_t frame = beginFrame; frame < endFrame; ++frame)
		{
			const double value = static_cast<double>(samples[frame * stride]) / 32768.0;
			accumulated += value * value;
			++counted;
		}
		if (counted == 0)
			return 0.0f;
		return static_cast<float>(std::sqrt(accumulated / static_cast<double>(counted)));
	}

	bool audio_deck::se_fire(const std::wstring& file_path, float volume)
	{
		if (!ready()) return false;
		engine* e = m_engine.get();
		const float level = std::clamp(volume, 0.f, 1.f);
		e->post([e, file_path, level]() {
			e->with_clip(file_path, [e, level](const clip_ref& clip) {
				if (!clip) return;
				for (auto it = e->effects.begin(); it != e->effects.end();)
					it = (*it)->device->finished() && (*it)->sink->state() != QAudio::ActiveState ? (engine::stop(*it), e->effects.erase(it)) : std::next(it);
				while (e->effects.size() >= kMaxEffects) { engine::stop(e->effects.front()); e->effects.pop_front(); }
				if (auto effect = e->play(clip, false, level)) e->effects.push_back(std::move(effect));
			});
		});
		return true;
	}

	void audio_deck::se_stop_all()
	{
		if (!ready()) return;
		engine* e = m_engine.get();
		e->post([e]() {
			for (auto& effect : e->effects) engine::stop(effect);
			e->effects.clear();
		});
	}
}
