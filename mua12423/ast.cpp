#include "ast.h"
using namespace mua::ast;

object* mua::ast::member_access::eval(runtime* rt) {
    auto u = obj->eval(rt);
    if (u->get_typeid() == TABLE) {
        auto t = static_cast<table_pointer*>(u)->ptr;
        auto key = member_name->eval(rt);
        auto result = t->get_copy(key);
        delete u;
        return result;
    } else {
        delete u;
        return new nil();
    }
}

void mua::ast::member_access::set_value(runtime* rt, const object* val) {
    auto u = obj->eval(rt);
    if (u->get_typeid() == TABLE) {
        auto t = static_cast<table_pointer*>(u)->ptr;
        auto key = member_name->eval(rt);
        t->set_copy(key, val);
        delete key;
    }
    delete u;
}

object* mua::ast::functional_call::eval(runtime* rt) {
    auto fun = func->eval(rt);
    if (fun->get_typeid() != FUNCTION) {
        utils::log("Trying to invoke a non-function object.");
        delete fun;
        return new nil();
    }
    auto pfun = static_cast<function_pointer*>(fun)->ptr;
    std::vector<const object*> param;
    for (auto i : params) {
        param.push_back(i->eval(rt));
    }
    auto res = pfun->invoke(rt, param);
    delete fun;
    for (auto i : param) {
        delete i;
    }
    return res;
}