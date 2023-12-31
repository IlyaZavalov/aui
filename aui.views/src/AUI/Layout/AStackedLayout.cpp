﻿// AUI Framework - Declarative UI toolkit for modern C++20
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

#include "AStackedLayout.h"


void ::AStackedLayout::onResize(int x, int y, int width, int height)
{
	for (auto& v : mViews)
	{
	    v->ensureAssUpdated();
		int finalX, finalY, finalWidth, finalHeight;
        auto margins = v->getMargin();
		if (v->getExpandingHorizontal() == 0)
		{
			finalWidth = glm::min(v->getMinimumWidth() + margins.horizontal(), width);
			finalX = (width - finalWidth) / 2;
		} else
		{
			finalX = 0;
			finalWidth = width;
		}
		if (v->getExpandingVertical() == 0)
		{
			finalHeight = glm::min(v->getMinimumHeight() + margins.vertical(), width);
			finalY = (height - finalHeight) / 2;
		} else
		{
			finalY = 0;
			finalHeight = height;
		}
		v->setGeometry(finalX + x + margins.left,
                   finalY + y + margins.top,
                   finalWidth - margins.horizontal(),
                   finalHeight - margins.vertical());
		// if view rejected the specified size, then we would recenter it
		if (v->getSize() != glm::ivec2(finalWidth - margins.horizontal(), finalHeight - margins.vertical())) {
			glm::ivec2 correctedSize = v->getSize() + margins.horizontal();
			glm::ivec2 correctedPos = (glm::ivec2(width, height) - correctedSize) / 2;
			v->setPosition(correctedPos + glm::ivec2(x + margins.left, y + margins.top));
		}
	}
}
int ::AStackedLayout::getMinimumWidth()
{
	int m = 0;
	for (auto& v : mViews)
		if (v->getVisibility() != Visibility::GONE)
			m = glm::max(int(v->getMinimumWidth() + v->getMargin().horizontal()), m);
	return m;
}

int ::AStackedLayout::getMinimumHeight()
{
	int m = 0;
	for (auto& v : mViews) {
		if (v->getVisibility() != Visibility::GONE)
			m = glm::max(int(v->getMinimumHeight() + v->getMargin().vertical()), m);
	}
	return m;
}
