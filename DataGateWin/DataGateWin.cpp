#include "stdafx.h"

#include <conio.h>

#include <iostream>

int main(int argc, char **argv)
{
	PGconn *conn;
	PGresult *res;

	conn = PQconnectdb("dbname=data_gate host=localhost user=postgres password=12345");
	if (PQstatus(conn) == CONNECTION_BAD)
	{
		puts("Не удается подключиться к базе данных");
		return 0;
	}

	res = PQexec(conn, "SELECT * FROM low_clients WHERE low_clients.user_id = 'user1' AND low_clients.client_id = 'lc1'");

	if (PQresultStatus(res) != PGRES_TUPLES_OK) {
		puts("Мы не получили данные");
		return 0;
	}

	int rec_count = PQntuples(res);
	for (int i = 0; i < rec_count; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			std::cout << PQgetvalue(res, i, j) << " | ";
		}
		std::cout << std::endl;
	}

	getch();

	return 0;
}

