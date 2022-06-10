#include "client.hh"
#include <cppiper/receiver.hh>
#include <cppiper/sender.hh>

stashcache::Client::Client(const std::string name, const std::string clientpipe,
                           const std::string serverpipe)
    : name(name), sender(name, clientpipe), receiver(name, serverpipe) {
  spdlog::info("Constructed client instance '{0}' with client pipe '{1}' and "
               "server pipe '{2}'",
               name, clientpipe, serverpipe);
}

bool stashcache::Client::set(const std::string key, const std::string value){
    sender.send("SET");
    sender.send(key);
    sender.send(value);
    return true;
}

std::optional<const std::string> stashcache::Client::get(const std::string key){
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
