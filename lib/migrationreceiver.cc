#include <click/config.h>
#include <click/migrationreceiver.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>

int MigrationReceiver :: connectToMachine()
{
	int ret = 0;

	TCPSocket socketTmp;
	TCPSocket listeningSocket(port);

	/* accept connection from the remote machine */
	if ( listeningSocket.listen() < 0) {
		ret = -1;
		goto out;
	}

	socketTmp = listeningSocket.accept();
	if (socket == NULL) {
		click_chatter("Error accepting remote connection\n");
		ret = -1;
		goto free_socket;
	}

	socket = &socketTmp;
	click_chatter("Accepted connection from sender\n");

free_socket:
	listeningSocket.close();
out:
	return ret;
}

void
MigrationReceiver :: storeMap(Protocol::MigrationHeader::MigrationType type, size_t no_maps)
{
	size_t i, value_id, recv_size;
	Protocol::MapEntry entry;

	/* receive all the mappings one by one */
	for (i = 0; i < no_maps; i++) {
		if (socket->recv(&entry, sizeof(Protocol::MapEntry)) < 0) {
			click_chatter("Error receiving %d mappings\n", type);
			return;
		}

		click_chatter("bucket_id = %d, bucket_size = %d\n", entry.bucket_id, entry.bucket_size);

		/* receive bucket content */
		struct Protocol::FlowEntry bucket_flows[entry.bucket_size];
		if (socket->recv(bucket_flows, entry.bucket_size * sizeof(Protocol::FlowEntry)) < 0) {
			click_chatter("Error receiving flows in bucket %d\n", entry.bucket_id);
			return;
		}
	}
}

void
MigrationReceiver :: storeHeap(Protocol::MigrationHeader::MigrationType type, size_t no_flows)
{
}

void MigrationReceiver :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	ssize_t recv_size;
	Protocol::MigrationHeader header;
	Protocol::MigrationInfo migrationInfo;
	
	while(true) {
		recv_size = socket->recv(&header, sizeof(Protocol::MigrationHeader));
		if (recv < 0) {
			click_chatter("Error reading message header\n");
			goto out;
		}

		switch(header.type) {
			case Protocol::MigrationHeader::T_INFO:
				memcpy(&migrationInfo, &header.migrationInfo, sizeof(header.migrationInfo));
				click_chatter("Migration info data:\n\tno_mappings_tcp = %d\n\tno_mappings_udp = %d\n\tno_heap_guranteed = %d\n\tno_heap_best_effort = %d\n",
						migrationInfo.no_mappings_tcp,
						migrationInfo.no_mappings_udp,
						migrationInfo.no_heap_guranteed,
						migrationInfo.no_heap_best_effort);
				print_info_header(header);
				goto out;
				break;
			case Protocol::MigrationHeader::T_MAP_TCP:
				storeMap(Protocol::MigrationHeader::T_MAP_TCP, migrationInfo.no_mappings_tcp);
				break;
			case Protocol::MigrationHeader::T_MAP_UDP:
				storeMap(Protocol::MigrationHeader::T_MAP_UDP, migrationInfo.no_mappings_udp);
				break;
			case Protocol::MigrationHeader::T_HEAP_GUARANTEED:
				break;
			case Protocol::MigrationHeader::T_HEAP_BEST_EFFORT:
				goto out;
				break;
		}
	}
out:
	socket->close();
}
