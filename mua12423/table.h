#pragma once
#include <initializer_list>
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

    typedef std::unordered_map<const object*, const object*, object_hasher,
                               object_comp>
        store_type;
    store_type store;
    size_t last_verified_size = 0;
    bool size_verified = false;

    friend struct iterator_pairs;

   public:
    inline table() {}
    table(std::initializer_list<std::pair<std::string, const object*>> list);
    object* get_copy(const object* key) const;
    const object* get(const object* key) const;
    void set_copy(const object* key, const object* val);
    void set(const object* key, const object* val);
    size_t size();
    ~table();

    table(const table&) = delete;
    table& operator=(const table&) = delete;
};

typedef managed_pointer<table, TABLE> table_pointer;

struct iterator : public table_pointer {
    inline iterator(table* tab) : table_pointer(tab) {}
    inline predefined_typeid get_typeid() const { return ITERATOR; }
    virtual bool is_end() = 0;
    virtual object* get() = 0;
};

struct iterator_pairs : public iterator {
    table::store_type::iterator it;
    inline iterator_pairs(table* tab) : iterator(tab) {
        it = tab->store.begin();
    }
    inline object* clone() const { return new iterator_pairs(*this); }
    inline bool is_end() { return it == ptr->store.end(); }
    inline object* get() {
        auto res = it->second->clone();
        it++;
        return res;
    }
};

struct iterator_ipairs : public iterator {
    size_t it;
    inline iterator_ipairs(table* tab) : iterator(tab) { it = 1; }
    inline object* clone() const { return new iterator_ipairs(*this); }
    inline bool is_end() { return it > ptr->size(); }
    inline object* get() {
        auto res = ptr->get_copy(&number(it));
        it++;
        return res;
    }
};

#ifdef _DEBUG
void test_table();
#endif  // _DEBUG
}  // namespace types
}  // namespace mua