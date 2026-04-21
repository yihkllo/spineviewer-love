#ifndef FRAME_CAPTURE_H_
#define FRAME_CAPTURE_H_

#include <Windows.h>

class CDxLibRecorder
{
public:
	CDxLibRecorder();
	~CDxLibRecorder();
	CDxLibRecorder(const CDxLibRecorder&) = delete;
	CDxLibRecorder& operator=(const CDxLibRecorder&) = delete;

	enum class EOutputType
	{
		Unknown,
		Gif,
		Video,
		Pngs,
		Jpgs,
		WebmFrames,
	};

	bool Start(EOutputType outputType, unsigned int fps = kDefaultFps);
	EOutputType GetOutputType() const;
	int  GetFps() const;

	enum class EState
	{
		Idle,
		UnderRecording,
		InitialisingVideoStream,
		WritingFrames,
	};
	EState GetState() const;

	bool CaptureFrame(const wchar_t* imageName = nullptr);
	bool CommitFrame(int graphicHandle, const wchar_t* imageName = nullptr);
	bool HasFrames() const;
	int  GetFrameCount() const;

	void GetWriteProgress(int* written, int* total) const;
	bool GetFirstFrameSize(int* width, int* height) const;

	void SetPipeHandle(HANDLE hPipeWrite);
	bool End(const wchar_t* pwzFilePath);

private:
	static constexpr unsigned int kDefaultFps = 30;

	class Impl;
	Impl* m_impl = nullptr;
};
#endif
