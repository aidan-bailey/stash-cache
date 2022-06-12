#ifndef CLIENT_HH_
#define CLIENT_HH_

#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <optional>
namespace stashcache {
class Client {
private:
  cppiper::Sender sender;
  cppiper::Receiver receiver;
  std::string name;

public:
  Client(void) = delete;
  Client(const std::string name, const std::string clientpipe,
         const std::string serverpipe);
  bool set(const std::string &key, const std::string &value);
  std::optional<const std::string> get(const std::string &key);
  void terminate(void);
};
} // namespace stashcache

#endif // CLIENT_HH_
