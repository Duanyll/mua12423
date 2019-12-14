#pragma once

#include <cmath>
#include "types.h"

using namespace mua::types;

namespace mua {
namespace libiary_functions {
object* tonumber(const object* obj);
object* tostring(const object* obj);
object* print(const object* obj);
namespace string {
object* rep(const object* s, const object* n);
object* sub(const object* s, const object* i, const object* j);
}  // namespace string
namespace table {
object* concat(const object* t, const object* sep);
object* sort(const object* t, const object* comp);
}  // namespace table
namespace math {
struct native_math_function1 : public function {
    double (*fun)(double);

   public:
    inline virtual object* invoke(std::vector<object*> params) const {
        if (params.size() < 1) return new nil();
        if (params[0]->get_typeid() != NUMBER) return new nil();
        double val = (static_cast<number*>(params[0]))->value;
        return new number(fun(val));
    }
    inline native_math_function1(decltype(fun) func) : fun(func) {}
};

// 如何构造 native_math_function1 对象: 
// const native_math_function1 name(static_cast<double (*)(double)>(std::name))

// 需要的 native_math_function1
std::string math_functions_to_declare[] = {
    "abs",  "floor", "ceil", "sqrt", "exp", "log", "log10",
    "asin", "acos",  "atan", "sin",  "cos", "tan"};

const double pi = std::atan(1) * 4;
object* rad(const object* a);
object* deg(const object* a);
object* atan2(const object* a, const object* b);
object* min(const object* a, const object* b);
object* max(const object* a, const object* b);
}  // namespace math
}  // namespace libiary_functions
}  // namespace mua