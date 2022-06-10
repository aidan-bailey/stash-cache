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
  cppiper::Sender sender;
  cppiper::Receiver receiver;
  const std::string name;
  static void serve(const std::string name, cppiper::Receiver &receiver,
                    cppiper::Sender &sender, std::shared_ptr<Cache> cache,
                    bool &running);

public:
  Service(void) = delete;
  Service(const std::string name, const std::string serverpipe,
          const std::string clientpipe, std::shared_ptr<Cache> cache);
  bool terminate(void);
};

} // namespace stashcache

#endif // SERVICE_HH_
