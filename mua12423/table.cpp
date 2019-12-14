#include "table.h"
#include <iostream>
#include <cassert>
using namespace mua::types;

object* mua::types::table::get_copy(const object* key) const {
    assert(key != nullptr);
    auto it = store.find(key);
    if (it == store.end()) return new nil();
    return it->second->clone();
}

const object* mua::types::table::get(const object* key) const {
    assert(key != nullptr);
    auto it = store.find(key);
    if (it == store.end()) return nullptr;
    return it->second;
}

void mua::types::table::set_copy(const object* key, const object* val) {
    set(key, val->clone());
}

void mua::types::table::set(const object* key, const object* val) {
    assert(key != nullptr);
    assert(val != nullptr);
    auto it = store.find(key);
    if (it != store.end()) {
        if (val->get_typeid() == NIL) {
            auto key_to_delete = it->first;
            auto val_to_delete = it->second;
            store.erase(it);
            delete key_to_delete;
            delete val_to_delete;

            if (key->get_typeid() == NUMBER && size_verified) {
                auto id = static_cast<const number*>(key)->value;
                if (id == round(id) && id > 0) {
                    size_t cur = (size_t)id;
                    if (cur == last_verified_size)
                        last_verified_size--;
                    else if (cur < last_verified_size)
                        size_verified = false;
                }
            }
        } else {
            delete it->second;
            it->second = val;
        }
    } else if (val->get_typeid() != NIL) {
        auto key_in_store = key->clone();
        store[key_in_store] = val;

        if (key->get_typeid() == NUMBER && size_verified) {
            auto id = static_cast<const number*>(key)->value;
            if (id == round(id) && id > 0) {
                size_t cur = (size_t)id;
                if (cur == last_verified_size + 1) {
                    last_verified_size++;
                    number* key_to_check = nullptr;
                    while (true) {
                        if (store.count(&number(cur + 1)) == 0) break;
                        cur++;
                        last_verified_size++;
                    }
                }
            }
        }
    }
}

size_t mua::types::table::size() {
    if (size_verified) return last_verified_size;
    size_t cur = 0;
    last_verified_size = 0;
    number* key_to_check = nullptr;
    while (true) {
        if (store.count(&number(cur + 1)) == 0) break;
        cur++;
        last_verified_size++;
    }
    size_verified = true;
    return last_verified_size;
}

void mua::types::test_table() {
    table tab;
    auto num1 = new number(1);
    auto num2 = new number(2);
    auto num3 = new number(3);
    auto str1 = new string("string1");
    auto str2 = new string("string2");
    auto valnil = new nil();

    tab.set_copy(num1, num2);
    tab.set_copy(num2, num3);
    tab.set_copy(str1, str2);
    tab.set_copy(num3, str1);
    assert(tab.size() == 3);
    assert(tab.get_copy(str2)->equal_to(valnil));

    tab.set_copy(num2, valnil);
    assert(tab.size() == 1);
    assert(tab.get_copy(num3)->equal_to(str1));
    tab.set_copy(num3, num1);
    assert(tab.get_copy(num3)->equal_to(num1));

	tab.set_copy(num2, str1);
    assert(tab.size() == 3);

	std::clog << "Table test passed." << std::endl;
}
