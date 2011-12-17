#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdint.h>

#define BYTE uint8_t
#define BYTE_MAX UINT8_MAX

/* This is done to get the htonl, ntohl, htons and ntohs functions portably. */
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#endif /* _DEFS_H_ */
