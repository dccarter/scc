//
// Created by Mpho Mbotho on 2020-11-05.
//

#ifndef SCC_DEBUG_HPP
#define SCC_DEBUG_HPP

#include <iostream>

namespace demo {
    class DebugPrinter;
    class Debug {
    public:
        virtual void dbgprint(DebugPrinter& dbp) const = 0;
    };

    class DebugPrinter final {
    public:
        DebugPrinter(std::ostream& os)
            : os(os)
        {}

        DebugPrinter()
            : DebugPrinter(std::cout)
        {}

        template<typename T>
        void print(const std::string& name, const T& data) {
            os << name << " : ";
            if constexpr (std::is_base_of_v<Debug, T>) {
                data.dbgprint(*this);
            }
            else {
                os << data;
            }
        }

        template<typename T>
        DebugPrinter& operator<<(const T& data) {
            if constexpr (std::is_base_of_v<Debug, T>) {
                data.dbgprint(*this);
            } else {
                os << data;
            }
            return *this;
        }
    private:
        std::ostream& os;
    };
}

#endif //SCC_DEBUG_HPP
