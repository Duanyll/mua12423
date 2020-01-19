#pragma once
#include <cmath>
#include <functional>
#include <unordered_map>

#include "ast.h"
using namespace mua::types;
using namespace mua::runtime;

namespace mua {
namespace ast {
class binop : public expr {
   public:
    pexpr larg;
    pexpr rarg;
};

class unop : public expr {
   public:
    pexpr arg;
};

template <typename T>
class simple_opr : public binop {
   public:
    inline object* eval(runtime_context* context) {
        auto l = larg->eval(context);
        auto r = rarg->eval(context);
        object* res;
        if (l->get_typeid() == NUMBER && r->get_typeid() == NUMBER) {
            res = new number(T()(static_cast<number*>(l)->value,
                                 static_cast<number*>(r)->value));
        } else {
            res = new nil();
        }
        delete l;
        delete r;
        return res;
    }
};

typedef simple_opr<std::plus<double>> opr_plus;
typedef simple_opr<std::minus<double>> opr_minus;
typedef simple_opr<std::multiplies<double>> opr_mul;
typedef simple_opr<std::divides<double>> opr_div;

template <typename T>
struct mua_mod {
    T operator()(T a, T b) const { return a - std::floor(a / b) * b; }
};

template <typename T>
struct mua_pow {
    T operator()(T a, T b) const { return std::pow(a, b); }
};

typedef simple_opr<mua_mod<double>> opr_mod;
typedef simple_opr<mua_pow<double>> opr_pow;

class opr_and : public binop {
   public:
    inline object* eval(runtime_context* context) {
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
};

class opr_or : public binop {
   public:
    inline object* eval(runtime_context* context) {
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
};

class opr_concat : public binop {
   public:
    object* eval(runtime_context* context) {
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
};

template <typename TNum, typename TStr>
class compare_opr : public binop {
   public:
    inline object* eval(runtime_context* context) {
        auto l = larg->eval(context);
        auto r = rarg->eval(context);
        object* res;
        if (l->get_typeid() == NUMBER && r->get_typeid() == NUMBER) {
            res = new boolean(TNum()(static_cast<number*>(l)->value,
                                     static_cast<number*>(r)->value));
        } else if (l->get_typeid() == STRING && r->get_typeid() == STRING) {
            res = new boolean(TStr()(static_cast<string*>(l)->value,
                                     static_cast<string*>(r)->value));
        } else {
            res = new nil();
        }
        delete l;
        delete r;
        return res;
    }
};

#define COMPARE_OPR(stdname, name) \
    typedef compare_opr<stdname<double>, stdname<std::string>> name
COMPARE_OPR(std::less, opr_lt);
COMPARE_OPR(std::greater, opr_gt);
COMPARE_OPR(std::less_equal, opr_leq);
COMPARE_OPR(std::greater_equal, opr_geq);

class opr_eq : public binop {
   public:
    inline object* eval(runtime_context* context) {
        auto l = larg->eval(context);
        auto r = rarg->eval(context);
        object* res = new boolean(l->equal_to(r));
        delete l;
        delete r;
        return res;
    }
};

class opr_neq : public binop {
   public:
    inline object* eval(runtime_context* context) {
        auto l = larg->eval(context);
        auto r = rarg->eval(context);
        object* res = new boolean(!l->equal_to(r));
        delete l;
        delete r;
        return res;
    }
};

class opr_not : public unop {
   public:
    inline object* eval(runtime_context* context) {
        auto x = arg->eval(context);
        auto res = new boolean(x->equal_to(&boolean(false)) || x->get_typeid() == NIL);
        delete x;
        return res;
    }
};

class opr_length : public unop {
   public:
    inline object* eval(runtime_context* context) {
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
};

class opr_neg : public unop {
   public:
    inline object* eval(runtime_context* context) {
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
};

// 不包括 ^ 运算符, 因为他是右结合的, 须单独考虑, 且优先级最高
extern std::unordered_map<std::string, int> opr_precedence;
extern std::unordered_map<std::string, std::function<std::shared_ptr<binop>()>>
    binop_mapping;
extern std::unordered_map<std::string, std::function<std::shared_ptr<unop>()>>
    unop_mapping;
std::shared_ptr<binop> make_binop(std::string opr);
std::shared_ptr<unop> make_unop(std::string opr);
}  // namespace ast
}  // namespace mua