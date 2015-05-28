#ifndef CLICK_MIGRATION_ACTIONS_HH
#define CLICK_MIGRATION_ACTIONS_HH

#include <click/tcpsocket.hh>
#include <click/protocol.hh>
#include "elements/ip/iprewriterbase.hh"

#include <stdlib.h>

CLICK_DECLS

class MigrationActions {
	public:
		typedef HashContainer<IPRewriterEntry> Map;
		MigrationActions() : port(0), hostname(NULL) {};
		MigrationActions(uint16_t port, char *hostname) :
			port(port), hostname(hostname) {};
		virtual void run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap) {};
		virtual int connectToMachine() { return 0; };

		void print_info_header(Protocol::MigrationHeader header) {
			click_chatter("===== T_INFO =====\n");
			click_chatter("type = %d, tcp_size = %d, udp_size = %d, best_effort = %d, guaranteed_size = %d\n",
					header.type,
					header.migrationInfo.no_mappings_tcp,
					header.migrationInfo.no_mappings_udp,
					header.migrationInfo.no_heap_best_effort,
					header.migrationInfo.no_heap_guranteed
					);
		}
	protected:
		uint16_t port;
		char *hostname;
		TCPSocket *socket;
};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_ACTIONS_HH */
