#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    const char *ip_str = "127.0.0.1";
    unsigned char addr[4];
    struct in_addr ip_addr;

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, ip_str, &ip_addr) == 1) {
        printf("inet_pton succeeded\n");
        printf("Binary form: 0x%x\n", ip_addr.s_addr);
        printf("Sizeof: %d",sizeof(ip_addr.s_addr));
    } else {
        perror("inet_pton failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
