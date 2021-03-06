//
//  network_utils.h
//
//  Created by WAYNE SMALL on 2/19/17.
//  Copyright © 2017 Waynemystir. All rights reserved.
//

#ifndef network_utils_h
#define network_utils_h

#include <netdb.h>

#define IPv4 "IPv4"
#define IPv6 "IPv6"

#define SOCK_STREAM_STR "SOCK_STREAM"               /* stream socket */
#define SOCK_DGRAM_STR  "SOCK_DGRAM"               /* datagram socket */
#define SOCK_RAW_STR "SOCK_RAW"               /* raw-protocol interface */

typedef struct addrinfop {
	char ip_str[256];
	char ip_ver[256];
	char socktype[20];
	struct addrinfop *next;
} addrinfop;

typedef enum IF_ADDR_PREFFERED {
	IPV4_WIFI,
	IPV6_WIFI,
	IPV4_CELLULAR,
	IPV6_CELLULAR,
} IF_ADDR_PREFFERED;

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

int addr_to_str_short(struct sockaddr *addr,
		char *addrbuf,
		unsigned short *port,
		unsigned short *family);

const char *inet_ntop_v4(in_addr_t *src, char *dst, size_t size);

int str_addr_str(const char *addr_str,
			const char *service,
			int family,
			int socktype,
			int flags,
			char *ip_str,
			char *port_str,
			char *family_str);

int ipv6_available_ios_wifi();

int get_if_addr_iOS_OSX(IF_ADDR_PREFFERED iap,
	struct sockaddr **addr,
	size_t *size_addr,
	char ip_str[INET6_ADDRSTRLEN]);

int get_if_addr_Ubuntu(struct sockaddr **addr,
		size_t *size_addr,
		char ip_str[INET6_ADDRSTRLEN]);

int get_if_addr_old(struct sockaddr **addr, size_t *size_addr, char ip_str[INET6_ADDRSTRLEN]);

int addr_equals(const struct sockaddr *addr1, const struct sockaddr *addr2);

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

char *if_addr_pref_to_str(IF_ADDR_PREFFERED ifap);

#endif /* udp_client_h */
