#pragma once
#include <unordered_map>
#include "basic_types.h"

namespace mua {
namespace types {
class table : public heap_object {
    struct object_hasher {
        inline size_t operator()(const object* obj) const {
            return obj->get_hashcode();
        }
    };

    struct object_comp {
        inline bool operator()(const object* a, const object* b) const {
            return a->equal_to(b);
        }
    };

    std::unordered_map<const object*, const object*, object_hasher, object_comp> store;
    size_t last_verified_size = 0;
    bool size_verified = false;

   public:
    object* get_copy(const object* key) const;
    const object* get(const object* key) const;
    void set_copy(const object* key, const object* val);
    void set(const object* key, const object* val);
    size_t size();
};

#ifdef _DEBUG
void test_table();
#endif  // _DEBUG


}  // namespace types
}  // namespace mua