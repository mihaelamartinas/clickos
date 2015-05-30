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
MigrationReceiver :: storeMap(Map *map, Protocol::MigrationHeader::MigrationType type,
							size_t no_maps)
{
	size_t i, j, packet_size;
	Protocol::MapEntry entry;
	HashContainer<IPRewriterEntry>::iterator it;
	IPRewriterEntry *ipRwEntry;
	IPFlowID *flowid;

	/* receive all the mappings one by one */
	for (i = 0; i < no_maps; i++) {
		if (socket->recv(&entry, sizeof(Protocol::MapEntry)) < 0) {
			click_chatter("Error receiving %d mappings\n", type);
			return;
		}

		click_chatter("bucket_id = %d, bucket_size = %d\n", entry.bucket_id, entry.bucket_size);

		/* receive bucket content */
		struct Protocol::FlowInfo bucket_flows[entry.bucket_size];
		packet_size = entry.bucket_size * sizeof(Protocol::FlowInfo);
		if (socket->recvNarrowed(bucket_flows, packet_size) < 0) {
			click_chatter("Error receiving flows in bucket %d\n",
					entry.bucket_id);
			return;
		}

		/* find position for each flow and add it to the map */
		for (j = 0; j < entry.bucket_size; j++) {
			ipRwEntry = new IPRewriterEntry();
			flowid = new IPFlowID(bucket_flows[j].flowId.s_addr,
					bucket_flows[j].flowId.d_addr,
					bucket_flows[j].flowId.s_port,
					bucket_flows[j].flowId.d_port);
			ipRwEntry->initialize(*flowid, bucket_flows[j].output, bucket_flows[j].direction);
			it = map->find(*flowid);
			map->insert_at(it, ipRwEntry);
		}
	}
}

void
MigrationReceiver :: storeHeap(IPRewriterHeap **heap, Protocol::MigrationHeader::MigrationType type, size_t no_flows)
{
	size_t i, packet_size;
	IPRewriterFlow *ipRwFlow;
	struct Protocol::HeapEntry *entries = new Protocol::HeapEntry[no_flows];
	int heap_type = (type == Protocol::MigrationHeader::T_HEAP_GUARANTEED)?
				IPRewriterHeap::h_guarantee : IPRewriterHeap::h_best_effort;

	packet_size = no_flows * sizeof(Protocol::HeapEntry);
	if(socket->recvNarrowed(entries, packet_size) < 0) {
		click_chatter("Error receiving heap information\n");
		return;
	}

	/** TODO - add to heap
	Vector<IPRewriterFlow *> &myheap = heap->_heaps[heap_type];
	for (i = 0; i < no_flows; i++) {
		ipRwFlow = 
		myheap.push_back(flow);
		push_heap(myheap.begin(), myheap.end(),
				IPRewriterFlow::heap_less(), IPRewriterFlow::heap_place());
	}
	*/
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
				storeMap(tcp_map, Protocol::MigrationHeader::T_MAP_TCP,
						migrationInfo.no_mappings_tcp);
				break;
			case Protocol::MigrationHeader::T_MAP_UDP:
				if (migrationInfo.no_mappings_udp < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeMap(udp_map, Protocol::MigrationHeader::T_MAP_UDP,
						migrationInfo.no_mappings_udp);
				break;
			case Protocol::MigrationHeader::T_HEAP_GUARANTEED:
				if (migrationInfo.no_heap_guranteed < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeHeap(heap, Protocol::MigrationHeader::T_HEAP_GUARANTEED,
						migrationInfo.no_heap_guranteed);
				break;
			case Protocol::MigrationHeader::T_HEAP_BEST_EFFORT:
				if (migrationInfo.no_heap_best_effort < 0) {
					click_chatter("ERROR: the information header wasn't received\n");
					goto out;
				}
				storeHeap(heap, Protocol::MigrationHeader::T_HEAP_BEST_EFFORT,
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
