#ifndef CLICK_LOCKEDMAP_HH
#define CLICK_LOCKEDMAP_HH

#include <click/sync.hh>

CLICK_DECLS

class LockedMap : public SimpleSpinlock {
	HashContainer<IPRewriterEntry> *_map;

public:
	LockedMap() {};
	~LockedMap() {};
};

CLICK_ENDDECLS
#endif
