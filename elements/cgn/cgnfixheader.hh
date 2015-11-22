#ifndef CLICK_CGNFIXHEADER_HH
#define CLICK_CGNFIXHEADER_HH

#include <click/config.h>
#include <click/element.hh>
#include <clicknet/ether.h>
#include "migration/cgnanno.hh"
#include "util/macint64.hh"

class CGNFixHeader: public Element
{
	/* the fast (but ugly) way to create a MacInt64 */
	inline uint64_t makeMAC(CGN::Annotation anno)
	{
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
		return ((uint64_t)anno) << (5 * 8);
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
		return ((uint64_t)anno) << (2 * 8);
#else
#error "CLICK_BYTE_ORDER not set"
#endif
	}
	
	inline Packet *process(Packet *p)
	{
		CGN::Annotation anno = CGN::getAnnotation(p);
		WritablePacket *wp;
		
		if (anno == CGN::ANNOTATION_NONE)
			return p;
		
		wp = p->uniqueify(); assert(wp);
		
		CGN::MacInt64::storeDestructive(makeMAC(anno), wp->ether_header()->ether_dhost);
		
		return wp;
	}

public:
	const char *class_name() const { return "CGNFixHeader"; }
	
	const char *port_count() const { return PORTS_1_1; }
	
	const char *processing() const { return AGNOSTIC; }
	
	virtual void push(int, Packet *p);
	
	virtual Packet *pull(int);
};

#endif /* CGNFIXHEADER_HH */
