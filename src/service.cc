#include "service.hh"
#include <cppiper/receiver.hh>
#include <functional>
#include <glog/logging.h>
#include <optional>
#include <string>
#include <thread>

stashcache::Service::Service(const std::string name,
                             const std::filesystem::path serverpipe,
                             const std::filesystem::path clientpipe,
                             std::shared_ptr<stashcache::Cache> cache)
    : name(name), sender(name, serverpipe), receiver(name, clientpipe),
      cache(cache), running(true),
      thread(serve, name, std::ref(receiver), std::ref(sender), cache,
             std::ref(running)) {
  DLOG(INFO) << "Constructed service instance " << name << " with server pipe "
             << serverpipe << " and client pipe " << clientpipe;
}

void stashcache::Service::serve(const std::string name,
                                cppiper::Receiver &receiver,
                                cppiper::Sender &sender,
                                std::shared_ptr<stashcache::Cache> cache,
                                bool &running) {
  while (running) {
    DLOG(INFO) << "Service " << name << " waiting for message...";
    const std::string cmd = receiver.receive(true).value();
    switch (REQUEST_STR_MAP.at(cmd)) {
    case SET: {
      DLOG(INFO) << "Received SET message for " << name;
      const std::string key = receiver.receive(true).value();
      const std::string value = receiver.receive(true).value();
      cache->set(key, value);
      break;
    }
    case GET: {
      DLOG(INFO) << "Received GET message for " << name;
      const std::string key = receiver.receive(true).value();
      std::optional<const std::string> result = cache->get(key);
      sender.send(result.value_or(std::string("\0", 1)));
      break;
    }
    case END: {
      DLOG(INFO) << "Received END message for " << name;
      running = false;
      break;
    }
    default:
      DLOG(INFO) << "Received unexpected message: " << name;
      break;
    }
  };
  DLOG(INFO) << "Terminating service sender for " << name;
  sender.terminate();
  DLOG(INFO) << "Awaiting end of service receiver for " << name;
  receiver.wait();
}

bool stashcache::Service::terminate(void) {
  DLOG(INFO) << "Terminating service instance " << name;
  thread.join();
  return true;
}

std::filesystem::path stashcache::Service::get_client_pipe(void) const {
  return receiver.get_pipe();
}

std::filesystem::path stashcache::Service::get_server_pipe(void) const {
  return sender.get_pipe();
}

bool stashcache::Service::is_running(void) const { return running; }
