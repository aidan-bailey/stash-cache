#ifndef CACHE_HH_
#define CACHE_HH_
#include "cachelib/allocator/CacheAllocator.h"
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
using LruCache = facebook::cachelib::LruAllocator;

namespace stashcache {

std::vector<std::string> segment_string(const std::string &str, const int &n);

class Cache {

private:
  const size_t slab_size;
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
