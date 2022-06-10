#include "cache.hh"
#include "client.hh"
#include "service.hh"
#include "stashcacheconfig.hh"
#include <cppiper/pipemanager.hh>
#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  std::cout << "stashcache v" << STASHCACHE_VERSION_MAJOR << '.'
            << STASHCACHE_VERSION_MINOR << std::endl;
  if (DEV) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::debug("Dev mode enabled");
  } else
    spdlog::set_level(spdlog::level::err);

  std::shared_ptr cache =
      std::make_shared<stashcache::Cache>(1024 * 1024 * 1024);
  cppiper::PipeManager pm("tmp/pipemanager");
  std::string cpipe = pm.make_pipe();
  std::string spipe = pm.make_pipe();
  stashcache::Client client("Client", cpipe, spipe);
  stashcache::Service service("Server", spipe, cpipe, cache);
  std::string key = cppiper::random_hex(64);
  std::string value = cppiper::random_hex(10000);
  int testset_size = 1000000;
  std::chrono::high_resolution_clock::time_point start =
      std::chrono::high_resolution_clock::now();
  for (int i = 0; i < testset_size; i++) {
    client.set(key, value);
    auto res = client.get(key);
  }
  std::chrono::high_resolution_clock::time_point end =
      std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::micro> delta = (end - start);
  std::cout << "Avg of " << delta.count() / testset_size << "us/msg"
            << std::endl;
  client.terminate();
  service.terminate();
  pm.clear();
  return 0;
}
