//
// Created by Mpho Mbotho on 2020-10-27.
//

#pragma once

#include <variant>
#include <memory>
#include <ostream>
#include <string>
#include <set>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <unordered_map>
#include "peglib.h"

struct mpc_ast_t;

namespace scc {

    class Formatter;
    class AstWrapper;

#define SCC_DISABLE_COPY(T)             \
    T (const T &) = delete;             \
    T & operator=(const T &) = delete;  \
    T ( T &&) = default;                \
    T & operator=( T &&) = default

#define  DECLARE_STREAM_OP(T)                                   \
    scc::Formatter& operator<<(scc::Formatter& fmt, const T& v)

#define  IMPL_STREAM_OP(T)      \
    DECLARE_STREAM_OP(T) {      \
        v.toString(fmt);        \
        return fmt;             \
    }

    template <typename T>
    using Vec = std::vector<T>;
    template <typename T>
    using Set = std::set<T>;

    struct Source {
        Source() = default;
        Source(const peg::Ast& ast);
        std::string_view Path{};
        std::size_t Line{0};
        std::size_t Column{0};
    };

    class Node {
    public:
        using Ptr = std::shared_ptr<Node>;
        virtual void toString(Formatter& fmt) const {}
        virtual void toString(std::ostream& os) const;
        std::size_t Tag{typeid(Node).hash_code()};
        SCC_DISABLE_COPY(Node);

    public:
        template<typename T>
            requires (std::is_base_of_v<Node,T>)
        bool is() const {
            return typeid(T).hash_code() == Tag;
        }

        template<typename T>
        requires (std::is_base_of_v<Node, T>)
        auto cast() const -> const T* {
            if (is<T>()) {
                return dynamic_cast<const T *>(this);
            }
            return nullptr;
        }

        template<typename T>
        requires (std::is_base_of_v<Node,T>)
        auto cast() -> T* {
            if (is<T>()) {
                return dynamic_cast<T *>(this);
            }
            return nullptr;
        }

        template<typename T>
        requires (std::is_base_of_v<Node,T>)
        auto as() const -> const T& {
            return *(dynamic_cast<const T *>(this));
        }

        template<typename T>
        requires (std::is_base_of_v<Node,T>)
        auto as() -> T& {
            return *(dynamic_cast<T *>(this));
        }

        const Source& src() const { return _source; }

    protected:
        Node() = default;
        Node(std::size_t id)
            : Tag(id)
        {}
        virtual void fromAst(const AstWrapper& ast) {}

    protected:
        Source _source{};
        static std::string _sPath;
    };

    class Ident : public Node {
    public:
        Ident(const AstWrapper& ast);
        Ident();
        std::string Content{};
        void toString(Formatter &fmt) const override;
        bool operator==(const std::string& name) const { return Content == name; }
        bool operator!=(const std::string& name) const { return Content != name; }
        SCC_DISABLE_COPY(Ident);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    struct NumberExpr {
        NumberExpr() = default;
        NumberExpr(const std::string& expr)
            : Expr{expr}
        {}
        operator bool() const { return !Expr.empty(); }
        NumberExpr(const NumberExpr&) = default;
        NumberExpr(NumberExpr&&) = default;
        NumberExpr& operator=(const NumberExpr&) = default;
        NumberExpr& operator=(NumberExpr&&) = default;
        std::string Expr;
    };

    template <typename T>
    concept IsLiteralAlternative = requires {
        std::is_arithmetic_v<T> or
        std::is_same_v<T, std::string> or
        std::is_same_v<T, char> or
        std::is_same_v<T, bool> or
        std::is_same_v<T, NumberExpr> or
        std::is_same_v<T, std::nullptr_t>;
    };

    class Literal: public Node {
    public:
        using Value_t = std::variant<bool, char, int64_t, double, std::string, NumberExpr, std::nullptr_t>;

        Literal(const AstWrapper& asw);
        Literal();

        void toString(Formatter &fmt) const override;

        Value_t Value{nullptr};
        template<typename T>
            requires IsLiteralAlternative<T>
        bool has() const {
            return std::holds_alternative<T>(Value);
        }

        template<typename T>
            requires IsLiteralAlternative<T>
        operator const T&() const {
            // static_assert(!std::is_same_v<T, NumberExpr>(), "Number expression cannot be converted to values");
            return std::get<T>(Value);
        }

        inline operator bool() const {
            return valid;
        }

        SCC_DISABLE_COPY(Literal);

    protected:
        void fromAst(const AstWrapper &asw) override;

    private:
        bool valid{false};
    };

    class KeyValuePairs final : public Node {
    public:
        KeyValuePairs(const AstWrapper& asw);
        KeyValuePairs();
        bool has(const std::string& name) const;
        const Literal& operator[](const std::string& name) const;
        inline operator bool() const {
            return valid;
        }
        void toString(Formatter &fmt) const override;

        const std::unordered_map<std::string, Literal>& operator()() const {
            return mPairs;
        }
        SCC_DISABLE_COPY(KeyValuePairs);

    protected:
        void fromAst(const AstWrapper &ast) override;

    private:
        std::unordered_map<std::string, Literal> mPairs;
        bool valid{false};
    };

    class Variables final {
    public:
        static const KeyValuePairs INVALID;
        Variables() = default;
        bool has(const std::string& name) const;
        const KeyValuePairs& operator[](const std::string& name) const;
        void add(const AstWrapper& asw);
        inline operator bool() const {
            return !mList.empty();
        }

        void toString(Formatter &fmt) const;

        SCC_DISABLE_COPY(Variables);

    private:
        std::unordered_map<std::string, KeyValuePairs> mList;
    };

    class Invoke : public Node {
    public:
        Invoke(const AstWrapper& asw);
        Invoke();

        SCC_DISABLE_COPY(Invoke);

        void toString(Formatter &fmt) const override;

    protected:
        friend struct ProgramGenerator;

        bool      ForCpp{false};
        Ident     Lib;
        Ident     Generator;
        Ident     Function;
        KeyValuePairs Params;
        Ident     ParamVar;
        void fromAst(const AstWrapper &ast) override;
    };

    class Comment: public Node {
    public:
        Comment(const AstWrapper& ast);
        Comment();
        std::string Content{};
        bool        IsBlock{false};
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Comment);

    protected:
        void fromAst(const AstWrapper& ) override;

    private:
        void buildComment(std::ostream& os, const AstWrapper& wr);
    };

    class Native: public Node {
    public:
        Native(const AstWrapper& ast);
        Native();
        std::string Code;
        bool        ForCpp{false};
        void toString(Formatter &fmt) const override;
        SCC_DISABLE_COPY(Native);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Scoped: public Node {
    public:
        Scoped(const AstWrapper& ast);
        Scoped();
        Vec<Ident> Parts;
        void toString(Formatter &fmt) const override;
        std::string toString() const;
        SCC_DISABLE_COPY(Scoped);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Generic: public Node {
    public:
        Generic(const AstWrapper& ast);
        Generic();
        Scoped Left;
        Vec<Generic::Ptr> Right;
        void toString(Formatter &fmt) const override;
        bool isVoid() const;
        SCC_DISABLE_COPY(Generic);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class AttributeParams {
    public:
        AttributeParams(const Vec<Ident>& params);
        AttributeParams();
        operator bool() const;
        bool check(int index, const std::string& str = "_") const;
        bool has(const std::string& attr) const;
        const Vec<Ident>& operator()() const { return params; }
    private:
        const Vec<Ident>& params;
        bool valid{false};
    };

    class Attribute : public Node {
    public:
        Attribute(const AstWrapper& ast);
        Attribute();
        Vec<Ident>  Name;
        Vec<Ident>  Params;
        void toString(Formatter &fmt) const override;
        void scopedString(Formatter& fmt, const std::string& macro) const;
        SCC_DISABLE_COPY(Attribute);
        static void buildAttributes(Vec<Attribute>& dest, const AstWrapper& ast);
        static void toString(Formatter& fmt, const Vec<Attribute>& attrs);
        static bool isGenerator(const Attribute& attr);
        static AttributeParams find(const Vec<Attribute>& attribs, const Vec<std::string>& name);
    protected:
        void fromAst(const AstWrapper &ast) override;
    };

    using KVPParams = KeyValuePairs;

    class Annotation : public Node {
    public:
        Annotation(const AstWrapper& ast);
        Annotation();
        SCC_DISABLE_COPY(Annotation);

        Vec<Ident>   Name;
        Vec<Literal> PositionalParam{};
        KVPParams    NamedParams{};
        const Literal& get(int index, const std::string& name) const;
        operator bool() const { return _valid; }
        bool operator==(const Vec<std::string>& name) const;
        bool operator!=(const Vec<std::string>& name) const { return !(*this == name); }
        void toString(Formatter &fmt) const override;

    protected:
        void fromAst(const AstWrapper &ast) override;

    private:
        bool _valid{true};
    };

    class AnnotationList : public Node {
    public:
        AnnotationList(const AstWrapper& ast);
        AnnotationList();
        SCC_DISABLE_COPY(AnnotationList);

        operator bool() const { return !_annotations.empty(); }

        void toString(Formatter &fmt) const override;
        const Annotation& operator[](const Vec<std::string>& name) const;
        const Vec<Annotation>& operator()() const { return _annotations; }
    protected:
        void fromAst(const AstWrapper &ast) override;

    private:
        friend class Type;
        Vec<Annotation> _annotations{};
    };

    class Generator: public Node {
    public:
        Generator(const AstWrapper& ast);
        Generator();
        SCC_DISABLE_COPY(Generator);

        Vec<Ident> Name;

        operator bool() const { return _valid; }
        void toString(Formatter &fmt) const override;

    protected:
        void fromAst(const AstWrapper &ast) override;

    private:
        bool _valid{false};
    };

    class GeneratorList: public Node {
    public:
        GeneratorList(const AstWrapper& ast);
        GeneratorList();
        SCC_DISABLE_COPY(GeneratorList);

        operator bool() const { return !_generators.empty(); }
        void toString(Formatter &fmt) const override;
        const Generator& operator[](Vec<std::string>& name) const;
        const Vec<Generator>& operator()() const { return _generators; }
    protected:
        void fromAst(const AstWrapper &ast) override;

    private:
        friend class Type;
        Vec<Generator> _generators;
    };

    class Modifier: public Node {
    public:
        Modifier(const AstWrapper& ast);
        Modifier();
        std::string Name;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Modifier);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Field: public Node {
    public:
        Field(const AstWrapper& ast);
        Field();
        AnnotationList  Annotations;
        Generic        Type;
        Ident          Name;
        std::string    Kind;
        Literal        Value;
        bool           Const{false};
        void toString(Formatter &fmt) const override;
        SCC_DISABLE_COPY(Field);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Parameter: public Node {
    public:
        Parameter(const AstWrapper& ast);
        Parameter();
        bool        Const{false};
        AnnotationList  Annotations;
        Generic      Type;
        std::string  Kind;
        Ident        Name;
        void toString(Formatter &fmt) const override;
        static void buildParameters(Vec<Parameter>& params, const AstWrapper& asw);

        SCC_DISABLE_COPY(Parameter);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Method: public Node {
    public:
        Method(const AstWrapper& ast);
        Method();
        AnnotationList  Annotations;
        struct RType {
            bool        Const{false};
            Generic     Type;
            std::string Kind;
            void toString(Formatter& fmt) const;
        };
        RType          ReturnType;
        Ident          Name;
        Vec<Parameter> Params;
        bool           Const{false};
        void toString(Formatter &fmt) const override;
        void toString0(Formatter &fmt) const;
        void signature(Formatter& fmt) const;
        SCC_DISABLE_COPY(Method);

    private:
        void fromAst(const AstWrapper& ast) override;
    };

    class Constructor: public Node {
    public:
        Constructor(const AstWrapper& ast);
        Constructor();
        AnnotationList  Annotations;
        Ident  Name;
        Vec<Parameter> Params;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Constructor);

    public:
        void fromAst(const AstWrapper& ast) override;
    };

    class Base: public Node {
    public:
        Base(const AstWrapper& ast);
        Base();
        std::string Modifier;
        Generic     Type;
        void toString(Formatter &fmt) const override;
        static void buildBases(Vec<Base>& bases, const AstWrapper& asw);
        static void toString(Formatter& fmt, const Vec<Base>& bases);
        SCC_DISABLE_COPY(Base);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class GeneratorAttribute : public Attribute {
    public:
        GeneratorAttribute(const AstWrapper& asw);
        GeneratorAttribute();
    };

    class Type: public Node {
    public:
        Type(std::size_t id);
        Type();
        SCC_DISABLE_COPY(Type);

        AnnotationList  Annotations;
        GeneratorList Generators;
        Ident Name;
        Vec<Node::Ptr> Members;

        void toString(Formatter &fmt) const override;



    protected:
        void parseGenAnno(const peg::Ast& ast);
        void fromAst(const AstWrapper& ast) override {}
    };

    class Class: public Type {
    public:
        Class(const AstWrapper& ast);
        Class() = default;
        Vec<Base> BaseClasses;

        SCC_DISABLE_COPY(Class);

    public:
        void fromAst(const AstWrapper& ast) override;
    };

    class Struct: public Type {
    public:
        Struct(const AstWrapper& ast);
        Struct() = default;
        bool IsUnion{false};

        SCC_DISABLE_COPY(Struct);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class EnumMember: public Node {
    public:
        EnumMember(const AstWrapper& ast);
        EnumMember();

        AnnotationList Annotations;
        Ident Name{};
        std::string Value{};

        SCC_DISABLE_COPY(EnumMember);

    protected:
        void fromAst(const AstWrapper& ) override;
    };

    class Enum: public Type {
    public:
        Enum(const AstWrapper& ast);
        Enum() = default;
        Ident Base;

        SCC_DISABLE_COPY(Enum);
    protected:
        void fromAst(const AstWrapper &ast) override;
    };

    class Include: public Node {
    public:
        Include(const AstWrapper& ast);
        Include();
        std::string Header;
        char Left;
        char Right;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Include);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Symbol: public Node {
    public:
        Symbol(const AstWrapper& ast);
        Symbol();
        std::string Name;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Symbol);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Library: public Node {
    public:
        Library(const AstWrapper& ast);
        Library();
        Ident Name;
        std::string  Path;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Library);

    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class Section: public Node {
    public:
        Section(std::size_t hash);
        Section();
        Vec<Node::Ptr> Content;
        void toString(Formatter &fmt) const override;

        SCC_DISABLE_COPY(Section);

        virtual bool empty() const;

    protected:
        friend class ProgramGenerator;
        void fromAst(const AstWrapper &ast) override {}
        Variables mVars;
    };

    class Before: public Section {
    public:
        Before(const AstWrapper& asw);
        Before();
        SCC_DISABLE_COPY(Before);

        inline operator bool() const {
            return !empty();
        }

    protected:
        void fromAst(const AstWrapper &ast) override;
    };

    class Namespace: public Section {
    public:
        Namespace(const AstWrapper& ast);
        Namespace();
        Scoped     Name;

        void toString(Formatter &fmt) const override;

        bool empty() const override;

        SCC_DISABLE_COPY(Namespace);

        operator bool() const;


    protected:
        void fromAst(const AstWrapper& ast) override;
    };

    class After: public Section {
    public:
        After(const AstWrapper& asw);
        After();

        SCC_DISABLE_COPY(After);

        inline operator bool() const {
            return !empty();
        }

    protected:
        void fromAst(const AstWrapper &ast) override;
    };

    class Program: public Node {
    public:
        Program(const AstWrapper& ast);
        Program();
        Before      before;
        Namespace   space;
        After       after;
        void toString(Formatter &fmt) const override;
        SCC_DISABLE_COPY(Program);
        operator bool () const;
    protected:
        void fromAst(const AstWrapper& ast) override;
    };
}

inline std::ostream& operator<<(std::ostream& os, const scc::Node& node)
{
    node.toString(os);
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const scc::Source& src)
{
    return (os << src.Path << ":" << src.Line << ":" << src.Column);
}

