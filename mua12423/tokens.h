#pragma once

#include <string>

namespace mua {

namespace lexer {

namespace tokens {

struct token {
    inline token() {}
    inline virtual std::string get_token_name() { return "UNKNOWN"; };
    inline virtual ~token() {}

    inline std::string get_orig_content() const { return orig_content; }
    inline virtual void set_orig_content(const std::string& str) {
        orig_content = str;
    }

   protected:
    std::string orig_content;
};

const std::string reserved_names[] = {
    "and", "break",    "do",     "else", "elseif", "end",   "false",
    "for", "function", "if",     "in",   "local",  "nil",   "not",
    "or",  "repeat",   "return", "then", "true",   "until", "while"};
const int reserved_name_count = 21;

struct reserved : public token {
    inline std::string get_token_name() { return "RESERVED"; }
    inline static bool is_reserved(std::string str) {
        for (int i = 0; i < reserved_name_count; i++) {
            if (str == reserved_names[i]) {
                return true;
            }
        }
        return false;
    }
};

struct number : public token {
    inline std::string get_token_name() { return "NUMBER"; }
    inline void set_orig_content(const std::string& x) {
        orig_content = x;
        value = std::atof(orig_content.c_str());
    }
    double value;
};

struct string : public token {
    inline std::string get_token_name() { return "STRING"; }
    std::string value;
    inline void set_orig_content(const std::string& x) {
        orig_content = x;
        value = "";
        for (size_t i = 1; i < orig_content.length() - 1; i++) {
            if (orig_content[i - 1] == '\\') {
                switch (orig_content[i]) {
                    case '\\':
                        value += '\\';
                        break;
                    case '\'':
                        value += '\'';
                        break;
                    case '\"':
                        value += '\"';
                        break;
                    case 'n':
                        value += '\n';
                        break;
                    default:
                        break;
                }
            } else {
                if (orig_content[i] != '\\') {
                    value += orig_content[i];
                }
            }
        }
    }
};

const std::string symbols[] = {"+", "-", "*",  "/",  "%",  "^", "#",  "==", "=",
                               "<", ">", "<=", ">=", "~=", "(", ")",  "{",  "}",
                               "[", "]", ";",  ":",  ",",  ".", "..", "..."};
const int symbol_count = 26;

struct symbol : public token {
    inline std::string get_token_name() { return "SYMBOL"; }
};

struct name : public token {
    inline std::string get_token_name() { return "NAME"; }
};

struct eol : public token {
    inline std::string get_token_name() { return "EOL"; }
};

struct comment : public token {
    inline std::string get_token_name() { return "COMMENT"; }
};

}  // namespace tokens

}  // namespace lexer

}  // namespace mua