
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <cstring>

#include "render_handle.h"
#include "frame_capture.h"
#include "sl_gfx_pixelmap.h"


#include "media_mux.h"
#include "bitmap_io.h"
#include "hw_caps.h"

#define DX_NON_USING_NAMESPACE_DXLIB
#include <DxLib.h>

using DxLibImageHandle = DxLibHandle<&DxLib::DeleteGraph>;


struct CpuFrame
{
	std::vector<unsigned char> pixels;
	int width  = 0;
	int height = 0;
	int stride = 0;
	std::wstring name;
};


class CDxLibRecorder::Impl
{
public:
	Impl()  { m_isAmdCpu = cpu::IsAmd(); }
	~Impl() { Clear(); }

	bool Start(EOutputType outputType, unsigned int fps);
	EOutputType GetoutputType() const { return m_outputType; }
	int GetFps()   const { return m_fps; }
	EState GetState() const { return m_recorderState; }

	bool Capture(const wchar_t* imageName);
	bool CommitFrame(const int iGraphicHandle, const wchar_t* imageName);
	bool HasFrames()    const { return m_hasFirstFrame; }
	int  GetFrameCount() const
	{

		return static_cast<int>(m_cpuFrames.size() + m_images.size());
	}

	void GetWriteProgress(int* written, int* total) const
	{
		if (written) *written = m_framesWritten.load(std::memory_order_relaxed);
		if (total)   *total   = m_framesWriteTotal.load(std::memory_order_relaxed);
	}

	bool GetFirstFrameSize(int* width, int* height) const
	{
		if (m_cpuFrames.empty()) return false;
		if (width)  *width  = m_cpuFrames.front().width;
		if (height) *height = m_cpuFrames.front().height;
		return true;
	}

	void SetPipeHandle(HANDLE h) { m_hPipeWrite = h; }

	bool End(const wchar_t* filePath);

private:

	std::vector<CpuFrame>     m_cpuFrames;


	std::vector<DxLibImageHandle> m_images;
	std::vector<std::wstring>     m_imageNames;


	CMfVideoEncoder m_videoEncoder;
	bool m_videoEncoderReady = false;
	std::wstring m_videoPendingPath;

	bool m_hasFirstFrame = false;
	bool m_isAmdCpu      = false;

	EOutputType m_outputType    = EOutputType::Unknown;
	EState      m_recorderState = EState::Idle;
	int         m_fps           = kDefaultFps;


	std::thread      m_writeThread;
	std::atomic<int> m_framesWritten{0};
	std::atomic<int> m_framesWriteTotal{0};


	HANDLE m_hPipeWrite = INVALID_HANDLE_VALUE;

	void Clear();
	void JoinWriteThread();
	void TruncateSize(int* width, int* height) const;


	static bool GpuToCpu(int hGraph, int w, int h, const wchar_t* name, CpuFrame& out)
	{
		DxLibImageHandle tmp(DxLib::MakeGraph(w, h));
		if (tmp.Empty()) return false;
		if (DxLib::BltDrawValidGraph(hGraph, 0, 0, w, h, 0, 0, tmp.Get()) == -1)
			return false;

		CDxLibMap s(tmp.Get());
		if (!s.IsAccessible()) return false;

		out.width  = s.width;
		out.height = s.height;
		out.stride = s.stride;
		out.pixels.resize(static_cast<size_t>(s.stride) * s.height);
		std::memcpy(out.pixels.data(), s.pPixels, out.pixels.size());
		out.name = name ? name : L"";
		return true;
	}
};


bool CDxLibRecorder::Impl::Start(EOutputType outputType, unsigned int fps)
{
	Clear();
	m_fps           = fps;
	m_outputType    = outputType;
	m_recorderState = EState::UnderRecording;
	return true;
}


bool CDxLibRecorder::Impl::Capture(const wchar_t* imageName)
{
	if (m_recorderState == EState::Idle ||
		m_recorderState == EState::InitialisingVideoStream)
		return false;

	int w = 0, h = 0;
	DxLib::GetScreenState(&w, &h, nullptr);
	TruncateSize(&w, &h);

	DxLibImageHandle hGraph(DxLib::MakeGraph(w, h));
	if (hGraph.Empty()) return false;
	if (DxLib::GetDrawScreenGraph(0, 0, w, h, hGraph.Get()) == -1) return false;

	m_images.push_back(std::move(hGraph));
	if (imageName) m_imageNames.push_back(imageName);
	m_hasFirstFrame = true;
	return true;
}


bool CDxLibRecorder::Impl::CommitFrame(const int iGraphicHandle,
                                        const wchar_t* imageName)
{
	int w = 0, h = 0;
	if (DxLib::GetGraphSize(iGraphicHandle, &w, &h) == -1) return false;
	TruncateSize(&w, &h);


	if (m_outputType == EOutputType::Video)
	{
		if (!m_videoEncoderReady)
		{
			wchar_t tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			::GetTempPathW(MAX_PATH, tmpDir);
			::GetTempFileNameW(tmpDir, L"vid", 0, tmpFile);
			m_videoPendingPath = tmpFile;

			m_recorderState = EState::InitialisingVideoStream;
			bool ok = m_videoEncoder.initialise(tmpFile, w, h, m_fps);
			m_recorderState = EState::UnderRecording;
			if (!ok) return false;
			m_videoEncoderReady = true;
		}

		DxLibImageHandle tmp(DxLib::MakeGraph(w, h));
		if (tmp.Empty()) return false;
		if (DxLib::BltDrawValidGraph(iGraphicHandle, 0, 0, w, h, 0, 0, tmp.Get()) == -1)
			return false;

		CDxLibMap s(tmp.Get());
		if (!s.IsAccessible()) return false;
		m_videoEncoder.commitCpuFrame(s.pPixels,
			static_cast<unsigned long>(s.stride * s.height), true);
		m_hasFirstFrame = true;
		return true;
	}


	if (m_outputType == EOutputType::Pngs  ||
		m_outputType == EOutputType::Jpgs  ||
		m_outputType == EOutputType::WebmFrames)
	{
		CpuFrame frame;
		if (!GpuToCpu(iGraphicHandle, w, h, imageName, frame)) return false;
		m_cpuFrames.push_back(std::move(frame));
		m_hasFirstFrame = true;
		return true;
	}


	DxLibImageHandle hGraph(DxLib::MakeGraph(w, h));
	if (hGraph.Empty()) return false;
	if (DxLib::BltDrawValidGraph(iGraphicHandle, 0, 0, w, h, 0, 0, hGraph.Get()) == -1)
		return false;

	m_images.push_back(std::move(hGraph));
	if (imageName) m_imageNames.push_back(imageName);
	m_hasFirstFrame = true;
	return true;
}


bool CDxLibRecorder::Impl::End(const wchar_t* filePath)
{


	const bool isPipeMode = (m_outputType == EOutputType::WebmFrames &&
	                         m_hPipeWrite != INVALID_HANDLE_VALUE);

	if (filePath == nullptr && !isPipeMode)
	{
		Clear();
		return false;
	}
	std::wstring basePath = (filePath != nullptr) ? filePath : L"";

	switch (m_outputType)
	{

	case EOutputType::Gif:
	{
		std::wstring path = basePath + L".gif";
		win_image::CWicGifEncoder enc;
		if (enc.Initialise(path.c_str()))
		{
			for (auto& img : m_images)
			{
				CDxLibMap s(img.Get());
				if (s.IsAccessible())
					enc.CommitFrame(s.width, s.height, s.stride, s.pPixels,
					                true, 1.f / m_fps);
				img.Reset();
			}
			enc.Finalise();
		}
		Clear();
		break;
	}


	case EOutputType::Video:
	{
		std::wstring path = basePath + L".mp4";
		if (m_videoEncoderReady)
		{
			m_videoEncoder.finalise();
			m_videoEncoderReady = false;
			if (!m_videoPendingPath.empty())
			{
				::MoveFileExW(m_videoPendingPath.c_str(), path.c_str(),
				              MOVEFILE_REPLACE_EXISTING);
				m_videoPendingPath.clear();
			}
		}
		Clear();
		break;
	}


	case EOutputType::Pngs:
	case EOutputType::Jpgs:
	case EOutputType::WebmFrames:
	{
		JoinWriteThread();


		auto        frames     = std::move(m_cpuFrames);
		std::wstring outBase   = basePath;
		EOutputType  outType   = m_outputType;
		const int    total     = static_cast<int>(frames.size());


		HANDLE hPipe = m_hPipeWrite;
		m_hPipeWrite = INVALID_HANDLE_VALUE;


		m_cpuFrames.clear();
		m_hasFirstFrame  = false;
		m_outputType     = EOutputType::Unknown;
		m_framesWritten.store(0, std::memory_order_relaxed);
		m_framesWriteTotal.store(total, std::memory_order_relaxed);
		m_recorderState  = EState::WritingFrames;


		m_writeThread = std::thread([this,
			frames   = std::move(frames),
			outBase,
			outType,
			hPipe,
			total]() mutable
		{
			for (int i = 0; i < total; ++i)
			{
				const CpuFrame& f = frames[i];
				if (!f.pixels.empty())
				{
					if (outType == EOutputType::WebmFrames && hPipe != INVALID_HANDLE_VALUE)
					{


						const int rowBytes = f.width * 4;
						for (int y = 0; y < f.height; ++y)
						{
							const unsigned char* row = f.pixels.data() + static_cast<size_t>(y) * f.stride;
							DWORD written = 0;
							if (!::WriteFile(hPipe, row, static_cast<DWORD>(rowBytes), &written, nullptr))
								break;
						}
					}
					else
					{
						std::wstring path = outBase + f.name;
						if (outType == EOutputType::Pngs ||
							outType == EOutputType::WebmFrames)
						{
							path += L".png";
							win_image::SaveImageAsPng(path.c_str(),
								f.width, f.height, f.stride,
								const_cast<unsigned char*>(f.pixels.data()), true);
						}
						else
						{
							path += L".jpg";
							win_image::SaveImageAsJpg(path.c_str(),
								f.width, f.height, f.stride,
								const_cast<unsigned char*>(f.pixels.data()), true);
						}
					}
				}
				m_framesWritten.fetch_add(1, std::memory_order_relaxed);
			}


			if (hPipe != INVALID_HANDLE_VALUE)
				::CloseHandle(hPipe);

			m_recorderState = EState::Idle;
		});

		break;
	}

	default:
		break;
	}

	return true;
}


void CDxLibRecorder::Impl::JoinWriteThread()
{
	if (m_writeThread.joinable())
		m_writeThread.join();
}

void CDxLibRecorder::Impl::Clear()
{
	JoinWriteThread();

	m_cpuFrames.clear();
	m_images.clear();
	m_imageNames.clear();

	if (m_videoEncoderReady)
	{
		m_videoEncoder.finalise();
		m_videoEncoderReady = false;
	}
	if (!m_videoPendingPath.empty())
	{
		::DeleteFileW(m_videoPendingPath.c_str());
		m_videoPendingPath.clear();
	}

	if (m_hPipeWrite != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(m_hPipeWrite);
		m_hPipeWrite = INVALID_HANDLE_VALUE;
	}

	m_framesWritten.store(0, std::memory_order_relaxed);
	m_framesWriteTotal.store(0, std::memory_order_relaxed);
	m_hasFirstFrame  = false;
	m_outputType     = EOutputType::Unknown;
	m_recorderState  = EState::Idle;
}

void CDxLibRecorder::Impl::TruncateSize(int* width, int* height) const
{

	if (!m_isAmdCpu || m_outputType != EOutputType::Video) return;
	constexpr int kAlignMask = ~0x3;
	if (width)  *width  &= kAlignMask;
	if (height) *height &= kAlignMask;
}


CDxLibRecorder::CDxLibRecorder()  { m_impl = new CDxLibRecorder::Impl(); }
CDxLibRecorder::~CDxLibRecorder() { delete m_impl; }

bool CDxLibRecorder::Start(EOutputType outputType, unsigned int fps)
	{ return m_impl->Start(outputType, fps); }
CDxLibRecorder::EOutputType CDxLibRecorder::GetOutputType() const
	{ return m_impl->GetoutputType(); }
int CDxLibRecorder::GetFps() const
	{ return m_impl->GetFps(); }
CDxLibRecorder::EState CDxLibRecorder::GetState() const
	{ return m_impl->GetState(); }
bool CDxLibRecorder::CaptureFrame(const wchar_t* imageName)
	{ return m_impl->Capture(imageName); }
bool CDxLibRecorder::CommitFrame(const int iGraphicHandle, const wchar_t* imageName)
	{ return m_impl->CommitFrame(iGraphicHandle, imageName); }
bool CDxLibRecorder::HasFrames() const
	{ return m_impl->HasFrames(); }
int CDxLibRecorder::GetFrameCount() const
	{ return m_impl->GetFrameCount(); }
bool CDxLibRecorder::End(const wchar_t* pwzFilePath)
	{ return m_impl->End(pwzFilePath); }
void CDxLibRecorder::GetWriteProgress(int* written, int* total) const
	{ m_impl->GetWriteProgress(written, total); }
bool CDxLibRecorder::GetFirstFrameSize(int* width, int* height) const
	{ return m_impl->GetFirstFrameSize(width, height); }
void CDxLibRecorder::SetPipeHandle(HANDLE hPipeWrite)
	{ m_impl->SetPipeHandle(hPipeWrite); }
