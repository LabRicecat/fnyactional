#ifndef FNYACTIONAL_HPP
#define FNYACTIONAL_HPP

#include "catpkgs/kittenlexer/kittenlexer.hpp"

#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <unordered_map>

namespace fnyactional {

enum class type {
    STRING,
    NUMBER,
    LIST,
    FUNCTION
};

inline std::string stringify(const type& my_type) {
    static const std::string s[] =
    { "str", "num", "list", "fn" };
    return s[static_cast<int>(my_type)];
}

struct value;

struct argument {
    std::string name;
    type my_type;
};

struct fncall;
struct fn {
    enum {
        NATIVE,
        DEFINED
    } type;

    std::string name;
    std::vector<argument> args;

    fncall* code = nullptr;
    std::function<value(const std::vector<value>&)> native;

    inline std::string stringify() const;
};

struct fncall {
    fn function;
    std::vector<value> args;

    inline std::string stringify() const;
};

struct value {
    type my_type;

    std::string str;
    long double number = 0.0;
    std::vector<value> list;
    fn function;
    fncall call;

    value(const std::string& str):          str(str),           my_type(type::STRING)   {}
    value(const long double& number):       number(number),     my_type(type::NUMBER)   {}
    value(const std::vector<value>& list):  list(list),         my_type(type::LIST)     {}
    value(const fn& function):              function(function), my_type(type::FUNCTION) {}

    std::string stringify() const {
        switch(my_type) {
            case type::NUMBER:
                return std::to_string(number);
            case type::STRING:
                return "'" + str + "'";
            case type::LIST:
            {
                std::string r = "[";
                for(auto& i : list) 
                    r += i.stringify() + ",";
                if(r != "[")
                    r.pop_back();
                return r + "]";
            }
            case type::FUNCTION:
                return function.stringify();
            default:
                return "null";
        }
    }
};

inline std::string fn::stringify() const {
    std::string r = name + "(";
    for(auto& i : args) 
        r += i.name + ": " + ::fnyactional::stringify(i.my_type) + ", ";
    if(r != name + "(") r.pop_back();
    r += ") => ";
    if(type == NATIVE) return r + "<native>";
    if(!code) return r;
    return r + code->stringify();
}

inline std::string fncall::stringify() const {
    std::string str = function.stringify() + "(";
    for(auto& i : args) {
        str += i.stringify() + ",";
    }
    if(!args.empty()) 
        str.pop_back();
    return str + ")";
}

#define nfn(x,y,...) {x,fn{ .type = fn::NATIVE, .name = x, .args = {}, .native = [](const std::vector<value>& args)->value { __VA_ARGS__; }}}
#define fnargs(...) std::vector<argument>({__VA_ARGS__})
#define fnarg(x,y) argument({ .name = x, .my_type = type::y })
static const std::unordered_map<std::string,fn> global_fns = {
    nfn("add",fnargs(
        fnarg("lhs",NUMBER),
        fnarg("rhs",NUMBER),
    ), {
        return args[0].number + args[1].number;
    })
};
#undef nfn
#undef fnargs
#undef fnarg

std::vector<fn> evaluate(const std::string& source) {
    struct line {
        std::string fn;
        std::string body;
        bool sw = false;
        bool line = 0;
    };
    std::vector<line> lines = {{}};
    int line_counter = 0;
    for(size_t i = 0; i < source.size(); ++i) {
        if(source[i] == ';') {
            while(i < source.size() && source[i] != '\n') ++i;
            lines.back().line = ++line_counter;
            lines.push_back({});
            --i;
        }
        else if(source[i] == '\n') {
            lines.back().line = ++line_counter;
            lines.push_back({});
        }
        else if(source[i] == '=' && source[i+1] == '>') lines.back().sw = true, ++i;
        else if(lines.back().sw) lines.back().body += source[i];
        else lines.back().fn += source[i];
    }

    static KittenLexer fn_lexer = KittenLexer()
        .add_ignore(' ')
        .add_ignore(' ')
        .add_ignore('\n')
        .erase_empty()
        .add_capsule('(',')')
        .ignore_backslash_opts();
    static KittenLexer body_lexer = KittenLexer()
        .add_ignore(' ')
        .add_ignore(' ')
        .add_ignore('\n')
        .erase_empty()
        .add_capsule('(',')')
        .ignore_backslash_opts();
    
    auto fndecl = fn_lexer.lex("");
}

value run_fn(fn function, std::vector<value> args) {
    if(function.type == fn::NATIVE)
        return function.native(args);
}

}

#endif