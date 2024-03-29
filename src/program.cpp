//
// Created by Mpho Mbotho on 2020-10-27.
//

#include <scc/generator.hpp>
#include <scc/program.hpp>
#include <scc/formatter.hpp>
#include <scc/astwrapper.hpp>
#include <scc/exception.hpp>

using namespace peg::udl;

namespace {

    void astUnrecognisedTag(const peg::Ast& ast)
    {
        error() << ast.path << ":" << ast.line << " unrecognised syntax" << std::endl;
    }
}

namespace scc {
#define tagged(ast, name) (ast.tag == name)

#define _NODE_CTOR(Tp)                           \
    Tp :: Tp ( const AstWrapper& asw )           \
        : Node(typeid( Tp ).hash_code())         \
    {                                            \
            fromAst(asw);                        \
            _source = Source{asw()};             \
            _source.Path =  Node::_sPath;        \
    }                                            \
    Tp :: Tp ()                                  \
        : Node (typeid( Tp ).hash_code())        \
    {}

    Source::Source(const peg::Ast& ast)
        : Path{ast.path},
          Line{ast.line},
          Column{ast.column}
    {}


    std::string Node::_sPath{};

    void Node::toString(std::ostream& os) const
    {
        Formatter fmt{os};
        toString(fmt);
    }

    _NODE_CTOR(Ident)

    void Ident::toString(Formatter &fmt) const
    {
        fmt << Content;
    }

    void Ident::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;

        if (tagged(ast, "ident"_ )) {
            Content = ast.token;
        }
    }

    _NODE_CTOR(Literal);

    void Literal::toString(Formatter& fmt) const {
        std::visit([&](const auto& arg) {
            using TT = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<char, TT>) {
                fmt << '\'' << arg << '\'';
            }
            else if constexpr (std::is_same_v<bool, TT>) {
                fmt << (arg ? "true" : "false");
            }
            else if constexpr (std::is_arithmetic_v<TT>) {
                fmt << arg;
            }
            else if constexpr (std::is_same_v<std::string, TT>) {
                fmt << '"' << arg << '"';
            }
            else if constexpr (std::is_same_v<std::nullptr_t, TT>){
                fmt << "nullptr";
            }
            else if constexpr (std::is_same_v<NumberExpr, TT>) {
                fmt << arg.Expr;
            }
        }, Value);
    }

    void Literal::setSource(Source src)
    {
        _source = src;
        _source.Path = _sPath;
    }

    void Literal::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        auto convert = [&](const std::string& str, int base = 10) -> int64_t  {
            try {
                return std::stoll(str, nullptr, base);
            }
            catch (...) {
                auto ex = Exception::fromCurrent();
                throw Exception(ast, "error converting '", str, "' to number: ", ex.what());
            }
        };
        auto convert2 = [&](const std::string& str) -> double  {
            try {
                return std::stod(str);
            }
            catch (...) {
                auto ex = Exception::fromCurrent();
                throw Exception(ast, "error converting '", str, "' to number: ", ex.what());
            }
        };

        switch (ast.tag) {
            case "null"_:
                Value = nullptr;
                break;
            case "char"_:
                Value = ast.token[0];
                break;
            case "str"_:
            case "rawstr"_:
                Value = ast.token;
                break;
            case "numext"_:
                Value = NumberExpr(ast.token);
                break;
            case "bool"_:
                Value = ast.token == "true";
                break;
            case "int"_:
                Value = convert(ast.token);
                break;
            case "oct"_:
            case "hex"_:
            case "bin"_:
                Value = convert(ast.token, 0);
                break;
            case "float"_:
            case "exp"_:
                Value = convert2(ast.token);
                break;
            default:
                throw Exception(ast, "unrecognised tag at '", ast.name, "' in Literal");
        }
        valid = true;
    }

    _NODE_CTOR(KeyValuePairs);

    bool KeyValuePairs::has(const std::string& name) const
    {
        return mPairs.find(name) != mPairs.end();
    }

    const Literal& KeyValuePairs::operator[](const std::string& name) const
    {
        static Literal INVALID;
        auto it = mPairs.find(name);
        if (it == mPairs.end()) {
            return INVALID;
        }
        return it->second;
    }

    void KeyValuePairs::toString(Formatter& fmt) const
    {
        fmt << '{';
        bool first{true};
        for (const auto& [name, value]: mPairs) {
            if (!first) {
                fmt << ", ";
            }
            fmt << name << ": ";
            value.toString(fmt);
            first = false;
        }
        fmt << '}';
    }

    void KeyValuePairs::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.tag == "kvp"_) {
            mPairs.emplace(Ident{ast.nodes[0]}.Content, Literal{ast.nodes[1]});
        }
        else {
            for (const auto& n: ast.nodes) {
                mPairs.emplace(Ident{n->nodes[0]}.Content, Literal{n->nodes[1]});
            }
        }
        valid = true;
    }

    const KeyValuePairs Variables::INVALID = {};

    bool Variables::has(const std::string& name) const
    {
        return mList.contains(name);
    }

    const KeyValuePairs& Variables::operator[](const std::string& name) const
    {
        auto it = mList.find(name);
        if (it == mList.cend()) {
            return INVALID;
        }
        return it->second;
    }

    void Variables::add(const AstWrapper& asw)
    {
        const auto& ast = asw();
        Ident name{ast.nodes[0]};
        if (mList.contains(name.Content)) {
            throw Exception(ast, " - variable '", name.Content, "' already declared");
        }
        mList.emplace(std::move(name.Content), KeyValuePairs{ast.nodes[1]});
    }

    void Variables::toString(Formatter& fmt) const
    {
        for (const auto& [name, var] : mList) {
            Line(fmt) << "#pragma var " << name;
            var.toString(fmt);
        }
    }

    _NODE_CTOR(Invoke);

    void Invoke::toString(Formatter& fmt) const
    {
        Line(fmt) << "#pragma invoke";
        if (ForCpp) {
            fmt << "[cpp]";
        }
        fmt << " " << Lib.Content << "::" << Generator.Content << "." << Function.Content;
        fmt << '(';
        Params.toString(fmt);
        fmt << ')';
    }

    void Invoke::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        auto buildCommand = [this](const peg::Ast& node) {
            Lib = Ident{node.nodes[0]};
            Generator = Ident{node.nodes[1]};
            Function = Ident{node.nodes[2]};
        };

        if (ast.tag == "invokecmd"_) {
            buildCommand(ast);
        }
        else {
            int off{0};
            if (ast.nodes[0]->tag == "cpp"_){
                ForCpp = true;
                off++;
            }
            buildCommand(*ast.nodes[off++]);
            if (ast.nodes.size() > off) {
                auto params = ast.nodes[off];
                if (params->tag == "ident"_) {
                    ParamVar = Ident{params};
                }
                else {
                    Params = KeyValuePairs{params};
                }
            }
        }
    }

    _NODE_CTOR(Comment)

    void Comment::toString(Formatter &fmt) const
    {
        if (IsBlock) {
            Line(fmt) << "/*" << Content;
            fmt << "*/";
        }
        else {
            Line(fmt) << "//" << Content;
        }
    }

    void Comment::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.is_token) {
            // single line comment
            Content = ast.token;
        }
        else {
            // block comment
            IsBlock = true;
            Content = ast.nodes[1]->token;
            if (ast.nodes[0]->token.size() > 2) {
                Content = ast.nodes[0]->token.substr(2) + Content;
            }
        }
    }

    _NODE_CTOR(Native)

    void Native::toString(Formatter &fmt) const
    {
        Line(fmt) << Code;
    }

    void Native::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.nodes[0]->tag == "cpp"_) {
            ForCpp = true;
        }
        Code = ast.nodes[1]->token;
    }

    _NODE_CTOR(Scoped);

    void Scoped::toString(Formatter &fmt) const
    {
        for (const auto& part: Parts) {
            if (&part != &Parts.front()) {
                fmt << "::";
            }
            part.toString(fmt);
        }
    }

    std::string Scoped::toString() const
    {
        std::stringstream  ss;
        for (const auto& part: Parts) {
            if (&part != &Parts.front()) {
                ss << "::";
            }
            ss << part.Content;
        }
        return ss.str();
    }

    void Scoped::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.is_token) {
            Parts.push_back(Ident{asw});
        }
        else {
            for (const auto& n: ast.nodes) {
                Parts.push_back(Ident{n});
            }
        }
    }

    _NODE_CTOR(Generic)

    void Generic::toString(Formatter &fmt) const
    {
        Left.toString(fmt);
        if (!Right.empty()) {
            fmt << '<';
            for (const auto &right: Right) {
                if (&right != &Right.front()) {
                    fmt << ',';
                }
                right->toString(fmt);
            }
            fmt << '>';
        }
    }

    bool Generic::isVoid() const
    {
        return not(Left.Parts.empty()) and (Left.Parts[0].Content == "void");
    }

    void Generic::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.is_token || ast.tag == "scoped"_) {
            Left = Scoped{asw};
        } else {
            Left = Scoped{ast.nodes[0]};
            for (int i = 1; i < ast.nodes.size(); i++) {
                Right.push_back(std::make_shared<Generic>(ast.nodes[i]));
            }
        }
    }

    AttributeParams::AttributeParams(const Vec<Ident>& params)
        : params{params},
          valid{true}
    {}

    static const Vec<Ident> EMPTY_PARAMS;
    AttributeParams::AttributeParams()
        : params{EMPTY_PARAMS}
    {}

    AttributeParams::operator bool() const {
        return valid;
    }

    bool AttributeParams::check(int index, const std::string& str) const
    {
        if (index < 0 || index >= params.size()) {
            return false;
        }
        return params[index].Content == str;
    }

    bool AttributeParams::has(const std::string& attr) const
    {
        return std::find_if(params.begin(), params.end(), [&](const Ident& id) {
            return id.Content == attr;
        }) != params.end();
    }

    _NODE_CTOR(Attribute);

    void Attribute::toString(Formatter &fmt) const
    {
        fmt << "[[";
        for (const auto& attr: Name) {
            if (&attr != &Name.front()) {
                fmt << "::";
            }
            attr.toString(fmt);
        }
        if (!Params.empty()) {
            fmt << '(';
            for (const auto& p: Params) {
                if (&p != &Params.front()) {
                    fmt << ", ";
                }
                p.toString(fmt);
            }
            fmt << ')';
        }
        fmt << "]]";
    }

    void Attribute::scopedString(Formatter& fmt, const std::string& macro) const
    {
        fmt << macro << '(';
        Name.back().toString(fmt);
        fmt << ')';
    }

    void Attribute::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        auto buildName = [this](const peg::Ast& n) {
            switch (n.tag) {
                case "ident"_:
                    Name.push_back(Ident{n});
                    break;
                case "attribname"_:
                    for (const auto& p: n.nodes) {
                        Name.push_back(Ident{p});
                    }
                    break;
                default:
                    throw Exception(n, "Invalid attribute name");
            }
        };

        if (ast.tag == "attrib"_) {
            buildName(*ast.nodes[0]);
            const auto& params = ast.nodes[1];
            if (params->is_token) {
                Params.push_back(Ident{params});
            }
            else {
                for (const auto& n: params->nodes) {
                    Params.push_back(Ident{n});
                }
            }
        }
        else {
            buildName(ast);
        }
    }

    void Attribute::buildAttributes(Vec<Attribute>& dest, const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.tag != "attribs"_) {
            dest.push_back(Attribute{asw});
        }
        else {
            for (const auto& n: ast.nodes) {
                dest.push_back(Attribute{n});
            }
        }
    }

    void Attribute::toString(Formatter& fmt, const Vec<Attribute>& attrs)
    {
        for (const auto& attrib: attrs) {
            if (&attrib != &attrs.front()) {
                fmt << ' ';
            }
            attrib.toString(fmt);
        }
    }

    bool Attribute::isGenerator(const Attribute& attr)
    {
        if ((attr.Name.size() != 2) || (attr.Name[0].Content != "gen")) {
            return false;
        }
        if (attr.Name[1].Content == "meta") {
            return true;
        }
        return attr.Params.size() == 1;
    }

    AttributeParams Attribute::find(const Vec<Attribute>& attribs, const Vec<std::string>& name)
    {
        if (name.size() == 0 or name.size() > 2) {
            return AttributeParams{};
        }

        for (const auto& attrib: attribs) {
            if (attrib.Name.size() != name.size()) {
                continue;
            }
            if (attrib.Name[0].Content != name[0]) {
                continue;
            }
            if (name.size() == 2 and attrib.Name[1].Content == name[1]) {
                return AttributeParams{attrib.Params};
            }
        }
        return AttributeParams{};
    }

    _NODE_CTOR(Generator);

    void Generator::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        auto buildGenerator = [&](const peg::Ast& node) {
            if (node.tag == "ident"_) {
                Name.push_back(Ident{node});
                Name.push_back(Ident{node});
            }
            else {
                Name.push_back(Ident{node.nodes[0]});
                Name.push_back(Ident{node.nodes[1]});
            }
        };

        if (ast.tag == "gennames"_) {
            for (auto& node: ast.nodes) {
                buildGenerator(*node);
            }
        }
        else {
            buildGenerator(ast);
        }
        _valid = true;
    }

    void Generator::toString(Formatter& fmt) const
    {
        fmt << Name[0];
        if (Name.size() == 2) {
            fmt << "/" << Name[1];
        }
    }

_NODE_CTOR(GeneratorList);

    void GeneratorList::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.tag == "gennames"_) {
            for (auto& node: ast.nodes) {
                _generators.push_back(Generator{*node});
            }
        }
        else {
            _generators.push_back(Generator{ast});
        }
    }

    const Generator& GeneratorList::operator[](Vec<std::string>& name) const
    {
        for (auto& gen: _generators) {
            if (gen.Name.size() != name.size()) {
                continue;
            }
            if (gen.Name[0].Content != name[0]) {
                continue;
            }

            if (gen.Name.size() == 2 && gen.Name[1].Content == name[1]) {
                return gen;
            }
        }

        static Generator INVALID_GENERATOR;
        return INVALID_GENERATOR;
    }

    static inline bool _when(bool& v) { if (!v) return true; v = false; return  false; }
    #define when(x) if (_when(x))

    void GeneratorList::toString(Formatter& fmt) const
    {
        fmt << "[[gen(";
        bool first{true};
        for (auto& gen: _generators) {
            when(first) fmt << ", ";
            fmt << gen;
        }
        fmt << ")]]";
    }

    _NODE_CTOR(AnnotationField);

    void AnnotationField::fromAst(const AstWrapper& asw)
    {
        Name = Ident{asw};
    }

    const Literal& AnnotationField::operator[](int param) const
    {
        if (param < Params.size()) {
            return Params[param];
        }
        static Literal INVALID_LITERAL;
        return INVALID_LITERAL;
    }

    void AnnotationField::toString(Formatter& fmt) const
    {
        fmt << Name << "(" << Params[0];
        for (int i = 1; i < Params.size(); i++) {
            fmt << ", " << Params[i];
        }
        fmt << ")";
    }

    const AnnotationField& Annotation::operator[](const std::string& name) const
    {
        for (auto& field: Fields) {
            if (field == name) {
                return field;
            }
        }
        static AnnotationField INVALID_FIELD;
        return INVALID_FIELD;
    }

    _NODE_CTOR(Annotation);

    void Annotation::fromAst(const AstWrapper& asw)
    {
        Name = Ident{asw};
    }

    void Annotation::toString(Formatter& fmt) const
    {
        for (int i = 0; i < Fields.size(); i++) {
            if(i) Line(fmt);
            fmt << "[[$" << Name << "::" << Fields[i] << "]]";
        }
    }

    _NODE_CTOR(AnnotationList);

    Annotation& AnnotationList::findOrAdd(const AstWrapper& ast)
    {
        for (auto& ann: _annotations) {
            if (ann == ast().token) {
                return ann;
            }
        }
        _annotations.push_back(Annotation{ast});
        return _annotations.back();
    }

    void AnnotationList::parseCompoundAnnotation(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        auto& ann = findOrAdd(ast.nodes[0]->nodes[1]);
        auto buildParam = [&](const peg::Ast& node) {
            AnnotationField field{node.nodes[0]};
            if (ann[field.Name.Content]) {
                throw Exception(field.Name.src(), "field '",
                                field.Name.Content, "' already defined in annotation '",
                                ann.Name.Content, "'");
            }
            field.Params.push_back(Literal{node.nodes[1]});
            ann.Fields.push_back(std::move(field));
        };

        if (ast.nodes[1]->tag == "annotgenparams"_) {
            for (auto& node: ast.nodes[1]->nodes) {
                buildParam(*node);
            }
        }
        else {
            buildParam(*ast.nodes[1]);
        }
    }

    void AnnotationList::parseIndexedAnnotation(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        auto buildParam = [&](const peg::Ast& node) -> AnnotationField& {
            auto& ann = findOrAdd(node.nodes[1]);
            AnnotationField field{node.nodes[2]};
            if (ann[field.Name.Content]) {
                throw Exception(field.Name.src(), "field '",
                                field.Name.Content, "' already defined in annotation '",
                                ann.Name.Content, "'");
            }
            ann.Fields.push_back(std::move(field));
            return ann.Fields.back();
        };

        if (ast.tag == "annotidx"_) {
            auto& field = buildParam(*ast.nodes[0]);
            const auto& params = *ast.nodes[1];
            if (params.tag == "annotidxparams"_) {
                for (auto& param: params.nodes) {
                    field.Params.push_back(Literal{param});
                }
            }
            else {
                field.Params.push_back(Literal{params});
            }
        }
        else {
            auto& field = buildParam(ast);
            Literal setTrue{true};
            setTrue.setSource(Source{*ast.nodes[0]});
            field.Params.push_back(std::move(setTrue));
        }
    }

    void AnnotationList::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw.ast;
        if (ast.tag == "annotidx"_ || ast.tag == "annotnameidx"_) {
            parseIndexedAnnotation(ast);
        }
        else if (ast.tag == "annotgen"_) {
            parseCompoundAnnotation(ast);
        }
        else if (ast.tag == "annotation"_) {
            fromAst(ast.nodes[0]);
        }
        else {
            for (auto& node: ast.nodes) {
                fromAst(node);
            }
        }
    }

    const Annotation& AnnotationList::operator[](const std::string& name) const
    {
        for (auto& ann : _annotations) {
            if (ann == name) {
                return ann;
            }
        }
        static Annotation INVALID_ANNOTATION;
        return INVALID_ANNOTATION;
    }

    void AnnotationList::toString(Formatter& fmt) const
    {
        bool first{true};
        for (auto& ann: _annotations) {
            fmt << ann;
        }
    }

    _NODE_CTOR(Modifier);

    void Modifier::toString(Formatter &fmt) const
    {
        ++(--fmt << Name << ":");
    }

    void Modifier::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        Name = ast.token;
    }

    _NODE_CTOR(Field);

    void Field::toString(Formatter &fmt) const
    {
        if (Annotations) {
            fmt << Annotations;
            Line(fmt);
        }

        if (Const) {
            fmt << "const ";
        }
        Type.toString(fmt);
        if (!Kind.empty()) {
            fmt << Kind;
        }
        fmt << ' ';
        Name.toString(fmt);
        if (Value) {
            fmt << '{';
            Value.toString(fmt);
            fmt << '}';
        }
    }

    void Field::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        int off{0};
        if (ast.nodes[off]->original_tag == "annotations"_) {
            Annotations = AnnotationList{ast.nodes[off++]};
        }
        if (ast.nodes[off]->original_tag == "const"_) {
            Const = true;
            off++;
        }
        Type = Generic{ast.nodes[off++]};
        if (ast.nodes[off]->original_tag == "typemode"_) {
            Kind = ast.nodes[off++]->token;
        }
        Name = Ident{ast.nodes[off++]};
        if (ast.nodes.size() > off) {
            // create default value from node
            Value = Literal{ast.nodes[off]};
        }
    }

    _NODE_CTOR(Parameter);

    void Parameter::toString(Formatter &fmt) const
    {
        if (Annotations) {
            fmt << Annotations << " ";
        }

        if (Const) {
            fmt << "const ";
        }
        Type.toString(fmt);
        fmt << Kind << " ";
        Name.toString(fmt);
    }

    void Parameter::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        int off{0};
        if (ast.nodes[off]->original_tag == "annotations"_) {
            Annotations = AnnotationList{ast.nodes[off++]};
        }

        if (ast.nodes[off]->original_tag == "const"_) {
            Const = true;
            off++;
        }
        Type = Generic{ast.nodes[off++]};
        if (ast.nodes[off]->original_tag == "typemode"_) {
            Kind = ast.nodes[off++]->token;
        }
        Name = Ident{ast.nodes[off]};
    }

    void Parameter::buildParameters(Vec<Parameter>& params, const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.tag == "param"_) {
            params.push_back(Parameter{asw});
        }
        else {
            for (const auto& n: ast.nodes) {
                params.push_back(Parameter{n});
            }
        }
    }

    _NODE_CTOR(Method)

    void Method::RType::toString(Formatter& fmt) const
    {
        if (Const) {
            fmt << "const ";
        }
        Type.toString(fmt);
        if (!Kind.empty()) {
            fmt << Kind;
        }
    }

    void Method::toString0(Formatter &fmt) const
    {
        if (Annotations) {
            fmt << Annotations;
            Line(fmt);
        }

        ReturnType.toString(fmt);

        fmt << ' ';
        Name.toString(fmt);

        signature(fmt);
    }

    void Method::signature(Formatter& fmt) const
    {
        fmt << '(';
        for (const auto& param: Params) {
            if (&param != &Params.front()) {
                fmt << ", ";
            }
            fmt << param;
        }
        fmt << ")";
        if (Const) {
            fmt << " const";
        }
    }


    void Method::toString(Formatter &fmt) const
    {
        toString0(fmt);
    }

    void Method::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        int off{0};
        if (ast.nodes[off]->original_tag == "annotations"_) {
            Annotations = AnnotationList{ast.nodes[off++]};
        }

        if (ast.nodes[off]->original_tag == "const"_) {
            ReturnType.Const = true;
            off++;
        }
        ReturnType.Type = Generic{ast.nodes[off++]};
        if (ast.nodes[off]->original_tag == "typemode"_) {
            ReturnType.Kind = ast.nodes[off++]->token;
        }
        Name = Ident{ast.nodes[off++]};

        if (ast.nodes.size() > off and ast.nodes[off]->original_tag == "params"_) {
            Parameter::buildParameters(Params, ast.nodes[off++]);
        }

        if (ast.nodes.size() > off) {
            Const = true;
        }
    }

    _NODE_CTOR(Constructor)

    void Constructor::toString(Formatter &fmt) const
    {
        if (Annotations) {
            fmt << Annotations;
            Line(fmt);
        }

        Name.toString(fmt);
        fmt << '(';
        for (const auto& param: Params) {
            if (&param != &Params.front()) {
                fmt << ", ";
            }
            fmt << param;
        }
        fmt << ")";
    }

    void Constructor::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.is_token) {
            Name = Ident{ast};
            return;
        }

        int off{0};
        if (ast.nodes[off]->original_tag == "annotations"_) {
            Annotations = AnnotationList{ast.nodes[off++]};
        }

        Name = Ident{ast.nodes[off++]};
        if (ast.nodes.size() > off) {
            Parameter::buildParameters(Params, ast.nodes[off]);
        }
    }

    _NODE_CTOR(Base);

    void Base::toString(Formatter &fmt) const
    {
        if (!Modifier.empty()) {
            fmt << Modifier << ' ';
        }
        Type.toString(fmt);
    }

    void Base::toString(Formatter& fmt, const Vec<Base>& bases)
    {
        for (const auto& base: bases) {
            if (&base != &bases.front()) {
                fmt << ", ";
            }
            base.toString(fmt);
        }
    }

    void Base::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.tag != "base"_) {
            Type = Generic{asw};
        }
        else {
            Modifier = ast.nodes[0]->token;
            Type = Generic{ast.nodes[1]};
        }
    }

    void Base::buildBases(Vec<Base>& bases, const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.is_token or ast.tag != "bases"_) {
            bases.push_back(Base{asw});
        }
        else {
            for (const auto& n: ast.nodes) {
                bases.push_back(Base{n});
            }
        }
    }

    void Type::toString(Formatter &fmt) const
    {
        if (is<Class>()) {
            fmt << "class ";
        }
        else if (is<Struct>()) {
            fmt << (as<Struct>().IsUnion? "union ": "struct ");
        }
        else {
            fmt << "enum ";
        }

        if (Generators) {
            fmt << Generators << " ";
        }
        if (Annotations) {
            fmt << Annotations << " ";
        }

        fmt << Name << ' ';

        if (is<Class>()) {
            auto& klass = as<Class>();
            if (!klass.BaseClasses.empty()) {
                fmt << ": ";
                for (const auto& base: klass.BaseClasses) {
                    if (&base != &klass.BaseClasses.front()) {
                        fmt << ", ";
                    }
                    base.toString(fmt);
                }
                fmt << ' ';
            }
        }
        fmt << '{';
        ++fmt;
        for(const auto& node: Members) {
            Line(fmt);
            node->toString(fmt);
            if (is<Enum>() and node->is<EnumMember>()) {
                fmt << ",";
            }
            else if (node->is<Constructor>() || node->is<Field>() || node->is<Method>()) {
                fmt << ";";
            }
        }

        Line(--fmt) << "};";
    }

    Type::Type()
        : Node(typeid(Type).hash_code())
    {}

    Type::Type(std::size_t id)
        : Node(id)
    {}

    void Type::parseGenAnno(const peg::Ast& ast)
    {
        auto buildGenAnno = [&](const peg::Ast& node) {
            if (node.tag == "generator"_) {
                Generators._generators.push_back(Generator{node.nodes[1]});
            }
            else {
                if (!Annotations) {
                    Annotations = AnnotationList{node};
                }
                else {
                    Annotations.fromAst(node);
                }
            }
        };

        if (ast.tag == "genanno"_) {
            for (auto& node: ast.nodes) {
                buildGenAnno(*node);
            }
        }
        else {
            buildGenAnno(ast);
        }
    }

    GeneratorAttribute::GeneratorAttribute(const AstWrapper &asw)
        : Attribute(asw)
    {
        Tag = typeid(GeneratorAttribute).hash_code();
    }

    GeneratorAttribute::GeneratorAttribute()
    {
        Tag = typeid(GeneratorAttribute).hash_code();
    }

    Class::Class(const AstWrapper& asw)
        : Type(typeid(Class).hash_code())
    {
        fromAst(asw);
    }

    void Class::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        int off{1};
        if (ast.nodes[off]->original_tag == "genanno"_) {
            parseGenAnno(*ast.nodes[off++]);
        }

        Name = Ident{ast.nodes[off++]};
        if (ast.nodes[off]->original_tag == "bases"_) {
            Base::buildBases(BaseClasses, ast.nodes[off++]);
        }
        if (ast.nodes.size() == off) {
            return;
        }
        auto buildMember = [this](const peg::Ast& member) {
            switch (member.tag) {
                case "modifier"_:
                case "encapsul"_:
                    Members.push_back(std::make_shared<Modifier>(member));
                    break;
                case "comment"_:
                case "blockcomment"_:
                case "lcommentdetails"_:
                    Members.push_back(std::make_shared<Comment>(member));
                    break;
                case "native"_:
                    Members.push_back(std::make_shared<Native>(member));
                    break;
                case "field"_:
                    Members.push_back(std::make_shared<Field>(member));
                    break;
                case "method"_:
                    Members.push_back(std::make_shared<Method>(member));
                    break;
                case "nestedenum"_:
                    Members.push_back(std::make_shared<Enum>(member));
                    break;
                case "nestedstruct"_:
                    Members.push_back(std::make_shared<Struct>(member));
                    break;
                case "constructor"_:
                case "ident"_:
                    Members.push_back(std::make_shared<Constructor>(member));
                    break;
                default:
                    throw Exception(member, "unrecognised tag '", member.name, "' in class");
            };
        };

        const auto& members = ast.nodes[off];
        if (members->tag != "members"_) {
            buildMember(*members);
        }
        else {
            for (const auto& member: members->nodes) {
                buildMember(*member);
            }
        }
    }

    Struct::Struct(const AstWrapper& asw)
        : Type(typeid(Struct).hash_code())
    {
        fromAst(asw);
    }

    void Struct::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        IsUnion = ast.nodes[0]->token == "union";
        int off{1};
        if (ast.nodes[off]->original_tag == "genanno"_ || ast.nodes[off]->original_tag == "annotations"_) {
            parseGenAnno(*ast.nodes[off++]);
        }

        Name = Ident{ast.nodes[off++]};
        if (ast.nodes.size() == off) {
            // empty struct
            return;
        }

        auto buildMember = [this](const peg::Ast& member) {
            switch (member.tag) {
                case "comment"_:
                case "blockcomment"_:
                case "lcommentdetails"_:
                    Members.push_back(std::make_shared<Comment>(member));
                    break;
                case "native"_:
                    Members.push_back(std::make_shared<Native>(member));
                    break;
                case "field"_:
                    Members.push_back(std::make_shared<Field>(member));
                    break;
                case "nestedenum"_:
                    Members.push_back(std::make_shared<Enum>(member));
                    break;
                case "nestedstruct"_:
                    Members.push_back(std::make_shared<Struct>(member));
                    break;
                default:
                    throw Exception(member, " unexpected tag '", member.name, "' in struct");
            };
        };

        const auto& members = ast.nodes[off];
        if (members->tag != "fields"_) {
            buildMember(*members);
        }
        else {
            for (const auto& member: members->nodes) {
                buildMember(*member);
            }
        }
    }

    Enum::Enum(const AstWrapper& asw)
        : Type(typeid(Enum).hash_code())
    {
        fromAst(asw);
    }

    void Enum::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        int off{1};

        if (ast.nodes[off]->original_tag == "genanno"_) {
            parseGenAnno(*ast.nodes[off++]);
        }

        Name = Ident{ast.nodes[off++]};
        if (ast.nodes.size() == off) {
            return;
        }

        if (ast.nodes[off]->original_tag == "ident"_) {
            Base = Ident{ast.nodes[off++]};
        }
        if (ast.nodes.size() == off) {
            return;
        }

        auto buildMember = [this](const peg::Ast& member) {
            switch (member.tag) {
                case "comment"_:
                case "blockcomment"_:
                case "lcommentdetails"_:
                    Members.push_back(std::make_shared<Comment>(member));
                    break;
                case "native"_:
                    Members.push_back(std::make_shared<Native>(member));
                    break;
                case "ident"_:
                case "enummember"_:
                    Members.push_back(std::make_shared<EnumMember>(member));
                    break;
                default:
                    throw Exception(member, "unexpected tag '", member.tag, "' in enum");
            };
        };

        const auto& members = ast.nodes[off];
        if (members->tag != "enumcontent"_) {
            buildMember(*members);
        }
        else {
            for (const auto& member: members->nodes) {
                buildMember(*member);
            }
        }
    }

    _NODE_CTOR(EnumMember)

    void EnumMember::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.tag == "ident"_) {
            Name = Ident{ast};
            return;
        }

        int off{0};
        if (ast.nodes[off]->original_tag == "annotations"_) {
            Annotations = AnnotationList{ast.nodes[off++]};
        }

        // get the name of the member
        Name = Ident{ast.nodes[off++]};
        if (off < ast.nodes.size()) {
            // enum assigned value
            Value = ast.nodes[off]->token;
        }
    }

    void EnumMember::toString(Formatter& fmt) const
    {
        if (Annotations) {
            fmt << Annotations;
            Line(fmt);
        }

        fmt << Name;
        if (!Value.empty()) {
            fmt << " = " << Value;
        }
    }

    _NODE_CTOR(Include)

    void Include::toString(Formatter &fmt) const
    {
        Line(fmt) << "#include " << Left << Header << Right;
    }

    void Include::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.nodes[1]->original_tag == "str"_) {
            Left = '"';
            Right = '"';
        }
        else {
            Left = '<';
            Right = '>';
        }
        Header = ast.nodes[1]->token;
    }

    _NODE_CTOR(Symbol)

    void Symbol::toString(Formatter &fmt) const
    {
        Line(fmt) << "#ifndef IOD_SYMBOL_" << Name;
        Line(fmt) << "#define IOD_SYMBOL_" << Name;
        Line(++fmt) << "iod_define_symbol(" << Name << ")";
        Line(--fmt) << "#endif";
    }

    void Symbol::fromAst(const AstWrapper& asw)
    {
        Name = asw().nodes[1]->token;
    }

    _NODE_CTOR(Library)

    void Library::toString(Formatter &fmt) const
    {
        Line(fmt) << "#pragma load ";
        Name.toString(fmt);
        if (Path.empty()) {
            fmt << " \"" << Path << '"';
        }
    }

    void Library::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        Name = Ident{ast.nodes[1]};
        if (ast.nodes.size() == 3) {
            Path = ast.nodes[2]->token;
        }
    }

    Section::Section()
            : Node(typeid(Section).hash_code())
    {}

    Section::Section(std::size_t hash)
            : Node(hash)
    {}

    void Section::toString(Formatter& fmt) const
    {
        for (const auto& node: Content) {
            node->toString(fmt);
        }
        Line(fmt);
    }

    bool Section::empty() const
    {
        return Content.empty();
    }

    Namespace::Namespace(const AstWrapper& ast)
            : Section(typeid(Namespace).hash_code())
    {
        fromAst(ast);
    }

    Namespace::Namespace()
        : Section(typeid(Namespace).hash_code())
    {}

    void Namespace::toString(Formatter &fmt) const
    {
        Line(fmt) << "namespace ";
        Name.toString(fmt);
        fmt << " {";
        Line(++fmt);
        for (const auto& node: Content) {
            node->toString(fmt);
        }
        Line(--fmt) << "}";
        Line(fmt);
    }

    void Namespace::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        if (ast.is_token or ast.tag != "namespace"_) {
            Name = Scoped{ast};
            return;
        }

        Name = Scoped{ast.nodes[1]};
        auto buildContent = [this](const peg::Ast& content) {
            switch (content.tag) {
                case "comment"_:
                case "lcommentdetails"_:
                case "blockcomment"_:
                    Content.push_back(std::make_shared<Comment>(content));
                    break;
                case "native"_:
                    Content.push_back(std::make_shared<Native>(content));
                    break;
                case "struct"_:
                    Content.push_back(std::make_shared<Struct>(content));
                    break;
                case "class"_:
                    Content.push_back(std::make_shared<Class>(content));
                    break;
                case "variable"_:
                    mVars.add(content);
                    break;
                case "invokecmd"_:
                case "invoke"_:
                    Content.push_back(std::make_shared<Invoke>(content));
                    break;
                case "enum"_:
                    Content.push_back(std::make_shared<Enum>(content));
                    break;
                default:
                    astUnrecognisedTag(content);
                    break;
            }
        };

        const auto& contents = ast.nodes[2];
        if (contents->tag != "nscontent"_) {
            buildContent(*contents);
        }
        else {
            for (const auto& content: contents->nodes) {
                buildContent(*content);
            }
        }
    }

    bool Namespace::empty() const
    {
        return Section::empty() or Name.Parts.empty();
    }

    Namespace::operator bool() const
    {
        return !Name.Parts.empty();
    }

    Before::Before()
        : Section(typeid(Before).hash_code())
    {}

    Before::Before(const AstWrapper& asw)
        : Section(typeid(Before).hash_code())
    {
        fromAst(asw);
    }

    void Before::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        auto buildContent = [this](const peg::Ast& content) {
            switch (content.tag) {
                case "lcommentdetails"_:
                case "blockcomment"_:
                    Content.push_back(std::make_shared<Comment>(content));
                    break;
                case "include"_:
                    Content.push_back(std::make_shared<Include>(content));
                    break;
                case "load"_:
                    Content.push_back(std::make_shared<Library>(content));
                    break;
                case "symbol"_:
                    Content.push_back(std::make_shared<Symbol>(content));
                    break;
                case "native"_:
                    Content.push_back(std::make_shared<Native>(content));
                    break;
                case "variable"_:
                    mVars.add(content);
                    break;
                case "invokecmd"_:
                case "invoke"_:
                    Content.push_back(std::make_shared<Invoke>(content));
                    break;
                default:
                    break;
            }
        };

        if (ast.tag != "before"_) {
            buildContent(ast);
        }
        else {
            for(const auto& node: ast.nodes) {
                buildContent(*node);
            }
        }
    }

    After::After()
        : Section(typeid(After).hash_code())
    {}

    After::After(const AstWrapper& asw)
        : Section(typeid(After).hash_code())
    {
        fromAst(asw);
    }

    void After::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        auto buildContent = [this](const peg::Ast& content) {
            switch (content.tag) {
                case "lcommentdetails"_:
                case "blockcomment"_:
                    Content.push_back(std::make_shared<Comment>(content));
                    break;
                case "native"_:
                    Content.push_back(std::make_shared<Native>(content));
                case "variable"_:
                    mVars.add(content);
                    break;
                case "invokecmd"_:
                case "invoke"_:
                    Content.push_back(std::make_shared<Invoke>(content));
                    break;
                default:
                    break;
            }
        };

        if (ast.tag != "after"_) {
            buildContent(ast);
        }
        else {
            for(const auto& node: ast.nodes) {
                buildContent(*node);
            }
        }
    }

    _NODE_CTOR(Program);

    void Program::toString(Formatter &fmt) const
    {
        Line(fmt);
        before.toString(fmt);
        Line(fmt);
        space.toString(fmt);
        Line(fmt);
        after.toString(fmt);
    }

    void Program::fromAst(const AstWrapper& asw)
    {
        const auto& ast = asw();
        Node::_sPath = ast.path;

        auto buildSection = [this](const peg::Ast& content) {
            switch (content.tag) {
                case "namespace"_:
                    space = scc::Namespace{content};
                    break;
                case "before"_:
                    before = scc::Before{content};
                    break;
                case "after"_:
                    after = scc::After{content};
                    break;
                default:
                    if (content.original_tag == "after"_) {
                        after = scc::After{content};
                    }
                    else {
                        before = scc::Before{content};
                    }
                    break;
            };
        };

        if (ast.tag != "program"_) {
            buildSection(ast);
        }
        else {
            for (const auto& node: ast.nodes) {
                buildSection(*node);
            }
        }
    }

    Program::operator bool() const
    {
        return !before.empty() || !space.empty() || !after.empty();
    }
}