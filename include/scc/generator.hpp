//
// Created by Mpho Mbotho on 2020-10-24.
//

#ifndef SCC_GENERATOR_HPP
#define SCC_GENERATOR_HPP

#include <scc/formatter.hpp>

#include <ostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>

extern "C" {

typedef void *Context;
typedef void *Handle;
typedef int  (*GeneratorInit)(Context ctx);
typedef void (*GeneratorDeinit)();
};


enum class Log : int {
    LV1,LV2,LV3
};

std::ostream& error();
std::ostream& error(std::ostream& os);
std::ostream& info();
std::ostream& info(std::ostream& os);
std::ostream& debug(Log lv = Log::LV1);
std::ostream& debug(std::ostream& os, Log lv = Log::LV1);

namespace scc {

    class Node;
    class Program;
    class Type;
    class Struct;
    class Enum;
    class KeyValuePairs;
    class IncludeBag;
    class Variables;
    class Literal;

    class GeneratorVariables {
    public:
        GeneratorVariables() = default;
        const KeyValuePairs& var(const std::string& name) const;
        const Literal& get(const std::string& var, const std::string& name);
        const std::string& getNamespace() const { return mNamespace; }
    private:
        friend class GeneratorLib;
        const Variables* mVariables{nullptr};
        std::string mNamespace{};
    };
    /**
     * A header generator is should be implemented to generate headers
     * for so suil code file
     */
    class HppGenerator : public GeneratorVariables {
    public:
        /**
         * Invoked by program generator to allow a specific generator to
         * write its includes
         * @param fmt
         * @param incs
         */
        virtual void includes(Formatter fmt, IncludeBag& incs) {}
        /**
         * Used to generate header code for a Type. This function will
         * be invoked by the generator with a \class Type which can either
         * be a struct or class and should generate respective C code
         * @param ct the type to generate
         *
         * @note When invoked, this is already within the code namespace and the generator
         * should specifically generate the type code
         */
        virtual void generate(Formatter fmt, const Type& ct) {}

        /**
         * This can be implemented to handle '#pragma invoke' statements. A generator
         * can use this generate specific code at specific parts of a generated file
         * @param method the method to invoke
         * @param fmt the output stream formatter
         * @param vars variables passed to invoke command
         */
        virtual void invoke(const std::string& method, Formatter fmt, const KeyValuePairs& vars) {}
    };

    /**
     * A source generator should be implemented to generate C++ implementation
     * code for so suil code file
     */
    class CppGenerator : public  GeneratorVariables {
    public:
        /**
         * Used to generate source code for a Type. This function will
         * be invoked by the generator with a \class Type which can either
         * be a struct or class and should generate respective C code
         * @param os the output stream object to generate the \class Type to
         * @param ct the type to generate
         *
         * @note When invoked, this is already within the code namespace and the generator
         * should specifically generate the type code
         */
        virtual void generate(Formatter fmt, const Type& ct) {};

        /**
         * This can be implemented to handle '#pragma invoke' statements. A generator
         * can use this generate specific code at specific parts of a generated file
         * @param method the method to invoke
         * @param fmt the output stream formatter
         * @param vars variables passed to invoke command
         */
        virtual void invoke(
                const std::string& method,
                Formatter fmt,
                const KeyValuePairs& vars)
        {}
    };

    void registerLibGenerator(
            Context ctx,
            const std::string& name,
            std::shared_ptr<HppGenerator> headerGenerator,
            std::shared_ptr<CppGenerator> sourceGenerator);
}
#endif //SCC_GENERATOR_HPP
