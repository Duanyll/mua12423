#include "ast_parser.h"

#include <cassert>
#include <list>
#include <stack>
using namespace mua::ast;
using namespace mua;

bool mua::ast_parser::is_operator(const token& x) {
    const std::string& val = x.get_orig();
    return opr_precedence.find(val) != opr_precedence.end();
}

bool mua::ast_parser::is_expr_start(const token& x) {
    const std::string& val = x.get_orig();
    return x.get_type() == token_type::NAME ||
           x.get_type() == token_type::STRING ||
           x.get_type() == token_type::NUMBER || x == "true" || x == "false" ||
           x == "nil" || x == "(" || x == "[" || x == "{" || x == "function";
}

bool mua::ast_parser::is_expr_end(const token& x) {
    return !(is_operator(x) || is_expr_start(x));
}

bool mua::ast_parser::is_unop(const std::string& x) {
    return x == "- (unop)" || x == "#" || x == "not";
}

#define BEGIN_BUILDER(type) \
    size_t cur = start_pos; \
    auto res = std::make_shared<type>()
#define ASSERT_TOKEN(name)              \
    while (cur < input.size()) {        \
        if (*input[cur] == name) break; \
        assert(*input[cur] == "\n");    \
        cur++;                          \
    }                                   \
    cur++;
#define PUSH_SCOPE context.back().scopes.push_back(scope())
#define POP_SCOPE context.back().scopes.pop_back()
#define END_BUILDER \
    end_pos = cur;  \
    return res;

bool mua::ast_parser::find_next(const std::string& name, size_t& cur) {
    size_t tmp = cur;
    while (tmp < input.size()) {
        if (*input[tmp] == name) {
            cur = tmp + 1;
            return true;
        } else if (!(*input[tmp] == "\n")) {
            return false;
        }
        tmp++;
    }
    return false;
}

pstat mua::ast_parser::parse_inner_block(size_t start_pos, size_t& end_pos) {
    BEGIN_BUILDER(block_statement);
    while (cur < input.size()) {
        auto& t = *input[cur];
        if (t == "\n") {
            cur++;
            continue;
        }
        if (t == "end" || t == "until" || t == "elseif" || t == "else") break;
        if (t == "return") {
            res->last_stat = block_statement::return_stat;
            if (!is_expr_end(*input[cur + 1])) {
                res->return_value = parse_expr(cur + 1, cur);
            }
            break;
        } else if (t == "break") {
            res->last_stat = block_statement::break_stat;
            break;
        }
        res->ch.push_back(parse_stat(cur, cur));
    }
    END_BUILDER;
}

pstat mua::ast_parser::parse_block(size_t start_pos, size_t& end_pos) {
    size_t cur = start_pos;
    ASSERT_TOKEN("do");
    PUSH_SCOPE;
    auto res = parse_inner_block(cur, cur);
    POP_SCOPE;
    ASSERT_TOKEN("end");
    END_BUILDER;
}

pstat mua::ast_parser::parse_if(size_t start_pos, size_t& end_pos) {
    BEGIN_BUILDER(if_statement);
    ASSERT_TOKEN("if");
    res->conditions.push_back(parse_expr(cur, cur));
    ASSERT_TOKEN("then");
    PUSH_SCOPE;
    res->stats.push_back(parse_inner_block(cur, cur));
    POP_SCOPE;
    while (find_next("elseif", cur)) {
        res->conditions.push_back(parse_expr(cur, cur));
        ASSERT_TOKEN("then");
        PUSH_SCOPE;
        res->stats.push_back(parse_inner_block(cur, cur));
        POP_SCOPE;
    }
    if (find_next("else", cur)) {
        PUSH_SCOPE;
        res->else_stat = parse_inner_block(cur, cur);
        POP_SCOPE;
    }
    ASSERT_TOKEN("end");
    END_BUILDER;
}

pstat mua::ast_parser::parse_for(size_t start_pos, size_t& end_pos) {
    size_t cur = start_pos;
    ASSERT_TOKEN("for");
    PUSH_SCOPE;
    auto loop_var = declare_local_var(input[cur++]->get_orig());
    if (find_next("=", cur)) {
        auto res = std::make_shared<for_statement>();
        res->loop_var = loop_var;
        res->begin = parse_expr(cur, cur);
        ASSERT_TOKEN(",");
        res->end = parse_expr(cur, cur);
        if (find_next(",", cur)) {
            res->step = parse_expr(cur, cur);
        }
        ASSERT_TOKEN("do");
        res->ch = parse_inner_block(cur, cur);
        ASSERT_TOKEN("end");
        POP_SCOPE;
        END_BUILDER;
    } else {
        ASSERT_TOKEN("in");
        auto res = std::make_shared<generic_for_statement>();
        res->loop_var = loop_var;
        res->range = parse_expr(cur, cur);
        ASSERT_TOKEN("do");
        res->ch = parse_inner_block(cur, cur);
        ASSERT_TOKEN("end");
        POP_SCOPE;
        END_BUILDER;
    }
}

pstat mua::ast_parser::parse_while(size_t start_pos, size_t& end_pos) {
    BEGIN_BUILDER(while_statement);
    ASSERT_TOKEN("while");
    res->condition = parse_expr(cur, cur);
    PUSH_SCOPE;
    ASSERT_TOKEN("do");
    res->ch = parse_inner_block(cur, cur);
    ASSERT_TOKEN("end");
    POP_SCOPE;
    END_BUILDER;
}

pstat mua::ast_parser::parse_repeat(size_t start_pos, size_t& end_pos) {
    BEGIN_BUILDER(repeat_statement);
    ASSERT_TOKEN("repeat");
    PUSH_SCOPE;
    res->ch = parse_inner_block(cur, cur);
    ASSERT_TOKEN("until");
    res->condition = parse_expr(cur, cur);
    POP_SCOPE;
    END_BUILDER;
}

pstat mua::ast_parser::parse_local(size_t start_pos, size_t& end_pos) {
    BEGIN_BUILDER(varible_declaration);
    ASSERT_TOKEN("local");
    if (find_next("function", cur)) {
        res->is_local_function = true;
        res->vid = declare_local_var(input[cur + 1]->get_orig());
        res->val = parse_function(cur, cur).first;
    } else {
        auto name = input[cur++]->get_orig();
        if (find_next("=", cur)) {
            res->val = parse_expr(cur, cur);
        }
        res->vid = declare_local_var(name);
    }
    END_BUILDER;
}

std::pair<pexpr, plexpr> mua::ast_parser::parse_function(size_t start_pos,
                                                         size_t& end_pos) {
    BEGIN_BUILDER(lambda_expression);
    ASSERT_TOKEN("function");
    plexpr name;
    if (!find_next("(", cur)) {
        name = parse_lexpr(cur, cur);
    } else {
        cur--;
    }
    ASSERT_TOKEN("(");
    context.push_back(frame());
    PUSH_SCOPE;
    while (cur < input.size()) {
        if (*input[cur] == ")") {
            break;
        } else if (*input[cur] == ",") {
            cur++;
            continue;
        }
        res->param_name.push_back(declare_local_var(input[cur++]->get_orig()));
    }
    ASSERT_TOKEN(")");
    PUSH_SCOPE;
    res->ch = parse_inner_block(cur, cur);
    ASSERT_TOKEN("end");
    POP_SCOPE;
    POP_SCOPE;
    res->captures = context.back().captures;
    context.pop_back();
    end_pos = cur;
    return std::make_pair(res, name);
}

pstat mua::ast_parser::parse_stat(size_t start_pos, size_t& end_pos) {
    const token& t = *input[start_pos];
    if (t == "do") {
        return parse_block(start_pos, end_pos);
    } else if (t == "if") {
        return parse_if(start_pos, end_pos);
    } else if (t == "for") {
        return parse_for(start_pos, end_pos);
    } else if (t == "while") {
        return parse_while(start_pos, end_pos);
    } else if (t == "repeat") {
        return parse_repeat(start_pos, end_pos);
    } else if (t == "local") {
        return parse_local(start_pos, end_pos);
    } else if (t == "function") {
        auto res = parse_function(start_pos, end_pos);
        return std::make_shared<assign_statement>(res.second, res.first);
    } else {
        size_t cur = start_pos;
        // 赋值和函数调用语句.
        pexpr lval = parse_expr(cur, cur);
        if (*input[cur] == "=") {
            // 赋值语句
            pexpr rval = parse_expr(cur + 1, cur);
            end_pos = cur;
            auto lexp = std::dynamic_pointer_cast<lexpr>(lval);
            assert(lexp);
            return std::make_shared<assign_statement>(lexp, rval);
        } else {
            end_pos = cur;
            return std::make_shared<expr_statement>(lval);
        }
    }
}

// 从 input[start_pos] (包括) 开始尝试提取表达式, 生成 pexpr, end_pos
// 指示第一个不属于当前表达式的 token 比如若当前表达式末尾是定界符, 则
// end_pos 指向该定界符. 如果是 EOL, 则指向该 EOL.
pexpr mua::ast_parser::parse_expr(size_t start_pos, size_t& end_pos) {
    // 第一步, 处理子表达式, 合并为由子表达式或操作符组成的序列
    std::list<term> terms;
    size_t cur = start_pos;
    while (cur < input.size()) {
        const token& t = *input[cur];
        if (is_expr_end(t)) {
            break;
        } else if (t == "(") {
            // 分类讨论是改变优先级还是函数调用
            if (terms.empty() || terms.back().is_opr) {
                terms.push_back(term(parse_expr(cur + 1, cur)));
            } else {
                terms.push_back(term(std::make_shared<functional_call>(
                    nullptr, parse_param_list(cur + 1, cur))));
            }
        } else if (t == "[") {
            terms.push_back(term("."));
            terms.push_back(term(parse_expr(cur + 1, cur)));
        } else if (t == "{") {
            terms.push_back(term(parse_table(cur + 1, cur)));
        } else if (is_operator(t)) {
            if (t.get_orig() == "-" && (terms.empty() || terms.back().is_opr)) {
                terms.push_back(term("- (unop)"));
            } else {
                terms.push_back(term(t.get_orig()));
            }
        } else if (t.get_type() == token_type::NAME) {
            if (terms.empty() || !terms.back().is_opr ||
                terms.back().val_opr != ".") {
                terms.push_back(use_var(t.get_orig()));
            } else {
                terms.push_back(term(std::make_shared<simple_constant>(
                    new types::string(t.get_orig()))));
            }
        } else if (t.get_type() == token_type::STRING) {
            terms.push_back(
                term(std::make_shared<simple_constant>(new types::string(
                    static_cast<const tokens::string*>(input[cur])->value))));
        } else if (t.get_type() == token_type::NUMBER) {
            terms.push_back(
                term(std::make_shared<simple_constant>(new types::number(
                    static_cast<const tokens::number*>(input[cur])->value))));
        } else if (t.get_orig() == "true") {
            terms.push_back(
                term(std::make_shared<simple_constant>(new boolean(true))));
        } else if (t.get_orig() == "false") {
            terms.push_back(
                term(std::make_shared<simple_constant>(new boolean(false))));
        } else if (t.get_orig() == "nil") {
            terms.push_back(term(std::make_shared<simple_constant>(new nil())));
        } else if (t.get_orig() == "function") {
            terms.push_back(term(parse_function(cur, cur).first));
        } else {
            assert(false);
        }
        cur++;
    }
    end_pos = cur;

    // 第一趟, 解决 "."
    for (auto it = terms.begin(); it != terms.end(); it++) {
        if (it->is_opr && it->val_opr == ".") {
            auto before = --it;
            it++;
            auto after = ++it;
            it--;
            assert(!before->is_opr);
            assert(!after->is_opr);
            *it = term(std::make_shared<member_access>(before->val_expr,
                                                       after->val_expr));
            terms.erase(before);
            terms.erase(after);
        }
    }

    // 第二趟, 解决函数调用
    for (auto it = terms.begin(); it != terms.end(); it++) {
        if (!it->is_opr) {
            auto func =
                std::dynamic_pointer_cast<functional_call>(it->val_expr);
            if (func) {
                auto before = --it;
                it++;
                assert(!before->is_opr);
                func->func = before->val_expr;
                terms.erase(before);
            }
        }
    }

    // 第三趟, 解决右结合的 "^"
    auto it = terms.end();
    do {
        it--;
        if (it->is_opr && it->val_opr == "^") {
            auto before = --it;
            it++;
            auto after = ++it;
            it--;
            assert(!before->is_opr);
            assert(!after->is_opr);
            auto t = std::make_shared<opr_pow>();
            t->larg = before->val_expr;
            t->rarg = after->val_expr;
            *it = term(t);
            terms.erase(before);
            terms.erase(after);
        }
    } while (it != terms.begin());

    // 用两个栈将剩下的不含括号的中缀表达式转化为后缀表达式得到 ast
    // 一元运算符的处理方法: 值入栈时检查栈顶有没有一元运算符
    std::stack<std::string> st_opr;
    std::stack<pexpr> st_expr;
    for (auto it = terms.begin(); it != terms.end(); it++) {
        if (it->is_opr) {
            if (is_unop(it->val_opr)) {
                st_opr.push(it->val_opr);
            } else {
                int pre = opr_precedence[it->val_opr];
                while (!st_opr.empty() && pre >= opr_precedence[st_opr.top()]) {
                    auto opr = st_opr.top();
                    assert(!is_unop(opr));
                    st_opr.pop();
                    auto right = st_expr.top();
                    st_expr.pop();
                    auto left = st_expr.top();
                    st_expr.pop();
                    auto node = make_binop(opr);
                    node->larg = left;
                    node->rarg = right;
                    st_expr.push(node);
                    // 由于此时一元运算符的优先级是最高的, 故不用考虑此时要 pop
                    // 一元运算符
                }
                st_opr.push(it->val_opr);
            }
        } else {
            pexpr val = it->val_expr;
            while (!st_opr.empty() && is_unop(st_opr.top())) {
                auto opr = st_opr.top();
                st_opr.pop();
                auto node = make_unop(opr);
                node->arg = val;
                val = node;
            }
            st_expr.push(val);
        }
    }
    while (!st_opr.empty()) {
        auto opr = st_opr.top();
        assert(!is_unop(opr));
        st_opr.pop();
        auto right = st_expr.top();
        st_expr.pop();
        auto left = st_expr.top();
        st_expr.pop();
        auto node = make_binop(opr);
        node->larg = left;
        node->rarg = right;
        st_expr.push(node);
    }
    assert(st_expr.size() == 1);
    return st_expr.top();
}

plexpr mua::ast_parser::parse_lexpr(size_t start_pos, size_t& end_pos) {
    size_t cur = start_pos;
    auto res = std::dynamic_pointer_cast<lexpr>(use_var(input[cur++]->get_orig()));
    if (cur == input.size()) {
        END_BUILDER;
    }
    do {
        if (*input[cur] == ".") {
            cur++;
            res = std::make_shared<member_access>(
                res, std::make_shared<simple_constant>(
                         new string(input[cur++]->get_orig())));
        } else if (*input[cur] == "[") {
            res =
                std::make_shared<member_access>(res, parse_expr(cur + 1, cur));
            cur++;
        } else {
            break;
        }
    } while (cur < input.size());
    END_BUILDER;
}

std::vector<pexpr> mua::ast_parser::parse_param_list(size_t start_pos,
                                                     size_t& end_pos) {
    std::vector<pexpr> res;
    size_t cur = start_pos;
    if (*input[cur] == ")") {
        end_pos = cur;
        return res;
    }
    while (cur < input.size()) {
        res.push_back(parse_expr(cur, cur));
        if (*input[cur] == ")") break;
        assert(*input[cur] == ",");
        cur++;
    }
    end_pos = cur;
    return res;
}

// mua 标准只要求实现空 table {}
std::shared_ptr<table_constant> mua::ast_parser::parse_table(size_t start_pos,
                                                             size_t& end_pos) {
    end_pos = start_pos;
    return std::make_shared<table_constant>();
}

local_var_id mua::ast_parser::declare_local_var(const std::string& name) {
    return context.back().scopes.back()[name] = id_begin++;
}

pexpr mua::ast_parser::use_var(const std::string& name) {
    auto fr = context.end();
    do {
        fr--;
        auto sc = fr->scopes.end();
        bool found = false;
        local_var_id result;
        do {
            sc--;
            auto resit = sc->find(name);
            if (resit != sc->end()) {
                found = true;
                result = resit->second;
                break;
            }
        } while (sc != fr->scopes.begin());
        if (found) {
            while (++fr != context.end()) {
                fr->captures.insert(result);
            }
            return std::make_shared<local_varible>(result, name);
        }
    } while (fr != context.begin());
    return std::make_shared<global_varible>(name);
}

#ifdef _DEBUG
void mua::test_expr(const std::string& str, const object* expected_result) {
    lexer::string_lexer lex;
    ast_parser p(lex(str));
    runtime rt;
    size_t end_pos;
    auto res = p.parse_expr(0, end_pos);
    auto eval_res = res->eval(&rt);
    assert(eval_res->equal_to(expected_result));
    delete eval_res;
}
#endif  // _DEBUG