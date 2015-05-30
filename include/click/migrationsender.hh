#ifndef CLICK_MIGRATION_SENDER_HH
#define CLICK_MIGRATION_SENDER_HH

#include <click/migrationactions.hh>
#include <click/protocol.hh>

CLICK_DECLS

class MigrationSender : public MigrationActions {
	public:
		MigrationSender() : MigrationActions() {};
		MigrationSender(uint16_t port, char *hostname) :
			MigrationActions(port, hostname) {};
		void run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap);
		int connectToMachine();

	private:
		void copyFlow(IPFlowID src, Protocol::FlowID *dst);

		int sendMigrationInfo(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap);

		Protocol::MapEntry* unfoldMap(Map *flowMap, size_t *no_flows);
		int sendMapping(Map *map, Protocol::MigrationHeader::MigrationType map_type);

		Protocol::HeapEntry* unfoldHeap(IPRewriterHeap **heap, int heap_type);
		int sendHeap(IPRewriterHeap **heap, Protocol::MigrationHeader::MigrationType header_type);
};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
