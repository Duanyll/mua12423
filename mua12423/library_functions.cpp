#include "library_functions.h"
#include <cstdlib>
#include <iostream>
#include <regex>

object* mua::libiary_functions::tonumber(const object* obj) {
    if (obj->get_typeid() == NUMBER) return obj->clone();
    if (obj->get_typeid() != STRING) return new nil();
    std::string str = (static_cast<const types::string*>(obj))->value;

    if (std::regex_match(
            str,
            std::regex(
                R"(^(-?(0|[1-9]\d*)?(\.\d+)?(?<=\d)(e[-+]?(0|[1-9]\d*))?|[-+]?0x[0-9a-f]+)$)",
                std::regex::icase))) {
        return new number(std::atof(str.c_str()));
    } else {
        return new nil();
    }
}

object* mua::libiary_functions::tostring(const object* obj) {
    switch (obj->get_typeid()) {
        case OBJECT:
            return new types::string("object");
        case BOOLEAN:
            if ((static_cast<const boolean*>(obj))->value) {
                return new types::string("true");
            } else {
                return new types::string("false");
            }
        case NUMBER:
            char buffer[50] = {0};
            sprintf(buffer, "%.14g", (static_cast<const number*>(obj)->value));
            return new types::string(buffer);
        case STRING:
            return obj->clone();
        case NIL:
            return new types::string("nil");
        case TABLE:
            return new types::string("table");
        case FUNCTION:
            return new types::string("function");
        default:
            return new types::string("");
    }
}

object* mua::libiary_functions::print(const object* obj) {
    auto str = static_cast<types::string*>(tostring(obj));
    std::cout << str->value << std::endl;
    delete str;
    return new nil();
}

object* mua::libiary_functions::string::rep(const object* s, const object* n) {
    if (s->get_typeid() != STRING) return new nil();
    if (n->get_typeid() != NUMBER) return new nil();
    std::string str = (static_cast<const types::string*>(s))->value;
    double cnt = (static_cast<const number*>(n))->value;
    if (std::round(cnt) != cnt) return new nil();
    std::string res = "";
    for (int i = 1; i <= cnt; i++) {
        res += str;
    }
    return new types::string(res);
}

object* mua::libiary_functions::string::sub(const object* s, const object* i,
                                            const object* j) {
    if (s->get_typeid() != STRING) return new nil();
    if (i->get_typeid() != NUMBER) return new nil();
    if (j->get_typeid() != NUMBER && j->get_typeid() != NIL) return new nil();
    std::string str = static_cast<const types::string*>(s)->value;
    double start = static_cast<const number*>(i)->value;
    double end =
        (j->get_typeid() == NIL) ? -1 : static_cast<const number*>(i)->value;
    if (std::round(start) != start) return new nil();
    if (std::round(end) != end) return new nil();
    int spos = (start > 0) ? (start - 1) : (str.length() + start);
    int epos = (end > 0) ? (end - 1) : (str.length() + end);
    if (spos < 0 || epos >= str.length()) return new nil();
    int count = epos - spos + 1;
    return new types::string(str.substr(spos, count));
}

object* mua::libiary_functions::table::concat(const object* t,
                                              const object* sep) {
    if (t->get_typeid() != TABLE) return new nil();
    if (sep->get_typeid() != NIL && sep->get_typeid() != STRING)
        return new nil();
    auto tab = static_cast<const table_pointer*>(t)->ptr;
    auto seperator = (sep->get_typeid() == NIL)
                         ? ""
                         : static_cast<const types::string*>(sep)->value;
    size_t size = tab->size();
    std::string res = "";
    for (int i = 1; i <= size; i++) {
        auto obj = tab->get_copy(&number(i));
        if (obj->get_typeid() == STRING) {
            res += static_cast<types::string*>(obj)->value;
            if (i < size) res += seperator;
        }
        delete obj;
    }
    return new types::string(res);
}

template <typename T>
void merge_sort(int l, int r, T* arr, T* tmp,
                const std::function<bool(T, T)>& comp) {
    if (l >= r) return;
    int mid = (l + r) >> 1;
    merge_sort(l, mid, arr, tmp, comp);
    merge_sort(mid + 1, r, arr, tmp, comp);
    int i = l, j = mid + 1;
    int k = l;
    while (i <= mid && j <= r) {
        if (comp(arr[i], arr[j])) {
            tmp[k] = arr[i];
            i++;
        } else {
            tmp[k] = arr[j];
            j++;
        }
        k++;
    }
    while (i <= mid) tmp[k++] = arr[i++];
    while (j <= r) tmp[k++] = arr[j++];
    for (int i = l; i <= r; i++) {
        arr[i] = tmp[i];
    }
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

object* mua::libiary_functions::table::sort(const object* t,
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
    auto arr = new const object*[size + 1];
    auto tmp = new const object*[size + 1];
    for (size_t i = 1; i <= size; i++) {
        arr[i] = tab->get(&number(i));
    }
    merge_sort(1, size, arr, tmp, comp_func);
    for (size_t i = 1; i <= size; i++) {
        tab->set(&number(i), arr[i]);
    }

    delete[] arr;
    delete[] tmp;
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

