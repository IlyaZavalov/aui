// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2023 Alex2772
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library. If not, see <http://www.gnu.org/licenses/>.

//
// Created by alex2 on 07.11.2020.
//

#include <AUI/Url/AUrl.h>
#include <AUI/Util/ATokenizer.h>
#include "AI18n.h"
#include <AUI/Traits/strings.h>
#include <AUI/Logging/ALogger.h>


AI18n::AI18n() {
    // TODO hardcoded
    mLangData["aui.cut"] = "Cut";
    mLangData["aui.copy"] = "Copy";
    mLangData["aui.paste"] = "Paste";
    mLangData["aui.select_all"] = "Select all";

    try {
        loadFromLang(userLanguage());
    } catch (...) {
        try {
            loadFromLang("en-US");
            ALogger::warn("User native language ({}) is not supported, using en-US"_format(userLanguage().toString()));
        } catch (...) {
            if (userLanguage().toString() == "en-US") {
                ALogger::err("US English (en-US) language file has not found! Please add at least en-US.lang file "
                             "if you want to use AUI _i18n feature.");
            } else {
                ALogger::err("Neither user native ({}) nor US English (en-US) language file have not found! Please"
                             "add at least en-US.lang file if you want to use AUI _i18n feature."_as
                             .format(userLanguage().toString()));
            }
        }
    }
}

void AI18n::loadFromStreamInto(const _<IInputStream>& iis, AMap<AString, AString>& langData) {
    ATokenizer t(iis);
    bool running = true;
    try {
        while (running) {
            if (t.readChar() == '#') {
                // comment
                t.skipUntilUnescaped('\n');
            } else {
                // string
                t.reverseByte();

                auto key = t.readStringUntilUnescaped('=');
                std::string value;
                try {
                    t.readStringUntilUnescaped(value, '\n');
                } catch (...) {
                    running = false;
                }

                langData[key] = AString(value).processEscapes();
            }
        }
    } catch (...) {

    }
}

AI18n& AI18n::inst() {
    static AI18n a;
    return a;
}

void AI18n::loadFromLang(const ALanguageCode& languageCode) {
    loadFromStream(AUrl(":lang/{}.lang"_format(languageCode.toString())).open());
}

void AI18n::loadFromStream(const _<IInputStream>& iis) {
    loadFromStreamInto(iis, mLangData);
}

#if AUI_PLATFORM_WIN

#include <windows.h>

ALanguageCode AI18n::userLanguage() {
    wchar_t buf[64];
    GetUserDefaultLocaleName(buf, sizeof(buf) / 2);
    return AString(buf);
}

#else

#include <langinfo.h>

ALanguageCode AI18n::userLanguage() {

    char* s;
    s = getenv("LANG");
    if (s) {
        try {
            AString code(s, s + 5);
            code[2] = '-';
            return code;
        } catch (...) {

        }
    }
    return "en-US";
}

#endif