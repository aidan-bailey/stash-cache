#include "cache.hh"
#include <stashcacheconfig.hh>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <glog/logging.h>
#include <sstream>
#include <string>

std::vector<std::string> stashcache::segment_string(const std::string &str,
                                                    const int &n) {
  const int str_length = str.size();
  std::vector<std::string> result;
  for (int i = 0; i < str_length; i += n)
    result.emplace_back(std::string(&str[i], std::min(str_length - i, n)));
  return result;
};

stashcache::Cache::Cache(int bytesize)
    : slab_size(4000000) { // TODO convert to size_t
  LruCache::Config config;
  config.setCacheSize(bytesize)
      .setCacheName("StashCache")
      .setAccessConfig({25, 10})
      .validate();
  cache = std::make_unique<LruCache>(config);
  pool_id =
      cache->addPool("default_pool", cache->getCacheMemoryStats().cacheSize);
  DLOG(INFO) << "Initialised cache";
};

stashcache::Cache::~Cache(void) { cache.reset(); }

bool stashcache::Cache::set(const std::string &key, const std::string &value) {
  const size_t value_size(value.size());
  if (value_size <= slab_size) {
    auto handle = cache->allocate(pool_id, key, value_size);
    if (!handle) {
      return false;
    }
    std::memcpy(handle->getMemory(), &value[0], value_size);
    cache->insertOrReplace(handle);
    return true;
  } else {
    std::vector<std::string> items = segment_string(value, slab_size);
    auto parent_handle = cache->allocate(pool_id, key, 0);
    if (!parent_handle) {
      return false;
    }
    cache->insertOrReplace(parent_handle);
    for (std::string &item : items) {
      auto child_handle =
          cache->allocateChainedItem(parent_handle, item.size());
      std::memcpy(child_handle->getMemory(), item.data(), item.size());
      cache->addChainedItem(parent_handle, std::move(child_handle));
    }
    return true;
  }
};

std::optional<std::string> stashcache::Cache::get(const std::string &key) {
  auto handle = cache->find(key);
  if (handle) {
    std::stringstream ss;
    if (handle->getSize() == 0) {
      auto chainedAllocs = cache->viewAsChainedAllocs(handle);
      for (auto &c : chainedAllocs.getChain()) {
        ss << std::string(reinterpret_cast<const char *>(c.getMemory()),
                          c.getSize());
      }
      return ss.str();
    } else {
      return std::string(reinterpret_cast<const char *>(handle->getMemory()),
                         handle->getSize());
    }
  }
  return {};
}
