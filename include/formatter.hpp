//
// Created by Mpho Mbotho on 2020-10-28.
//

#ifndef SCC_FORMATTER_HPP
#define SCC_FORMATTER_HPP

#include <string>
#include <ostream>

namespace scc {

    class Formatter final {
    public:
        std::ostream& operator()(bool noNewLine = false);
        std::ostream& push(bool newLine = true);
        std::ostream& pop(bool newLine = true);
        Formatter();
    private:
        friend class ProgramGenerator;
        friend class Node;
        Formatter(std::ostream& os);
        Formatter(std::ostream& os, const std::string& tab);
        Formatter operator!();
        std::ostream& os;
        std::string tab{};
        const std::string& enforced;
    };
}
#endif //SCC_FORMATTER_HPP
