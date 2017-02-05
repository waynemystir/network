#include <netdb.h>

#define IPv4 "IPv4"
#define IPv6 "IPv6"

#define SOCK_STREAM_STR "SOCK_STREAM"               /* stream socket */
#define SOCK_DGRAM_STR  "SOCK_DGRAM"               /* datagram socket */
#define SOCK_RAW_STR "SOCK_RAW"               /* raw-protocol interface */

typedef struct addrinfop {
	const char *hostname;
	const char *ip_str;
	const char *ip_ver;
	const char *socktype;
	struct addrinfop *next;
} addrinfop;

int gethostname(const char *ip_str,
					char hostname[256]);

int get_addrinfos(const char *hostname,
					struct addrinfo **addrinfos);

int addrinfo_to_p(struct addrinfo *ai,
					struct addrinfop *aip);

int iterate_addrinfos(struct addrinfo *addrinfos,
						struct addrinfop **addrinfops,
						void (*iterate_callback)(struct addrinfop *aip),
						void (*iterate_complete)(void));

int get_iterate_addr_infos(const char *hostname,
							void (*iterate_callback)(struct addrinfop *aip),
							void (*iterate_complete)(void));

int get_and_iterate_addr_infos(const char *hostname,
								struct addrinfo **addrinfos,
								struct addrinfop **addrinfops,
								void (*iterate_callback)(struct addrinfop *aip),
								void (*iterate_complete)(void));

int get_addrinfops(const char*hostname,
						struct addrinfop **addrinfops);