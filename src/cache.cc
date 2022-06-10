#include "cache.hh"
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

stashcache::Cache::Cache(int bytesize) { // TODO convert to size_t
  LruCache::Config config;
  config.setCacheSize(bytesize)
      .setCacheName("StashCache")
      .setAccessConfig({25, 10})
      .validate();
  cache = std::make_unique<LruCache>(config);
  pool_id = cache->addPool("default_pool", cache->getCacheMemoryStats().cacheSize);
  spdlog::debug("Initialised cache");
};

stashcache::Cache::~Cache(void) { cache.reset(); }

bool stashcache::Cache::set(std::string key, std::string value) {
  auto handle = cache->allocate(pool_id, key, value.size());
  if (handle) {
    std::memcpy(handle->getMemory(), value.data(), value.size());
    cache->insertOrReplace(handle);
    return true;
  } else {
    return false;
  }
};

std::optional<std::string> stashcache::Cache::get(std::string key) {
  auto handle = cache->find(key);
  if (handle) {
    auto value = reinterpret_cast<const char *>(handle->getMemory());
    return std::string(value, handle->getSize());
  }
  return {};
}
