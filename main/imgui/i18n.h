


#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace i18n
{

    namespace detail
    {
        inline std::unordered_map<std::string, std::string>& table()
        {
            static std::unordered_map<std::string, std::string> t;
            return t;
        }
        inline std::string& currentLangId()
        {
            static std::string id = "en";
            return id;
        }
        inline std::string& exeDir()
        {
            static std::string dir;
            return dir;
        }
    }


    inline const char* displayName(const std::string& id)
    {
        if (id == "zh_CN") return u8"简体中文";
        if (id == "zh_TW") return u8"繁體中文";
        if (id == "ja_JP") return u8"日本語";
        if (id == "ko_KR") return u8"한국어";
        if (id == "en")    return "English";
        return id.c_str();
    }


    inline std::string getExeDir()
    {
#ifdef _WIN32
        wchar_t buf[MAX_PATH]{};
        ::GetModuleFileNameW(nullptr, buf, MAX_PATH);
        wchar_t* slash = ::wcsrchr(buf, L'\\');
        if (slash) *(slash + 1) = L'\0';

        int len = ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, nullptr, 0, nullptr, nullptr);
        std::string s(len > 0 ? len - 1 : 0, '\0');
        if (len > 0) ::WideCharToMultiByte(CP_UTF8, 0, buf, -1, &s[0], len, nullptr, nullptr);
        return s;
#else
        return "./";
#endif
    }


    inline std::vector<std::string> getAvailableLangs()
    {
        std::vector<std::string> langs;
        const std::string langDir = detail::exeDir() + "lang\\";

#ifdef _WIN32
        std::wstring wPattern;
        {
            int wlen = ::MultiByteToWideChar(CP_UTF8, 0, langDir.c_str(), -1, nullptr, 0);
            wPattern.resize(wlen > 0 ? wlen - 1 : 0);
            if (wlen > 0) ::MultiByteToWideChar(CP_UTF8, 0, langDir.c_str(), -1, &wPattern[0], wlen);
        }
        wPattern += L"*.txt";

        WIN32_FIND_DATAW fd{};
        HANDLE h = ::FindFirstFileW(wPattern.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do {
                std::wstring wname = fd.cFileName;

                if (wname.size() > 4 && wname.substr(wname.size() - 4) == L".txt")
                    wname = wname.substr(0, wname.size() - 4);
                int len = ::WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string id(len > 0 ? len - 1 : 0, '\0');
                if (len > 0) ::WideCharToMultiByte(CP_UTF8, 0, wname.c_str(), -1, &id[0], len, nullptr, nullptr);
                langs.push_back(id);
            } while (::FindNextFileW(h, &fd));
            ::FindClose(h);
        }
#endif

        auto it = std::find(langs.begin(), langs.end(), "en");
        if (it != langs.end() && it != langs.begin())
            std::rotate(langs.begin(), it, it + 1);

        return langs;
    }


    inline bool load(const std::string& exeDir, const std::string& langId)
    {
        detail::exeDir() = exeDir;
        detail::table().clear();
        detail::currentLangId() = langId;

        if (langId == "en") return true;

        std::string path = exeDir + "lang\\" + langId + ".txt";
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) return false;

        std::string line;
        while (std::getline(f, line))
        {

            if (line.size() >= 3 &&
                (unsigned char)line[0] == 0xEF &&
                (unsigned char)line[1] == 0xBB &&
                (unsigned char)line[2] == 0xBF)
                line = line.substr(3);


            if (!line.empty() && line.back() == '\r')
                line.pop_back();


            if (line.empty() || line[0] == '#') continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            detail::table()[key] = val;
        }
        return true;
    }


    inline void init(const std::string& exeDir)
    {
        detail::exeDir() = exeDir;
    }


    inline const char* t(const char* key)
    {
        auto& tbl = detail::table();
        auto it = tbl.find(key);
        if (it != tbl.end()) return it->second.c_str();
        return key;
    }

    inline const std::string& currentLang()
    {
        return detail::currentLangId();
    }
}


#define TR(key)  i18n::t(key)
