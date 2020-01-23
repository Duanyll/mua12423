#pragma once
#include <list>
#include <unordered_map>
#include <unordered_set>

#include "basic_types.h"
#include "table.h"

namespace mua {
using namespace mua::types;
typedef size_t local_var_id;
typedef size_t storage_id;
class runtime_context {
    table* global_varibles = nullptr;

    struct varible_storge {
        int reference_count = 1;
        const object* obj = new nil();
        ~varible_storge() { delete obj; }
    };

    storage_id next_sid = 0;

    std::unordered_map<storage_id, varible_storge> store;
    std::list<std::unordered_map<local_var_id, storage_id>> frames;

    void clear();
    void init_predefined_varibles();

   public:
    runtime_context();
    ~runtime_context();
    void reset();

    object* get_global_varible(const std::string& name);
    void set_global_varible(const std::string& name, const object* val);

    void create_storage_reference(storage_id sid);
    void remove_storage_reference(storage_id sid);

    void push_frame();
    storage_id get_varible_sid(local_var_id vid);
    void add_caputured_varible(local_var_id vid, storage_id sid);
    storage_id alloc_local_varible(local_var_id vid);
    void pop_frame();

    object* get_local_varible(local_var_id vid);
    void set_local_varible(local_var_id vid, const object* val);
};
}  // namespace mua