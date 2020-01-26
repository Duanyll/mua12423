#include "statement.h"

#include <cassert>

#include "library_functions.h"
#include "operator.h"
using namespace mua::types;

void mua::ast::expr_statement::eval(rt_context* context) {
    auto res = exp->eval(context);
    delete res;
}

void mua::ast::assign_statement::eval(rt_context* context) {
    auto val = rexp->eval(context);
    lexp->set_value(context, val);
    delete val;
}

void mua::ast::block_statement::eval(rt_context* context) {
    for (auto& i : ch) {
        i->eval(context);
        if (context->exec_status != rt_context::normal) return;
    }
    switch (last_stat) {
        case none:
            break;
        case return_stat:
            context->return_value =
                return_value ? return_value->eval(context) : new nil();
            context->exec_status = rt_context::on_return;
            break;
        case break_stat:
            context->exec_status = rt_context::on_break;
            break;
    }
}

bool mua::ast::is_control_flow_true(pexpr x, rt_context* context) {
    auto val = x->eval(context);
    bool res = !(val->equal_to(&boolean(false)) || val->equal_to(&nil()));
    delete val;
    return res;
}

void mua::ast::if_statement::eval(rt_context* context) {
    assert(conditions.size() == stats.size());
    for (size_t i = 0; i < conditions.size(); i++) {
        if (is_control_flow_true(conditions[i], context)) {
            stats[i]->eval(context);
            return;
        }
    }
    else_stat->eval(context);
}

void mua::ast::while_statement::eval(rt_context* context) {
    while (is_control_flow_true(condition, context)) {
        ch->eval(context);
        if (context->exec_status == rt_context::on_break) {
            context->exec_status = rt_context::normal;
            break;
        } else if (context->exec_status == rt_context::on_return) {
            break;
        }
    }
}

void mua::ast::repeat_statement::eval(rt_context* context) {
    do {
        ch->eval(context);
        if (context->exec_status == rt_context::on_break) {
            context->exec_status = rt_context::normal;
            break;
        } else if (context->exec_status == rt_context::on_return) {
            break;
        }
    } while (is_control_flow_true(condition, context));
}

void mua::ast::for_statement::eval(rt_context* context) {
    auto obegin = begin->eval(context);
    auto ostep = step ? step->eval(context) : new number(1);
    auto oend = end->eval(context);

    auto vbegin = libiary_functions::tonumber(obegin);
    auto vstep = static_cast<number*>(libiary_functions::tonumber(ostep));
    auto vend = libiary_functions::tonumber(oend);

    delete obegin;
    delete ostep;
    delete oend;

    context->alloc_local_varible(loop_var);
    context->set_local_varible(loop_var, vbegin);

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

    while (is_control_flow_true(condition, context)) {
        ch->eval(context);
        if (context->exec_status == rt_context::on_break) {
            context->exec_status = rt_context::normal;
            break;
        } else if (context->exec_status == rt_context::on_return) {
            break;
        }
        step->eval(context);
    }
}

void mua::ast::generic_for_statement::eval(rt_context* context) {
    context->alloc_local_varible(loop_var);
    auto obj = range->eval(context);
    assert(obj->get_typeid() == ITERATOR);
    auto it = static_cast<iterator*>(obj);
    while (!it->is_end()) {
        auto val = it->get();
        context->set_local_varible(loop_var, val);
        delete val;

        ch->eval(context);
        if (context->exec_status == rt_context::on_break) {
            context->exec_status = rt_context::normal;
            break;
        } else if (context->exec_status == rt_context::on_return) {
            break;
        }
    }
    delete obj;
}

object* mua::ast::ast_function::invoke(
    rt_context* context, std::vector<const object*> params) const {
    context->push_frame();
    if (!captures.empty()) {
        assert(context == binded_context);
    }
    for (auto& i : captures) {
        context->add_caputured_varible(i.first, i.second);
    }
    for (int i = 0; i < param_name.size(); i++) {
        if (i < params.size()) {
            context->set_local_varible(param_name[i], params[i]);
        } else {
            context->set_local_varible(param_name[i], &nil());
        }
    }
    ch->eval(context);
    context->exec_status = rt_context::normal;
    context->pop_frame();
    if (context->return_value == nullptr) {
        return new nil();
    } else {
        auto val = context->return_value;
        context->return_value = nullptr;
        return val;
    }
}

mua::ast::ast_function::~ast_function() {
    for (auto& i : captures) {
        binded_context->remove_storage_reference(i.second);
    }
}

object* mua::ast::lambda_expression::eval(rt_context* context) {
    auto res = new ast_function();
    res->binded_context = context;
    for (auto& i : captures) {
        res->captures[i] = context->get_varible_sid(i);
    }
    res->param_name = param_name;
    res->ch = ch;
    return new function_pointer(res);
}

void mua::ast::varible_declaration::eval(rt_context* context) {
    context->alloc_local_varible(vid);
}
