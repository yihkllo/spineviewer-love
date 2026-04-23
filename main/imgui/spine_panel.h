#ifndef SPINE_PANEL_H_
#define SPINE_PANEL_H_

#include <functional>
#include <string>
#include <vector>

#include "custom_titlebar.h"

namespace spine_panel
{
	static constexpr int kDefaultImageFps = 30;
	static constexpr int kDefaultVideoFps = 60;

	struct SSpineToolDatum
	{
		void* pSpinePlayer;
		int iTextureWidth;
		int iTextureHeight;

		int iImageFps = kDefaultImageFps;
		int iVideoFps = kDefaultVideoFps;
		bool toExportPerAnim = true;

		bool isWindowToBeResized = false;

		unsigned int loadGeneration = 0;


		std::function<void()> onOpenFiles;
		std::function<void()> onOpenFolder;
		std::function<void()> onExtensionSetting;
		std::function<void()> onShowTool;
		std::function<void()> onFontSetting;
		std::function<void(const std::wstring&)> onAddSpineFromFile;
		std::function<void()> onWindowSettings;
		std::function<void()> onAllowDragResize;
		std::function<void()> onReverseZoom;
		std::function<void()> onFitToManualSize;
		std::function<void()> onFitToDefaultSize;
		std::function<void()> onLoadBackground;
		std::function<void(bool)> onPmaChanged;


		std::function<void()> onLoadAudio;
		std::function<void()> onPlayAudio;
		std::function<void()> onStopAudio;
		std::function<void()> onToggleLoopAudio;
		std::function<void(float)> onSetAudioVolume;
		std::function<void()> onAudioPrev;
		std::function<void()> onAudioNext;
		std::function<void()> onToggleAudioAuto;
		std::wstring audioFileName;
		bool isAudioPlaying = false;
		bool isAudioLooping = false;
		bool isAudioAuto = false;
		float audioVolume = 0.5f;
		int audioIndex = 0;
		int audioTotal = 0;

		std::function<void()> onProButtonClicked;


		std::vector<std::wstring> skelFileList;
		std::vector<std::wstring> favoriteSkelFileList;
		std::vector<std::wstring> loadedSpineFileList;
		std::vector<std::string> loadedSpineNames;
		std::vector<bool> loadedSpineVisibility;
		bool showLoadedSpinePanel = false;
		int selectedLoadedSpine = 0;
		bool showLoadedSpineReplaceConfirm = false;
		std::string loadedSpineReplaceConfirmTitle = "Warning";
		std::string loadedSpineReplaceConfirmMessage;
		bool showFavoriteSkelFiles = false;
		std::wstring currentSkelFile;
		std::string currentFileName;
		std::function<void()> onPickFolder;
		std::function<void()> onToggleFavoriteView;
		std::function<void(const std::wstring&)> onToggleFavoriteFile;
		std::function<void(const std::wstring&)> onOpenFileFolder;
		std::function<void(const std::wstring&)> onPlayFile;
		std::function<void(size_t)> onSelectLoadedSpine;
		std::function<void(size_t)> onToggleLoadedSpineVisibility;
		std::function<void(size_t)> onMoveLoadedSpineUp;
		std::function<void(size_t)> onMoveLoadedSpineDown;
		std::function<void()> onConfirmLoadedSpineReplace;
		std::function<void()> onCancelLoadedSpineReplace;


		std::function<void()> onSnapPng;
		std::function<void()> onSnapJpg;
		std::function<void()> onExportGif;
		std::function<void()> onExportVideo;
		std::function<void()> onExportWebm;
		bool isWebmEncoding = false;
		int recordFramesCaptured = 0;
		int recordFramesTotal = 0;
		int recordFramesWritten = 0;
		bool isWritingFrames = false;
		std::function<void()> onExportPngs;
		std::function<void()> onExportJpgs;
		std::function<void()> onEndRecording;
		std::function<bool()> isRecording;


		std::vector<std::string>* pAnimQueue = nullptr;
		std::function<void()> onQueuePlay;
		std::function<void()> onQueueStop;
		std::function<bool()> isQueuePlaying;
		std::function<size_t()> getQueueIndex;


		float leftPanelEndX = 300.f;
		float rightPanelStartX = 9999.f;


		bool isSettingWindowOpen = false;


		custom_titlebar::STitleBarConfig titleBar;
		std::function<void()> onLoadTitleBg;


		std::function<void(int, int, int)> onSetRenderBgColor;
		int renderBgR = 0;
		int renderBgG = 0;
		int renderBgB = 0;


		bool exportWithAlpha = true;


		bool mouseHoverEnabled = false;

		bool isFullscreen = false;
	};

	void Display(SSpineToolDatum& spineToolDatum, bool* pIsOpen);

	bool HasSlotExclusionFilter();
	bool (*GetSlotExcludeCallback())(const char*, size_t);


	void SetExternalSlotExcludeCallback(bool (*pFunc)(const char*, size_t));
	void ClearExternalSlotExcludeCallback();
}
#endif
