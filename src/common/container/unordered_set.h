#pragma once

#include "common/container_hash.h"

#include <boost/unordered/unordered_flat_set.hpp>

namespace Common {

template <class Key, class Hash = std::hash<Key>, class Pred = std::equal_to<Key>,
          class Allocator = std::allocator<Key>>
using unordered_set = boost::unordered::unordered_flat_set<Key, Hash, Pred, Allocator>;

}
