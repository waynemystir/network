#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "network_utils.h"

char hostname[NI_MAXHOST];

void iterate_ips_callback(struct addrinfop *aip) {
	printf("%s::%s::%s\n", aip->ip_str, aip->ip_ver, aip->socktype);
}

void iterate_complete(void) {
	printf("~~~~~~~~~~~~~~~~~~~~ %s ~~~~~~~~~~~~~~~~~~~~ \n\n", hostname);
}

int main() {
	printf("network_utils_test::main::0\n\n");

	int ret;
	struct addrinfo *ai1;
	struct addrinfop *aip;

	ret = get_addrinfos("google.com", &ai1);
	ret = iterate_addrinfos(ai1, &aip, NULL, NULL);
	struct addrinfop *aip_iterate = aip;
	printf("^^^^^^^^^^^^^^^^^^^^ Google addrinfops ^^^^^^^^^^^^^^^^^^^^\n");
	while (aip_iterate) {
		printf("%s::%s\n", aip_iterate->ip_str, aip_iterate->ip_ver);
		aip_iterate = aip_iterate->next;
	}
	freeaddrinfo(ai1);
	freeaddrinfo_p(aip);

	printf("\n");

	strcpy(hostname, "github.com");
	ret = get_addrinfos(hostname, &ai1);
	printf("^^^^^^^^^^^^^^^^^^^^ %s ips ^^^^^^^^^^^^^^^^^^^^\n", hostname);
	ret = iterate_addrinfos(ai1, NULL, iterate_ips_callback, iterate_complete);
	freeaddrinfo(ai1);

	strcpy(hostname, "reddit.com");
	printf("^^^^^^^^^^^^^^^^^^^^ %s ips ^^^^^^^^^^^^^^^^^^^^\n", hostname);
	ret = get_iterate_addr_infos(hostname, iterate_ips_callback, iterate_complete);

	const char *hostnames[256] = { "ycombinator.com.com",
					"yahoo.com",
					"thenextweb.com/",
									NULL };
	const char **hns = hostnames;
	while (*hns) {
		printf("^^^^^^^^^^^^^^^^^^^^ %s ^^^^^^^^^^^^^^^^^^^^\n", *hns);
		strcpy(hostname, *hns);
		get_iterate_addr_infos(*hns, iterate_ips_callback, iterate_complete);
		hns++;
	}

	strcpy(hostname, "facebook.com");
	get_addrinfops(hostname, &aip);
	aip_iterate = aip;
	printf("^^^^^^^^^^^^^^^^^^^^ %s ^^^^^^^^^^^^^^^^^^^^\n", hostname);
	while (aip_iterate) {
		printf("%s::%s::%s\n", aip_iterate->ip_str, aip_iterate->ip_ver, aip_iterate->socktype);
		aip_iterate = aip_iterate->next;
	}
	iterate_complete();
	freeaddrinfo_p(aip);

	const char *hostnames2[256] = {"youtube.com", "stackoverflow.com",
					"gcc.gnu.org", "developer.apple.com"};
	const char **hns2 = hostnames2;
	while (*hns2) {
		printf("^^^^^^^^^^^^^^^^^^^^ %s ^^^^^^^^^^^^^^^^^^^^\n", *hns2);
		strcpy(hostname, *hns2);
		get_addrinfops(*hns2, &aip);
		aip_iterate = aip;

		while (aip_iterate) {
			printf("%s::%s::%s\n", aip_iterate->ip_str, aip_iterate->ip_ver, aip_iterate->socktype);
			aip_iterate = aip_iterate->next;
		}
		freeaddrinfo_p(aip);
		iterate_complete();
		hns2++;
	}

	strcpy(hostname, "");
	puts("\nNow let's get hostnames from their IP address:\n");

	char service[NI_MAXSERV];
	const char *ips[256] = {"216.58.219.238", "172.217.4.78", "172.217.3.14",
				"2607:f8b0:4006:80f::200e", "52.33.196.199",
				"98.138.253.109", "2001:4998:c:a06::2:4008",
				"31.13.71.36"};
	const char **ip = ips;
	while (*ip) {
		get_hostname(*ip, 8080, hostname, service);
		printf("%s :: %s :: %s\n", *ip, hostname, service);
		ip++;
	}

	printf("\n\n^^^^^^^^^^^ Check str_to_addr and addr_to_str ^^^^^^^^^^^\n");

	struct sockaddr *sa1;
	char w0[] = "localhost";
	char w1[256];
	char w2[256];
	char w3[256];

	ret = str_to_addr(&sa1, w0, "http", AF_INET, SOCK_DGRAM, 0);
	if (!ret) {
		addr_to_str(sa1, w1, w2, w3);
		printf("main 1 %s %s %s %s\n", w0, w1, w2, w3);
		free(sa1);
	}
	ret = str_to_addr(&sa1, w0, "http", AF_INET6, SOCK_STREAM, 0);
	if (!ret) {
		addr_to_str(sa1, w1, w2, w3);
		printf("main 2 %s %s %s %s\n", w0, w1, w2, w3);
		free(sa1);
	}

	str_addr_str("google.com", "http", AF_INET, SOCK_DGRAM, 0, w1, w2, w3);
	printf("main 3 google.com %s %s %s\n", w1, w2, w3);
	str_addr_str("google.com", "http", AF_INET6, SOCK_DGRAM, 0, w1, w2, w3);
	printf("main 4 google.com %s %s %s\n", w1, w2, w3);
	str_addr_str("reddit.com", "http", AF_INET, SOCK_DGRAM, 0, w1, w2, w3);
	printf("main 5 reddit.com %s %s %s\n", w1, w2, w3);
	str_addr_str("reddit.com", "http", AF_INET6, SOCK_STREAM, 0, w1, w2, w3);
	printf("main 6 reddit.com %s %s %s\n", w1, w2, w3);
	str_addr_str("facebook.com", "http", AF_INET, SOCK_DGRAM, 0, w1, w2, w3);
	printf("main 7 facebook.com %s %s %s\n", w1, w2, w3);
	str_addr_str("facebook.com", "http", AF_INET6, SOCK_DGRAM, 0, w1, w2, w3);
	printf("main 8 facebook.com %s %s %s\n", w1, w2, w3);

	// strcpy(w0, "google.com");
	// ret = str_to_addr(&sa1, w0, "http", AF_INET6, SOCK_STREAM, 0);
	// if (!ret) {
	// 	unsigned short port;
	// 	unsigned short family;
	// 	addr_to_str_short(sa1, w1, &port, &family);
	// 	printf("addr_to_str_short %s %u %u\n", w1, port, family);
	// 	free(sa1);
	// }

	printf("\n\n^^^^^^^^^^^ Check get_if_addr ^^^^^^^^^^^\n");

	size_t size_sa1 = 0;
	char ip_str[INET6_ADDRSTRLEN];
	get_if_addr(IPV4_WIFI, &sa1, &size_sa1, ip_str);
	printf("get_if_addr 1 %s %zu\n", ip_str, size_sa1);

	int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd == -1) perror("socket");

	int br = bind(sock_fd, sa1, size_sa1);
	if (br == -1) perror("bind");

	socklen_t len = sizeof(sa1);
	int gsn = getsockname(sock_fd, sa1, &len);
	if (gsn == -1) perror("getsockname");

	char sa_str[INET6_ADDRSTRLEN];
	unsigned short port;
	unsigned short family;
	addr_to_str_short(sa1, sa_str, &port, &family);
	printf("get_if_addr 2 %s p1:%d p2:%d f:%d\n",
		sa_str,
		port,
		ntohs(((struct sockaddr_in*)sa1)->sin_port),
		family);

	free(sa1);

	return ret;
}
