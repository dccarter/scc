#ifdef __dclparser
__dclparser(Ident);
__dclparser(Attrib);
__dclparser(Type);
__dclparser(Generic);
__dclparser(Attrib);
__dclparser(Attribs);
__dclparser(Modifier);
__dclparser(Field);
__dclparser(FieldDcls);
__dclparser(Param);
__dclparser(ParamDcls);
__dclparser(Func);
__dclparser(FuncDcls);
__dclparser(Struct);
__dclparser(Class);
__dclparser(Scc);
__dclparser(Symbol);
__dclparser(Symbols);
__dclparser(Include);
__dclparser(Includes);
__dclparser(Load);
__dclparser(Loads);
__dclparser(Namespace);
__dclparser(Program);
#elif defined(__initparser)
__initparser(Ident,         "ident");
__initparser(Attrib,        "attrib");
__initparser(Attribs,       "attribs");
__initparser(Type,          "type");
__initparser(Generic,       "generic");
__initparser(Attribs,       "attribs");
__initparser(Modifier,      "modifier");
__initparser(Field,         "field");
__initparser(FieldDcls,     "fieldDcls");
__initparser(Param,         "param");
__initparser(ParamDcls,     "paramDcls");
__initparser(Func,          "func");
__initparser(FuncDcls,      "funcDcls");
__initparser(Struct,        "struct");
__initparser(Class,         "class");
__initparser(Symbol,        "symbol");
__initparser(Symbols,       "symbols");
__initparser(Include,       "include");
__initparser(Includes,      "includes");
__initparser(Load,          "load");
__initparser(Loads,         "loads");
__initparser(Namespace,     "namespace");
__initparser(Program,       "program");
#else
Ident,
Attrib,
Attribs,
Type,
Generic,
Attribs,
Modifier,
Field,
FieldDcls,
Param,
ParamDcls,
Func,
FuncDcls,
Struct,
Class,
Symbol,
Symbols,
Include,
Includes,
Load,
Loads,
Namespace,
Program
#endif
