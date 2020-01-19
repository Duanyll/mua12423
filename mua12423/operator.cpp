#include "operator.h"
using namespace mua::ast;

// 运算符优先级, 越小优先级越大
std::unordered_map<std::string, int> mua::ast::opr_precedence{
    {".", -2}, {"^", -1}, {"- (unop)", 0}, {"#", 0},  {"not", 0},
    {"*", 1},  {"/", 1},  {"%", 1},        {"+", 2},  {"-", 2},
    {"..", 3}, {"<", 4},  {">", 4},        {"<=", 4}, {">=", 4},
    {"~=", 4}, {"==", 4}, {"and", 5},      {"or", 6}};

#define OPR(x) std::make_shared<x>
std::unordered_map<std::string, std::function<std::shared_ptr<binop>()>>
    mua::ast::binop_mapping{
        {"*", OPR(opr_mul)},   {"/", OPR(opr_div)},   {"%", OPR(opr_mod)},
        {"+", OPR(opr_plus)},  {"-", OPR(opr_minus)}, {"..", OPR(opr_concat)},
        {"<", OPR(opr_lt)},    {">", OPR(opr_gt)},    {"<=", OPR(opr_leq)},
        {">=", OPR(opr_geq)},  {"~=", OPR(opr_neq)},  {"==", OPR(opr_eq)},
        {"and", OPR(opr_and)}, {"or", OPR(opr_or)}};

std::unordered_map<std::string, std::function<std::shared_ptr<unop>()>>
    mua::ast::unop_mapping{{"- (unop)", OPR(opr_neg)},
                           {"#", OPR(opr_length)},
                           {"not", OPR(opr_not)}};
#undef OPR

std::shared_ptr<binop> mua::ast::make_binop(std::string opr) {
    return binop_mapping[opr]();
}

std::shared_ptr<unop> mua::ast::make_unop(std::string opr) {
    return unop_mapping[opr]();
}

object* mua::ast::opr_and::eval(runtime_context* context) {
    auto l = larg->eval(context);
    if (l->equal_to(&boolean(false))) {
        delete l;
        return new boolean(false);
    }
    delete l;
    auto r = rarg->eval(context);
    if (r->equal_to(&boolean(true))) {
        delete r;
        return new boolean(true);
    } else {
        delete r;
        return new boolean(false);
    }
}

object* mua::ast::opr_or::eval(runtime_context* context) {
    auto l = larg->eval(context);
    if (l->equal_to(&boolean(true))) {
        delete l;
        return new boolean(true);
    }
    delete l;
    auto r = rarg->eval(context);
    if (r->equal_to(&boolean(true))) {
        delete r;
        return new boolean(true);
    } else {
        delete r;
        return new boolean(false);
    }
}

object* mua::ast::opr_concat::eval(runtime_context* context) {
    auto l = larg->eval(context);
    auto r = rarg->eval(context);
    object* res;
    if (l->get_typeid() == STRING && r->get_typeid() == STRING) {
        res = new string(static_cast<string*>(l)->value +
                         static_cast<string*>(r)->value);
    } else {
        res = new nil();
    }
    delete l;
    delete r;
    return res;
}

object* mua::ast::opr_eq::eval(runtime_context* context) {
    auto l = larg->eval(context);
    auto r = rarg->eval(context);
    object* res = new boolean(l->equal_to(r));
    delete l;
    delete r;
    return res;
}

inline object* mua::ast::opr_neq::eval(runtime_context* context) {
    auto l = larg->eval(context);
    auto r = rarg->eval(context);
    object* res = new boolean(!l->equal_to(r));
    delete l;
    delete r;
    return res;
}

inline object* mua::ast::opr_not::eval(runtime_context* context) {
    auto x = arg->eval(context);
    auto res =
        new boolean(x->equal_to(&boolean(false)) || x->get_typeid() == NIL);
    delete x;
    return res;
}

object* mua::ast::opr_length::eval(runtime_context* context) {
    auto x = arg->eval(context);
    object* res;
    if (x->get_typeid() == STRING) {
        res = new number(static_cast<string*>(x)->value.length());
    } else if (x->get_typeid() == TABLE) {
        res = new number(static_cast<table_pointer*>(x)->ptr->size());
    } else {
        res = new nil();
    }
    delete x;
    return res;
}

object* mua::ast::opr_neg::eval(runtime_context* context) {
    auto x = arg->eval(context);
    object* res;
    if (x->get_typeid() == NUMBER) {
        res = new number(-static_cast<number*>(x)->value);
    } else {
        res = new nil();
    }
    delete x;
    return res;
}
