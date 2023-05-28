// This code is not protected by any copyright
// Prepared by qs0va_ in 2023

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

#include <iostream>
#include <regex>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

#define PACKAGE_SIZE 32

std::string rcv();
void snd(int sock, struct sockaddr_in addr, const std::string& mes);
void snd(int sock, struct sockaddr_in addr, const char* mes);

void run();

void doDir();
void doGet(const std::string& filename);

int sock;

struct sockaddr_in service;

struct sockaddr_in client;

int main(int argc, char* argv[]) {
    // Process args
    if (argc < 2) {
        printf("Not enought args in argv.\n");
        return 1;
    }

    for (auto c : std::string(argv[1])) {
        if (!std::isdigit(c)) {
            printf("Port must be a number.\n");
            return 1;
        }
    }

    if (atoi(argv[1]) < 1024 or atoi(argv[1]) > 65535) {
        printf("Invalid port.\n");
        return 1;
    }


    // Create a socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == -1) {
        perror("Cant create socket");
        return 1;
    }

    // Bind the socket
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = htonl(INADDR_ANY);
    service.sin_port = htons(atoi(argv[1]));

    if (bind(sock, reinterpret_cast<sockaddr*>(&service),
            sizeof(service)) == -1) {
        perror("bind() failed.\n");
        close(sock);
        return 1;
    }

    run();
}

void run() {
    while (true) {
        std::cout << "Waiting for command...\n";
        std::string command = rcv();
        std::cout << "Command recieved: " << command << '\n';
        if (command == "dir") {
            doDir();
        } else if (std::regex_match(command,
                    std::regex("get+\\s+\\S+"))) {
                std::string filename = command.substr(
                command.find_first_of(' ') + 1,
                command.length());
            doGet(filename);
        } else if (command == "quit") {
            snd(sock, client, "0");
            exit(0);
        } else {
            snd(sock, client, "-1");
            std::cout << "Invalid command\n";
        }
    }
}

void doDir() {
    std::string out;
    // Get content of the directory
    DIR* dir = opendir(".");
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        out += entry->d_name;
        out += "\n";
    }

    // Send header
    snd(sock, client, "0 " + std::to_string(out.length()) + " ");

    // Send data
    snd(sock, client, out);
}

void doGet(const std::string& filename) {
    // Open file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        snd(sock, client, "1");
        return;
    }
    std::string header = "0 ";

    // Measure the file size
    header += std::to_string(static_cast<int>(file.tellg())) + " ";

    // Send header
    snd(sock, client, header);
    file.close();

    // Send file
    char buffer[PACKAGE_SIZE];
    file.open(filename);
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer) - 1);
        buffer[file.gcount()] = '\0';
        snd(sock, client, buffer);
    }
}

std::string rcv() {
    char out[PACKAGE_SIZE];
    unsigned int clientlen = sizeof(client);
    if (recvfrom(sock, out, PACKAGE_SIZE,
            0, reinterpret_cast<sockaddr*>(&client),
            &clientlen) == -1) {
        perror("Cant recieve");
    }
    return std::string(out);
}

void snd(int sock, struct sockaddr_in addr, const std::string& mes) {
    snd(sock, addr, mes.c_str());
}

void snd(int sock, struct sockaddr_in addr, const char* mes) {
    for (int i = 0; i <= strlen(mes); i += PACKAGE_SIZE) {
        sendto(sock, mes + i, PACKAGE_SIZE,
            0, reinterpret_cast<sockaddr*>(&addr),
            sizeof(client));
    }
}
