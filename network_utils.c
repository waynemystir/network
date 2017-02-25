#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

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
		default:
			sprintf( buf, "<invalid address>" );
			port = -1;
			sprintf( familybuf, "UNS" );
	}
	sprintf( addrbuf, "%s", buf );
	sprintf( portbuf, "%d", port );
	return 0;
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

int get_hostname(const char *ip_str,
		int port,
		char hostname[NI_MAXHOST],
		char service[NI_MAXSERV]) {

	if (!ip_str) {
		fprintf(stderr, "get_hostname: ip_str argument is null\n");
		return -1;
	}

	struct sockaddr *sa;
	size_t sa_size;

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
	addrinfop *aip;
	addrinfop *aip_prev = NULL;
	addrinfop *aip_first_node;

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