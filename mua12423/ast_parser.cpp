#include "ast_parser.h"

#include <cassert>
#include <list>
#include <stack>
using namespace mua::ast;

bool mua::ast_parser::is_operator(const token& x) {
    const std::string& val = x.get_orig_content();
    return opr_precedence.find(val) != opr_precedence.end();
}

bool mua::ast_parser::is_expr_start(const token& x) {
    const std::string& val = x.get_orig_content();
    return x.get_type() == token_type::NAME ||
           x.get_type() == token_type::STRING ||
           x.get_type() == token_type::NUMBER || x == "true" || x == "false" ||
           x == "nil" || x == "(" || x == "[" || x == "{";
}

bool mua::ast_parser::is_expr_end(const token& x) {
    return !(is_operator(x) || is_expr_start(x));
}

bool mua::ast_parser::is_unop(const std::string& x) {
    return x == "- (unop)" || x == "#" || x == "not";
}

pstat mua::ast_parser::parse_stat(size_t start_pos, size_t& end_pos) {
    size_t cur = start_pos;
    const token& t = *input[cur];
    if (t == "do") {
    } else if (t == "if") {
        // TODO: if statement
    } else if (t == "for") {
        // TODO: for statement
    } else if (t == "while") {
        // TODO: while statement
    } else if (t == "repeat") {
        // TODO: repeat-until statement
    } else if (t == "local") {
        // TODO: local varible declaration
    } else if (t == "function") {
        // TODO: function declaration
    } else {
        // ��ֵ�ͺ����������.
        pexpr lval = parse_expr(cur, cur);
        if (*input[cur] == "=") {
            // ��ֵ���
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

// �� input[start_pos] (����) ��ʼ������ȡ���ʽ, ���� pexpr, end_pos
// ָʾ��һ�������ڵ�ǰ���ʽ�� token ��������ǰ���ʽĩβ�Ƕ����, ��
// end_pos ָ��ö����. ����� EOL, ��ָ��� EOL.
pexpr mua::ast_parser::parse_expr(size_t start_pos, size_t& end_pos) {
    // ��һ��, �����ӱ��ʽ, �ϲ�Ϊ���ӱ��ʽ���������ɵ�����
    std::list<term> terms;
    size_t cur = start_pos;
    while (cur < input.size()) {
        const token& t = *input[cur];
        if (is_expr_end(t)) {
            break;
        } else if (t == "(") {
            // ���������Ǹı����ȼ����Ǻ�������
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
            if (t.get_orig_content() == "-" &&
                (terms.empty() || terms.back().is_opr)) {
                terms.push_back(term("- (unop)"));
            } else {
                terms.push_back(term(t.get_orig_content()));
            }
        } else if (t.get_type() == token_type::NAME) {
            if (terms.empty() || !terms.back().is_opr ||
                terms.back().val_opr != ".") {
                terms.push_back(get_var_reference(t.get_orig_content()));
            } else {
                terms.push_back(term(std::make_shared<simple_constant>(
                    new types::string(t.get_orig_content()))));
            }
        } else if (t.get_type() == token_type::STRING) {
            terms.push_back(
                term(std::make_shared<simple_constant>(new types::string(
                    static_cast<const tokens::string*>(input[cur])->value))));
        } else if (t.get_type() == token_type::NUMBER) {
            terms.push_back(
                term(std::make_shared<simple_constant>(new types::number(
                    static_cast<const tokens::number*>(input[cur])->value))));
        } else if (t.get_orig_content() == "true") {
            terms.push_back(
                term(std::make_shared<simple_constant>(new boolean(true))));
        } else if (t.get_orig_content() == "false") {
            terms.push_back(
                term(std::make_shared<simple_constant>(new boolean(false))));
        } else if (t.get_orig_content() == "nil") {
            terms.push_back(term(std::make_shared<simple_constant>(new nil())));
        } else {
            assert(false);
        }
        cur++;
    }
    end_pos = cur;

    // ��һ��, ��� "."
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

    // �ڶ���, �����������
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

    // ������, ����ҽ�ϵ� "^"
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

    // ������ջ��ʣ�µĲ������ŵ���׺���ʽת��Ϊ��׺���ʽ�õ� ast
    // һԪ������Ĵ�����: ֵ��ջʱ���ջ����û��һԪ�����
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
                    // ���ڴ�ʱһԪ����������ȼ�����ߵ�, �ʲ��ÿ��Ǵ�ʱҪ pop
                    // һԪ�����
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

// mua ��׼ֻҪ��ʵ�ֿ� table {}
std::shared_ptr<table_constant> mua::ast_parser::parse_table(
    size_t start_pos, size_t& end_pos) {
    end_pos = start_pos;
    return std::make_shared<table_constant>();
}

void mua::ast_parser::declare_local_var(const std::string& name) {
    frames.back()[name] = ++id_begin;
}

pexpr mua::ast_parser::get_var_reference(const std::string& name) {
    if (frames.empty()) {
        return std::make_shared<global_varible>(name);
    } else {
        for (auto it = frames.rbegin(); it != frames.rend(); it++) {
            auto res = it->find(name);
            if (res != it->end()) {
                return std::make_shared<local_varible>(res->second, name);
            }
        }
        return std::make_shared<global_varible>(name);
    }
}

void mua::ast_parser::push_frame() {
    frames.push_back(std::unordered_map<std::string, local_var_id>());
}

void mua::ast_parser::pop_frame() { frames.pop_back(); }

#include "library_functions.h"

void mua::test_expr(const std::string& str, const object* expected_result) {
    lexer::string_lexer lex;
    ast_parser p(lex(str));
    runtime_context context;
    size_t end_pos;
    auto res = p.parse_expr(0, end_pos);
    auto eval_res = res->eval(&context);
    assert(eval_res->equal_to(expected_result));
    delete eval_res;
}
