#include <netdb.h>

#define IPv4 "IPv4"
#define IPv6 "IPv6"

#define SOCK_STREAM_STR "SOCK_STREAM"               /* stream socket */
#define SOCK_DGRAM_STR  "SOCK_DGRAM"               /* datagram socket */
#define SOCK_RAW_STR "SOCK_RAW"               /* raw-protocol interface */

typedef struct addrinfop {
	char ip_str[256];
	char ip_ver[256];
	char socktype[11];
	struct addrinfop *next;
} addrinfop;

int str_to_addr(struct sockaddr **addr,
		const char *addr_str,
		const char *service,
		int family,
		int socktype,
		int flags);

int addr_to_str(struct sockaddr *addr,
		char *addrbuf,
		char *portbuf,
		char *familybuf);

int str_addr_str(const char *addr_str,
			const char *service,
			int family,
			int socktype,
			int flags,
			char *ip_str,
			char *port_str,
			char *family_str);

int get_hostname(const char *ip_str,
		int port,
		char hostname[NI_MAXHOST],
		char service[NI_MAXSERV]);

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

void freeaddrinfo_p(struct addrinfop *addrinfops);