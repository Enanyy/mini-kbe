/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "entity_table_mysql.h"
#include "kbe_table_mysql.h"
#include "db_exception.h"
#include "db_interface_mysql.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
//#include "entitydef/entitydef.h"
//#include "entitydef/scriptdef_module.h"
#include "server/serverconfig.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::syncToDB(DBInterface* dbi)
{
	std::string sqlstr = "DROP TABLE kbe_entitylog;";

	try
	{
		dbi->query(sqlstr.c_str(), sqlstr.size(), false);
	}
	catch(...)
	{
	}
	
	bool ret = false;

	sqlstr = "CREATE TABLE IF NOT EXISTS kbe_entitylog "
			"(entityDBID bigint(20) unsigned not null DEFAULT 0,"
			"entityID int unsigned not null DEFAULT 0,"
			"ip varchar(64),"
			"port int unsigned not null DEFAULT 0,"
			"componentID bigint unsigned not null DEFAULT 0,"
			"PRIMARY KEY (entityDBID))"
		"ENGINE=" MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::logEntity(DBInterface * dbi, const char* ip, uint32 port, DBID dbid,
					COMPONENT_ID componentID, ENTITY_ID entityID)
{
	std::string sqlstr = "insert into kbe_entitylog (entityDBID, entityID, ip, port, componentID) values(";

	char* tbuf = new char[MAX_BUF * 3];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;
	sqlstr += ",";
	
	//kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	//sqlstr += tbuf;
	//sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%d", entityID);
	sqlstr += tbuf;
	sqlstr += ",\"";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, ip, strlen(ip));

	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", port);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, componentID);
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	try
	{
		if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		{
			// 1062 int err = dbi->getlasterror(); 
			return false;
		}
	}
	catch (std::exception & e)
	{
		DBException& dbe = static_cast<DBException&>(e);
		if(dbe.isLostConnection())
		{
			if(dbi->processException(e))
				return true;
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::queryEntity(DBInterface * dbi, DBID dbid, EntityLog& entitylog)
{
	std::string sqlstr = "select entityID, ip, port, componentID from kbe_entitylog where entityDBID=";

	char tbuf[MAX_BUF];
	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;
	
	//sqlstr += " and entityType=";
	//kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	//sqlstr += tbuf;
	sqlstr += " LIMIT 1";

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return true;
	}
	
	entitylog.dbid = dbid;
	entitylog.componentID = 0;
	entitylog.entityID = 0;
	entitylog.ip[0] = '\0';
	entitylog.port = 0;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			StringConv::str2value(entitylog.entityID, arow[0]);
			kbe_snprintf(entitylog.ip, MAX_IP, "%s", arow[1]);
			StringConv::str2value(entitylog.port, arow[2]);
			StringConv::str2value(entitylog.componentID, arow[3]);
		}

		mysql_free_result(pResult);
	}

	return entitylog.componentID > 0;
}

//-------------------------------------------------------------------------------------
bool KBEEntityLogTableMysql::eraseEntityLog(DBInterface * dbi, DBID dbid)
{
	std::string sqlstr = "delete from kbe_entitylog where entityDBID=";

	char tbuf[MAX_BUF];

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, dbid);
	sqlstr += tbuf;

	/*sqlstr += " and entityType=";
	kbe_snprintf(tbuf, MAX_BUF, "%u", entityType);
	sqlstr += tbuf;*/

	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
KBEEntityLogTableMysql::KBEEntityLogTableMysql():
	KBEEntityLogTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::syncToDB(DBInterface* dbi)
{
	bool ret = false;

	std::string sqlstr = "CREATE TABLE IF NOT EXISTS kbe_accountinfos "
			"(`accountName` varchar(255) not null, PRIMARY KEY idKey (`accountName`),"
			"`password` varchar(255),"
			"`bindata` blob,"
			"`email` varchar(255),"
			"`entityDBID` bigint(20) unsigned not null DEFAULT 0, UNIQUE KEY `entityDBID` (`entityDBID`),"
			"`flags` int unsigned not null DEFAULT 0,"
			"`deadline` bigint(20) not null DEFAULT 0,"
			"`regtime` bigint(20) not null DEFAULT 0,"
			"`lasttime` bigint(20) not null DEFAULT 0,"
			"`numlogin` int unsigned not null DEFAULT 0)"
		"ENGINE=" MYSQL_ENGINE_TYPE;

	ret = dbi->query(sqlstr.c_str(), sqlstr.size(), true);
	KBE_ASSERT(ret);
	return ret;
}

//-------------------------------------------------------------------------------------
KBEAccountTableMysql::KBEAccountTableMysql():
	KBEAccountTable()
{
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::setFlagsDeadline(DBInterface * dbi, const std::string& name, uint32 flags, uint64 deadline)
{
	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	std::string sqlstr = fmt::format("update kbe_accountinfos set flags={}, deadline={} where accountName=\"{}\"", 
		flags, deadline, tbuf);

	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	return false;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccount(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, flags, deadline from kbe_accountinfos where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;
	sqlstr += "\" LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			KBEngine::StringConv::str2value(info.dbid, arow[0]);
			info.name = name;
			info.password = arow[1];

			KBEngine::StringConv::str2value(info.flags, arow[2]);
			KBEngine::StringConv::str2value(info.deadline, arow[3]);
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::queryAccountAllInfos(DBInterface * dbi, const std::string& name, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "select entityDBID, password, email, flags, deadline from kbe_accountinfos where accountName=\"";

	char* tbuf = new char[name.size() * 2 + 1];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, name.c_str(), name.size());

	sqlstr += tbuf;
	sqlstr += "\" LIMIT 1";
	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
		return true;

	info.dbid = 0;
	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(dbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow = mysql_fetch_row(pResult);
		if(arow != NULL)
		{
			KBEngine::StringConv::str2value(info.dbid, arow[0]);
			info.name = name;
			info.password = arow[1];
			info.email = arow[2];
			KBEngine::StringConv::str2value(info.flags, arow[3]);
			KBEngine::StringConv::str2value(info.deadline, arow[4]);
		}

		mysql_free_result(pResult);
	}

	return info.dbid > 0;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::updateCount(DBInterface * dbi, const std::string& name, DBID dbid)
{
	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(fmt::format("update kbe_accountinfos set lasttime={}, numlogin=numlogin+1 where entityDBID={}",
		time(NULL), dbid), false))
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::updatePassword(DBInterface * dbi, const std::string& name, const std::string& password)
{
	char* tbuf = new char[MAX_BUF * 3];
	char* tbuf1 = new char[MAX_BUF * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, password.c_str(), password.size());

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf1, name.c_str(), name.size());

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(fmt::format("update kbe_accountinfos set password=\"{}\" where accountName like \"{}\"", 
		password, tbuf1), false))
	{
		SAFE_RELEASE_ARRAY(tbuf);
		SAFE_RELEASE_ARRAY(tbuf1);
		return false;
	}
	
	SAFE_RELEASE_ARRAY(tbuf);
	SAFE_RELEASE_ARRAY(tbuf1);
	return true;
}

//-------------------------------------------------------------------------------------
bool KBEAccountTableMysql::logAccount(DBInterface * dbi, ACCOUNT_INFOS& info)
{
	std::string sqlstr = "insert into kbe_accountinfos (accountName, password, bindata, email, entityDBID, flags, deadline, regtime, lasttime) values(";

	char* tbuf = new char[MAX_BUF > info.datas.size() ? MAX_BUF * 3 : info.datas.size() * 3];

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.name.c_str(), info.name.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.password.c_str(), info.password.size());

	sqlstr += "md5(\"";
	sqlstr += tbuf;
	sqlstr += "\"),";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.datas.data(), info.datas.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, info.email.c_str(), info.email.size());

	sqlstr += "\"";
	sqlstr += tbuf;
	sqlstr += "\",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRDBID, info.dbid);
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%u", info.flags);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRIu64, info.deadline);
	sqlstr += tbuf;
	sqlstr += ",";
	
	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ",";

	kbe_snprintf(tbuf, MAX_BUF, "%" PRTime, time(NULL));
	sqlstr += tbuf;
	sqlstr += ")";

	SAFE_RELEASE_ARRAY(tbuf);

	// 如果查询失败则返回存在， 避免可能产生的错误
	if(!dbi->query(sqlstr.c_str(), sqlstr.size(), false))
	{
		ERROR_MSG(fmt::format("KBEAccountTableMysql::logAccount({}): sql({}) is failed({})!\n", 
				info.name, sqlstr, dbi->getstrerror()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
}
