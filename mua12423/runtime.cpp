#include <cassert>
#include <cmath>

#include "library_functions.h"
#include "runtime.h"
using namespace mua::libiary_functions;
using namespace mua;

void mua::runtime::clear() {
    for (auto& i : store) {
        delete i.second.obj;
    }
    store.clear();
    frames.clear();
}

#define MF(name)                            \
    new fp(new math::native_math_function1( \
        static_cast<double (*)(double)>(std::name)))
// 只在 clear 之后调用
void mua::runtime::init_predefined_varibles() {
    using fp = function_pointer;
    using f1 = native_function1;
    using f2 = native_function2;
    using f3 = native_function3;
    using tp = table_pointer;
    if (global_varibles != nullptr) delete global_varibles;
    global_varibles = new table{
        {"tonumber", new fp(new f1(tonumber))},
        {"tostring", new fp(new f1(tostring))},
        {"print", new fp(new f1(print))},
        {"pairs", new fp(new f1(pairs))},
        {"ipairs", new fp(new f1(ipairs))},
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
                                  {"tan", MF(tan)}})}};
    global_varibles->set(&string("_G"), new tp(global_varibles, false));

    push_frame();
}

#undef MF

mua::runtime::runtime() { init_predefined_varibles(); }

mua::runtime::~runtime() {
    clear();
    if (global_varibles != nullptr) delete global_varibles;
    for (auto& i : store) {
        delete i.second.obj;
    }
}

void mua::runtime::reset() {
    clear();
    init_predefined_varibles();
}

object* mua::runtime::get_global_varible(const std::string& name) {
    return global_varibles->get_copy(&string(name));
}

void mua::runtime::set_global_varible(const std::string& name,
                                      const object* val) {
    global_varibles->set_copy(&string(name), val);
}

void mua::runtime::create_storage_reference(storage_id sid) {
    store[sid].reference_count++;
}

void mua::runtime::remove_storage_reference(storage_id sid) {
    auto it = store.find(sid);
    assert(it != store.end());
    it->second.reference_count--;
    assert(it->second.reference_count >= 0);
    if (it->second.reference_count == 0) {
        delete it->second.obj;
        store.erase(sid);
    }
}

void mua::runtime::push_frame() {
    frames.push_back(std::unordered_map<local_var_id, storage_id>());
}

storage_id mua::runtime::get_varible_sid(local_var_id vid) {
    return frames.back()[vid];
}

void mua::runtime::add_caputured_varible(local_var_id vid, storage_id sid) {
    frames.back()[vid] = sid;
    create_storage_reference(sid);
}

storage_id mua::runtime::alloc_local_varible(local_var_id vid) {
    storage_id sid = next_sid++;
    store[sid].obj = new nil();
    frames.back()[vid] = sid;
    return sid;
}

void mua::runtime::pop_frame() {
    for (auto& i : frames.back()) {
        remove_storage_reference(i.second);
    }
    frames.pop_back();
}

object* mua::runtime::get_local_varible(local_var_id vid) {
    return store[frames.back()[vid]].obj->clone();
}

void mua::runtime::set_local_varible(local_var_id vid, const object* val) {
    auto it = store.find(frames.back()[vid]);
    delete it->second.obj;
    it->second.obj = val->clone();
}