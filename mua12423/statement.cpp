#include "statement.h"

#include <cassert>

#include "library_functions.h"
#include "operator.h"
using namespace mua::types;

void mua::ast::expr_statement::eval(runtime* rt) {
    auto res = exp->eval(rt);
    delete res;
}

void mua::ast::assign_statement::eval(runtime* rt) {
    auto val = rexp->eval(rt);
    lexp->set_value(rt, val);
    delete val;
}

void mua::ast::block_statement::eval(runtime* rt) {
    for (auto& i : ch) {
        i->eval(rt);
        if (rt->exec_status != runtime::normal) return;
    }
    switch (last_stat) {
        case none:
            break;
        case return_stat:
            rt->return_value =
                return_value ? return_value->eval(rt) : new nil();
            rt->exec_status = runtime::on_return;
            break;
        case break_stat:
            rt->exec_status = runtime::on_break;
            break;
    }
}

bool mua::ast::is_control_flow_true(pexpr x, runtime* rt) {
    auto val = x->eval(rt);
    bool res = !(val->equal_to(&boolean(false)) || val->equal_to(&nil()));
    delete val;
    return res;
}

void mua::ast::if_statement::eval(runtime* rt) {
    assert(conditions.size() == stats.size());
    for (size_t i = 0; i < conditions.size(); i++) {
        if (is_control_flow_true(conditions[i], rt)) {
            stats[i]->eval(rt);
            return;
        }
    }
    if (else_stat) else_stat->eval(rt);
}

void mua::ast::while_statement::eval(runtime* rt) {
    while (is_control_flow_true(condition, rt)) {
        ch->eval(rt);
        if (rt->exec_status == runtime::on_break) {
            rt->exec_status = runtime::normal;
            break;
        } else if (rt->exec_status == runtime::on_return) {
            break;
        }
    }
}

void mua::ast::repeat_statement::eval(runtime* rt) {
    do {
        ch->eval(rt);
        if (rt->exec_status == runtime::on_break) {
            rt->exec_status = runtime::normal;
            break;
        } else if (rt->exec_status == runtime::on_return) {
            break;
        }
    } while (!is_control_flow_true(condition, rt));
}

void mua::ast::for_statement::eval(runtime* rt) {
    auto obegin = begin->eval(rt);
    auto ostep = step ? step->eval(rt) : new number(1);
    auto oend = end->eval(rt);

    auto vbegin = libiary_functions::tonumber(obegin);
    auto vstep = static_cast<number*>(libiary_functions::tonumber(ostep));
    auto vend = libiary_functions::tonumber(oend);

    delete obegin;
    delete ostep;
    delete oend;

    rt->alloc_local_varible(loop_var);
    rt->set_local_varible(loop_var, vbegin);

    std::shared_ptr<binop> condition;
    if (vstep->value > 0) {
        condition = std::make_shared<opr_leq>();
    } else {
        condition = std::make_shared<opr_geq>();
    }
    condition->larg = std::make_shared<local_varible>(loop_var);
    condition->rarg = std::make_shared<simple_constant>(vend->clone());

    auto plus_opr = std::make_shared<opr_plus>();
    plus_opr->larg = std::make_shared<local_varible>(loop_var);
    plus_opr->rarg = std::make_shared<simple_constant>(vstep->clone());
    pstat step = std::make_shared<assign_statement>(
        std::make_shared<local_varible>(loop_var), plus_opr);

    delete vbegin;
    delete vstep;
    delete vend;

    while (is_control_flow_true(condition, rt)) {
        ch->eval(rt);
        if (rt->exec_status == runtime::on_break) {
            rt->exec_status = runtime::normal;
            break;
        } else if (rt->exec_status == runtime::on_return) {
            break;
        }
        step->eval(rt);
    }
}

void mua::ast::generic_for_statement::eval(runtime* rt) {
    rt->alloc_local_varible(loop_var);
    auto obj = range->eval(rt);
    assert(obj->get_typeid() == ITERATOR);
    auto it = static_cast<iterator*>(obj);
    while (!it->is_end()) {
        auto val = it->get();
        rt->set_local_varible(loop_var, val);
        delete val;

        ch->eval(rt);
        if (rt->exec_status == runtime::on_break) {
            rt->exec_status = runtime::normal;
            break;
        } else if (rt->exec_status == runtime::on_return) {
            break;
        }
    }
    delete obj;
}

object* mua::ast::ast_function::invoke(
    runtime* rt, std::vector<const object*> params) const {
    rt->push_frame();
    if (!captures.empty()) {
        assert(rt == binded_context);
    }
    for (auto& i : captures) {
        rt->add_caputured_varible(i.first, i.second);
    }
    for (int i = 0; i < param_name.size(); i++) {
        rt->alloc_local_varible(param_name[i]);
        if (i < params.size()) {
            rt->set_local_varible(param_name[i], params[i]);
        } else {
            rt->set_local_varible(param_name[i], &nil());
        }
    }
    ch->eval(rt);
    rt->exec_status = runtime::normal;
    rt->pop_frame();
    if (rt->return_value == nullptr) {
        return new nil();
    } else {
        auto val = rt->return_value;
        rt->return_value = nullptr;
        return val;
    }
}

mua::ast::ast_function::~ast_function() {
    for (auto& i : captures) {
        binded_context->remove_storage_reference(i.second);
    }
}

object* mua::ast::lambda_expression::eval(runtime* rt) {
    auto res = new ast_function();
    res->binded_context = rt;
    for (auto& i : captures) {
        res->captures[i] = rt->get_varible_sid(i);
        rt->create_storage_reference(res->captures[i]);
    }
    res->param_name = param_name;
    res->ch = ch;
    return new function_pointer(res);
}

void mua::ast::varible_declaration::eval(runtime* rt) {
    if (is_local_function) {
        rt->alloc_local_varible(vid);
        auto v = val->eval(rt);
        rt->set_local_varible(vid, v);
        delete v;
    } else if (val) {
        auto v = val->eval(rt);
        rt->alloc_local_varible(vid);
        rt->set_local_varible(vid, v);
        delete v;
    } else {
        rt->alloc_local_varible(vid);
    }
}