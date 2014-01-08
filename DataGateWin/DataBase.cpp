#include "stdafx.h"

#include "DataBase.h"

#include "StringService.h"

namespace
{

const std::string query_connection_db = "dbname={dbname} host={host} user={user} password={password}";

const std::string tag_dbname = "{dbname}";
const std::string tag_host = "{host}";
const std::string tag_user = "{user}";
const std::string tag_password = "{password}";

const std::string dbname = "data_gate";
const std::string host = "localhost";
const std::string user = "postgres";
const std::string password = "12345";

const std::string query_check_user_id =
	"SELECT * "
	"FROM users "
	"WHERE users.user_id = '{checked_user_id}'";
const std::string query_check_low_rank_client =
	"SELECT * "
	"FROM low_clients "
	"WHERE low_clients.user_id = '{checked_user_id}' AND low_clients.client_id = '{checked_lc_id}'";

const std::string tag_checked_user_id = "{checked_user_id}";
const std::string tag_checked_lc_id = "{checked_lc_id}";

} // namespace

data_base::data_base(const std::string& dbname,
	const std::string& host,
	const std::string& user,
	const std::string& password)
	: is_connected_(false)
{
	std::string connection_query = query_connection_db;
	StringService::Replace(connection_query, tag_dbname, dbname);
	StringService::Replace(connection_query, tag_host, host);
	StringService::Replace(connection_query, tag_user, user);
	StringService::Replace(connection_query, tag_password, password);

	conn = PQconnectdb(connection_query.c_str());

	if (PQstatus(conn) == CONNECTION_BAD)
	{
		is_connected_ = true;
	}
}

data_base::~data_base()
{
	PQfinish(conn);
}

bool data_base::check_user_id(const std::string& user_id)
{
	std::string query = query_check_user_id;
	StringService::Replace(query, tag_checked_user_id, user_id);

	res = PQexec(conn, query_check_user_id.c_str());

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		return false;
	}

	int rec_count = PQntuples(res);

	PQclear(res);

	if (rec_count == 1)
	{
		return true;
	}
	return false;
}

bool data_base::check_lc_id(const std::string& user_id, const std::string& lc_id)
{
	std::string query = query_check_user_id;
	StringService::Replace(query, tag_checked_user_id, user_id);
	StringService::Replace(query, tag_checked_lc_id, lc_id);

	res = PQexec(conn, query_check_user_id.c_str());

	if (PQresultStatus(res) != PGRES_TUPLES_OK)
	{
		return false;
	}

	int rec_count = PQntuples(res);

	PQclear(res);

	if (rec_count == 1)
	{
		return true;
	}
	return false;
}
