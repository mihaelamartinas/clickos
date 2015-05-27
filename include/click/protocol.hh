#ifndef CLICKOS_MIGRATION_PROTOCOL
#define CLICKOS_MIGRATION_PROTOCOL

#include <click/config.h>

CLICK_DECLS
namespace Protocol {
	struct Instantiate
	{
		uint32_t ip;
		uint32_t subnetCount;
	};

	struct Destroy
	{
		uint32_t ip;
	};

	struct SetPrefixLength
	{
		uint8_t length;
	};

	struct Migrate
	{
		uint32_t ip;
		uint8_t index;
		uint8_t destinationLength;
		uint16_t destinationPort; /* in host byte order */
	};

	struct AcceptMigration
	{
		uint32_t ip;
		uint32_t subnetCount;
	};

	struct ControllerHeader
	{
		enum Type
		{
		T_INSTANTIATE,
		T_DESTROY,

		T_SET_PREFIX_LENGTH,

		T_MIGRATE,
		T_ACCEPT_MIGRATION,

		T_PRECOPY,
		T_ACCEPT_PRECOPY,

		T_ACK,
		T_NACK,
		};

		Type type;

		union
		{
			Instantiate instantiate;
			Destroy destroy;
			SetPrefixLength setPrefixLength;
			Migrate migrate;
			AcceptMigration acceptMigration;
		};

		uint32_t subnets[0];
		char destination[0];
	};

	struct MigrationInfo {
		int no_mappings_tcp;
		int no_mappings_udp;
		int no_heap_guranteed;
		int no_heap_best_effort;
	};

	struct FlowEntry {
		uint32_t s_addr;
		uint16_t s_port;
		uint32_t d_addr;
		uint16_t d_port;
	};

	struct FlowPair {
		FlowEntry originalFlow;
		FlowEntry rewrittenFlow;
	};

	struct MigrationHeader {
		enum MigrationType {
			T_INFO,
			T_MAP_TCP,
			T_MAP_UDP,
			T_HEAP_GUARANTEED,
			T_HEAP_BEST_EFFORT
		};

		MigrationType type;
		union {
			FlowPair flows[0];
			MigrationInfo migrationInfo;
		};
	};
}
CLICK_ENDDECLS
#endif /* CGN_CONTROLLER_PROTOCOL */
