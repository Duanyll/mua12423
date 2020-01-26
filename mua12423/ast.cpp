#include "ast.h"
using namespace mua::ast;

object* mua::ast::member_access::eval(rt_context* context) {
    auto u = obj->eval(context);
    if (u->get_typeid() == TABLE) {
        auto t = static_cast<table_pointer*>(u)->ptr;
        auto key = member_name->eval(context);
        auto result = t->get_copy(key);
        delete u;
        return result;
    } else {
        delete u;
        return new nil();
    }
}

void mua::ast::member_access::set_value(rt_context* context,
                                               const object* val) {
    auto u = obj->eval(context);
    if (u->get_typeid() == TABLE) {
        auto t = static_cast<table_pointer*>(u)->ptr;
        auto key = member_name->eval(context);
        t->set_copy(key, val);
        delete u;
        return;
    } else {
        delete u;
        return;
    }
}

object* mua::ast::functional_call::eval(rt_context* context) {
    auto fun = func->eval(context);
    if (fun->get_typeid() != FUNCTION) {
        utils::log("Trying to invoke a non-function object.");
        delete fun;
        return new nil();
    }
    auto pfun = static_cast<function_pointer*>(fun)->ptr;
    std::vector<const object*> param;
    for (auto i : params) {
        param.push_back(i->eval(context));
    }
    auto res = pfun->invoke(context, param);
    delete fun;
    for (auto i : param) {
        delete i;
    }
    return res;
}
