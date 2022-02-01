#pragma once

#include <cstdlib>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

template<class... Ts> struct visitor : Ts... { using Ts::operator()...;  };
template<class... Ts> visitor(Ts...) -> visitor<Ts...>;

class CLI
{
    using ValuePtr = std::variant<bool*, std::int64_t*, std::string*, std::vector<std::string>*>;
    template<class T, class TL> struct Contains;
    template<typename T, template<class...> class Container, class... Ts>
    struct Contains<T, Container<Ts...>> : std::disjunction<std::is_same<T, Ts>...> {};
    struct Info
    {
        ValuePtr valPtr;
        std::function<void(std::string, ValuePtr)> converter;
        std::string help;
    };
public:
    CLI(int argc, char* argv[])
    {
        for(size_t i = 0; i < argc; ++i) {
            argList.emplace_back(argv[i]);
        }
    }
    template<typename T>
    void option(const std::vector<std::string>& names, T& destVal, std::string description = std::string())
    {
        static_assert(Contains<T*, ValuePtr>::value, "CLI: supported option types are only bool, std::int64_t, std::string or std::vector<std::string>");
        handler[names] = {&destVal,
                          [](const std::string& arg, ValuePtr valp) {
                              std::visit(visitor{[arg](bool* val) { *val = !*val; },
                                                 [arg](std::int64_t* val) { *val = std::strtoll(arg.c_str(), nullptr, 0); },
                                                 [arg](std::string* val) { *val = arg; },
                                                 [arg](std::vector<std::string>* val) { val->push_back(arg); }}, valp);
                          },
                          description};
    }
    bool parse()
    {
        auto iter = ++argList.begin();
        while(iter != argList.end()) {
            if(!handleOption(iter)) {
                throw std::runtime_error("Unexpected argument " + *iter);
            }
            ++iter;
        }
        return false;
    }
    void usage()
    {
        std::cout << "USAGE: " << argList[0] << " [options]\n" << std::endl;
        std::cout << "OPTIONS:\n" << std::endl;
        for(const auto& [names, info] : handler) {
            std::string delimiter = "";
            for(const auto& name : names) {
                std::cout << delimiter << name;
                if(info.valPtr.index()) {
                    std::cout << " <arg>";
                }
                delimiter = ", ";
            }
            std::cout << std::endl << "    " << info.help << "\n" << std::endl;
        }
    }
private:
    bool handleOption(std::vector<std::string>::iterator& iter)
    {
        for(const auto& [names, info] : handler) {
            for(const auto& name : names) {
                if(name == *iter) {
                    if(info.valPtr.index()) {
                        if(++iter == argList.end()) {
                            throw std::runtime_error("Missing argument to option " + name);
                        }
                    }
                    errno = 0;
                    info.converter(*iter, info.valPtr);
                    if(errno) {
                        throw std::runtime_error("Conversion error for option " + name);
                    }
                    return true;
                }
            }
        }
        return false;
    }
    std::vector<std::string> argList;
    std::map<std::vector<std::string>, Info> handler;
};
