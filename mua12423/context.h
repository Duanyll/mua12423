#pragma once
#include <list>
#include <unordered_map>
#include "types.h"
using namespace mua::types;

namespace mua {
namespace runtime {
class runtime_context {
    typedef std::unordered_map<std::string, const object*> scope;

    scope global_scope;
    std::list<scope> scopes;

    void clear();
    void init_predefined_varibles();

   public:
    runtime_context();
    ~runtime_context();
    void reset();
    void push_frame();
    void pop_frame();
    void declare_local_varible(const std::string& name);
    object* get_varible(const std::string& name);
    void set_varible(const std::string& name, const object* val);
    size_t get_frame_depth();
};
}
}