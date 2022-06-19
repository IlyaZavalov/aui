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

#pragma once
#include <cstdio>
#include "AUI/Core.h"
#include "IInputStream.h"

class AString;

class API_AUI_CORE AFileInputStream: public IInputStream
{
private:
	FILE* mFile;
	
public:
	AFileInputStream(const AString& path);
	virtual ~AFileInputStream();

    AFileInputStream(AFileInputStream&& rhs) noexcept {
        operator=(std::move(rhs));
    }
    AFileInputStream& operator=(AFileInputStream&& rhs) noexcept {
        mFile = rhs.mFile;
        rhs.mFile = nullptr;
        return *this;
    }

    FILE* nativeHandle() const {
        return mFile;
    }

	enum class Seek {
	    /**
	     * Seek relatively to the begin of file
	     */
	    BEGIN,

	    /**
	     * Seek relatively to the current position
	     */
	    CURRENT,

	    /**
	     * Seek relative to the end of file
	     */
	    END
	};

	void seek(std::streamoff offset, Seek dir) noexcept;
	void seek(std::streampos pos) noexcept;
    std::streampos tell() noexcept;

	size_t read(char* dst, size_t size) override;
};
