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
	printf("^^^^^^^^^^^^^^^^^^^^ Google addrinfops ^^^^^^^^^^^^^^^^^^^^\n");
	while (aip) {
		printf("%s::%s\n", aip->ip_str, aip->ip_ver);
		aip = aip->next;
	}
	free(ai1);
	free(aip);

	printf("\n");

	strcpy(hostname, "github.com");
	ret = get_addrinfos(hostname, &ai1);
	printf("^^^^^^^^^^^^^^^^^^^^ %s ips ^^^^^^^^^^^^^^^^^^^^\n", hostname);
	ret = iterate_addrinfos(ai1, NULL, iterate_ips_callback, iterate_complete);
	free(ai1);

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
	printf("^^^^^^^^^^^^^^^^^^^^ %s ^^^^^^^^^^^^^^^^^^^^\n", hostname);
	while (aip) {
		printf("%s::%s::%s\n", aip->ip_str, aip->ip_ver, aip->socktype);
		aip = aip->next;
	}
	iterate_complete();
	free(aip);

	const char *hostnames2[256] = {"youtube.com", "stackoverflow.com", "gcc.gnu.org", "developer.apple.com"};
	const char **hns2 = hostnames2;
	while (*hns2) {
		printf("^^^^^^^^^^^^^^^^^^^^ %s ^^^^^^^^^^^^^^^^^^^^\n", *hns2);
		strcpy(hostname, *hns2);
		get_addrinfops(*hns2, &aip);

		while (aip) {
			printf("%s::%s::%s\n", aip->ip_str, aip->ip_ver, aip->socktype);
			aip = aip->next;
		}
		iterate_complete();
		hns2++;
	}
	free(aip);

	strcpy(hostname, "");
	puts("\nNow let's get hostnames from their IP address:\n");

	char service[NI_MAXSERV];
	const char *ips[256] = {"216.58.219.238", "172.217.4.78", "172.217.3.14", "2607:f8b0:4006:80f::200e",
								"52.33.196.199", "98.138.253.109", "2001:4998:c:a06::2:4008"};
	const char **ip = ips;
	while (*ip) {
		gethostname(*ip, 8080, hostname, service);
		printf("%s :: %s :: %s\n", *ip, hostname, service);
		ip++;
	}

	return ret;
}
