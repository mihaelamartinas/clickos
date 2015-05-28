#ifndef CLICK_MIGRATION_RECEIVER_HH
#define CLICK_MIGRATION_RECEIVER_HH

#include <click/migrationactions.hh>
CLICK_DECLS

class MigrationReceiver : public MigrationActions {
	public:
		MigrationReceiver() : MigrationActions() {};
		MigrationReceiver(uint16_t port, char *hostname) :
			MigrationActions(port, hostname) {};
		void run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap);
		int connectToMachine();

};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
