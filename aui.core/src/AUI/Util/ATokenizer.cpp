﻿/*
 * =====================================================================================================================
 * Copyright (c) 2021 Alex2772
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
 * Original code located at https://github.com/aui-framework/aui
 * =====================================================================================================================
 */

#include "ATokenizer.h"
#include "AUI/Common/AStringVector.h"
#include "AUI/Common/AMap.h"
#include "AUI/Common/ASet.h"
#include "AUI/IO/AStringStream.h"

ATokenizer::ATokenizer(const AString& fromString):
    mInput(_new<AStringStream>(fromString))
{
}

const std::string& ATokenizer::readString()
{
    mTemporaryStringBuffer.clear();
    char c;

    try
    {
        for (;;) {
            c = readChar();
            if (isalnum(uint8_t (c)))
            {
                mTemporaryStringBuffer.push_back(c);
            }
            else
            {
                reverseByte();
                return mTemporaryStringBuffer;
            }
        }
    } catch (...)
    {
        mEof = true;
    }
    return mTemporaryStringBuffer;
}
const std::string& ATokenizer::readString(const ASet<char>& applicableChars)
{
    mTemporaryStringBuffer.clear();
    char c;

    try
    {
        for (;;) {
            c = readChar();
            if (isalnum(uint8_t (c)) || applicableChars.find(c) != applicableChars.end())
            {
                mTemporaryStringBuffer.push_back(c);
            }
            else
            {
                reverseByte();
                return mTemporaryStringBuffer;
            }
        }
    } catch (...)
    {
        mEof = true;
    }
    return mTemporaryStringBuffer;
}


void ATokenizer::reverseByte()
{
    mReverse = true;
}

float ATokenizer::readFloat()
{
    mTemporaryAStringBuffer.clear();
    try {
        bool dot = false;
        char c;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '-':
                mTemporaryAStringBuffer << c;
                break;
            case '.':
                if (!dot)
                {
                    mTemporaryAStringBuffer << c;
                    break;
                }
            default:
                reverseByte();
                return mTemporaryAStringBuffer.toFloat().valueOr(0);
            }
        }
    } catch (...) {
        mEof = true;
    }
    return mTemporaryAStringBuffer.toFloat().valueOr(0);
}

template<typename T>
T ATokenizer::readIntImpl() {
    static_assert(std::is_integral_v<T>, "readIntImpl accepts only integral type");
    mTemporaryAStringBuffer.clear();
    auto value = [&] {
        if constexpr(sizeof(T) > 4) {
            return mTemporaryAStringBuffer.toLongInt().valueOr(0);
        } else {
            return mTemporaryAStringBuffer.toInt().valueOr(0);
        }
    };
    try {
        char c;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'x':
                case 'X':

                    // hex
                case 'a':
                case 'A':
                case 'b':
                case 'B':
                case 'c':
                case 'C':
                case 'd':
                case 'D':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case '-':
                    mTemporaryAStringBuffer << c;
                    break;
                default:
                    reverseByte();
                    return value();
            }
        }
    }
    catch (...) {
        mEof = true;
    }
    return value();
}

ATokenizer::Hexable<unsigned> ATokenizer::readUIntX() {
    mTemporaryAStringBuffer.clear();
    bool isHex = false;
    try {
        char c;
        for (;;)
        {
            c = readChar();
            switch (c)
            {
                case 'x':
                case 'X':
                    isHex = true;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':

                    // hex
                case 'a':
                case 'A':
                case 'b':
                case 'B':
                case 'c':
                case 'C':
                case 'd':
                case 'D':
                case 'e':
                case 'E':
                case 'f':
                case 'F':
                    mTemporaryAStringBuffer << c;
                    break;
                default:
                    reverseByte();
                    return {mTemporaryAStringBuffer.toUInt().valueOr(0), isHex};
            }
        }
    }
    catch (...) {
        mEof = true;
    }
    return {mTemporaryAStringBuffer.toUInt().valueOr(0), isHex};
}

void ATokenizer::skipUntilUnescaped(char c) {
    for (char current; (current = readChar()) != c;)
    {
        if (current == '\\')
        {
            readChar();
        }
    }
}

const std::string& ATokenizer::readStringUntilUnescaped(char c)
{
    mTemporaryStringBuffer.clear();
    readStringUntilUnescaped(mTemporaryStringBuffer, c);
    return mTemporaryStringBuffer;
}
const std::string& ATokenizer::readStringUntilUnescaped(const ASet<char>& characters) {
    mTemporaryStringBuffer.clear();
    readStringUntilUnescaped(mTemporaryStringBuffer, characters);
    return mTemporaryStringBuffer;
}

void ATokenizer::readStringUntilUnescaped(std::string& out, char c)
{
    try {
        for (char current; (current = readChar()) != c;)
        {
            if (current == '\\')
            {
                char tmp = readChar();
                switch (tmp) {
                    case 'r': out += '\r'; break;
                    case 'n': out += '\n'; break;
                    case 't': out += '\t'; break;
                    default: out += tmp;
                }
            }
            else
            {
                out += current;
            }
        }
    }
    catch (...) {
        mEof = true;
    }
}

void ATokenizer::readStringUntilUnescaped(std::string& out, const ASet<char>& characters) {
    try {
        for (char current; !characters.contains(current = readChar());)
        {
            if (current == '\\')
            {
                out += '\\';
                out += readChar();
            }
            else
            {
                out += current;
            }
        }
    }
    catch (...) {
        mEof = true;
    }
}

glm::vec2 ATokenizer::readVec2()
{
    glm::vec2 result;
    result.x = readFloat();
    readChar();
    result.y = readFloat();
    return result;
}

const std::string& ATokenizer::readString(size_t n) {
    mTemporaryStringBuffer.clear();
    mTemporaryStringBuffer.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        mTemporaryStringBuffer += readChar();
    }
    return mTemporaryStringBuffer;
}

void ATokenizer::skipUntil(char c) {
    for (char x; (x = readChar()) != c; );
}

unsigned ATokenizer::readUInt() {
    return readIntImpl<unsigned>();
}

int ATokenizer::readInt() {
    return readIntImpl<int>();
}

int64_t ATokenizer::readLongInt() {
    return readIntImpl<int64_t>();
}
