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
        UsingNamespace(getNamespace(), fmt);
        if (!type.is<Class>()) {
            throw Exception("demo generator only supports 'Class' types");
        }

        const auto& ct = type.as<Class>();
        Line(fmt) << "class";
        if (ct.Generators) {
            fmt << " " << ct.Generators;
        }

        if (ct.Annotations) {
            fmt << " " << ct.Annotations;
        }

        fmt << ' ' << ct.Name;
        fmt << " : public demo::Debug";
        if (!ct.BaseClasses.empty()) {
            fmt << ' ';
            Base::toString(fmt, ct.BaseClasses);
        }
        fmt << " {";
        ++fmt;
        Visitor<Class>(ct).visit<Node>([&](const Node& node) {
            Line(fmt);
            node.toString(fmt);
            if (node.is<Field>() or node.is<Method>() or node.is<Constructor>()) {
                fmt << ';';
            }
        });
        Line(--fmt);
        Line(fmt) << "public:" ;
        Line(++fmt) << "void dbgprint(demo::DebugPrinter& dbp) const override;";
        Line(--fmt) << "};";
    }

    void DemoHppGenerator::invoke(const std::string& method, Formatter fmt, const KeyValuePairs& vars)
    {
        if (method == "incs") {
            Line(fmt) << R"(#include "debug.hpp")";
            Line(fmt) << R"(#include "symbols.scc.hpp")";
            Line(fmt);
            Formatter fmt1;
            debug() << "demo/debug the parameter dump";
            ++fmt1;
            for (const auto& [name, var]: vars()) {
                Line(fmt1) << name << ":";
                var.toString(fmt1);
            }
            --Line(fmt1);
        }
    }

    void DemoCppGenerator::generate(Formatter& fmt, const std::string& klass, const Field& field)
    {
        auto& dbg = field.Annotations[ {"demo"}][{"debug"}];
        if (dbg and !dbg.Params.empty()) {
            Line(fmt) << R"(dbp << ")" << field.Name.Content << R"( : {";)";
            //  os << "mUser = {";
            //  Email: " << mUser.Email << ", Age: " << mUser.Age << "}";
            for (const auto& param: dbg.Params) {
                if (!param.has<std::string>()) {
                    throw Exception("annotation demo/debug only accepts string field names");
                }

                if (&param != &dbg.Params.front()) {
                    Line(fmt) << R"(dbp << ", ";)";
                }

                const std::string& value = param;
                Line(fmt) << R"(dbp << ")" << value << R"(: " << )" << field.Name.Content << "." << value << ";";
            }
            Line(fmt) << R"(dbp << "}";)";
        }
    }

    void DemoCppGenerator::generate(Formatter fmt, const Type& type)
    {
        UsingNamespace(getNamespace(), fmt);

        if (!type.is<Class>()) {
            throw Exception("demo generator only supports 'Class' types");
        }

        const auto& ct = type.as<Class>();

        Line(fmt) << "void " << type.Name.Content << "::dbgprint(demo::DebugPrinter& dbp) const {";
        Line(++fmt) << R"(dbp << "{";)";
        bool first{true};
        Visitor<Class>(ct).visit<Field>([&](const Field& field) {
            if (!first) {
                Line(fmt) << R"(dbp << ", ";)";
            }
            first = false;
            generate(fmt, type.Name.Content, field);
        });
        Line(fmt) << R"(dbp << "}";)";
        Line(--fmt) << "}";
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