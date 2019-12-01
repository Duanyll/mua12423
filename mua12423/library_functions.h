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
#define MATH_FUNC(name)                                \
    inline object* name(const object* a) {             \
        if (a->get_typeid != NUMBER) return new nil(); \
        auto num = static_cast<const number*>(a);      \
        return new number(std::name(num->value));      \
    }

MATH_FUNC(abs)
MATH_FUNC(floor)
MATH_FUNC(ceil)
MATH_FUNC(sqrt)
MATH_FUNC(exp)
MATH_FUNC(log)
MATH_FUNC(log10)
const number pi(std::atan(1) * 4);
object* rad(const object* a);
object* deg(const object* a);
MATH_FUNC(asin)
MATH_FUNC(acos)
MATH_FUNC(atan)
object* atan2(const object* a, const object* b);
MATH_FUNC(cos)
MATH_FUNC(sin)
MATH_FUNC(tan)
object* min(const object* a, const object* b);
object* max(const object* a, const object* b);
}  // namespace math
}  // namespace libiary_functions
}  // namespace mua