#include "cache.hh"
#include "service.hh"
#include "stashcacheconfig.hh"
#include <cppiper/pipemanager.hh>
#include <cstdint>
#include <map>
#include <memory>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>
#include <vector>
#include <thread>
#define PORT 8080


static void cleaner(std::vector<stashcache::Service*> &services, cppiper::PipeManager &pm, std::mutex &lock) {
  while (1) {
    int i(0);
    lock.lock();
    while (i < services.size()){
      if (not services[i]->is_running()){
        services[i]->terminate();
        pm.remove_pipe(services[i]->get_client_pipe().filename());
        pm.remove_pipe(services[i]->get_server_pipe().filename());
        std::cout << "Client " << services[i]->get_client_name() << " deregistered" << std::endl;
        delete services[i];
        services.erase(services.begin()+i);
      }
      else {
        i += 1;
      }
    }
    lock.unlock();
    sleep(1);
  }
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "First argument must be size in GB." << std::endl;
    exit(1);
  }

  fLS::FLAGS_log_dir = "./";
  google::InitGoogleLogging(argv[0]);

  std::cout << "stash-cache server v" << STASHCACHE_VERSION_MAJOR << '.'
            << STASHCACHE_VERSION_MINOR << std::endl;

  std::shared_ptr cache =
      std::make_shared<stashcache::Cache>(atoi(argv[1]) * 1024 * 1024 * 1024);
  cppiper::PipeManager pm("pipe_manager");
  pm.clear();
  std::vector<stashcache::Service*> services;
  std::mutex lock;
  std::thread cleanup_thread(cleaner, std::ref(services), std::ref(pm), std::ref(lock));

/*
** Following code is a derivative of that found at https://www.geeksforgeeks.org/socket-programming-cc/
 */

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  while (1) {
    std::cout << "Listening for new client..." << std::endl;
    if (listen(server_fd, 3) < 0) {
      perror("listen");
      exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }
    char name_buffer[1024];
    valread = read(new_socket, name_buffer, 1024);
    std::string client_name = std::string(name_buffer, valread);
    std::string client_pipe = pm.make_pipe().string();
    std::string server_pipe = pm.make_pipe().string();
    std::string ret_msg = client_pipe + ' ' + server_pipe + '\0';
    send(new_socket, ret_msg.c_str(), ret_msg.size(), 0);
    lock.lock();
    services.emplace_back(new stashcache::Service(client_name,
                                           server_pipe, client_pipe, cache));
    lock.unlock();
    close(new_socket);
    std::cout << "Client " << client_name << " registered" << std::endl;
  }

  shutdown(server_fd, SHUT_RDWR);
}
