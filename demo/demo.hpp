//
// Created by Mpho Mbotho on 2020-11-03.
//

#ifndef SCC_DEMO_HPP
#define SCC_DEMO_HPP

#include <scc/generator.hpp>
#include <scc/program.hpp>

namespace scc {

    class DemoHppGenerator : public HppGenerator {
    public:
        void generate(Formatter fmt, const Type &ct) override;
        void invoke(const std::string &method, Formatter fmt, const KeyValuePairs &vars) override;
    };

    class DemoCppGenerator: public CppGenerator {
    public:
        void generate(Formatter fmt, const Type &ct) override;
    private:
        void generate(Formatter& fmt, const std::string& klass, const Field& field);
    };

}
#endif //SCC_DEMO_HPP
