//
// Created by Mpho Mbotho on 2020-10-29.
//

#ifndef SCC_PARSER_HPP
#define SCC_PARSER_HPP

#include <scc/program.hpp>

#include <filesystem>
#include <memory>

namespace peg {
    class parser;
}

namespace scc {

    class Parser {
    public:
        bool load(const std::filesystem::path& path = {});
        Program parse(const std::filesystem::path& path);
        void repl();
    private:
        std::shared_ptr<peg::parser> P;
    };
}
#endif //SCC_PARSER_HPP
