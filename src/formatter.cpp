//
// Created by Mpho Mbotho on 2020-10-28.
//

#include <scc/formatter.hpp>

#include <iostream>

namespace {
    static const std::string EMPTY_STR{""};
}
namespace scc {

    Formatter::Formatter()
        : Formatter(std::cout, EMPTY_STR)
    {}

    Formatter::Formatter(std::ostream& os)
        : Formatter(os, EMPTY_STR)
    {}

    Formatter::Formatter(std::ostream& os, const std::string& tab)
        : os{os},
          enforced{tab},
          tab{tab}
    {}

    std::ostream & Formatter::push(bool newLine)
    {
        tab = std::string(tab.size()+4, ' ');
        if (newLine) {
            os << '\n' << tab;
        }
        return os;
    }

    std::ostream & Formatter::pop(bool newLine)
    {
        if (tab.size() > enforced.size()) {
            tab = std::string(tab.size()-4, ' ');
        }
        if (newLine) {
            os << '\n';
        }
        return os;
    }

    std::ostream & Formatter::operator()(bool noNewLine)
    {
        if (!noNewLine) {
            return os << '\n' << tab;
        }
        return os;
    }

    Formatter Formatter::operator!()
    {
        return Formatter{os, tab};
    }
}