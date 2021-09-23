//
// Created by Mpho Mbotho on 2020-10-31.
//

#include <scc/exception.hpp>
#include <scc/astwrapper.hpp>
#include <scc/program.hpp>

namespace scc {

    Exception Exception::fromCurrent()
    {
        if (auto current = std::current_exception()) {
            try {
                std::rethrow_exception(current);
            }
            catch (std::exception& ex) {
                return Exception{ex.what()};
            }
        }
        return Exception{};
    }

    Exception::operator bool() const
    {
        return valid;
    }

    void Exception::buildMessage(std::ostream &os, const AstWrapper &astWrapper)
    {
        const auto& ast = astWrapper();
        os << "syntax error - " << ast.path << ":" << ast.line << " ";
    }

    const char* Exception::what() const noexcept
    {
        return mMessage.c_str();
    }

    const std::string& Exception::message() const noexcept
    {
        return mMessage;
    }
}