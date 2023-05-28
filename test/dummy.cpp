// This code is not protected by any copyright
// Prepared by qs0va_ in 2023

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

struct sockaddr_in serv;

int main(int argc, char* argv[]) {

    // Create a socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1) {
        perror("Cant create socket");
        return 1;
    }

    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serv.sin_port = htons(7777);

    std::string mes("quit");
    sendto(sock, mes.c_str(), mes.length() + 1, 0, reinterpret_cast<sockaddr*>(&serv), sizeof(serv));
}

