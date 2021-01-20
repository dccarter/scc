//
// Created by Mpho Mbotho on 2020-11-04.
//

#include "includes.hpp"
#include "program.hpp"
#include "generator.hpp"

namespace scc {

    void IncludeBag::write(Formatter& fmt, const Include& inc)
    {
        if (mIncluded.contains(inc.Header)) {
            debug(Log::LV3) << "header '" << inc.Header << "' already included";
            return;
        }
        inc.toString(fmt);
        mIncluded.emplace(inc.Header);
    }

    void IncludeBag::write(Formatter& fmt, const std::string& header, char c)
    {
        if (mIncluded.contains(header)) {
            debug(Log::LV3) << "header '" << header << "' already included";
            return;
        }
        fmt() << "#include " << c << header << (c == '<'? '>': '"');
        mIncluded.emplace(header);
    }
}