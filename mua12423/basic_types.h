#pragma once
#include <string>

namespace mua {
namespace types {
enum predefined_typeid {
    OBJECT,
    BOOLEAN,
    NUMBER,
    STRING,
    NIL,
    TABLE,
    FUNCTION
};

struct object {
    inline object() {}
    inline virtual ~object() {}

    inline virtual predefined_typeid get_typeid() const { return OBJECT; }
    inline virtual size_t get_hashcode() const { return 0; }
    inline virtual bool equal_to(const object* b) const {
        return b->get_typeid() == get_typeid();
    }
    inline virtual object* clone() const { return new object(); }
};

class heap_object {};

template <class T, predefined_typeid id>
struct value_object : public object {
    T value;
    inline virtual ~value_object() {}
    constexpr value_object(T val) : value(val) {}
    inline predefined_typeid get_typeid() const { return id; }
    inline size_t get_hashcode() const { return std::hash<T>()(value); }
    inline bool equal_to(const object* b) const {
#ifdef USE_DYNAMIC_CAST
        auto obj = dynamic_cast<value_object<T, id>*>(b);
        if (obj == nullptr) return false;
        return obj->value == value;
#else
        if (get_typeid() != b->get_typeid()) return false;
        auto obj = static_cast<const value_object<T, id>*>(b);
        return obj->value == value;
#endif
    }
    inline object* clone() const { return new value_object<T, id>(value); }
};

typedef value_object<bool, BOOLEAN> boolean;
typedef value_object<double, NUMBER> number;
typedef value_object<std::string, STRING> string;

struct nil : public object {
    inline predefined_typeid get_typeid() const { return NIL; }
    inline size_t get_hashcode() const { return 0; }
    inline object* clone() const { return new nil(); }
};

template <class T, predefined_typeid id>
struct managed_pointer : public object {
    T* ptr;
    inline constexpr managed_pointer(T* p) : ptr(p) {}
    inline predefined_typeid get_typeid() const { return id; }
    inline size_t get_hashcode() const { return std::hash<T*>()(ptr); }
    inline bool equal_to(const object* b) const {
#ifdef USE_DYNAMIC_CAST
        auto obj = dynamic_cast<managed_pointer<T, id>*>(b);
        if (obj == nullptr) return false;
        return obj->ptr == ptr;
#else
        if (get_typeid() != b->get_typeid()) return false;
        auto obj = static_cast<const managed_pointer<T, id>*>(b);
        return obj->ptr == ptr;
#endif
    }
    inline T& operator->() const { return *ptr; }
    inline object* clone() const { return new managed_pointer<T, id>(ptr); }
};
}  // namespace types
}  // namespace mua