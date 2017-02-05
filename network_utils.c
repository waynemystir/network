#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "network_utils.h"

int gethostname(const char *ip_str,
					char hostname[256]) {

	if (!ip_str) {
		fprintf(stderr, "gethostname: ip_str argument is null\n");
		return -1;
	}

	struct sockaddr *sa;

	struct sockaddr_in sai;
	sai.sin_family = AF_INET;
	int ret = inet_pton(AF_INET, ip_str, &(sai.sin_addr));

	if (ret != 1) {
		struct sockaddr_in6 sai6;
		sai6.sin6_family = AF_INET6;
		ret = inet_pton(AF_INET6, ip_str, &(sai6.sin6_addr));

		if (ret != 1) {
			fprintf(stderr, "inet_pton not working for either IPv4 or IPv6\n");
			return -1;
		} else {
			sa = (struct sockaddr *) &sai6;
		}
	} else {
		sa = (struct sockaddr *) &sai;
	}

	char service[20];
	ret = getnameinfo(sa, sizeof sa, hostname, 256, service, sizeof service, 0);

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
	aip->ip_str = strdup(ip_str);
	aip->ip_ver = ip_ver;
	aip->socktype = (ai->ai_socktype = SOCK_STREAM) ? SOCK_STREAM_STR :
					(ai->ai_socktype = SOCK_DGRAM) ? SOCK_DGRAM_STR : "SOCK_UNKNOWN";

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

	while (ai) {
		aip = malloc(sizeof(addrinfop));
		if (aip_prev) aip_prev->next = aip;
		else if (addrinfops) *addrinfops = aip;

		addrinfo_to_p(ai, aip);
		if (iterate_callback) iterate_callback(aip);

		ai = ai->ai_next;
		aip_prev = aip;
	}

	if (iterate_complete) iterate_complete();
	aip->next = NULL;

	return 0;
}

int get_iterate_addr_infos(const char *hostname,
							void (*iterate_callback)(struct addrinfop *aip),
							void (*iterate_complete)(void)) {

	struct addrinfo *ai;
	return get_and_iterate_addr_infos(hostname, &ai, NULL, iterate_callback, iterate_complete);
}

int get_and_iterate_addr_infos(const char *hostname,
								struct addrinfo **addrinfos,
								struct addrinfop **addrinfops,
								void (*iterate_callback)(struct addrinfop *aip),
								void (*iterate_complete)(void)) {

	int ret;
	if ((ret = get_addrinfos(hostname, addrinfos)) < 0) { return ret; }

	return iterate_addrinfos(*addrinfos, addrinfops, iterate_callback, iterate_complete);
}

int get_addrinfops(const char*hostname,
						struct addrinfop **addrinfops) {
	struct addrinfo *ai;
	return get_and_iterate_addr_infos(hostname, &ai, addrinfops, NULL, NULL);
}