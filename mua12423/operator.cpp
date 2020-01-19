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
        {">=", OPR(opr_geq)},  {"~=", OPR(opr_neq)},  {"==", OPR(opr_neq)},
        {"and", OPR(opr_and)}, {"or", OPR(opr_or)}};

std::unordered_map<std::string, std::function<std::shared_ptr<unop>()>>
    unop_mapping{{"- (unop)", OPR(opr_neg)},
                 {"#", OPR(opr_length)},
                 {"not", OPR(opr_not)}};
#undef OPR

std::shared_ptr<binop> mua::ast::make_binop(std::string opr) {
    return binop_mapping[opr]();
}

std::shared_ptr<unop> mua::ast::make_unop(std::string opr) {
    return unop_mapping[opr]();
}
