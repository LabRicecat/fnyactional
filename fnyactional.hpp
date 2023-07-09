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

static inline type parse_type(const std::string& str) {
    if(str == "str") return type::STRING;
    if(str == "num") return type::NUMBER;
    if(str == "list") return type::LIST;
    if(str == "fn") return type::FUNCTION;
    else; // error
}

static inline std::string stringify(const type& my_type) {
    static const std::string s[] =
    { "str", "num", "list", "fn" };
    return s[static_cast<int>(my_type)];
}

struct value;

struct fn {
    enum {
        NATIVE,
        DEFINED
    } type = DEFINED;

    std::string name;
    std::vector<std::string> args;

    std::string code;
    std::function<value(const std::vector<value>&)> native;

    inline std::string stringify() const;
};

struct value {
    type my_type;

    std::string str;
    long double number = 0.0;
    std::vector<value> list;
    fn function;

    value(const std::string& str):          str(str),           my_type(type::STRING)   {}
    value(const long double& number):       number(number),     my_type(type::NUMBER)   {}
    value(const std::vector<value>& list):  list(list),         my_type(type::LIST)     {}
    value(const fn& function):              function(function), my_type(type::FUNCTION) {}
    value() {}

    bool operator==(const value& v) const {
        if(my_type != v.my_type) return 0;
        switch(my_type) {
            case type::NUMBER:
                return number == v.number;
            case type::STRING:
                return str == v.str;
            case type::LIST:
                if(list.size() != v.list.size()) return false;
                for(size_t i = 0; i < list.size(); ++i) {
                    if(!(v.list[i] == v.list[i])) return false;
                }
                return  true;
            case type::FUNCTION:
                return function.name == v.function.name;
            default:
                return false;
        }
        return false;
    }
    bool operator!=(const value& v) const { return !operator==(v); }

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
        r += i + ", ";
    if(r != name + "(") r.pop_back();
    r += ") => ";
    if(type == NATIVE) return r + "<native>";
    return r + code;
}

#define nfn(x,y,...) {x,fn{ .type = fn::NATIVE, .name = x, .args = {}, .native = [](const std::vector<value>& args)->value { __VA_ARGS__; }}}
#define fnargs(...) std::vector<argument>({__VA_ARGS__})
#define fnarg(x,y) argument({ .name = x, .my_type = type::y })
static std::unordered_map<std::string,fn> global_fns = {
    nfn("dbg",fnargs(
        fnarg("lhs",NUMBER),
    ), {
        std::cout << args[0].stringify() << "\n";
        return args[0];
    }),
    
    nfn("equal",fnargs(
        fnarg("lhs",NUMBER),
        fnarg("rhs",NUMBER),
    ), {
        return args[0] == args[1];
    }),
    nfn("sub",fnargs(
        fnarg("lhs",NUMBER),
        fnarg("rhs",NUMBER),
    ), {
        return args[0].number - args[1].number;
    }),
    nfn("push",fnargs(
        fnarg("lhs",LIST),
        fnarg("rhs",NUMBER),
    ), {
        auto cp = args[0].list;
        cp.push_back(args[1]);
        return cp;
    }),
    nfn("pop",fnargs(
        fnarg("lhs",LIST),
    ), {
        auto cp = args[0].list;
        cp.pop_back();
        return cp;
    }),
    nfn("last",fnargs(
        fnarg("lhs",LIST),
    ), {
        return args[0].list.back();
    }),
};
#undef nfn
#undef fnargs
#undef fnarg

inline std::vector<std::string> parse_params(std::string source) {
    if(source.size() > 1);
    if(source.front() != '(' || source.back() != ')');
    source.pop_back();
    source.erase(source.begin());

    static KittenLexer lexer = KittenLexer()
        .add_extract(',')
        .add_ignore(' ')
        .add_ignore('\n')
        .add_ignore('\t')
        .ignore_backslash_opts()
        .erase_empty();

    std::vector<std::string> r;
    auto lexed = lexer.lex(source);
    for(size_t i = 0; i < lexed.size(); i += 2) {
        r.push_back(lexed[i].src);
        if(i+1 > lexed.size() && lexed[i+1].src != ","); // error
    }
    return r;
}

static inline fn& get_fn(const std::string& str) {
    if(global_fns.count(str) == 0) std::cout << "No such function: " << str << "\n";
    return global_fns[str];
}

static inline bool is_num(const std::string& str) {
    if(str.empty()) return false;
    char* ptr;
    strtof(str.c_str(), &ptr);
    return (*ptr) == '\0';
}

static inline bool is_list(const std::string& str) {
    static KittenLexer lexer = KittenLexer()
        .add_capsule('[',']')
        .ignore_backslash_opts()
        .erase_empty();
    auto lexed = lexer.lex(str);
    return lexed.size() == 1 && lexed.front().src.front() == '[' && lexed.front().src.back() == ']';
}

inline std::vector<value> parse_args(std::string source, std::unordered_map<std::string,value> params);
static inline std::vector<value> parse_list(const std::string& str, std::unordered_map<std::string,value> vars) {
    return parse_args(str,vars);
}

static inline bool is_fn(const std::string& str) {
    return global_fns.find(str) != global_fns.end();
}

inline value get_value(const KittenToken& token, std::unordered_map<std::string,value> vars) {
    if(token.str)               return token.src;
    else if(is_num(token.src))  return std::stold(token.src);
    else if(is_list(token.src)) return parse_list(token.src,vars);
    else if(is_fn(token.src))   return get_fn(token.src);
    else {
        std::cout << token.src << "\n";
        // error??
    }
}

value eval_code(const std::string& code, std::unordered_map<std::string,value> vars);
inline value call(fn function, const std::vector<value>& args);
inline value call(std::string function, const std::vector<value>& args);
inline std::vector<value> parse_args(std::string source, std::unordered_map<std::string,value> params) {
    if(source.size() > 1);
    if(source.front() != '(' || source.back() != ')');
    source.pop_back();
    source.erase(source.begin());

    static KittenLexer lexer = KittenLexer()
        .add_extract(',')
        .add_ignore(' ')
        .add_ignore('\n')
        .add_ignore('\t')
        .add_capsule('(',')')
        .add_capsule('[',']')
        .add_capsule('{','}')
        .ignore_backslash_opts()
        .erase_empty();

    std::vector<value> r;
    auto lexed = lexer.lex(source);
    for(size_t i = 0; i < lexed.size(); i += 2) {
        if(!lexed[i].str && lexed[i].src.front() == '(') {
            fn fnc;
            fnc.args = parse_params(lexed[i].src);
            if(i+1 < lexed.size() && !lexed[i+1].str && lexed[i+1].src.front() == '{') {
                fnc.code = lexed[i+1].src.substr(1,lexed[i+1].src.size()-2);
                fnc.type = fn::DEFINED;
                fnc.name = "anon";
                r.push_back(fnc);
            }
            else; // error
            ++i;
        }
        else if(i+1 < lexed.size() && lexed[i+1].src.front() == '(') {
            if(params.count(lexed[i].src) != 0) 
                r.push_back(call(params[lexed[i].src].function, parse_args(lexed[i+1].src,params)));
            else r.push_back(call(lexed[i].src, parse_args(lexed[i+1].src,params)));
            ++i;
        }
        else r.push_back(eval_code(lexed[i].src,params));

        if(i+1 < lexed.size() && lexed[i+1].src != ","); // error
    }
    return r;
}

inline std::vector<fn> evaluate(const std::string& source) {
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
    
    std::vector<fn> fns;
    for(auto& i : lines) {
        if(i.fn == "" && i.body == "" && i.sw == false) continue;
        auto fndecl = fn_lexer.lex(i.fn);
        if((fndecl.size() > 2 && !fndecl.front().str && fndecl.front().src != "when") || fndecl.size() == 0); // error
        fns.push_back({fn::DEFINED,fndecl[0].src});
        if(fndecl.size() == 2) fns.back().args = parse_params(fndecl[1].src);

        fns.back().code = i.body;
        global_fns[fns.back().name] = fns.back();
    }

    return fns;
}

value eval_code(const std::string& code, std::unordered_map<std::string,value> vars) {
    static KittenLexer lexer = KittenLexer()
        .add_ignore(' ')
        .add_ignore('\t')
        .add_ignore('\n')
        .add_capsule('[',']')
        .add_capsule('(',')')
        .add_capsule('{','}')
        .ignore_backslash_opts()
        .erase_empty();
    auto lexed = lexer.lex(code);
    if(lexed.size() == 1) {
        if(!lexed.front().str && vars.count(lexed.front().src) != 0) {
            return vars[lexed.front().src];
        }
        return get_value(lexed.front(),vars);
    }
    else if(lexed.size() == 2) {
        if(!lexed[0].str && lexed[0].src.front() == '(') {
            fn fnc;
            fnc.args = parse_params(lexed[0].src);
            if(!lexed[1].str && lexed[1].src.front() == '{') {
                fnc.code = lexed[1].src.substr(1,lexed[1].src.size()-2);
                fnc.type = fn::DEFINED;
                fnc.name = "anon";
                return fnc;
            }
            else; // error
        }
        else if(vars.count(lexed.front().src) != 0)
            return call(vars[lexed.front().src].function,parse_args(lexed[1].src,vars));
        else return call(get_fn(lexed.front().src),parse_args(lexed[1].src,vars));
    }
    else if(lexed.size() != 0) {
        int i = 0;
        if(!lexed[i].str && lexed[i].src == "when") {
            ++i;
            value c;
            bool cond = false;
            if(!lexed[i].str && vars.count(lexed[i].src) != 0) 
                c = vars[lexed[i].src];
            else c = get_value(lexed[i],vars);
            if(i+1 < lexed.size() && lexed[i+1].src.front() == '(') {
                c = call(c.function,parse_args(lexed[i+1].src,vars));
                ++i;
            }
            cond = c.my_type == type::NUMBER && c.number != 0;
            ++i;
            if(i < lexed.size() && !lexed[i].str && lexed[i].src == "then") {
                if(i+1 >= lexed.size()); // error
                ++i;
                if(!lexed[i].str && vars.count(lexed[i].src) != 0) 
                    c = vars[lexed[i].src];
                else c = get_value(lexed[i],vars);
                if(lexed[i+1].src.front() == '(') {
                    if(cond)
                        c = call(c.function,parse_args(lexed[i+1].src,vars));
                    ++i;
                }
                if(cond) return c;
                ++i;
                if(i < lexed.size() && !lexed[i].str && lexed[i].src == "else") {
                    if(i+1 >= lexed.size()); // error
                    ++i;
                    if(!lexed[i].str && vars.count(lexed[i].src) != 0) 
                        c = vars[lexed[i].src];
                    else c = get_value(lexed[i],vars);
                    if(i+1 < lexed.size() && lexed[i+1].src.front() == '(') {
                        if(!cond)
                            c = call(c.function,parse_args(lexed[i+1].src,vars));
                        ++i;
                    }
                    if(!cond)
                        return c;
                }
            }
            else; // error
        }
        
    }
    std::cout << "err, code:" << code << "\n";
    return 0;
}

inline value call(fn function, const std::vector<value>& args = {}) {
    if(function.type == fn::NATIVE)
        return function.native(args);
    else {
        std::unordered_map<std::string,value> vars;
        for(size_t i = 0; i < function.args.size(); ++i)
            vars[function.args[i]] = args[i];

        return eval_code(function.code,vars);
    }
}

inline value call(std::string function, const std::vector<value>& args = {}) {
    return call(get_fn(function),args);
}

}

#endif