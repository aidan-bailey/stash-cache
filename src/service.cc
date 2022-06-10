#include "service.hh"
#include <cppiper/receiver.hh>
#include <functional>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <thread>

stashcache::Service::Service(const std::string name,
                             const std::string serverpipe,
                             const std::string clientpipe,
                             std::shared_ptr<stashcache::Cache> cache)
    : name(name), sender(name, serverpipe), receiver(name, clientpipe),
      cache(cache), running(true),
      thread(serve, name, std::ref(receiver), std::ref(sender), cache,
             std::ref(running)) {
  spdlog::info("Constructed service instance '{0}' with server pipe '{1}' and "
               "client pipe '{2}'",
               name, serverpipe, clientpipe);
}

void stashcache::Service::serve(const std::string name,
                                cppiper::Receiver &receiver,
                                cppiper::Sender &sender,
                                std::shared_ptr<stashcache::Cache> cache,
                                bool &running) {
  sleep(1);
  while (running) {
    const std::string cmd = receiver.receive(true).value();
    switch (REQUEST_STR_MAP.at(cmd)) {
    case SET: {
      spdlog::info("Received SET command for '{}'", name);
      const std::string key = receiver.receive(true).value();
      const std::string value = receiver.receive(true).value();
      cache->set(key, value);
      break;
    }
    case GET: {
      spdlog::info("Received GET command for '{}'", name);
      const std::string key = receiver.receive(true).value();
      std::optional<const std::string> result = cache->get(key);
      sender.send(result.value_or(std::string('\0', 1)));
      break;
    }
    case END: {
      spdlog::info("Received END command for '{}'", name);
      running = false;
      break;
    }
    default:
      break;
    }
  };
  spdlog::info("Terminating service sender for '{}'", name);
  sender.terminate();
  spdlog::info("Awaiting end of service receiver for '{}'", name);
  receiver.wait();
}

bool stashcache::Service::terminate(void) {
  spdlog::debug("Terminating service instance '{0}'...", name);
  thread.join();
  return true;
}
