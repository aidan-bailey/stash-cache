#include "client.hh"
#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <glog/logging.h>
#include <unistd.h>
#include <utility>
#define PORT 8080

stashcache::Client::Client(const std::string name, const std::string clientpipe,
                           const std::string serverpipe)
    : name(name), sender(name, clientpipe), receiver(name, serverpipe) {

  DLOG(INFO) << "Constructed client instance " << name
      << " with client pipe " << clientpipe
      << " and server pipe " << serverpipe;
}

std::pair<std::string, std::string> stashcache::Client::get_pipes(const std::string name){

/*
** Following code is a derivative of that found at https://www.geeksforgeeks.org/socket-programming-cc/
 */

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        exit(1);
    }

    if ((client_fd
         = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }
    send(sock, name.c_str(), name.size(), 0);
    char buffer[1024];
    valread = read(sock, buffer, 1024);
    std::string pipes = std::string(buffer);
    const int pos = pipes.find(" ");
    std::string client_pipe = pipes.substr(0, pos);
    std::string server_pipe = pipes.c_str() + pos + 1;
    std::cout << client_pipe << std::endl;
    std::cout << server_pipe << std::endl;
    close(client_fd);
    return std::pair<std::string, std::string>(client_pipe, server_pipe);
}

stashcache::Client::Client(const std::string name, const std::pair<std::string, std::string> client_server_pipes): Client(name, client_server_pipes.first, client_server_pipes.second){};

stashcache::Client::Client(const std::string name): Client(name, get_pipes(name)){};

bool stashcache::Client::set(const std::string &key, const std::string &value){
    sender.send("SET");
    sender.send(key);
    sender.send(value);
    return true;
}

std::optional<const std::string> stashcache::Client::get(const std::string &key){
    sender.send("GET");
    sender.send(key);
    std::optional<const std::string> response = receiver.receive(true);
    return response;
}

void stashcache::Client::terminate(void){
    sender.send("END");
    receiver.wait();
    sender.terminate();
}
