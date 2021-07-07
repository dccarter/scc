//
// Created by Mpho Mbotho on 2020-10-24.
//

#include <scc/generator.hpp>
#include <scc/program_generator.hpp>
#include <scc/program.hpp>

#include <sstream>
#include <iostream>

namespace {
    class Nope : public std::ostream {
    public:
        template<typename T>
        Nope &operator<<(const T &) {
            return *this;
        }
    };
}

std::ostream &error() {
    return error(std::cerr);
}

std::ostream& error(std::ostream& os)
{
    return os << "error: ";
}

std::ostream &info() {
    return info(std::cout);
}

std::ostream &info(std::ostream& os) {
    return os;
}

extern Log getConfiguredLog();

std::ostream &debug(Log lv) {
    return debug(std::cout, lv);
}

std::ostream &debug(std::ostream& os, Log lv) {
    if (lv <= getConfiguredLog()) {
        return os << "-- ";
    }
    else {
        static Nope np;
        return np;
    }
}

namespace scc {

    const KeyValuePairs& GeneratorVariables::var(const std::string& name) const
    {
        if (mVariables == nullptr) {
            return Variables::INVALID;
        }
        return (*mVariables)[name];
    }

    const Literal& GeneratorVariables::get(const std::string& varName, const std::string& name)
    {
        return var(varName)[name];
    }

    void registerLibGenerator(Context ctx, const std::string& name,
                              std::shared_ptr<HppGenerator> headerGenerator,
                              std::shared_ptr<CppGenerator> sourceGenerator)
    {
        auto generatorLib = reinterpret_cast<GeneratorLib *>(ctx);
        if (generatorLib == nullptr) {
            std::stringstream  ss;
            ss << "registerLibGenerator('" << name << "'): given context is invalid";
            throw std::runtime_error(ss.str().c_str());
        }

        if ((headerGenerator != nullptr) or (sourceGenerator != nullptr)) {
            // one of the two must be
            generatorLib->addGenerator(name, sourceGenerator, headerGenerator);
        }
        else {
            std::stringstream ss;
            ss << "registerLibGenerator('" << name << "'): either a source or header generator required";
            throw std::runtime_error(ss.str().c_str());
        }
    }
}

