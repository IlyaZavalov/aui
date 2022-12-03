// AUI Framework - Declarative UI toolkit for modern C++17
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

#include <AUI/Data.h>
#include "AUI/Common/SharedPtrTypes.h"
#include "AUI/Common/AMap.h"
#include "ISqlDriver.h"
#include "AUI/Common/AVector.h"
#include "AUI/Common/AVariant.h"
#include "ASqlQueryResult.h"
#include "ASqlDriverType.h"

class AString;

class API_AUI_DATA ASqlDatabase
{
private:
	static AMap<AString, _<ISqlDriver>>& getDrivers();

	_<ISqlDatabase> mDriverInterface;

	explicit ASqlDatabase(const _<ISqlDatabase>& driver_interface, const AString& driverName)
		: mDriverInterface(driver_interface)
	{
	}

public:
	~ASqlDatabase();

    /**
     * @brief Execute a query with the result (SELECT).
     *
     * @param query the SQL query
     * @param params query arguments
     * @return query result
     * \throws SQLException when any error occurs
     */
	_<ASqlQueryResult> query(const AString& query, const AVector<AVariant>& params = {});

	/**
	 * @brief Execute a query with no result (UPDATE, INSERT, DELETE, etc.)
     *
     * @param query the SQL query
     * @param params query arguments
     * @return number of affected rows
     * \throws SQLException if any error occurs
	 */
	int execute(const AString& query, const AVector<AVariant>& params = {});


	/**
	 * @brief Connect to the database using the specified details and driver.
     * @param drivername name of the database driver. If the driver is not loaded, an attempt will be made to load the
     *        driver based on the template aui.DRIVERNAME (aui.DRIVERNAME.dll for Windows, libaui.DRIVERNAME.so for
     *        Linux, etc...). If the driver failed to load SQLException will be thrown.
     *
     * @param address server host (IP address or domain)
     * @param port server port
     * @param databaseName name of the database
     * @param username user name; optional in some DBMS
     * @param password user password; optional in some DBMS
     * @return object for communicating with the database; in some DBMS - optional
     * \throws SQLException when any error occurs
	 */
	static _<ASqlDatabase> connect(const AString& driverName, const AString& address, uint16_t port = 0,
	                               const AString& databaseName = {}, const AString& username = {},
	                               const AString& password = {});

	/**
	 * @brief the type of the driver. Required to correct queries in the database due to driver differences.
     * @return type of driver
	 */
	SqlDriverType getDriverType();

	static void registerDriver(_<ISqlDriver> driver);
};
