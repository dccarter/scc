//
// Created by Mpho Mbotho on 2020-10-28.
//

#ifndef SCC_FORMATTER_HPP
#define SCC_FORMATTER_HPP

#include <string>
#include <ostream>

namespace scc {
    class Node;
    const struct NewLine{} _NL{};

    class Formatter final {
    public:
        std::ostream& operator()(bool noNewLine = false);
        std::ostream& push(bool newLine = true);
        std::ostream& pop(bool newLine = true);
        Formatter& operator++() { push(false); return *this; }
        Formatter& operator--() { pop(false); return *this; }

        Formatter& operator<<(const NewLine& v) {
            os << "\n";
            newLine = true;
            return *this;
        }

        template <typename T>
            requires (!std::is_base_of_v<Node, T>)
        Formatter& operator<<(const T& v) {
            if (newLine) {
                os << tab;
                newLine = false;
            }
            os << v;
            return *this;
        }

        template <typename T>
            requires (std::is_base_of_v<Node, T>)
        Formatter& operator<<(const T& v) {
            v.toString(*this);
            return *this;
        }

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
        bool newLine{true};
    };

    #define Line(fmt) (fmt << scc::_NL)

    struct NamespaceWriter {
        NamespaceWriter(Formatter& fmt, const std::string& ns)
            : fmt{fmt}
        {
            Line(fmt) << "namespace " << ns << " {";
            Line(++fmt);
        }

        ~NamespaceWriter() {
            Line(--fmt) << "} // end namespace";
        }

    private:
        Formatter fmt;
    };

#define SCC_PASTE(x, y) x##y
#define UsingNamespace(ns, fmt) scc::NamespaceWriter SCC_PASTE(_ns, __LINE__){fmt, ns}
}
#endif //SCC_FORMATTER_HPP
