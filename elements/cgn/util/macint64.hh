#ifndef CLICK_CGN_MACINT64_HH
#define CLICK_CGN_MACINT64_HH

#include <click/config.h>
#include <click/glue.hh>
#include "staticbyteswap.hh"

CLICK_DECLS

namespace CGN
{

class MacInt64
{
public:

	/* assumes mac[6:7] are accessible */
	static uint64_t getUnsafe(const uint8_t *mac)
	{
		static const uint64_t mask = StaticHTONLL<0xffffffffffff0000ULL>::value;
		
		return *(reinterpret_cast<const uint64_t *>(mac)) & mask;
	}
	
	/* this is slow */
	//TODO: optimize!
	static uint64_t getSafe(const uint8_t *mac)
	{
		uint64_t ret = 0;
		
		memcpy(&ret, mac, 6);
		return ret;
	}
	
	static void storeDestructive(uint64_t intMac, uint8_t *mac)
	{
		*(reinterpret_cast<uint64_t *>(mac)) = intMac;
	}
	
	static void storeUnsafe(uint64_t intMac, uint8_t *mac)
	{
		static const uint64_t invMask = StaticHTONLL<0xffff>::value;
		
		intMac |= *(reinterpret_cast<uint64_t *>(mac)) & invMask;
		storeDestructive(intMac, mac);
	}
	
	static void storeSafe(uint64_t intMac, uint8_t *mac)
	{
		memcpy(mac, &intMac, 6);
	}

};

}

CLICK_ENDDECLS

#endif /* CLICK_CGN_MACINT64_HH */
