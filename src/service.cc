#include "service.hh"
#include "stashcacheconfig.hh"
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
    : name(name), receiver(name, clientpipe), sender(name, serverpipe),
      cache(cache), running(true) {
  thread = std::thread(&Service::serve, this);
  DLOG(INFO) << "Constructed service instance " << name << " with server pipe "
             << serverpipe << " and client pipe " << clientpipe;
}

void stashcache::Service::serve() {
  sleep(1);
  while (running) {
    DLOG(INFO) << "Service " << name << " waiting for message...";
    const std::optional<const std::string> cmd_opt = receiver.receive(true);
    if (not cmd_opt.has_value()) {
      LOG(ERROR) << "Service " << name << " received empty command";
      running = false;
      continue;
    }
    const std::string &cmd = cmd_opt.value();
    switch (REQUEST_STR_MAP.at(cmd)) {
    case SET: {
      DLOG(INFO) << "Received SET message for " << name;
      const std::optional<const std::string> key_opt = receiver.receive(true);
      if (not key_opt.has_value()) {
        LOG(ERROR) << "Service " << name << " received empty key";
        running = false;
        continue;
      }
      const std::string &key = key_opt.value();
      const std::optional<const std::string> value_opt = receiver.receive(true);
      if (not value_opt.has_value()) {
        LOG(ERROR) << "Service " << name << " received empty key";
        running = false;
        continue;
      }
      const std::string &value = value_opt.value();
      cache->set(key, value);
      break;
    }
    case GET: {
      DLOG(INFO) << "Received GET message for " << name;
      const std::optional<const std::string> key_opt = receiver.receive(true);
      if (not key_opt.has_value()) {
        LOG(ERROR) << "Service " << name << " received empty key";
        running = false;
        continue;
      }
      const std::string &key = key_opt.value();
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

std::string stashcache::Service::get_client_name(void) const { return name; }

std::filesystem::path stashcache::Service::get_client_pipe(void) const {
  return receiver.get_pipe();
}

std::filesystem::path stashcache::Service::get_server_pipe(void) const {
  return sender.get_pipe();
}

bool stashcache::Service::is_running(void) const { return running; }
