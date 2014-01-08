#ifndef _DATABASE_H_
#define _DATABASE_H_

class data_base
{
public:
	data_base(const std::string& dbname,
		const std::string& host,
		const std::string& user,
		const std::string& password);
	~data_base();

	bool is_connected() {return is_connected_;}

	bool check_user_id(const std::string& user_id);
	bool check_lc_id(const std::string& user_id, const std::string& lc_id);

private:
	PGconn *conn;
	PGresult *res;
	bool is_connected_;
};

#endif // _DATABASE_H_
