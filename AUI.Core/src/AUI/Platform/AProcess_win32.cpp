//
// Created by alex2 on 31.10.2020.
//

#ifdef _WIN32
#include "AProcess.h"
#include <windows.h>
#include <AUI/Traits/memory.h>
#include <AUI/Logging/ALogger.h>


int AProcess::execute(const AString& applicationFile, const AString& args, const APath& workingDirectory, bool waitForExit) {
    AProcess p(applicationFile);
    p.setArgs(args);
    p.setWorkingDirectory(workingDirectory);
    p.run();

    if (waitForExit)
        return p.getExitCode();

    return 0;
}

void AProcess::executeAsAdministrator(const AString& applicationFile, const AString& args, const APath& workingDirectory) {
    SHELLEXECUTEINFO sei = { sizeof(sei) };


    sei.lpVerb = L"runas";
    sei.lpFile = applicationFile.c_str();
    sei.lpParameters = args.c_str();
    sei.lpDirectory = workingDirectory.c_str();
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteEx(&sei)) {
        AString message = "Could not create process as admin " + applicationFile;
        if (!args.empty())
            message += " with args " + args;
        if (!workingDirectory.empty())
            message += " in " + workingDirectory;
        throw AProcessException(message);
    }
}

void AProcess::run() {
    STARTUPINFO startupInfo;
    aui::zero(startupInfo);
    startupInfo.cb = sizeof(startupInfo);

    PROCESS_INFORMATION processInformation;

    if (!CreateProcess(mApplicationFile.c_str(),
                       const_cast<wchar_t*>(mArgs.empty() ? nullptr : mArgs.c_str()),
                       nullptr,
                       nullptr,
                       false,
                       0,
                       nullptr,
                       mWorkingDirectory.empty() ? nullptr : mWorkingDirectory.c_str(),
                       &startupInfo,
                       &processInformation)) {
        AString message = "Could not create process " + mApplicationFile;
        if (!mArgs.empty())
            message += " with args " + mArgs;
        if (!mWorkingDirectory.empty())
            message += " in " + mWorkingDirectory;
        throw AProcessException(message);
    }
    mHandle = processInformation.hProcess;
}

void AProcess::wait() {
    WaitForSingleObject(mHandle, INFINITE);
}

int AProcess::getExitCode() {
    DWORD exitCode;
    wait();
    int r = GetExitCodeProcess(mHandle, &exitCode);
    assert(r && r != STILL_ACTIVE);
    return exitCode;
}

AProcess& AProcess::current() {
    static AProcess p(GetCurrentProcess());
    return p;
}

AProcess::CpuTime AProcess::getCpuTime() const {
    auto convert = [](FILETIME f) {
        time_t time = f.dwHighDateTime;
        time <<= 32u;
        time |= (f.dwLowDateTime);
        return time / 100;
    };

    FILETIME ignored;
    FILETIME kernel;
    FILETIME user;
    auto r = GetProcessTimes(mHandle, &ignored, &ignored, &kernel, &user);
    assert(r);
    return {convert(kernel), convert(user)};
}


#endif

