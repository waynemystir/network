#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "network_utils.h"

int str_to_addr(struct sockaddr **addr,
		const char *addr_str,
		const char *service,
		int family,
		int socktype,
		int flags ) {
	if (!addr) {
		printf("A NULL sockaddr** was given to str_to_addr\n");
		return -1;
	}
	struct addrinfo hints, *info = NULL;

	memset( &hints, '\0', sizeof(struct addrinfo) );
	hints.ai_family = family;
	hints.ai_socktype = socktype;
	hints.ai_flags = flags;

	int status = getaddrinfo( addr_str, service, &hints, &info );
	if ( status != 0 ) {
		fprintf(stderr, "getaddrinfo: %s addr_str: %s service: %s\n", gai_strerror(status), addr_str, service);
		return -1;
	}

	if ( info ) {
		*addr = malloc(info->ai_addrlen);
		memcpy( *addr, info->ai_addr, info->ai_addrlen );
		freeaddrinfo( info );
	}

	return 0;
}

int addr_to_str(struct sockaddr *addr,
		char *addrbuf,
		char *portbuf,
		char *familybuf) {

	if (!addr) return -1;

	char buf[INET6_ADDRSTRLEN+1];
	socklen_t sl = sizeof(buf);
	unsigned short port;

	switch(addr->sa_family) {
		case AF_INET6:
			inet_ntop( AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, buf, sl );
			port = ntohs( ((struct sockaddr_in6 *)addr)->sin6_port );
			sprintf( familybuf, "IP6" );
			break;
		case AF_INET:
			inet_ntop( AF_INET, &((struct sockaddr_in *)addr)->sin_addr, buf, sl );
			port = ntohs( ((struct sockaddr_in *)addr)->sin_port );
			sprintf( familybuf, "IP4" );
			break;
		case AF_UNSPEC:
			sprintf( buf, "<unspecified address> %d", addr->sa_family );
			port = -1;
			sprintf( familybuf, "UNS" );
			break;
		default:
			sprintf( buf, "<invalid address> %d", addr->sa_family );
			port = -1;
			sprintf( familybuf, "UKN" );
	}
	sprintf( addrbuf, "%s", buf );
	sprintf( portbuf, "%d", port );
	return 0;
}

int addr_to_str_short(struct sockaddr *addr,
		char *addrbuf,
		unsigned short *port,
		unsigned short *family) {

	if (!addr) return -1;

	if (family) *family = addr->sa_family;
	char buf[INET6_ADDRSTRLEN+1];
	socklen_t sl = sizeof(buf);

	switch(addr->sa_family) {
		case AF_INET6:
			inet_ntop( AF_INET6, &((struct sockaddr_in6 *)addr)->sin6_addr, buf, sl );
			if (port) *port = ntohs( ((struct sockaddr_in6 *)addr)->sin6_port );
			break;
		case AF_INET:
			inet_ntop( AF_INET, &((struct sockaddr_in *)addr)->sin_addr, buf, sl );
			if (port) *port = ntohs( ((struct sockaddr_in *)addr)->sin_port );
			break;
		case AF_UNSPEC:
			sprintf( buf, "<unspecified address> %d", addr->sa_family );
			if (port) *port = -1;
			break;
		default:
			sprintf( buf, "<invalid address> %d", addr->sa_family );
			if (port) *port = -1;
	}
	sprintf( addrbuf, "%s", buf );
	return 0;
}

const char *inet_ntop_v4(in_addr_t *src, char *dst, size_t size) {
	const char digits[] = "0123456789";
	int i;
	struct in_addr *addr = (struct in_addr *)src;
	u_long a = ntohl(addr->s_addr);
	const char *orig_dst = dst;

	if (size < INET_ADDRSTRLEN) {
		printf("inet_ntop_v4 ERROR: arg size < INET_ADDRSTRLEN\n");
		return NULL;
	}
	for (i = 0; i < 4; ++i) {
		int n = (a >> (24 - i * 8)) & 0xFF;
		int non_zerop = 0;

		if (non_zerop || n / 100 > 0) {
			*dst++ = digits[n / 100];
			n %= 100;
			non_zerop = 1;
		}
		if (non_zerop || n / 10 > 0) {
			*dst++ = digits[n / 10];
			n %= 10;
			non_zerop = 1;
		}
		*dst++ = digits[n];
		if (i != 3)
			*dst++ = '.';
	}
	*dst++ = '\0';
	return orig_dst;
}

int str_addr_str(const char *addr_str,
			const char *service,
			int family,
			int socktype,
			int flags,
			char *ip_str,
			char *port_str,
			char *family_str) {

	struct sockaddr *sa1;
	int ret = str_to_addr(&sa1, addr_str, service, family, socktype, flags);
	if (!ret) {
		addr_to_str(sa1, ip_str, port_str, family_str);
		free(sa1);
	}
	return ret;
}

int startswith(char *str, char *w) {
	size_t sl = strlen(str);
	size_t wl = strlen(w);
	if (wl > sl) return 0;
	for (int i = 0; i < wl; i++) {
		if (w[i] != str[i]) return 0;
	}
	return 1;
}

int ipv6_available_ios_wifi() {

	// retrieve the current interfaces - returns 0 on success
	struct ifaddrs *interfaces;
	if (!getifaddrs(&interfaces)) {
		// Loop through linked list of interfaces
		struct ifaddrs *interface;
		for (interface=interfaces; interface; interface=interface->ifa_next) {

			if (!(interface->ifa_flags & IFF_UP) || (interface->ifa_flags & IFF_LOOPBACK) ) {
				continue; // deeply nested code harder to read
			}
			const struct sockaddr *addr = (const struct sockaddr*)interface->ifa_addr;
			char addrBuf[ INET6_ADDRSTRLEN ];

			if (addr && (addr->sa_family==AF_INET || addr->sa_family==AF_INET6)) {
				char *name = interface->ifa_name;
				if (strcmp(name, "pdp_ip0") != 0) continue;

				unsigned short family = 0;
				if (addr->sa_family == AF_INET) {
					const struct sockaddr_in *addr4 = (const struct sockaddr_in*)interface->ifa_addr;
					if (inet_ntop(AF_INET, &addr4->sin_addr, addrBuf, INET_ADDRSTRLEN)) {
						family = AF_INET;
					}
				} else {
					const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6*)interface->ifa_addr;
					if (inet_ntop(AF_INET6, &addr6->sin6_addr, addrBuf, INET6_ADDRSTRLEN)) {
						family = AF_INET6;
					}
				}
				if (family != AF_INET6) continue;
				printf("ZZZZZZZZZZZZZZZZZZZ (%s)\n", addrBuf);

				char w[20] = {20};
				strcpy(w, "fe80");
				if (startswith(addrBuf, w)) continue;
				strcpy(w, "fc00");
				if (startswith(addrBuf, w)) continue;
				strcpy(w, "fd00");
				if (startswith(addrBuf, w)) continue;
				strcpy(w, "ff00");
				if (startswith(addrBuf, w)) continue;

				freeifaddrs(interfaces);
				return 1;
			}
		}
	}

	freeifaddrs(interfaces);
	return 0;
}

int get_if_addr_iOS_OSX(IF_ADDR_PREFFERED iap,
	struct sockaddr **ret_addr,
	size_t *size_addr,
	char ip_str[INET6_ADDRSTRLEN]) {

	if (!ret_addr || !size_addr) {
		printf("A NULL sockaddr** or size_addr was given to str_to_addr\n");
		return -1;
	}

	char pref_net[20];
	unsigned int pref_family = 0;
	switch (iap) {
		case IPV4_WIFI: {
			strcpy(pref_net, "en0");
			pref_family = AF_INET;
			break;
		}
		case IPV6_WIFI: {
			strcpy(pref_net, "en0");
			pref_family = AF_INET6;
			break;
		}
		case IPV4_CELLULAR: {
			strcpy(pref_net, "pdp_ip0");
			pref_family = AF_INET;
			break;
		}
		case IPV6_CELLULAR: {
			strcpy(pref_net, "pdp_ip0");
			pref_family = AF_INET6;
			break;
		}
	}

	// retrieve the current interfaces - returns 0 on success
	struct ifaddrs *interfaces;
	if (!getifaddrs(&interfaces)) {
		// Loop through linked list of interfaces
		struct ifaddrs *interface;
		for (interface=interfaces; interface; interface=interface->ifa_next) {

			if (!(interface->ifa_flags & IFF_UP) /* || (interface->ifa_flags & IFF_LOOPBACK) */ ) {
				continue; // deeply nested code harder to read
			}
			const struct sockaddr *addr = (const struct sockaddr*)interface->ifa_addr;
			char addrBuf[ INET6_ADDRSTRLEN ];

			if (addr && (addr->sa_family==AF_INET || addr->sa_family==AF_INET6)) {
				char *name = interface->ifa_name;
				if (strcmp(name, pref_net) != 0) continue;
				unsigned short family = 0;
				if (addr->sa_family == AF_INET) {
					const struct sockaddr_in *addr4 = (const struct sockaddr_in*)interface->ifa_addr;
					if (inet_ntop(AF_INET, &addr4->sin_addr, addrBuf, INET_ADDRSTRLEN)) {
						family = AF_INET;
						*size_addr = sizeof(struct sockaddr_in);
					}
				} else {
					const struct sockaddr_in6 *addr6 = (const struct sockaddr_in6*)interface->ifa_addr;
					if (inet_ntop(AF_INET6, &addr6->sin6_addr, addrBuf, INET6_ADDRSTRLEN)) {
						family = AF_INET6;
						*size_addr = sizeof(struct sockaddr_in6);
					}
				}

				if (family == pref_family) {

					char w[20] = {20};
					strcpy(w, "fe80");
					if (startswith(addrBuf, w)) continue;
					strcpy(w, "fc00");
					if (startswith(addrBuf, w)) continue;
					strcpy(w, "fd00");
					if (startswith(addrBuf, w)) continue;
					strcpy(w, "ff00");
					if (startswith(addrBuf, w)) continue;

					*ret_addr = malloc(*size_addr);
					memcpy(*ret_addr, addr, *size_addr);
					// char key[256];
					// sprintf(key, "name:(%s) family:(%d)", name, family);
					// printf("ZZZZZZZ %s(%s)\n", key, addrBuf);
					strcpy(ip_str, addrBuf);
					freeifaddrs(interfaces);
					return 1;
				} else {
					*size_addr = 0;
				}
			}
		}
	}

	freeifaddrs(interfaces);
	return -1;
}

int ends_with(const char *str, const char *suffix) {
	if (!str || !suffix)
		return 0;
	size_t lenstr = strlen(str);
	size_t lensuffix = strlen(suffix);
	if (lensuffix >  lenstr)
		return 0;
	return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

// TODO add tests for this function
int get_if_addr_Ubuntu(struct sockaddr **ret_addr, size_t *size_addr, char ip_str[INET6_ADDRSTRLEN]) {
	if (!ret_addr || !size_addr) {
		printf("A NULL sockaddr** or size_addr was given to str_to_addr\n");
		return -1;
	}

	struct ifaddrs *interfaces;
	if (!getifaddrs(&interfaces)) {
		// Loop through linked list of interfaces
		struct ifaddrs *interface;
		int j = 0;
		for (interface=interfaces; interface; interface=interface->ifa_next) {

			if (!(interface->ifa_flags & IFF_UP)
				|| !(interface->ifa_flags & IFF_MULTICAST)
				|| !(interface->ifa_flags & IFF_RUNNING)
				|| (interface->ifa_flags & IFF_LOOPBACK)
				) {
				continue; // deeply nested code harder to read
			}
			// TODO handle IPv6
			// it's strange that none of the IPv6 addresses are turning up here?
			*size_addr = sizeof(struct sockaddr_in);
			struct sockaddr_in *addr = (struct sockaddr_in*)interface->ifa_addr;
			char addrBuf[ INET6_ADDRSTRLEN ];
			inet_ntop(AF_INET, &addr->sin_addr, addrBuf, INET_ADDRSTRLEN);

			if (ends_with(addrBuf, "0.0.0")) continue;
			printf("list (%d)IFF_UP(%d)(%s)(%s)\n", ++j, (interface->ifa_flags & IFF_UP), interface->ifa_name, addrBuf);
			*ret_addr = malloc(*size_addr);
			memcpy(*ret_addr, addr, *size_addr);
			strcpy(ip_str, addrBuf);
			freeifaddrs(interfaces);
			return 0;
		}
	}

	freeifaddrs(interfaces);
	return -1;
}

int get_if_addr_old(struct sockaddr **addr, size_t *size_addr, char ip_str[INET6_ADDRSTRLEN]) {
	if (!addr || !size_addr) {
		printf("A NULL sockaddr** or size_addr was given to str_to_addr\n");
		return -1;
	}

	struct ifaddrs *iflist, *iface;

	if (getifaddrs(&iflist) < 0) {
		perror("getifaddrs");
	}

	char addrp[INET6_ADDRSTRLEN];

	for (iface = iflist; iface; iface = iface->ifa_next) {
		int af = iface->ifa_addr->sa_family;
		switch (af) {
			case AF_INET: {
				*size_addr = sizeof(struct sockaddr_in);
				break;
			}
			case AF_INET6: {
				*size_addr = sizeof(struct sockaddr_in6);
				break;
			}
			default:
				continue;
		}

		struct sockaddr *potential_addr = iface->ifa_addr;

		if (potential_addr) {
			unsigned short port;
			unsigned short family;
			addr_to_str_short(potential_addr, addrp, &port, &family);

			if (strcmp(addrp, "127.0.0.1") != 0) {
				*addr = malloc(*size_addr);
				memcpy(*addr, potential_addr, *size_addr);
				strcpy(ip_str, addrp);
				break;
			} else *size_addr = 0;
		} else *size_addr = 0;
	}

	freeifaddrs(iflist);
	return 0;
}

int addr_equals(const struct sockaddr *addr1, const struct sockaddr *addr2) {
	if( addr1->sa_family != addr2->sa_family ) {
		return 0;
	} else if( addr1->sa_family == AF_INET ) {
		const struct sockaddr_in *a1 = (struct sockaddr_in *)addr1;
		const struct sockaddr_in *a2 = (struct sockaddr_in *)addr2;
		return (memcmp( &a1->sin_addr, &a2->sin_addr, 4 ) == 0) && (a1->sin_port == a2->sin_port);
	} else if( addr1->sa_family == AF_INET6 ) {
		const struct sockaddr_in6 *a1 = (struct sockaddr_in6 *)addr1;
		const struct sockaddr_in6 *a2 = (struct sockaddr_in6 *)addr2;
		return (memcmp( &a1->sin6_addr, &a2->sin6_addr, sizeof(struct in6_addr)) == 0) && (a1->sin6_port == a2->sin6_port);
	} else {
		return 0;
	}
}

int get_hostname(const char *ip_str,
		int port,
		char hostname[NI_MAXHOST],
		char service[NI_MAXSERV]) {

	if (!ip_str) {
		fprintf(stderr, "get_hostname: ip_str argument is null\n");
		return -1;
	}

	struct sockaddr *sa;
	socklen_t sa_size;

	struct sockaddr_in sai;
	sai.sin_family = AF_INET;
	if (port > 0) sai.sin_port = htons(port);
	int ret = inet_pton(AF_INET, ip_str, &(sai.sin_addr));

	if (ret != 1) {
		struct sockaddr_in6 sai6;
		sai6.sin6_family = AF_INET6;
		if (port > 0) sai6.sin6_port = htons(port);
		ret = inet_pton(AF_INET6, ip_str, &(sai6.sin6_addr));

		if (ret != 1) {
			fprintf(stderr, "inet_pton not working for either IPv4 or IPv6\n");
			return -1;
		} else {
			sa_size = sizeof sai6;
			sa = (struct sockaddr *) &sai6;
		}
	} else {
		sa_size = sizeof sai;
		sa = (struct sockaddr *) &sai;
	}

	ret = getnameinfo(sa, sa_size, hostname, NI_MAXHOST, service, NI_MAXSERV, 0);

	if (ret != 0) {
		fprintf(stderr, "getnameinfo::%d::%s\n", ret, gai_strerror(ret));
		return -2;
	}

	return 0;
}

int get_addrinfos(const char *hostname,
			struct addrinfo **addrinfos) {

	struct addrinfo hints;

	if (!hostname) {
		fprintf(stderr, "get_addrinfos: hostname argument is null\n");
		return -1;
	}

	memset(&hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(hostname, NULL, &hints, addrinfos);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo %s\n", gai_strerror(status));
		return -2;
	}

	return 0;
}

int addrinfo_to_p(struct addrinfo *ai,
			struct addrinfop *aip) {
	if (!ai || !aip) {
		return 0;
	}

	char ip_str[INET6_ADDRSTRLEN];
	char *ip_ver;
	void *addr;

	if (ai->ai_family == AF_INET) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *) ai->ai_addr;
		addr = &(ipv4->sin_addr);
		ip_ver = IPv4;
	} else {
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) ai->ai_addr;
		addr = &(ipv6->sin6_addr);
		ip_ver = IPv6;
	}

	inet_ntop(ai->ai_family, addr, ip_str, sizeof(ip_str));
	strcpy(aip->ip_str, ip_str);
	strcpy(aip->ip_ver, ip_ver);
	strcpy(aip->socktype, (ai->ai_socktype = SOCK_STREAM) ? SOCK_STREAM_STR :
					(ai->ai_socktype = SOCK_DGRAM) ? SOCK_DGRAM_STR : "SOCK_UNKNOWN");

	return 0;
}

int iterate_addrinfos(struct addrinfo *addrinfos,
			struct addrinfop **addrinfops,
			void (*iterate_callback)(struct addrinfop *aip),
			void (*iterate_complete)(void)) {
	struct addrinfo *ai;
	ai = addrinfos;
	addrinfop *aip = NULL;
	addrinfop *aip_prev = NULL;
	addrinfop *aip_first_node = NULL;

	while (ai) {
		aip = malloc(sizeof(addrinfop));
		if (aip_prev) aip_prev->next = aip;
		else if (addrinfops) *addrinfops = aip;
		else aip_first_node = aip;

		addrinfo_to_p(ai, aip);
		if (iterate_callback) iterate_callback(aip);

		ai = ai->ai_next;
		aip_prev = aip;
	}

	if (iterate_complete) iterate_complete();
	aip->next = NULL;
	if (!addrinfops) freeaddrinfo_p(aip_first_node);

	return 0;
}

int get_iterate_addr_infos(const char *hostname,
				void (*iterate_callback)(struct addrinfop *aip),
				void (*iterate_complete)(void)) {

	struct addrinfo *ai;
	int ret = get_and_iterate_addr_infos(hostname, &ai, NULL, iterate_callback, iterate_complete);
	if (!ret) freeaddrinfo(ai);
	return ret;
}

int get_and_iterate_addr_infos(const char *hostname,
				struct addrinfo **addrinfos,
				struct addrinfop **addrinfops,
				void (*iterate_callback)(struct addrinfop *aip),
				void (*iterate_complete)(void)) {

	int ret = get_addrinfos(hostname, addrinfos);
	if (ret < 0) {
		if (iterate_complete) iterate_complete();
		return ret;
	}
	return iterate_addrinfos(*addrinfos, addrinfops, iterate_callback, iterate_complete);
}

int get_addrinfops(const char*hostname,
			struct addrinfop **addrinfops) {
	struct addrinfo *ai;
	int ret = get_and_iterate_addr_infos(hostname, &ai, addrinfops, NULL, NULL);
	if (!ret) freeaddrinfo(ai);
	return ret;
}

void freeaddrinfo_p(struct addrinfop *addrinfop) {
	if (!addrinfop) return;
	if (addrinfop->next) freeaddrinfo_p(addrinfop->next);
	free(addrinfop);
}



char *if_addr_pref_to_str(IF_ADDR_PREFFERED ifap) {
	switch (ifap) {
		case IPV4_WIFI: return "IPV4_WIFI";
		case IPV6_WIFI: return "IPV6_WIFI";
		case IPV4_CELLULAR: return "IPV4_CELLULAR";
		case IPV6_CELLULAR: return "IPV6_CELLULAR";
	}
	return "UNKNOWN_IP_FAM_AND_TYPE";
}
