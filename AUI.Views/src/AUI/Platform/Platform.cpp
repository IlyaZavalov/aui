﻿#include "Platform.h"
#include "AUI/Common/AString.h"

#ifdef _WIN32
#include <Windows.h>

AString Platform::getFontPath(const AString& font)
{
    try {
        if (std::filesystem::is_regular_file(font.toStdString()))
            return font;
    } catch(...) {}
    try {
        HKEY fontsKey;
        for (auto dir : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER}) {
            if (RegOpenKeyEx(dir, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ,
                             &fontsKey))
                throw std::exception{};
            if (fontsKey == nullptr)
                throw std::exception{};
            DWORD maxValueNameSize, maxValueDataSize;
            DWORD valueNameSize, valueDataSize, valueType;

            if (RegQueryInfoKey(fontsKey, 0, 0, 0, 0, 0, 0, 0, &maxValueNameSize, &maxValueDataSize, 0, 0)) {
                throw std::exception{};
            }

            wchar_t* valueName = new wchar_t[maxValueNameSize];
            wchar_t* valueData = new wchar_t[maxValueDataSize];

            for (DWORD index = 0; RegEnumValue(fontsKey, index, valueName, &valueNameSize, 0, &valueType,
                                               reinterpret_cast<LPBYTE>(valueData), &valueDataSize) !=
                                  ERROR_NO_MORE_ITEMS; ++index) {
                valueDataSize = maxValueDataSize;
                valueNameSize = maxValueNameSize;

                if (valueType != REG_SZ) {
                    continue;
                }

                // Found a match
                if (AString(valueName).startsWith(font + " (")) {
                    RegCloseKey(fontsKey);
                    return AString{valueData, valueDataSize};
                }
            }
        }
        RegCloseKey(fontsKey);
    } catch(...) {}
	return "C:/Windows/Fonts/" + font + ".ttf";
}

void Platform::playSystemSound(Sound s)
{
	switch (s)
	{
	case S_QUESTION:
		PlaySound(L"SystemQuestion", nullptr, SND_ASYNC);
		break;
		
	case S_ASTERISK:
		PlaySound(L"SystemAsterisk", nullptr, SND_ASYNC);
		break;
		
	}
}

float Platform::getDpiRatio()
{
	return GetDpiForSystem() / 96.f;
}
#else

AString Platform::getFontPath(const AString& font)
{
    if (std::filesystem::is_regular_file(font.toStdString()))
        return font;

    return "/usr/share/fonts/truetype/" + font;
}

void Platform::playSystemSound(Sound s)
{
    // unsupported
}

float Platform::getDpiRatio()
{
    return 1.f;
}

#endif