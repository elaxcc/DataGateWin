#include "stdafx.h"

#include "LowRankClients.h"
#include "HighRankClients.h"
#include "DataBase.h"

int main(int argc, char **argv)
{
	Net::init();

	data_base db("data_gate", "localhost", "postgres", "12345");
	Net::net_manager net_manager;
	Net::local_communicator_manager local_comm_manager;

	high_rank_clients::server hs(&db, &net_manager, &local_comm_manager, 1234, true, true);
	low_rank_clients::server ls(&db, &net_manager, &hs, &local_comm_manager, 1235, true, true);

	net_manager.add_member(&hs);
	net_manager.add_member(&ls);

	while(true)
	{
		net_manager.process_sockets();
		local_comm_manager.process();
	}

	return 0;
}

