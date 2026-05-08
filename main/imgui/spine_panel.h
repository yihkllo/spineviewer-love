#ifndef SPINE_PANEL_H_
#define SPINE_PANEL_H_

#include <functional>
#include <string>
#include <vector>

#include "custom_titlebar.h"

class IOverlayDrawer;

namespace spine_panel
{
	struct SpinePanelState
	{
		void* pSpinePlayer;
		int iTextureWidth;
		int iTextureHeight;

		bool requestContentResize = false;

		unsigned int loadGeneration = 0;


		std::function<void()> onOpenFiles;
		std::function<void()> onOpenFolder;
		std::function<void()> onShowTool;
		std::function<void(const std::wstring&)> onAddSpineFromFile;
		std::function<void()> onWindowSettings;
		std::function<void()> onAllowDragResize;
		std::function<void()> onReverseZoom;
		std::function<void()> onFitToManualSize;
		std::function<void()> onFitToDefaultSize;
		std::function<void()> onLoadBackground;
		std::function<void(bool)> onExportPng;
		std::function<void(bool)> onExportJpeg;
		std::function<void(bool)> onExportPngFrames;
		std::function<void(bool)> onExportJpegFrames;
		std::function<void(bool)> onExportMp4;
		std::function<void(bool)> onExportWebm;
		std::function<void(bool)> onExportGif;
		bool exportAlpha = true;
		int exportFps = 30;
		int exportImageFps = 30;
		int exportVideoFps = 60;
		bool exportRunning = false;
		bool exportFailed = false;
		int exportDone = 0;
		int exportTotal = 0;
		std::string exportStatus;
		bool exportQueueEnabled = false;
		bool exportQueueActive = false;
		size_t exportQueueIndex = 0;
		std::function<void(bool)> onPmaChanged;


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

		bool mouseHoverEnabled = false;
		IOverlayDrawer* overlayDrawer = nullptr;

		bool isFullscreen = false;
	};

	void DrawControlPanel(SpinePanelState& panel, bool* open);

	bool HasSlotNameQueryFilter();
	bool (*GetSlotNameQueryExcludeCallback())(const char*, size_t);


	void SetExternalSlotExcludeCallback(bool (*pFunc)(const char*, size_t));
	void ClearExternalSlotExcludeCallback();
}
#endif
