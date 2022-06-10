#ifndef CACHE_HH_
#define CACHE_HH_
#include "cachelib/allocator/CacheAllocator.h"
#include <memory>
#include <optional>
#include <string>
using LruCache = facebook::cachelib::LruAllocator;

namespace stashcache {

class Cache {

private:
  signed char pool_id;
  std::unique_ptr<LruCache> cache;

public:
  Cache(void) = delete;
  Cache(int bytesize);
  ~Cache(void);
  bool set(std::string key, std::string value);
  std::optional<std::string> get(std::string key);
};

} // namespace stashcache

#endif // CACHE_HH_
