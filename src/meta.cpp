//
// Created by Mpho Mbotho on 2020-10-24.
//

#include <scc/meta.hpp>
#include <scc/program.hpp>
#include <scc/visitor.hpp>


namespace scc {


    void MetaHeader::generate(Formatter fmt, const Type& tp)
    {
        if (tp.is<Class>()) {
            error() << "meta generator can only generate struct types";
            return;
        }

        if (tp.is<Enum>()) {
            generateEnum(fmt, tp.as<Enum>());
        }
        else {
            auto st = tp.cast<Struct>();
            if (st == nullptr || st->IsUnion) {
                error() << "meta generator can only generate struct types";
                return;
            }
            generateStruct(fmt, *st);
        }

        Line(fmt);
    }

    void MetaHeader::generateStruct(Formatter fmt, const Struct& st)
    {
        UsingNamespace(getNamespace(), fmt);
        Line(fmt) << "typedef decltype(iod::D(";
        ++fmt;

        bool first{true};
        Visitor<Struct>(st).visit<Field>([&](const Field& field){
            if (!first) {
                fmt << ",";
            }
            Line(fmt) << "prop(" << field.Name.Content;
            if (!field.Attribs.empty()) {
                fmt << "(";
                for (const auto& attr: field.Attribs) {
                    if (&attr != &field.Attribs.front()) {
                        fmt << ", ";
                    }
                    attr.scopedString(fmt, "var");
                }
                fmt << ")";
            }
            fmt << ", ";
            field.Type.toString(fmt);
            fmt << "())";
            first = false;
        });
        Line(--fmt) << ")) " << st.Name << ";";
    }

    void MetaHeader::generateEnum(Formatter fmt, const Enum& enm)
    {
        Line(fmt) << "enum ";
        enm.Name.toString(fmt);
        if (!enm.Base.Content.empty()) {
            fmt << " : ";
            enm.Base.toString(fmt);
        }
        fmt << " {";
        Line(++fmt);

        bool first{true};
        Visitor<Enum>(enm).visit<EnumMember>([&](const EnumMember& em){
            if (!first) {
                fmt << ",";
                Line(fmt);
            }
            em.Name.toString(fmt);
            if (!em.Value.empty()) {
                fmt << " = " << em.Value;
            }
            first = false;
        });
        Line(--fmt) << "};";
    }
}