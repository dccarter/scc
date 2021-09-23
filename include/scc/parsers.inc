//
// Created by dc on 24/12/18.
//

#if defined(__PARSER_DECLARE)
constexpr int __count() const {return __LINE__; }
__PARSER_DECLARE(Ident);
__PARSER_DECLARE(Line);
__PARSER_DECLARE(CommentOpen);
__PARSER_DECLARE(CommentClose);
__PARSER_DECLARE(CommentBlock);
__PARSER_DECLARE(Comment);
__PARSER_DECLARE(NativeOpen);
__PARSER_DECLARE(NativeClose);
__PARSER_DECLARE(NativeBlock);
__PARSER_DECLARE(Native);
__PARSER_DECLARE(Scoped);
__PARSER_DECLARE(Generic);
__PARSER_DECLARE(Attrib);
__PARSER_DECLARE(Attribs);
__PARSER_DECLARE(Modifier);
__PARSER_DECLARE(Field);
__PARSER_DECLARE(Fields);
__PARSER_DECLARE(Param);
__PARSER_DECLARE(Params);
__PARSER_DECLARE(Method);
__PARSER_DECLARE(Ctor);
__PARSER_DECLARE(Members);
__PARSER_DECLARE(Base);
__PARSER_DECLARE(Bases);
__PARSER_DECLARE(Struct);
__PARSER_DECLARE(Class);
__PARSER_DECLARE(Types);
__PARSER_DECLARE(Namespace);
__PARSER_DECLARE(Symbol);
__PARSER_DECLARE(Include);
__PARSER_DECLARE(Load);
__PARSER_DECLARE(Outside);
__PARSER_DECLARE(Program);
__PARSER_DECLARE(D0);
__PARSER_DECLARE(D1);
__PARSER_DECLARE(D2);
__PARSER_DECLARE(D3);
__PARSER_DECLARE(D4);
__PARSER_DECLARE(D5);
__PARSER_DECLARE(D6);
__PARSER_DECLARE(D7);
__PARSER_DECLARE(D8);
__PARSER_DECLARE(D9);
constexpr int count() const { return __LINE__ - __count() - 1; }
#elif defined(__PARSER_INITIALIZE)
__PARSER_INITIALIZE(Ident,                  "ident");
__PARSER_INITIALIZE(Line,                  "line");
__PARSER_INITIALIZE(CommentOpen,            "commentopen");
__PARSER_INITIALIZE(CommentClose,           "commentclose");
__PARSER_INITIALIZE(CommentBlock,           "commentblock");
__PARSER_INITIALIZE(Comment,                "comment");
__PARSER_INITIALIZE(NativeOpen,             "nativeopen");
__PARSER_INITIALIZE(NativeClose,            "nativeclose");
__PARSER_INITIALIZE(NativeBlock,            "nativeblock");
__PARSER_INITIALIZE(Native,                 "native");
__PARSER_INITIALIZE(Scoped,                 "scoped");
__PARSER_INITIALIZE(Generic,                "generic");
__PARSER_INITIALIZE(Attrib,                 "attrib");
__PARSER_INITIALIZE(Attribs,                "attribs");
__PARSER_INITIALIZE(Modifier,               "modifier");
__PARSER_INITIALIZE(Field,                  "field");
__PARSER_INITIALIZE(Fields,                 "fields");
__PARSER_INITIALIZE(Param,                  "param");
__PARSER_INITIALIZE(Params,                 "params");
__PARSER_INITIALIZE(Method,                 "method");
__PARSER_INITIALIZE(Ctor,                   "ctor");
__PARSER_INITIALIZE(Members,                "members");
__PARSER_INITIALIZE(Base,                   "base");
__PARSER_INITIALIZE(Bases,                  "bases");
__PARSER_INITIALIZE(Struct,                 "struct");
__PARSER_INITIALIZE(Class,                  "class");
__PARSER_INITIALIZE(Types,                  "types");
__PARSER_INITIALIZE(Namespace,              "namespace");
__PARSER_INITIALIZE(Symbol,                 "symbol");
__PARSER_INITIALIZE(Include,                "include");
__PARSER_INITIALIZE(Load,                   "load");
__PARSER_INITIALIZE(Outside,                "outer");
__PARSER_INITIALIZE(Program,                "program");
__PARSER_INITIALIZE(D0,                     "d0");
__PARSER_INITIALIZE(D1,                     "d1");
__PARSER_INITIALIZE(D2,                     "d2");
__PARSER_INITIALIZE(D3,                     "d3");
__PARSER_INITIALIZE(D4,                     "d4");
__PARSER_INITIALIZE(D5,                     "d5");
__PARSER_INITIALIZE(D6,                     "d6");
__PARSER_INITIALIZE(D7,                     "d7");
__PARSER_INITIALIZE(D8,                     "d8");
__PARSER_INITIALIZE(D9,                     "d9");
#elif defined(__PARSER_ASSIGN)
__USE_PARSER(Ident)     = __PARSER_ASSIGN(Ident);
__USE_PARSER(Line)      = __PARSER_ASSIGN(Line);
__USE_PARSER(CommentOpen)    = __PARSER_ASSIGN(CommentOpen);
__USE_PARSER(CommentClose)   = __PARSER_ASSIGN(CommentClose);
__USE_PARSER(CommentBlock)   = __PARSER_ASSIGN(CommentBlock);
__USE_PARSER(Comment)   = __PARSER_ASSIGN(Comment);
__USE_PARSER(NativeOpen)     = __PARSER_ASSIGN(NativeOpen);
__USE_PARSER(NativeClose)    = __PARSER_ASSIGN(NativeClose);
__USE_PARSER(NativeBlock)    = __PARSER_ASSIGN(NativeBlock);
__USE_PARSER(Native)    = __PARSER_ASSIGN(Native);
__USE_PARSER(Scoped)    = __PARSER_ASSIGN(Scoped);
__USE_PARSER(Generic)   = __PARSER_ASSIGN(Generic);
__USE_PARSER(Attrib)    = __PARSER_ASSIGN(Attrib);
__USE_PARSER(Attribs)   = __PARSER_ASSIGN(Attribs);
__USE_PARSER(Modifier)  = __PARSER_ASSIGN(Modifier);
__USE_PARSER(Field)     = __PARSER_ASSIGN(Field);
__USE_PARSER(Fields)    = __PARSER_ASSIGN(Fields);
__USE_PARSER(Param)     = __PARSER_ASSIGN(Param);
__USE_PARSER(Params)    = __PARSER_ASSIGN(Params);
__USE_PARSER(Method)    = __PARSER_ASSIGN(Method);
__USE_PARSER(Ctor)      = __PARSER_ASSIGN(Ctor);
__USE_PARSER(Members)   = __PARSER_ASSIGN(Members);
__USE_PARSER(Base)      = __PARSER_ASSIGN(Base);
__USE_PARSER(Bases)     = __PARSER_ASSIGN(Bases);
__USE_PARSER(Struct)    = __PARSER_ASSIGN(Struct);
__USE_PARSER(Class)     = __PARSER_ASSIGN(Class);
__USE_PARSER(Types)     = __PARSER_ASSIGN(Types);
__USE_PARSER(Namespace) = __PARSER_ASSIGN(Namespace);
__USE_PARSER(Symbol)    = __PARSER_ASSIGN(Symbol);
__USE_PARSER(Include)   = __PARSER_ASSIGN(Include);
__USE_PARSER(Load)      = __PARSER_ASSIGN(Load);
__USE_PARSER(Outside)   = __PARSER_ASSIGN(Outside);
__USE_PARSER(Program)   = __PARSER_ASSIGN(Program);
__USE_PARSER(D0)        = __PARSER_ASSIGN(D0);
__USE_PARSER(D1)        = __PARSER_ASSIGN(D1);
__USE_PARSER(D2)        = __PARSER_ASSIGN(D2);
__USE_PARSER(D3)        = __PARSER_ASSIGN(D3);
__USE_PARSER(D4)        = __PARSER_ASSIGN(D4);
__USE_PARSER(D5)        = __PARSER_ASSIGN(D5);
__USE_PARSER(D6)        = __PARSER_ASSIGN(D6);
__USE_PARSER(D7)        = __PARSER_ASSIGN(D7);
__USE_PARSER(D8)        = __PARSER_ASSIGN(D8);
__USE_PARSER(D9)        = __PARSER_ASSIGN(D9);
#else
__USE_PARSER(Ident),
__USE_PARSER(Line),
__USE_PARSER(CommentOpen),
__USE_PARSER(CommentClose),
__USE_PARSER(CommentBlock),
__USE_PARSER(Comment),
__USE_PARSER(NativeOpen),
__USE_PARSER(NativeClose),
__USE_PARSER(NativeBlock),
__USE_PARSER(Native),
__USE_PARSER(Scoped),
__USE_PARSER(Generic),
__USE_PARSER(Attrib),
__USE_PARSER(Attribs),
__USE_PARSER(Modifier),
__USE_PARSER(Field),
__USE_PARSER(Fields),
__USE_PARSER(Param),
__USE_PARSER(Params),
__USE_PARSER(Method),
__USE_PARSER(Ctor),
__USE_PARSER(Members),
__USE_PARSER(Base),
__USE_PARSER(Bases),
__USE_PARSER(Struct),
__USE_PARSER(Class),
__USE_PARSER(Types),
__USE_PARSER(Namespace),
__USE_PARSER(Symbol),
__USE_PARSER(Include),
__USE_PARSER(Load),
__USE_PARSER(Outside),
__USE_PARSER(Program),
__USE_PARSER(D0),
__USE_PARSER(D1),
__USE_PARSER(D2),
__USE_PARSER(D3),
__USE_PARSER(D4),
__USE_PARSER(D5),
__USE_PARSER(D6),
__USE_PARSER(D7),
__USE_PARSER(D8),
__USE_PARSER(D9)
#endif
