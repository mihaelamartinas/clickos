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
	size_t i, packet_size;
	Protocol::MapEntry entry;

	/* receive all the mappings one by one */
	for (i = 0; i < no_maps; i++) {
		if (socket->recv(&entry, sizeof(Protocol::MapEntry)) < 0) {
			click_chatter("Error receiving %d mappings\n", type);
			return;
		}

		click_chatter("bucket_id = %d, bucket_size = %d\n", entry.bucket_id, entry.bucket_size);

		/* receive bucket content */
		struct Protocol::FlowID bucket_flows[entry.bucket_size];
		packet_size = entry.bucket_size * sizeof(Protocol::FlowID);
		if (socket->recvNarrowed(bucket_flows, packet_size) < 0) {
			click_chatter("Error receiving flows in bucket %d\n", entry.bucket_id);
			return;
		}
	}
}

void
MigrationReceiver :: storeHeap(Protocol::MigrationHeader::MigrationType type, size_t no_flows)
{
	size_t packet_size;
	struct Protocol::HeapEntry *entries = new Protocol::HeapEntry[no_flows];

	int heap_type = (type == Protocol::MigrationHeader::T_HEAP_GUARANTEED)?
				IPRewriterHeap::h_guarantee : IPRewriterHeap::h_best_effort;

	packet_size = no_flows * sizeof(Protocol::HeapEntry);
	if(socket->recvNarrowed(entries, packet_size) < 0) {
		click_chatter("Error receiving heap information\n");
		return;
	}
}

void MigrationReceiver :: initMigrationInfo(Protocol::MigrationInfo *migrationInfo)
{
	migrationInfo->no_mappings_tcp = -1;
	migrationInfo->no_mappings_udp = -1;
	migrationInfo->no_heap_guranteed = -1;
	migrationInfo->no_heap_best_effort = -1;
}

void MigrationReceiver :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	Protocol::MigrationHeader header;
	Protocol::MigrationInfo migrationInfo;

	initMigrationInfo(&migrationInfo);
	
	while(true) {
		if (socket->recvNarrowed(&header, sizeof(Protocol::MigrationHeader::MigrationType)) < 0) {
			click_chatter("Error reading message header\n");
			goto out;
		}

		switch(header.type) {
			case Protocol::MigrationHeader::T_INFO:
				memcpy(&migrationInfo,
						&header.migrationInfo,
						sizeof(header.migrationInfo));
				click_chatter("Migration info data:\n\tno_mappings_tcp = %d\n \
						\tno_mappings_udp = %d\n \
						\tno_heap_guranteed = %d\n \
						\tno_heap_best_effort = %d\n",
						migrationInfo.no_mappings_tcp,
						migrationInfo.no_mappings_udp,
						migrationInfo.no_heap_guranteed,
						migrationInfo.no_heap_best_effort);
				print_info_header(header);
				goto out;
				break;
			case Protocol::MigrationHeader::T_MAP_TCP:
				if (migrationInfo.no_mappings_tcp < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeMap(Protocol::MigrationHeader::T_MAP_TCP,
						migrationInfo.no_mappings_tcp);
				break;
			case Protocol::MigrationHeader::T_MAP_UDP:
				if (migrationInfo.no_mappings_udp < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeMap(Protocol::MigrationHeader::T_MAP_UDP,
						migrationInfo.no_mappings_udp);
				break;
			case Protocol::MigrationHeader::T_HEAP_GUARANTEED:
				if (migrationInfo.no_heap_guranteed < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeHeap(Protocol::MigrationHeader::T_HEAP_GUARANTEED,
						migrationInfo.no_heap_guranteed);
				break;
			case Protocol::MigrationHeader::T_HEAP_BEST_EFFORT:
				if (migrationInfo.no_heap_best_effort < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeHeap(Protocol::MigrationHeader::T_HEAP_BEST_EFFORT,
						migrationInfo.no_heap_best_effort);
				goto out;
				break;
			default:
				click_chatter("Unknown header type\n");
				goto out;
		}
	}
out:
	socket->close();
}
