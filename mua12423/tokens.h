#pragma once

#include <string>

namespace mua {
namespace lexer {
enum class token_type {
    UNKNOWN,
    RESERVED,
    STRING,
    NUMBER,
    SYMBOL,
    NAME,
    EOL,
    COMMENT
};

struct token {
    inline token() {}
    inline virtual std::string get_token_name() const = 0;
    inline virtual token_type get_type() const = 0;
    inline virtual ~token() {}

    inline std::string get_orig() const { return orig_content; }
    inline virtual void set_orig(const std::string& str) { orig_content = str; }

    inline bool operator==(const std::string& a) const {
        return orig_content == a;
    }

   protected:
    std::string orig_content;
};

const std::string reserved_names[] = {
    "and", "break",    "do",     "else", "elseif", "end",   "false",
    "for", "function", "if",     "in",   "local",  "nil",   "not",
    "or",  "repeat",   "return", "then", "true",   "until", "while"};

const std::string symbols[] = {"+", "-", "*",  "/",  "%",  "^", "#",  "==", "=",
                               "<", ">", "<=", ">=", "~=", "(", ")",  "{",  "}",
                               "[", "]", ";",  ":",  ",",  ".", "..", "..."};

namespace tokens {
struct reserved : public token {
    inline std::string get_token_name() const { return "RESERVED"; }
    inline virtual token_type get_type() const { return token_type::RESERVED; }
    inline static bool is_reserved(std::string str) {
        for (auto& i : reserved_names) {
            if (i == str) return true;
        }
        return false;
    }
};

struct number : public token {
    inline std::string get_token_name() const { return "NUMBER"; }
    inline virtual token_type get_type() const { return token_type::NUMBER; }
    inline void set_orig(const std::string& x) {
        token::set_orig(x);
        value = std::atof(orig_content.c_str());
    }
    double value = 0;
};

struct string : public token {
    inline std::string get_token_name() const { return "STRING"; }
    inline virtual token_type get_type() const { return token_type::STRING; }
    std::string value;
    inline void set_orig(const std::string& x) {
        token::set_orig(x);
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

struct symbol : public token {
    inline virtual token_type get_type() const { return token_type::SYMBOL; }
    inline std::string get_token_name() const { return "SYMBOL"; }
};

struct name : public token {
    inline virtual token_type get_type() const { return token_type::NAME; }
    inline std::string get_token_name() const { return "NAME"; }
};

struct eol : public token {
    inline eol() : token() { set_orig("\n"); }
    inline virtual token_type get_type() const { return token_type::EOL; }
    inline std::string get_token_name() const { return "EOL"; }
};

struct comment : public token {
    inline virtual token_type get_type() const { return token_type::COMMENT; }
    inline std::string get_token_name() const { return "COMMENT"; }
};
}  // namespace tokens
}  // namespace lexer
}  // namespace mua