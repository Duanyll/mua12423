#pragma once
#include <list>
#include <unordered_map>
#include <unordered_set>
#include "basic_types.h"
using namespace mua::types;

namespace mua {
namespace runtime {
typedef size_t local_var_id;
class runtime_context {
    std::unordered_map<std::string, const object*> global_scope;
    std::list<std::unordered_set<local_var_id>> frames;
    std::unordered_map<local_var_id, const object*> stack_var;
    std::unordered_map<local_var_id, int> captured_var;

    void clear();
    void init_predefined_varibles();

   public:
    runtime_context();
    ~runtime_context();
    void reset();

    object* get_global_varible(const std::string& name);
    void set_global_varible(const std::string& name, const object* val);

    void declare_local_varible(local_var_id id);
    object* get_local_varible(local_var_id id);
    void set_local_varible(local_var_id id, const object* val);
    void capture_local(local_var_id id);
    void decapture_local(local_var_id id);
    void enter_frame();
    void leave_frame();
};
}
}