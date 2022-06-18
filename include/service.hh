#ifndef SERVICE_HH_
#define SERVICE_HH_

#include "cache.hh"
#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <memory>
#include <string>
#include <thread>
namespace stashcache {

enum RequestType { SET, GET, END };

const std::map<std::string, RequestType> REQUEST_STR_MAP{
    {"SET", SET}, {"GET", GET}, {"END", END}};

class Service {

private:
  std::thread thread;
  bool running;
  std::shared_ptr<Cache> cache;
  cppiper::Receiver receiver;
  cppiper::Sender sender;
  const std::string name;
  void serve();

public:
  Service(void) = delete;
  Service(const std::string name, const std::filesystem::path serverpipe,
          const std::filesystem::path clientpipe, std::shared_ptr<Cache> cache);
  std::filesystem::path get_client_pipe(void) const;
  std::filesystem::path get_server_pipe(void) const;
  bool is_running(void) const;
  bool terminate(void);
};

} // namespace stashcache

#endif // SERVICE_HH_
