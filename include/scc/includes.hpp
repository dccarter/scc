//
// Created by Mpho Mbotho on 2020-11-04.
//

#ifndef SCC_INCLUDES_HPP
#define SCC_INCLUDES_HPP

#include <string>
#include <unordered_set>

namespace scc {
    class Include;
    class Formatter;

    class IncludeBag final {
    public:
        IncludeBag() = default;
        void write(Formatter& fmt, const Include& inc);
        void write(Formatter& fmt, const std::string& header, char c = '<');
    public:
        std::unordered_set<std::string> mIncluded;
    };
}
#endif //SCC_INCLUDES_HPP
