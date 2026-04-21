#ifndef SPINELOVE_SHELL_DIALOG_SERVICE_H_
#define SPINELOVE_SHELL_DIALOG_SERVICE_H_

#include <string>
#include <vector>

namespace shell_dialogs
{
	struct FileTypeFilter
	{
		const wchar_t* label = nullptr;
		const wchar_t* pattern = nullptr;
	};

	struct FileDialogRequest
	{
		const wchar_t* title = nullptr;
		FileTypeFilter primaryFilter{};
		bool allowAnyFile = false;
	};

	class CFileDialogService final
	{
	public:
		std::wstring PickFolder(void* ownerWindow, const wchar_t* title = nullptr) const;
		std::wstring PickSingleFile(const FileDialogRequest& request, void* ownerWindow) const;
		std::vector<std::wstring> PickMultipleFiles(const FileDialogRequest& request, void* ownerWindow) const;
		std::wstring PickSavePath(const FileDialogRequest& request, const wchar_t* defaultName, void* ownerWindow) const;

		void ShowAnsiAlert(const char* title, const char* message) const;
		void ShowOwnerError(const wchar_t* message, void* ownerWindow) const;
	};
}

#endif

