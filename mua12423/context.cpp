#include "context.h"

#include <cassert>

#include "library_functions.h"
using namespace mua::libiary_functions;

void mua::runtime::runtime_context::clear() {
    for (auto& i : stack_var) {
        delete i.second;
    }
    frames.clear();
    captured_var.clear();
    for (auto& i : global_scope) {
        delete i.second;
    }
}

#define DEF_FUNCTION(name, type) \
    global_scope[#name] = new function_pointer(new type(name))
#define DEF_FUNCTION_IN_NAMESP(scope, name, obj) \
    scope->set(&string(#name), new function_pointer(obj))
#define DEF_MATH_FUNCTION(name)                                            \
    namesp_math->set(&string(#name),                                       \
                     new function_pointer(new math::native_math_function1( \
                         static_cast<double (*)(double)>(std::name))))
// 只在 clear 之后调用
void mua::runtime::runtime_context::init_predefined_varibles() {
    DEF_FUNCTION(tonumber, native_function1);
    DEF_FUNCTION(tostring, native_function1);
    DEF_FUNCTION(print, native_function1);

    auto namesp_string = new table();
    DEF_FUNCTION_IN_NAMESP(namesp_string, rep,
                           new native_function2(string_rep));
    DEF_FUNCTION_IN_NAMESP(namesp_string, sub,
                           new native_function3(string_sub));
    global_scope["string"] = new table_pointer(namesp_string);

    auto namesp_table = new table();
    DEF_FUNCTION_IN_NAMESP(namesp_table, concat,
                           new native_function2(table_concat));
    DEF_FUNCTION_IN_NAMESP(namesp_table, sort,
                           new native_sort_function());
    global_scope["table"] = new table_pointer(namesp_table);

    auto namesp_math = new table();
    DEF_FUNCTION_IN_NAMESP(namesp_math, atan2,
                           new native_function2(math::atan2));
    DEF_FUNCTION_IN_NAMESP(namesp_math, deg, new native_function1(math::deg));
    DEF_FUNCTION_IN_NAMESP(namesp_math, rad, new native_function1(math::rad));
    DEF_FUNCTION_IN_NAMESP(namesp_math, min, new native_function2(math::min));
    DEF_FUNCTION_IN_NAMESP(namesp_math, max, new native_function2(math::max));
    // 需要的 native_math_function1
    // std::string math_functions_to_declare[] = {
    //    "abs",  "floor", "ceil", "sqrt", "exp", "log", "log10",
    //    "asin", "acos",  "atan", "sin",  "cos", "tan"};
    DEF_MATH_FUNCTION(abs);
    DEF_MATH_FUNCTION(floor);
    DEF_MATH_FUNCTION(ceil);
    DEF_MATH_FUNCTION(sqrt);
    DEF_MATH_FUNCTION(exp);
    DEF_MATH_FUNCTION(log);
    DEF_MATH_FUNCTION(log10);
    DEF_MATH_FUNCTION(asin);
    DEF_MATH_FUNCTION(acos);
    DEF_MATH_FUNCTION(atan);
    DEF_MATH_FUNCTION(sin);
    DEF_MATH_FUNCTION(cos);
    DEF_MATH_FUNCTION(tan);
    namesp_math->set(&string("pi"), new number(math::pi));
    global_scope["math"] = new table_pointer(namesp_math);

    frames.push_back(std::unordered_set<local_var_id>());
}

mua::runtime::runtime_context::runtime_context() { init_predefined_varibles(); }

mua::runtime::runtime_context::~runtime_context() { clear(); }

void mua::runtime::runtime_context::reset() {
    clear();
    init_predefined_varibles();
}

object* mua::runtime::runtime_context::get_global_varible(
    const std::string& name) {
    auto res = global_scope.find(name);
    if (res != global_scope.end()) {
        return res->second->clone();
    } else {
        return new nil();
    }
}

void mua::runtime::runtime_context::set_global_varible(const std::string& name,
                                                       const object* val) {
    auto res = global_scope.find(name);
    if (res != global_scope.end()) {
        delete res->second;
    }
    if (val->get_typeid() == NIL) {
        global_scope.erase(name);
    } else {
        global_scope[name] = val->clone();
    }
}

void mua::runtime::runtime_context::declare_local_varible(local_var_id id) {
    frames.back().insert(id);
    stack_var[id] = new nil();
}

object* mua::runtime::runtime_context::get_local_varible(local_var_id id) {
    return stack_var[id]->clone();
}

void mua::runtime::runtime_context::set_local_varible(local_var_id id, const object* val) {
    delete stack_var[id];
    stack_var[id] = val->clone();
}

void mua::runtime::runtime_context::capture_local(local_var_id id) {
    captured_var[id]++;
}

void mua::runtime::runtime_context::decapture_local(local_var_id id) {
    captured_var[id]--;
    if (captured_var[id] == 0) {
        captured_var.erase(id);
        bool found = false;
        for (auto i = frames.rbegin(); i != frames.rend(); i++) {
            if (i->count(id) != 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            delete stack_var[id];
            stack_var.erase(id);
        }
    }
}

void mua::runtime::runtime_context::enter_frame() {
    frames.push_back(std::unordered_set<local_var_id>());
}

void mua::runtime::runtime_context::leave_frame() {
    for (auto& i : frames.back()) {
        if (captured_var.count(i) != 0) continue;
        delete stack_var[i];
        stack_var.erase(i);
    }
}