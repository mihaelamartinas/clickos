#ifndef CLICK_MIGRATION_RECEIVER_HH
#define CLICK_MIGRATION_RECEIVER_HH

#include <click/migrationactions.hh>
#include <click/migrationreceiver.hh>

CLICK_DECLS

class MigrationReceiver : public MigrationActions {
	public:
		MigrationReceiver() : MigrationActions() {};
		MigrationReceiver(uint16_t port, char *hostname) :
			MigrationActions(port, hostname) {};
		void run(Map *tcp_map, Map *udp_map, IPRewriterHeap **heap);
		int connectToMachine();

	private:
		void initMigrationInfo(Protocol::MigrationInfo *migrationInfo);
		void storeMap(Protocol::MigrationHeader::MigrationType type, size_t no_maps);
		void storeHeap(Protocol::MigrationHeader::MigrationType type, size_t no_flows);

};

CLICK_ENDDECLS
#endif /* CLICK_MIGRATION_SENDER_HH */
