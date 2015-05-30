#include <click/config.h>
#include <click/migrationsender.hh>
#include <click/args.hh>
#include <click/straccum.hh>
#include <click/error.hh>
#include <click/timer.hh>
#include <click/router.hh>
#include <click/protocol.hh>

CLICK_DECLS

#define MAP_PACKET_SIZE(no_mappings, no_flows) \
	(no_mappings) * (no_flows) * (sizeof(Protocol::FlowID) + 2 * sizeof(size_t))

int MigrationSender :: connectToMachine()
{
	socket = new TCPSocket(TCPSocket :: ANY_PORT);

	/* connect to the remote machine */
	return socket->connect(port, hostname);
}

void MigrationSender :: copyFlow(IPFlowID src, Protocol::FlowID *dst)
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

		Protocol::FlowID *flows = new Protocol::FlowID[bucket_size];

		for (counter = 0; it != flowMap->end(); it++, counter++) {
			copyFlow(it.get()->flowid(), &flows[counter]);
		}
		memcpy(entries[no_entries].flows, flows, sizeof(Protocol::FlowID) * bucket_size);

		no_entries++;
	}

	return entries;
}

Protocol::HeapEntry* MigrationSender::unfoldHeap(IPRewriterHeap **heap, int heap_type)
{
	size_t i, j, heap_size;
	struct Protocol::HeapEntry *heap_pairs;
	IPFlowID flowID;
	IPRewriterEntry rwEntry;

	if (heap_type != IPRewriterHeap::h_best_effort &&
			heap_type != IPRewriterHeap:: h_guarantee) {
		click_chatter("Unknown heap type\n");
		return NULL;
	}

	/* extract flowIDs */
	heap_size = (*heap)->_heaps[heap_type].size();
	Vector<IPRewriterFlow *> &heap_data = (*heap)->_heaps[heap_type];
	heap_pairs = new Protocol::HeapEntry[heap_size];

	for (i = 0; i < heap_size; i++) {
		heap_pairs[i].expiry = heap_data[i]->expiry();
		for (j = 0; j <= 1; j++) {
			rwEntry = heap_data[i]->entry(j);
			flowID = rwEntry.flowid();
			heap_pairs[i].flows[j].output = rwEntry.output();
			heap_pairs[i].flows[j].direction = rwEntry.direction();
			copyFlow(flowID, &heap_pairs[i].flows[j].flowId);
		}
	}

	return heap_pairs;
}

int MigrationSender::sendMapping(Map *map, Protocol::MigrationHeader::MigrationType map_type)
{
	size_t no_flows, packet_size;
	int no_mappings;
	Protocol::MapEntry *map_data;
	Protocol::MigrationHeader map_header;

	/* extract mapping information  */
	no_mappings = map->size();
	map_data =  unfoldMap(map, &no_flows);
	packet_size = MAP_PACKET_SIZE(no_mappings, no_flows);

	memcpy(map_header.mapEntries, map_data, packet_size);
	map_header.type = map_type;
	packet_size += sizeof(Protocol::MigrationHeader::MigrationType);

	/* send mappings */
	if (socket->sendNarrowed(&map_header, packet_size) < 0) {
		click_chatter("Error sending %d mappings\n", map_type);
		return -1;
	}

	return 0;
}

int MigrationSender::sendHeap(IPRewriterHeap **heap, Protocol::MigrationHeader::MigrationType header_type)
{
	size_t no_flows, packet_size;
	Protocol::MigrationHeader heap_header;
	int heap_type;
	
	heap_type = (header_type == Protocol::MigrationHeader::T_HEAP_GUARANTEED)?
				IPRewriterHeap::h_guarantee : IPRewriterHeap::h_best_effort;

	no_flows = (*heap)->_heaps[heap_type].size();
	packet_size = no_flows * sizeof(Protocol::HeapEntry);
	memcpy(heap_header.heapEntries,
			unfoldHeap(heap, heap_type),
			packet_size);

	packet_size += sizeof(Protocol::MigrationHeader::MigrationType);
	heap_header.type = header_type;
	if (socket->sendNarrowed(&heap_header, packet_size) < 0) {
		click_chatter("Error sending %d heap\n", header_type);
		return -1;
	}

	return 0;
}
int MigrationSender::sendMigrationInfo(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	size_t packet_size;
	Protocol::MigrationHeader header;

	header.type = Protocol::MigrationHeader::T_INFO;
	header.migrationInfo.no_mappings_tcp = tcp_map->size();
	header.migrationInfo.no_mappings_udp = udp_map->size();
	header.migrationInfo.no_heap_best_effort = (*heap)->_heaps[IPRewriterHeap :: h_best_effort].size();
	header.migrationInfo.no_heap_guranteed = (*heap)->_heaps[IPRewriterHeap :: h_guarantee].size();

	print_info_header(header);

	packet_size = sizeof(Protocol::MigrationHeader::MigrationType) +
				sizeof(Protocol::MigrationInfo);
	if (socket->send(&header, packet_size) < 0) {
		click_chatter("Error sending information header\n");
		return -1;
	}

	return 0;
}

void MigrationSender :: run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap)
{
	if (sendMigrationInfo(tcp_map, udp_map, heap) < 0)
		goto out;

	/* send TCP and UDP mappings */
	if (sendMapping(tcp_map, Protocol::MigrationHeader::T_MAP_TCP) < 0)
		goto out;

	if (sendMapping(udp_map, Protocol::MigrationHeader::T_MAP_UDP) < 0)
		goto out;

	/* send BEST_EFFORT and GUARANTEED heaps */
	if (sendHeap(heap, Protocol::MigrationHeader::T_HEAP_GUARANTEED) < 0)
		goto out;

	if (sendHeap(heap, Protocol::MigrationHeader::T_HEAP_BEST_EFFORT) < 0)
		goto out;

out:
	socket->close();
}


CLICK_ENDDECLS
