#ifndef _DEFS_H_
#define _DEFS_H_

#define BYTE unsigned char

/* This is done to get the htonl, ntohl, htons and ntohs functions portably. */
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#endif /* _DEFS_H_ */
