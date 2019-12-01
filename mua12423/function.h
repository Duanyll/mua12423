#pragma once

#include "basic_types.h"
#include <vector>
#include <functional>

namespace mua {
namespace types {
class function : public heap_object {
   public:
    inline virtual ~function() {}
    virtual object* invoke(std::vector<object*> params) = 0;
};

class native_function1 : public function {
   public:
    typedef std::function<object*(object*)> func_type;
    inline object* invoke(std::vector<object*> params) {
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
   public:
    typedef std::function<object*(object*, object*)> func_type;
    inline object* invoke(std::vector<object*> params) {
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
   public:
    typedef std::function<object*(object*, object*)> func_type;
    inline object* invoke(std::vector<object*> params) {
        auto param1 = (params.size() >= 1) ? params[0]->clone() : new nil();
        auto param2 = (params.size() >= 2) ? params[1]->clone() : new nil();
        auto param3 = (params.size() >= 3) ? params[2]->clone() : new nil();
        auto result = fun(param1, param2);
        delete param1;
        delete param2;
        delete param3;
        return result;
    }
    inline native_function3(const func_type func) : fun(func) {}

   protected:
    func_type fun;
};
}
}