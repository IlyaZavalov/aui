﻿// AUI Framework - Declarative UI toolkit for modern C++20
// Copyright (C) 2020-2024 Alex2772 and Contributors
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
#include "AUI/Rsa.h"
#include "AUI/Common/AByteBuffer.h"

/**
 * @brief x509 certificate object.
 * @ingroup crypt
 */
class API_AUI_CRYPT AX509: public aui::noncopyable
{
	friend class AX509Store;
private:
	void* mX509;

	explicit AX509(void* x509);

public:
	~AX509();

	bool isCA() const;
	bool checkHost(const AString& name) const;
	bool checkTrust() const;
	
	static _<AX509> fromPEM(AByteBufferView buffer);
};
