#ifndef CLICK_CGNSPLITTER_HH
#define CLICK_CGNSPLITTER_HH

#include <click/element.hh>
#include "migration/cgnanno.hh"

CLICK_DECLS

class CGNSplitter: public Element
{
	enum PacketDirection
	{
		PD_OUTBOUND = 0,
		PD_INBOUND  = 1,
		PD_HOST     = 2,
	};
	
	uint64_t insideMAC;
	uint64_t outsideMAC;
	
public:
	CGNSplitter() {}
	
	~CGNSplitter() {}
	
	const char *class_name() const { return "CGNSplitter"; }
	
	const char *port_count() const { return "1/3"; }
	
	const char *processing() const { return PUSH; }
	
	int configure(Vector<String> &conf, ErrorHandler *errh);
	
	void push(int, Packet *);
};

CLICK_ENDDECLS

#endif /* CLICK_CGNSPLITTER_HH */
