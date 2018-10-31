#include <machine/endian.h>
#include <stdio.h>

#define htonl(_x) __htonl(_x)
#define htons(_x) __htons(_x)
#define ntohl(_x) __ntohl(_x)
#define ntohs(_x) __ntohs(_x)
