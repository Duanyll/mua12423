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
    FUNCTION,
    ITERATOR  // 由于没有实现函数的多返回值, 使用 ITERATOR 类型处理 table 的迭代
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
    inline const object* operator&() const { return this; }
};

// 堆上分配的对象, 简单进行基于引用计数的GC
class heap_object {
   public:
    int reference_count = 0;
    inline virtual ~heap_object() {}
};

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
    bool auto_delete;
    inline managed_pointer(T* p, bool should_auto_delete = true)
        : ptr(p), auto_delete(should_auto_delete) {
        p->reference_count++;
    }
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
    inline object* clone() const {
        return new managed_pointer<T, id>(ptr, auto_delete);
    }
    inline ~managed_pointer() {
        ptr->reference_count--;
        if (auto_delete && ptr->reference_count == 0) {
            delete ptr;
        }
    }
};
}  // namespace types
}  // namespace mua