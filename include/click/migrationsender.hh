#ifndef CLICK_MIGRATION_SENDER_HH
#define CLICK_MIGRATION_SENDER_HH

#include <click/migrationactions.hh>
CLICK_DECLS

class MigrationSender : public MigrationActions {
	public:
		MigrationSender() : MigrationActions() {};
		MigrationSender(uint16_t port, char *hostname) :
			MigrationActions(port, hostname) {};
		void run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap);
		int connectToMachine();
};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
