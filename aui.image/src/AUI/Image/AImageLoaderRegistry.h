﻿// AUI Framework - Declarative UI toolkit for modern C++17
// Copyright (C) 2020-2022 Alex2772
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

#pragma once

#include <AUI/Url/AUrl.h>
#include <AUI/Logging/ALogger.h>
#include <AUI/Util/Cache.h>
#include "IImageLoader.h"
#include "AUI/Common/ADeque.h"
#include "AUI/Common/SharedPtr.h"

/**
 * Image loader used for IDrawable::fromUrl and AImage::fromUrl
 */
class API_AUI_IMAGE AImageLoaderRegistry
{
    friend class AImage::Cache;
    friend class IDrawable;
    friend class AImage;

private:
	ADeque<_<IImageLoader>> mImageLoaders;

    _<IImageFactory> loadVector(AByteBufferView buffer);
    _<AImage> loadRaster(AByteBufferView buffer);
    inline _<IImageFactory> loadVector(const AUrl& url) {
        auto s = AByteBuffer::fromStream(url.open());
        return loadVector(s);
    }
    _<AImage> loadImage(const AUrl& url);

public:
	AImageLoaderRegistry() = default;

	void registerImageLoader(_<IImageLoader> imageLoader);

	static AImageLoaderRegistry& inst();
};
