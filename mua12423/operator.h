#pragma once
#include <cmath>
#include <functional>
#include <unordered_map>

#include "ast.h"
#include "utils.h"

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
    object* eval(runtime* rt) {
        auto l = larg->eval(rt);
        auto r = rarg->eval(rt);
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

typedef simple_opr<utils::mua_mod<double>> opr_mod;
typedef simple_opr<utils::mua_pow<double>> opr_pow;

class opr_and : public binop {
   public:
    object* eval(runtime* rt);
};

class opr_or : public binop {
   public:
    object* eval(runtime* rt);
};

class opr_concat : public binop {
   public:
    object* eval(runtime* rt);
};

template <typename TNum, typename TStr>
class compare_opr : public binop {
   public:
    inline object* eval(runtime* rt) {
        auto l = larg->eval(rt);
        auto r = rarg->eval(rt);
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
    object* eval(runtime* rt);
};

class opr_neq : public binop {
   public:
    object* eval(runtime* rt);
};

class opr_not : public unop {
   public:
    object* eval(runtime* rt);
};

class opr_length : public unop {
   public:
    object* eval(runtime* rt);
};

class opr_neg : public unop {
   public:
    object* eval(runtime* rt);
};

extern std::unordered_map<std::string, int> opr_precedence;
extern std::unordered_map<std::string, std::function<std::shared_ptr<binop>()>>
    binop_mapping;
extern std::unordered_map<std::string, std::function<std::shared_ptr<unop>()>>
    unop_mapping;
std::shared_ptr<binop> make_binop(std::string opr);
std::shared_ptr<unop> make_unop(std::string opr);
}  // namespace ast
}  // namespace mua