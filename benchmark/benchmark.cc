#include "cache.hh"
#include "client.hh"
#include "service.hh"
#include "stashcacheconfig.hh"
#include <cppiper/pipemanager.hh>
#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <iostream>
#include <memory>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cout << "usage: benchmark <value_count> <value_size> <key_size>" << std::endl;
    exit(1);
  }
  const int value_count(atoi(argv[1]));
  const int value_size(atoi(argv[2]));
  const int key_size(atoi(argv[3]));
  std::cout << "stash-cache v" << STASHCACHE_VERSION_MAJOR << '.'
            << STASHCACHE_VERSION_MINOR << " benchmark" << std::endl
            << "Value Count: " << value_count << std::endl
            << "Value Size: " << value_size << "B" << std::endl
            << "Key Size:  " << key_size << "B" << std::endl;
  fLS::FLAGS_log_dir = "./";
  google::InitGoogleLogging(argv[0]);
  std::shared_ptr cache =
      std::make_shared<stashcache::Cache>(1024 * 1024 * 1024);
  cppiper::PipeManager pm("tmp/pipemanager");
  std::string cpipe = pm.make_pipe();
  std::string spipe = pm.make_pipe();
  stashcache::Client client("Client", cpipe, spipe);
  stashcache::Service service("Server", spipe, cpipe, cache);
  const std::string key(cppiper::random_hex(key_size));
  const std::string value(cppiper::random_hex(value_size));
  const int testset_size(value_count);
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
