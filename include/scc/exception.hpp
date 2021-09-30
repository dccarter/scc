//
// Created by Mpho Mbotho on 2020-10-31.
//

#ifndef SCC_EXCEPTION_HPP
#define SCC_EXCEPTION_HPP

#include <exception>
#include <string>
#include <sstream>

namespace scc {
    class Node;
    class AstWrapper;
    struct Source;

    class Exception : std::exception {
    public:
        template<typename ...Args>
        Exception(const Args... args) noexcept
        {
            if constexpr (sizeof...(args) != 0) {
                std::stringstream ss;
                (ss << ... << args);
                mMessage = ss.str();
            }
        }

        template<typename ...Args>
        Exception(const AstWrapper& asw, const Args... args) noexcept
        {
            std::stringstream ss;
            buildMessage(ss, asw);
            (ss << ... << args);
            mMessage = ss.str();
        }

        template<typename ...Args>
        Exception(const Source& src, const Args... args) noexcept
        {
            std::stringstream ss;
            buildMessage(ss, src);
            (ss << ... << args);
            mMessage = ss.str();
        }

        operator bool() const;

        const char* what() const noexcept override;
        const std::string& message() const noexcept;

        static Exception fromCurrent();
    private:
        void buildMessage(std::ostream& os, const AstWrapper& astWrapper);
        void buildMessage(std::ostream& os, const Source& src);
        Exception()
            : valid{false}
        {}
        std::string mMessage{};
        bool valid{true};
    };
}
#endif //SCC_EXCEPTION_HPP
