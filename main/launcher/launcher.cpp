#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <string>

static std::wstring quoted(const std::wstring& value)
{
    std::wstring result=L"\"";
    size_t slashes=0;
    for(const wchar_t ch:value){
        if(ch==L'\\'){++slashes;continue;}
        if(ch==L'\"'){
            result.append(slashes*2+1,L'\\');
            result.push_back(L'\"');
        }else{
            result.append(slashes,L'\\');
            result.push_back(ch);
        }
        slashes=0;
    }
    result.append(slashes*2,L'\\');
    result.push_back(L'\"');
    return result;
}

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int)
{
    std::wstring module(32768,L'\0');
    const DWORD length=GetModuleFileNameW(nullptr,module.data(),DWORD(module.size()));
    if(length==0||length>=module.size())return 1;
    module.resize(length);
    const size_t slash=module.find_last_of(L"\\/");
    if(slash==std::wstring::npos)return 1;
    const std::wstring root=module.substr(0,slash);
    const std::wstring working=root+L"\\main";
    const std::wstring executable=working+L"\\spinelove_qt.exe";
    if(GetFileAttributesW(executable.c_str())==INVALID_FILE_ATTRIBUTES){
        MessageBoxW(nullptr,L"程序运行文件不完整，请重新解压完整安装包。",L"SpineLoveEX",MB_OK|MB_ICONERROR);
        return 2;
    }
    int argc=0;
    LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(),&argc);
    std::wstring command=quoted(executable);
    if(argv){
        for(int i=1;i<argc;++i){command.push_back(L' ');command+=quoted(argv[i]);}
        LocalFree(argv);
    }
    STARTUPINFOW startup{};
    startup.cb=sizeof(startup);
    PROCESS_INFORMATION process{};
    if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,working.c_str(),&startup,&process)){
        MessageBoxW(nullptr,L"程序启动失败，请重新解压完整安装包。",L"SpineLoveEX",MB_OK|MB_ICONERROR);
        return 3;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return 0;
}
