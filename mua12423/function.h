#pragma once

#include <functional>
#include <vector>

#include "basic_types.h"
#include "context.h"

namespace mua {
namespace types {
class function : public heap_object {
   public:
    inline virtual ~function() {}
    virtual object* invoke(runtime_context* context,
                           std::vector<const object*> params) const = 0;
};

class native_function1 : public function {
    typedef std::function<object*(object*)> func_type;

   public:
    inline virtual object* invoke(runtime_context* context,
                                  std::vector<const object*> params) const {
        auto param1 = (params.size() >= 1) ? params[0]->clone() : new nil();
        auto result = fun(param1);
        delete param1;
        return result;
    }
    inline native_function1(const func_type func) : fun(func) {}

   protected:
    func_type fun;
};

class native_function2 : public function {
    typedef std::function<object*(object*, object*)> func_type;

   public:
    inline object* invoke(runtime_context* context,
                          std::vector<const object*> params) const {
        auto param1 = (params.size() >= 1) ? params[0]->clone() : new nil();
        auto param2 = (params.size() >= 2) ? params[1]->clone() : new nil();
        auto result = fun(param1, param2);
        delete param1;
        delete param2;
        return result;
    }
    inline native_function2(const func_type func) : fun(func) {}

   protected:
    func_type fun;
};

class native_function3 : public function {
    typedef std::function<object*(object*, object*, object*)> func_type;

   public:
    inline object* invoke(runtime_context* context,
                          std::vector<const object*> params) const {
        auto param1 = (params.size() >= 1) ? params[0]->clone() : new nil();
        auto param2 = (params.size() >= 2) ? params[1]->clone() : new nil();
        auto param3 = (params.size() >= 3) ? params[2]->clone() : new nil();
        auto result = fun(param1, param2, param3);
        delete param1;
        delete param2;
        delete param3;
        return result;
    }
    inline native_function3(const func_type func) : fun(func) {}

   protected:
    func_type fun;
};
}  // namespace types
}  // namespace mua