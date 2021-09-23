//
// Created by Mpho Mbotho on 2020-10-24.
//

#ifndef SCC_META_HPP
#define SCC_META_HPP

#include <scc/generator.hpp>

namespace scc {

    class MetaHeader final : public virtual HppGenerator {
    public:
        void generate(Formatter fmt, const Type& st) override;

    private:
        void generateStruct(Formatter fmt, const Struct& st);
        void generateEnum(Formatter fmt, const Enum& enm);
    };
}
#endif //SCC_META_HPP
