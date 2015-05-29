#include <click/config.h>
#include <click/migrationsender.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>
#include <click/protocol.hh>

CLICK_DECLS
int MigrationSender :: connectToMachine()
{
	int ret;

	socket = new TCPSocket(TCPSocket :: ANY_PORT);

	/* connect to the remote machine */
	return socket->connect(port, hostname);
}

void MigrationSender :: copyFlow(IPFlowID src, Protocol::FlowEntry *dst)
{
	dst->s_addr = src.saddr();
	dst->d_addr = src.daddr();
	dst->s_port = src.sport();
	dst->d_port = src.dport();
}

Protocol::MapEntry* MigrationSender :: unfoldMap(Map *flowMap, size_t *no_flows)
{
	size_t i, bucket_size, counter;
	size_t no_entries = 0;
	size_t no_buckets = flowMap->bucket_count();
	Protocol::MapEntry *entries = new Protocol::MapEntry[flowMap->size()];

	*no_flows = 0;

	for (i = 0; i < no_buckets; i++) {
		bucket_size = flowMap->bucket_size(i);
		if (bucket_size == 0)
			continue;

		/* the bucket contains values */
		HashContainer<IPRewriterEntry>::iterator it = flowMap->begin(i);
		entries[no_entries].bucket_id = i;
		entries[no_entries].bucket_size = bucket_size;
		*no_flows += bucket_size;

		Protocol::FlowEntry *flows = new Protocol::FlowEntry[bucket_size];

		for (counter = 0; it != flowMap->end(); it++, counter++) {
			copyFlow(it.get()->flowid(),
					&(entries[no_entries].flows[counter]));
		}
		memcpy(entries[no_entries].flows, flows, sizeof(Protocol::FlowEntry) * bucket_size);

		no_entries++;
	}

	return entries;
}

Protocol::FlowPair* MigrationSender::unfoldHeap(IPRewriterHeap **heap)
{
	return new Protocol::FlowPair[100];
}

void MigrationSender :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	ssize_t sent;
	size_t no_flows;
	/* send first packet containing size information */
	Protocol::MigrationHeader header;
	Protocol::MigrationHeader tcp_header;
	Protocol::MigrationHeader udp_header;

	header.type = Protocol::MigrationHeader::T_INFO;
	header.migrationInfo.no_mappings_tcp = tcp_map->size();
	header.migrationInfo.no_mappings_udp = udp_map->size();
	header.migrationInfo.no_heap_best_effort = (*heap)->size();
	click_chatter("Best_effort %d\n", (*heap)->size());
	header.migrationInfo.no_heap_guranteed = (*heap)->size();

	print_info_header(header);

	sent = socket->send(&header, sizeof(Protocol::MigrationHeader::MigrationType) + sizeof(Protocol::MigrationInfo));
	if (sent < 0) {
		click_chatter("Error sending information header\n");
		goto err_out;
	}

	/* extract mapping information  */
	memcpy(tcp_header.entries,
			unfoldMap(tcp_map, &no_flows),
			header.migrationInfo.no_mappings_tcp * no_flows * (sizeof(Protocol::FlowEntry) + 2 * sizeof(size_t)));

	memcpy(udp_header.entries,
			unfoldMap(udp_map, &no_flows),
			header.migrationInfo.no_mappings_udp * no_flows * (sizeof(Protocol::FlowEntry) + 2 * sizeof(size_t)));

err_out:
	socket->close();
}


CLICK_ENDDECLS
