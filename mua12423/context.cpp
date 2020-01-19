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
    global_varibles = table();
}

#define MF(name)                            \
    new fp(new math::native_math_function1( \
        static_cast<double (*)(double)>(std::name)))
// 只在 clear 之后调用
void mua::runtime::runtime_context::init_predefined_varibles() {
    using fp = function_pointer;
    using f1 = native_function1;
    using f2 = native_function2;
    using f3 = native_function3;
    using tp = table_pointer;
    global_varibles = table{
        {"tonumber", new fp(new f1(tonumber))},
        {"tostring", new fp(new f1(tostring))},
        {"print", new fp(new f1(print))},
        {"string", new tp(new table{{"rep", new fp(new f2(string_rep))},
                                    {"sub", new fp(new f3(string_sub))}})},
        {"table",
         new tp(new table{{"concat", new fp(new f2(table_concat))},
                          {"sort", new fp(new native_sort_function())}})},
        {"math", new tp(new table{{"atan2", new fp(new f2(math::atan2))},
                                  {"deg", new fp(new f1(math::deg))},
                                  {"rad", new fp(new f1(math::rad))},
                                  {"min", new fp(new f2(math::min))},
                                  {"max", new fp(new f2(math::max))},
                                  {"pi", new number(math::pi)},
                                  {"abs", MF(abs)},
                                  {"floor", MF(floor)},
                                  {"ceil", MF(ceil)},
                                  {"sqrt", MF(sqrt)},
                                  {"exp", MF(exp)},
                                  {"log", MF(log)},
                                  {"log10", MF(log10)},
                                  {"asin", MF(asin)},
                                  {"acos", MF(acos)},
                                  {"atan", MF(atan)},
                                  {"sin", MF(sin)},
                                  {"cos", MF(cos)},
                                  {"tan", MF(tan)}})},
        {"_G", new tp(&global_varibles, false)}};

    frames.push_back(std::unordered_set<local_var_id>());
}

#undef MF

mua::runtime::runtime_context::runtime_context() { init_predefined_varibles(); }

mua::runtime::runtime_context::~runtime_context() { clear(); }

void mua::runtime::runtime_context::reset() {
    clear();
    init_predefined_varibles();
}

object* mua::runtime::runtime_context::get_global_varible(
    const std::string& name) {
    return global_varibles.get_copy(&string(name));
}

void mua::runtime::runtime_context::set_global_varible(const std::string& name,
                                                       const object* val) {
    global_varibles.set_copy(&string(name), val);
}

void mua::runtime::runtime_context::declare_local_varible(local_var_id id) {
    frames.back().insert(id);
    stack_var[id] = new nil();
}

object* mua::runtime::runtime_context::get_local_varible(local_var_id id) {
    return stack_var[id]->clone();
}

void mua::runtime::runtime_context::set_local_varible(local_var_id id,
                                                      const object* val) {
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