﻿#include "AUrl.h"

#include <functional>

#include "AUI/Common/AMap.h"
#include "AUI/Util/BuiltinFiles.h"

AUrl::AUrl(const AString& full)
{
	auto posColon = full.find(':');
	if (posColon == AString::NPOS)
	{
		mPath = full;
		mProtocol = "file";
	} else
	{
		mProtocol = full.mid(0, posColon);
		if (mProtocol.empty())
		{
			mProtocol = "builtin";
			mPath = full.mid(posColon + 1);
		} else
		{
			auto posHost = posColon + 3;
			auto posSlash = full.find('/', posHost);
			if (posSlash == AString::NPOS)
				posSlash = 0;
			mHost = full.mid(posHost, posSlash - posHost);
			if (posSlash)
			{
				mPath = full.mid(posSlash + 1);
			}
		}
	}
}

_<IInputStream> AUrl::open() const {
	static AMap<AString, std::function<_<IInputStream>(const AUrl&)>> resolvers = {
		{"builtin", [](const AUrl& u)
		{
			return BuiltinFiles::open(u.mPath);
		}}
	};

	return resolvers[mProtocol](*this);
}
