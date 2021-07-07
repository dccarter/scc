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

        fmt();
    }

    void MetaHeader::generateStruct(Formatter fmt, const Struct& st)
    {
        fmt() << "typedef decltype(iod::D(";
        fmt.push(false);

        bool first{true};
        Visitor<Struct>(st).visit<Field>([&](const Field& field){
            if (!first) {
                fmt(true) << ",";
            }
            fmt() << "prop(" << field.Name.Content;
            if (!field.Attribs.empty()) {
                fmt(true) << "(";
                for (const auto& attr: field.Attribs) {
                    if (&attr != &field.Attribs.front()) {
                        fmt(true) << ", ";
                    }
                    attr.scopedString(fmt, "var");
                }
                fmt(true) << ")";
            }
            fmt(true) << ", ";
            field.Type.toString(fmt);
            fmt(true) << "())";
            first = false;
        });
        fmt.pop() << ")) " << st.Name.Content << ";";
    }

    void MetaHeader::generateEnum(Formatter fmt, const Enum& enm)
    {
        fmt() << "enum ";
        enm.Name.toString(fmt);
        if (!enm.Base.Content.empty()) {
            fmt(true) << " : ";
            enm.Base.toString(fmt);
        }
        fmt(true) << " {";
        fmt.push();

        bool first{true};
        Visitor<Enum>(enm).visit<EnumMember>([&](const EnumMember& em){
            if (!first) {
                fmt(true) << ",";
                fmt();
            }
            em.Name.toString(fmt);
            if (!em.Value.empty()) {
                fmt(true) << " = " << em.Value;
            }
            first = false;
        });
        fmt.pop() << "};";
    }
}