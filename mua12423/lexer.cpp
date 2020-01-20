#include "lexer.h"
#include <iostream>
#include <typeinfo>
using namespace mua::lexer;

inline token* mua::lexer::string_lexer::get_token(const std::string& buffer,
                                           possible_token_type buffer_type) {
    token* tmp = nullptr;
    switch (buffer_type) {
        case NAME:
            if (tokens::reserved::is_reserved(buffer)) {
                tmp = new tokens::reserved();
            } else {
                tmp = new tokens::name();
            }
            break;
        case DEC:
        case HEX:
        case SCI:
            tmp = new tokens::number();
            break;
        case SYMBOL:
            tmp = new tokens::symbol();
            break;
        case COMMENT:
            tmp = new tokens::comment();
            break;
        case STRING:
            tmp = new tokens::string();
            break;
    }
    if (tmp != nullptr) {
        tmp->set_orig_content(buffer);
    }
    return tmp;
}

token_array mua::lexer::string_lexer::operator()(const std::string& input) {
    token_array ret;
    int len = input.length();
    std::string buffer = "";
    possible_token_type buffer_type = UNKNOWN;
    int pos = -1;
    while (++pos < len) {
        //尝试拓展当前token
        bool expanded = false;
        switch (buffer_type) {
            case NAME:
                if (isalnum(input[pos]) || input[pos] == '_') {
                    buffer += input[pos];
                    expanded = true;
                }
                break;
            case DEC:
                if (isdigit(input[pos])) {
                    buffer += input[pos];
                    expanded = true;
                    break;
                }
                if (buffer == "0" && (input[pos] == 'x' || input[pos] == 'X')) {
                    buffer += input[pos];
                    buffer_type = HEX;
                    expanded = true;
                    break;
                }
                if (input[pos] == '.' || input[pos] == 'e') {
                    buffer += input[pos];
                    buffer_type = SCI;
                    expanded = true;
                    break;
                }
                break;
            case HEX:
                if (isdigit(input[pos]) ||
                    ('a' <= input[pos] && input[pos] <= 'f') ||
                    ('A' <= input[pos] && input[pos] <= 'F')) {
                    buffer += input[pos];
                    expanded = true;
                }
                break;
            case SCI:
                if (isdigit(input[pos]) ||
                    (buffer[buffer.length() - 1] == 'e' &&
                     (input[pos] == '+' || input[pos] == '-')) ||
                    input[pos] == 'e') {
                    buffer += input[pos];
                    expanded = true;
                }
                break;
            case SYMBOL:
                for (auto& i : symbols) {
                    if (i.length() > buffer.length() &&
                        buffer ==
                            i.substr(0, buffer.length()) &&
                        input[pos] == i[buffer.length()]) {
                        buffer += input[pos];
                        expanded = true;
                        break;
                    }
                }
                if (buffer == "-" && input[pos] == '-') {
                    buffer = "--";
                    buffer_type = COMMENT;
                    expanded = true;
                }
                if (buffer == "." &&
                    (isdigit(input[pos]) || input[pos] == 'e')) {
                    buffer += input[pos];
                    buffer_type = SCI;
                    expanded = true;
                }
                break;
            case COMMENT:
                if (input[pos] != '\n') {
                    buffer += input[pos];
                    expanded = true;
                }
                break;
            case STRING:
                if (input[pos] != '\n') {
                    if (buffer.length() == 1 ||
                        buffer[buffer.length() - 1] != buffer[0] ||
                        buffer[buffer.length() - 2] == '\\') {
                        buffer += input[pos];
                        expanded = true;
                    }
                }
                break;
        }
        if (!expanded) {
            //拓展失败，结束当前token
            auto tmp = get_token(buffer, buffer_type);
            if (tmp != nullptr) {
                ret.push_back(tmp);
            }

            buffer = "";
            buffer_type = UNKNOWN;

            //尝试创建新的token
            if (isalpha(input[pos])) {
                buffer = input[pos];
                buffer_type = NAME;
            } else if (isdigit(input[pos])) {
                buffer = input[pos];
                buffer_type = DEC;
            } else if (input[pos] == '\"' || input[pos] == '\'') {
                buffer = input[pos];
                buffer_type = STRING;
            } else if (is_symbol_first_char(input[pos])) {
                buffer = input[pos];
                buffer_type = SYMBOL;
            } else if (input[pos] == '\n') {
                ret.push_back(new tokens::eol());
            }
        }
    }
    if (buffer != "") {
        auto tmp = get_token(buffer, buffer_type);
        if (tmp != nullptr) {
            ret.push_back(tmp);
        }
    }
    return ret;
}

void mua::lexer::test_lexer() {
    std::string str;
    std::getline(std::cin, str, '\0');
    token_array arr = string_lexer()(str);
    for (auto i : arr) {
        if (typeid(*i) == typeid(tokens::comment)) {
            continue;
        }
        if (typeid(*i) == typeid(tokens::eol)) {
            std::cout << "[" << i->get_token_name() << "]" << std::endl;
        } else {
            std::cout << "[" << i->get_token_name() << "] "
                      << i->get_orig_content() << std::endl;
        }
        delete i;
    }
}
