#include "library_functions.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <vector>
#include "utils.h"

object* mua::libiary_functions::tonumber(const object* obj) {
    if (obj->get_typeid() == NUMBER) return obj->clone();
    if (obj->get_typeid() != STRING) return new nil();
    std::string str = (static_cast<const string*>(obj))->value;
    std::regex pattern_number(R"(^([+-]?\d+\.\d+)|([+-]?\d+)|([+-]?\.\d+)$)");
    std::regex pattern_scientific(
        R"(^[+-]?((\d+\.?\d*)|(\.\d+))[Ee][+-]?\d+$)");
    std::regex pattern_heximal(R"(^[+-]?0[xX]([A-Fa-f0-9])+$)");
    if (std::regex_match(str, pattern_number) ||
        std::regex_match(str, pattern_scientific) ||
        std::regex_match(str, pattern_heximal)) {
        return new number(std::atof(str.c_str()));
    } else {
        return new nil();
	}
}

object* mua::libiary_functions::tostring(const object* obj) {
    char buffer[50] = {0};
    switch (obj->get_typeid()) {
        case OBJECT:
            return new string("object");
        case BOOLEAN:
            if ((static_cast<const boolean*>(obj))->value) {
                return new string("true");
            } else {
                return new string("false");
            }
        case NUMBER:
            sprintf(buffer, "%.14g", (static_cast<const number*>(obj)->value));
            return new string(buffer);
        case STRING:
            return obj->clone();
        case NIL:
            return new string("nil");
        case TABLE:
            return new string("table");
        case FUNCTION:
            return new string("function");
        default:
            return new string("");
    }
}

object* mua::libiary_functions::print(const object* obj) {
    auto str = static_cast<string*>(tostring(obj));
    std::cout << str->value << std::endl;
    delete str;
    return new nil();
}

object* mua::libiary_functions::string_rep(const object* s, const object* n) {
    if (s->get_typeid() != STRING) return new nil();
    if (n->get_typeid() != NUMBER) return new nil();
    std::string str = (static_cast<const string*>(s))->value;
    double cnt = (static_cast<const number*>(n))->value;
    if (std::round(cnt) != cnt) return new nil();
    std::string res = "";
    for (int i = 1; i <= cnt; i++) {
        res += str;
    }
    return new string(res);
}

object* mua::libiary_functions::string_sub(const object* s, const object* i,
                                            const object* j) {
    if (s->get_typeid() != STRING) return new nil();
    if (i->get_typeid() != NUMBER) return new nil();
    if (j->get_typeid() != NUMBER && j->get_typeid() != NIL) return new nil();
    std::string str = static_cast<const string*>(s)->value;
    double start = static_cast<const number*>(i)->value;
    double end =
        (j->get_typeid() == NIL) ? -1 : static_cast<const number*>(j)->value;
    if (std::round(start) != start) return new nil();
    if (std::round(end) != end) return new nil();
    int spos = (start > 0) ? (start - 1) : (str.length() + start);
    int epos = (end > 0) ? (end - 1) : (str.length() + end);
    if (spos < 0 || epos >= str.length()) return new nil();
    int count = epos - spos + 1;
    return new string(str.substr(spos, count));
}

object* mua::libiary_functions::table_concat(const object* t,
                                              const object* sep) {
    if (t->get_typeid() != TABLE) return new nil();
    if (sep->get_typeid() != NIL && sep->get_typeid() != STRING)
        return new nil();
    auto tab = static_cast<const table_pointer*>(t)->ptr;
    auto seperator = (sep->get_typeid() == NIL)
                         ? ""
                         : static_cast<const string*>(sep)->value;
    size_t size = tab->size();
    std::string res = "";
    for (int i = 1; i <= size; i++) {
        auto obj = tab->get_copy(&number(i));
        if (obj->get_typeid() == STRING) {
            res += static_cast<string*>(obj)->value;
            if (i < size) res += seperator;
        }
        delete obj;
    }
    return new string(res);
}

// 排序用比较函数与默认比较运算符不同, 必须返回一个确定的值
bool default_sort_comp(const object* a, const object* b) {
    if (a->get_typeid() == b->get_typeid()) {
        switch (a->get_typeid()) {
            case NUMBER:
                return static_cast<const number*>(a)->value <
                       static_cast<const number*>(b)->value;
            case STRING:
                return static_cast<const string*>(a)->value <
                       static_cast<const string*>(b)->value;
            default:
                return a->get_hashcode() < b->get_hashcode();
        }
    } else {
        return a->get_typeid() < b->get_typeid();
    }
}

object* mua::libiary_functions::table_sort(const object* t,
                                            const object* comp) {
    if (t->get_typeid() != TABLE) return new nil();

    std::function<bool(const object*, const object*)> comp_func;
    if (comp->get_typeid() != FUNCTION) {
        comp_func = default_sort_comp;
    } else {
        auto fun = static_cast<const function_pointer*>(comp)->ptr;
        comp_func = [&fun](const object* a, const object* b) -> bool {
            auto res = fun->invoke({a, b});
            if (res->equal_to(&boolean(true))) {
                delete res;
                return true;
            } else {
                delete res;
                return false;
            }
        };
    }

    auto tab = static_cast<const table_pointer*>(t)->ptr;
    auto size = tab->size();
    std::vector<const object*> arr(size + 1);
    std::vector<const object*> tmp(size + 1);
    for (size_t i = 1; i <= size; i++) {
        arr[i] = tab->get_copy(&number(i));
    }
    merge_sort(1, size, arr, tmp, comp_func);
    for (size_t i = 1; i <= size; i++) {
        tab->set(&number(i), arr[i]);
    }


    return new nil();
}

object* mua::libiary_functions::math::rad(const object* a) {
    if (a->get_typeid() != NUMBER) return new nil();
    double val = static_cast<const number*>(a)->value;
    return new number(val * pi / 180);
}

object* mua::libiary_functions::math::deg(const object* a) {
    if (a->get_typeid() != NUMBER) return new nil();
    double val = static_cast<const number*>(a)->value;
    return new number(val * 180 / pi);
}

object* mua::libiary_functions::math::atan2(const object* a, const object* b) {
    if (a->get_typeid() != NUMBER) return new nil();
    if (b->get_typeid() != NUMBER) return new nil();
    return new number(std::atan2(static_cast<const number*>(a)->value,
                                 static_cast<const number*>(b)->value));
}

object* mua::libiary_functions::math::min(const object* a, const object* b) {
    return default_sort_comp(a, b) ? a->clone() : b->clone();
}

object* mua::libiary_functions::math::max(const object* a, const object* b) {
    return default_sort_comp(a, b) ? b->clone() : a->clone();
}

void mua::libiary_functions::test_libirary_function() {
    assert(tonumber(&string("1.2345"))->equal_to(&number(1.2345)));
    assert(tonumber(&string("1.2345aa"))->equal_to(&nil()));

    assert(tostring(&number(1.23))->equal_to(&string("1.23")));

	assert(string_sub(&string("abcdef"), &number(1), &number(3))
               ->equal_to(&string("abc")));
    assert(string_sub(&string("abcdef"), &number(-3), &nil())
               ->equal_to(&string("def")));
    assert(string_sub(&string("abcdef"), &number(2), &number(-2))
                   ->equal_to(&string("bcde")));

	types::table tab;
    tab.set_copy(&number(1), &string("2"));
    tab.set_copy(&number(2), &string("5"));
    tab.set_copy(&number(3), &string("1"));
    tab.set_copy(&number(4), &string("3"));
    tab.set_copy(&number(5), &string("4"));

	assert(table_concat(&table_pointer(&tab, false), &string(","))->equal_to(&string("2,5,1,3,4")));

    table_sort(&table_pointer(&tab, false), &nil());
    for (int i = 1; i < 5; i++) {
        std::string a =
            static_cast<const string*>(tab.get(&number(i)))->value;
        std::string b =
            static_cast<const string*>(tab.get(&number(i + 1)))->value;
        assert(a <= b);
	}

    std::clog << "Library function test passed." << std::endl;
}