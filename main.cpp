#include "fnyactional.hpp"
#include "catpkgs/argparser/argparser.h"
#include <fstream>
#include <iostream>

std::string read(std::string file) {
    std::ifstream f(file);
    std::string r;
    while(f.good()) r += f.get();
    if(r != "") r.pop_back();
    return r;
}

void help_message() {
    std::cout 
    << "## fnyactional\n"
    << "A pure functional programming language\n\n"
    << "Usage: fnya <option> [args]\n\n"
    << "option := \n"
    << "   eval <string>   : evaluate STRING and print the result\n"
    << "   run <file>      : executes FILE and prints the result\n" 
    << "   shell           : opens the fnyactional shell\n"
    << "   help            : this\n"
    << "   version         : prints the version\n"
    << "\nBy: LabRicecat (https://github.com/labricecat/fnyactional)"
    << "\n";
}

int main(int argc, char** argv) {
    ArgParser parser = ArgParser()
        .addArg("eval",ARG_SET,{"-e"},0)
        .addArg("run",ARG_SET,{"-r"},0)
        .addArg("version",ARG_TAG,{"-v"},0)
        .addArg("help",ARG_TAG,{"-v"},0)
        .setbin()
    ;
    
    auto pargs = parser.parse(argv,argc);
    if(!pargs || pargs["help"]) {
        help_message();
        return 0;
    }
    else if(pargs["version"] && !pargs.has_bin()) {
        std::cout << FNYACTIONAL_VERSION << "\n";
        return 0;
    }
    else if(pargs("eval") != "" && !pargs.has_bin()) {
        std::cout << fnyactional::eval_code(pargs("eval"),{}).stringify() << "\n";
        return 0;
    }
    else if(pargs("run") != "") {
        auto fns = fnyactional::evaluate(read(pargs("run")));
        auto bin = pargs.get_bin();
        std::vector<fnyactional::value> args;
        for(auto& i : bin) {
            args.push_back(fnyactional::get_value(KittenToken{i},{}));
        }
        std::cout << fnyactional::call("main",args).stringify() << "\n";
        return 0;
    }
    else if(pargs("shell") != "" && !pargs.has_bin()) {
        std::cout << "Not implemented\n";
        return 0;
    }
    else {
        help_message();
        return 1;
    }  
}