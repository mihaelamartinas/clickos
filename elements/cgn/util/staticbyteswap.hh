#ifndef CLICK_CGN_STATICBYTESWAP_HH
#define CLICK_CGN_STATICBYTESWAP_HH

#include <click/config.h>
#include <click/glue.hh>

CLICK_DECLS

namespace CGN
{

template <uint16_t X> struct StaticHTONS
{
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
	static const uint16_t value = (X >> 8) | (uint16_t)(X << 8);
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
	static const uint16_t value = X;
#endif
};

template <uint32_t X> struct StaticHTONL
{
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
	static const uint32_t value = (uint32_t)StaticHTONS<(uint16_t)(X >> 16)>::value |
		((uint32_t)StaticHTONS<(uint16_t)X>::value << 16);
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
	static const uint32_t value = X;
#endif
};

template <uint64_t X> struct StaticHTONLL
{
#if CLICK_BYTE_ORDER == CLICK_LITTLE_ENDIAN
	static const uint64_t value = (uint64_t)StaticHTONL<(uint32_t)(X >> 32)>::value |
		((uint64_t)StaticHTONL<(uint32_t)X>::value << 32);
#elif CLICK_BYTE_ORDER == CLICK_BIG_ENDIAN
	static const uint64_t value = X;
#endif
};

template <uint16_t X> struct StaticNTOHS: public StaticHTONS<X> {};

template <uint32_t X> struct StaticNTOHL: public StaticHTONL<X> {};

template <uint64_t X> struct StaticNTOHLL: public StaticHTONLL<X> {};

}

CLICK_ENDDECLS

#endif /* CLICK_CGN_STATICBYTESWAP_HH */
