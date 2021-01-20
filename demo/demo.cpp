//
// Created by Mpho Mbotho on 2020-11-03.
//

#include "demo.hpp"
#include <scc/program.hpp>
#include <scc/exception.hpp>
#include <scc/visitor.hpp>

namespace scc {

    void DemoHppGenerator::generate(Formatter fmt, const Type& type)
    {
        if (!type.is<Class>()) {
            throw Exception("demo generator only supports 'Class' types");
        }

        const auto& ct = type.as<Class>();
        fmt() << "class";
        Attribute::toString(fmt, ct.Attribs);
        fmt(true) << ' ';
        ct.Name.toString(fmt);
        fmt(true) << " : public demo::Debug";
        if (! ct.BaseClasses.empty()) {
            fmt(true) << ' ';
            Base::toString(fmt, ct.BaseClasses);
        }
        fmt(true) << " {";
        fmt.push(false);
        Visitor<Class>(ct).visit<Node>([&](const Node& node) {
            node.toString(fmt);
            if (node.is<Field>() or node.is<Method>() or node.is<Constructor>()) {
                fmt(true) << ';';
            }
        });
        fmt.pop();
        fmt() << "public:" ;
        fmt.push() << "void dbgprint(demo::DebugPrinter& dbp) const override;";
        fmt.pop() << "};";
    }

    void DemoHppGenerator::invoke(const std::string& method, Formatter fmt, const KeyValuePairs& vars)
    {
        if (method == "incs") {
            fmt() << R"(#include "debug.hpp")";
            fmt() << R"(#include "symbols.scc.hpp")";
            fmt();
            Formatter fmt1;
            debug() << "demo/debug the parameter dump";
            fmt1.push(false);
            for (const auto& [name, var]: vars()) {
                fmt1() << name << ":";
                var.toString(fmt1);
            }
            fmt1.pop();
        }
    }

    void DemoCppGenerator::generate(Formatter& fmt, const std::string& klass, const Field& field)
    {
        auto dbg = Attribute::find(field.Attribs, {"demo", "debug"});
        if (dbg and !dbg().empty()) {
            fmt() << R"(dbp << ")" << field.Name.Content << R"( : {";)";
            const auto& params = dbg();
            //  os << "mUser = {";
            //  Email: " << mUser.Email << ", Age: " << mUser.Age << "}";
            for (const auto& param: params) {
                if (&param != &params.front()) {
                    fmt() << R"(dbp << ", ";)";
                }
                fmt() << R"(dbp << ")" << param.Content << R"(: " << )" << field.Name.Content << "." << param.Content << ";";
            }
            fmt() << R"(dbp << "}";)";
        }
    }

    void DemoCppGenerator::generate(Formatter fmt, const Type& type)
    {
        if (!type.is<Class>()) {
            throw Exception("demo generator only supports 'Class' types");
        }

        const auto& ct = type.as<Class>();

        fmt() << "void " << type.Name.Content << "::dbgprint(demo::DebugPrinter& dbp) const {";
        fmt.push(false);
        fmt() << R"(dbp << "{";)";
        bool first{true};
        Visitor<Class>(ct).visit<Field>([&](const Field& field) {
            if (!first) {
                fmt() << R"(dbp << ", ";)";
            }
            first = false;
            generate(fmt, type.Name.Content, field);
        });
        fmt() << R"(dbp << "}";)";
        fmt.pop() << "}";
    }
}

extern "C" {

    int  LibInitialize(Context ctx)
    {
        scc::registerLibGenerator(
                ctx,
                "debug",
                std::make_shared<scc::DemoHppGenerator>(),
                std::make_shared<scc::DemoCppGenerator>());
        return 0;
    }
}