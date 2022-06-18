#ifndef CLIENT_HH_
#define CLIENT_HH_

#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>
#include <optional>
#include <string>
#include <utility>
namespace stashcache {
class Client {
private:
  cppiper::Sender sender;
  cppiper::Receiver receiver;
  std::string name;
  static std::pair<std::filesystem::path, std::filesystem::path>get_pipes(const std::string name);

public:
  Client(void) = delete;
  Client(const std::string name, const std::filesystem::path clientpipe,
         const std::filesystem::path serverpipe);
  Client(const std::string name, const std::pair<std::filesystem::path, std::filesystem::path> client_server_pipes);
  Client(const std::string name);
  bool set(const char * key, const size_t key_size, const char * value, const size_t value_size);
  bool set(const std::string &key, const std::string &value);
  std::optional<const std::string> get(const char * key, const size_t key_size);
  std::optional<const std::string> get(const std::string &key);
  void terminate(void);
};
} // namespace stashcache

#endif // CLIENT_HH_
