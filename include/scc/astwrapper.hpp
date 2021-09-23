//
// Created by Mpho Mbotho on 2020-10-30.
//

#pragma once

#include <scc/peglib.h>

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

