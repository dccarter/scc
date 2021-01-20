//
// Created by Mpho Mbotho on 2020-10-30.
//

#ifndef SCC_ASTWRAPPER_HPP
#define SCC_ASTWRAPPER_HPP

#include "peglib.h"

namespace scc {

    class AstWrapper {
    public:
        AstWrapper(const std::shared_ptr<peg::Ast>& asw)
                : ast(*asw)
        {}
        AstWrapper(const peg::Ast& asw)
                : ast(asw)
        {}

        const peg::Ast& operator() () const {
            return ast;
        }
        const peg::Ast& ast;
    };

}
#endif //SCC_ASTWRAPPER_HPP
