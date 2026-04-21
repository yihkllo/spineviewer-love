#include <Windows.h>
#include <shobjidl.h>
#include <atlbase.h>

#include <array>
#include <utility>

#include "shell_dialog_service.h"

namespace shell_dialogs
{
	namespace
	{
		class ScopedComApartment
		{
		public:
			ScopedComApartment()
				: m_ready(SUCCEEDED(::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE))) {}

			~ScopedComApartment()
			{
				if (m_ready)
					::CoUninitialize();
			}

			bool Ready() const { return m_ready; }

		private:
			bool m_ready = false;
		};

		std::wstring ReadItemPath(IShellItem* item)
		{
			if (item == nullptr)
				return {};

			wchar_t* rawPath = nullptr;
			if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &rawPath)) || rawPath == nullptr)
				return {};

			std::wstring path(rawPath);
			::CoTaskMemFree(rawPath);
			return path;
		}

		class DialogFilterPack
		{
		public:
			explicit DialogFilterPack(const FileDialogRequest* request)
			{
				if (request == nullptr)
					return;
				if (request->primaryFilter.label == nullptr || request->primaryFilter.pattern == nullptr)
					return;

				m_specs[0].pszName = request->primaryFilter.label;
				m_specs[0].pszSpec = request->primaryFilter.pattern;
				m_count = 1;
				if (request->allowAnyFile)
				{
					m_specs[1].pszName = L"All files";
					m_specs[1].pszSpec = L"*";
					m_count = 2;
				}
			}

			const COMDLG_FILTERSPEC* Data() const { return &m_specs[0]; }
			UINT Count() const { return m_count; }

		private:
			std::array<COMDLG_FILTERSPEC, 2> m_specs{};
			UINT m_count = 0;
		};

		void ApplyDialogRequest(IFileDialog* dialog, const FileDialogRequest* request)
		{
			if (dialog == nullptr || request == nullptr)
				return;

			DialogFilterPack filters(request);
			if (filters.Count() > 0)
				dialog->SetFileTypes(filters.Count(), filters.Data());
			if (request->title != nullptr)
				dialog->SetTitle(request->title);
		}

		FILEOPENDIALOGOPTIONS BuildDialogOptions(IFileDialog* dialog, FILEOPENDIALOGOPTIONS extraOptions)
		{
			FILEOPENDIALOGOPTIONS options{};
			dialog->GetOptions(&options);
			return options | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM | extraOptions;
		}

		class DialogSession
		{
		public:
			bool Ready() const
			{
				return m_comApartment.Ready();
			}

			template <typename TResult>
			bool Open(void* ownerWindow,
				const FileDialogRequest* request,
				FILEOPENDIALOGOPTIONS extraOptions,
				TResult&& resultHandler) const
			{
				if (!Ready())
					return false;

				CComPtr<IFileOpenDialog> dialog;
				if (FAILED(dialog.CoCreateInstance(CLSID_FileOpenDialog)))
					return false;

				ApplyDialogRequest(dialog.p, request);
				dialog->SetOptions(BuildDialogOptions(dialog.p, extraOptions));
				if (FAILED(dialog->Show(static_cast<HWND>(ownerWindow))))
					return false;

				return resultHandler(dialog.p);
			}

			bool Save(void* ownerWindow,
				const FileDialogRequest& request,
				const wchar_t* defaultName,
				std::wstring& outputPath) const
			{
				if (!Ready())
					return false;

				CComPtr<IFileSaveDialog> dialog;
				if (FAILED(dialog.CoCreateInstance(CLSID_FileSaveDialog)))
					return false;

				ApplyDialogRequest(dialog.p, &request);
				if (defaultName != nullptr)
					dialog->SetFileName(defaultName);
				dialog->SetOptions(BuildDialogOptions(dialog.p, 0));
				if (FAILED(dialog->Show(static_cast<HWND>(ownerWindow))))
					return false;

				CComPtr<IShellItem> item;
				if (FAILED(dialog->GetResult(&item)))
					return false;
				outputPath = ReadItemPath(item);
				return !outputPath.empty();
			}

		private:
			mutable ScopedComApartment m_comApartment;
		};

		std::vector<std::wstring> CollectItemArrayPaths(IShellItemArray* items)
		{
			std::vector<std::wstring> paths;
			if (items == nullptr)
				return paths;

			DWORD count = 0;
			if (FAILED(items->GetCount(&count)))
				return paths;

			paths.reserve(count);
			for (DWORD index = 0; index < count; ++index)
			{
				CComPtr<IShellItem> item;
				if (FAILED(items->GetItemAt(index, &item)))
					continue;

				std::wstring path = ReadItemPath(item);
				if (!path.empty())
					paths.push_back(path);
			}
			return paths;
		}
	}

	std::wstring CFileDialogService::PickFolder(void* ownerWindow, const wchar_t* title) const
	{
		DialogSession session;
		FileDialogRequest request{};
		request.title = title;

		std::wstring folderPath;
		session.Open(ownerWindow, &request, FOS_PICKFOLDERS, [&](IFileOpenDialog* dialog) {
			CComPtr<IShellItem> item;
			if (FAILED(dialog->GetResult(&item)))
				return false;
			folderPath = ReadItemPath(item);
			return !folderPath.empty();
		});
		return folderPath;
	}

	std::wstring CFileDialogService::PickSingleFile(const FileDialogRequest& request, void* ownerWindow) const
	{
		DialogSession session;
		std::wstring selectedPath;
		session.Open(ownerWindow, &request, 0, [&](IFileOpenDialog* dialog) {
			CComPtr<IShellItem> item;
			if (FAILED(dialog->GetResult(&item)))
				return false;
			selectedPath = ReadItemPath(item);
			return !selectedPath.empty();
		});
		return selectedPath;
	}

	std::vector<std::wstring> CFileDialogService::PickMultipleFiles(const FileDialogRequest& request, void* ownerWindow) const
	{
		DialogSession session;
		std::vector<std::wstring> selectedPaths;
		session.Open(ownerWindow, &request, FOS_ALLOWMULTISELECT, [&](IFileOpenDialog* dialog) {
			CComPtr<IShellItemArray> items;
			if (FAILED(dialog->GetResults(&items)))
				return false;
			selectedPaths = CollectItemArrayPaths(items);
			return !selectedPaths.empty();
		});
		return selectedPaths;
	}

	std::wstring CFileDialogService::PickSavePath(const FileDialogRequest& request, const wchar_t* defaultName, void* ownerWindow) const
	{
		DialogSession session;
		std::wstring savePath;
		session.Save(ownerWindow, request, defaultName, savePath);
		return savePath;
	}

	void CFileDialogService::ShowAnsiAlert(const char* title, const char* message) const
	{
		::MessageBoxA(nullptr, message, title, MB_ICONERROR);
	}

	void CFileDialogService::ShowOwnerError(const wchar_t* message, void* ownerWindow) const
	{
		HWND windowHandle = static_cast<HWND>(ownerWindow);
		::ValidateRect(windowHandle, nullptr);
		::MessageBoxW(windowHandle, message, L"Error", MB_ICONERROR);
		::InvalidateRect(windowHandle, nullptr, FALSE);
	}
}
