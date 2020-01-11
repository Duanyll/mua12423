#include "operator.h"
using namespace mua::ast;

std::unordered_map<std::string, int> mua::ast::precedence{
    {"*", 1},  {"/", 1},  {"%", 1},   {"+", 2},  {"-", 2},
    {"..", 3}, {"<", 4},  {">", 4},   {"<=", 4}, {">=", 4},
    {"~=", 4}, {"==", 4}, {"and", 5}, {"or", 6}};

std::shared_ptr<binop> mua::ast::make_binop(std::string opr, pexpr larg,
                                            pexpr rarg) {
    return std::shared_ptr<binop>();
}
