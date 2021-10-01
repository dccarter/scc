//
// Created by Mpho Mbotho on 2020-10-29.
//

#include <scc/parser.hpp>
#include <scc/generator.hpp>
#include <scc/astwrapper.hpp>
#include <scc/exception.hpp>

#include <fstream>

namespace {

    static const std::string BUILTIN_GRAMMAR = R"(
program         <- _ before? namespace? after?
namespace       <- namespacekey _ scoped _ '{' _ nscontent? _ '}' _
namespacekey    <- 'namespace'
nscontent       <- (variable / class / struct / native / comment / invoke / enum)+
enum            <- enumkey _ genanno? _ ident (_ ':' _ ident)? _ '{' _ enumcontent? _ '}' sp ';' _
nestedenum      <- enumkey _ annotations? _ ident (_ ':' _ ident)? _ '{' _ enumcontent? _ '}' sp ';' _
enumcontent     <- (comment / (enummember sp ',' _))* enummember comment?
enummember      <- annotations? _ ident (_ '=' _ int)?
enumkey         <- 'enum'
class           <- classkey _ genanno? _ ident (_ ':' _ bases)? _ '{' _ members? _ '}' sp ';' _
classkey        <- 'class'
struct          <- structkey _ genanno? _ ident _ '{' _ fields? _ '}' sp ';' _
nestedstruct    <- structkey _ annotations? _ ident _ '{' _ fields? _ '}' sp ';' _
structkey       <- 'struct' / 'union'
members         <- (modifier / constructor / method / field / nestedstruct / nestedenum / comment / native)+
fields          <- (field / comment / native / nestedstruct / nestedenum)+
bases           <- base (_ ',' _ base)*
base            <- encapsul? sp generic
modifier        <- encapsul sp ':' _
encapsul        <- 'public' / 'protected' / 'private'
method          <- annotations? _ (const sp)? generic typemode? _ ident _ '(' (_ params)? _ ')' (sp const)? sp ';' _
constructor     <- annotations? _ ident _ '(' (_ params)? _ ')' sp ';' _
params          <- param (_ ',' _ param)*
param           <- (annotations _)? (const sp)? generic typemode? _ ident
field           <- annotations? _ const? sp generic typemode? sp ident (sp fieldvalue)? sp ';' _
fieldvalue      <- '{' literal '}'
generic         <- scoped '<' sp generic? (sp ',' sp generic)* '>' / scoped
genanno         <- ((annotation / generator) _)+
annotations     <- annotation (_ annotation)*
annotation      <- (annotidx / annotgen) (_ comments)?
annotgen        <- '[[' _ annotnamegen ( '(' _ annotgenparams _ ')' ) _ ']]'
annotidx        <- '[[' _ annotnameidx ( '(' _ annotidxparams _ ')' )? _ ']]'
annotnameidx    <- annottag ident '::' ident
annotnamegen    <- annottag ident
annottag        <- '$'
annotidxparams  <- literal (sp ',' (_ comments)? _ literal)* (_ comments)?
annotgenparams  <- annotgenparam (sp ',' (_ comments)? _ annotgenparam)* (_ comments)?
annotgenparam   <- ident sp '=' sp literal
generator       <- '[[' gentag '(' _ gennames _ ')' _ ']]' (_ comments)?
gennames        <- genname (',' _ genname)*
genname         <- ident ('/' ident)?
gentag          <- 'gen'
scoped          <- ident ('::' ident)*
before          <- (variable / include / comment / symbol / load / native / invoke) +
after           <- (variable / native / comment / invoke)+
load            <- '#pragma' sp loadkey sp ident (sp str)? _
loadkey         <- 'load'
symbol          <- '#pragma' sp symbolkey sp ident _
symbolkey       <- 'symbol'
invoke          <- '#pragma' sp 'invoke' cpp? sp invokecmd sp '(' sp (ident / kvps)? sp ')' _
invokecmd       <- ident '::' ident '.' ident
variable        <- '#pragma' sp 'var' sp ident sp kvps _
kvps            <- '{' _ kvp (sp ',' _ kvp)* _ '}'
kvp             <- ident sp ':' sp literal
include         <- includekey sp (str / include0) _
include0        <- < [<] <(![<>] .)* > [>] >
includekey      <- '#include'
literal         <- numext / null / bool / number / string / char
char            <- < ['] < escaped / (!['] .) > ['] >
escaped         <- [\\] ['"?\\abfnrtv]
null            <- 'nullptr'
bool            <- 'true' / 'false'
numext          <- <number '_' ident>
number          <- exp / float / hex / bin / oct / int
exp             <- < (float / oct / int) [eE] int >
bin             <- < [-+]? '0' [bB] [0-1]+ >
hex             <- < [-+]? '0' [xX] [0-9a-fA-F]+ >
float           <- <int ('.' [0-9]*) >
oct             <- < [-+]? [0] [1-7]+ >
int             <- < [-+]? [1-9][0-9]* > / <[0]>
string          <- str / rawstr
str             <- < ["] <(!["] .)* > ["] >
rawstr          <- 'R"(' < (!rawstrend .)* > rawstrend
rawstrend       <- ')\"'
comments        <- (comment _)+
comment         <- linecomment / blockcomment
blockcomment    <- startcomment commentblock endcomment
linecomment     <- '//' lcommentdetails _
lcommentdetails <- (!nl .)*
startcomment    <- '/*'
commentblock    <- (!endcomment .)*
endcomment      <- '*/' _
native          <- startnative nativeblock endnative
startnative     <- '#pragma' sp 'native' cpp? _
cpp             <- '[' sp 'cpp' sp ']'
nativeblock     <- (!endnative .)*
endnative       <- '#pragma' sp 'endnative' _
typemode        <-  '&&' / '&' / '*'
const           <- 'const'
ident           <- < [a-zA-Z_$] [a-zA-Z0-9_$]* >
~nl             <- '\r\n' / '\r' / '\n' / !.
~sp             <- [ \t]*
~_              <- [ \t\r\n]*
)";

bool readFile(const std::filesystem::path& path, std::vector<char>& buf)
{
    std::ifstream ifs;
    ifs.exceptions(ifs.exceptions()|std::ios::failbit);
    try {
        ifs.open(path, std::ios::in);
        auto size = static_cast<size_t>(ifs.seekg(0, std::ios::end).tellg());
        buf.resize(size);
        ifs.seekg(0, std::ios::beg).read(&buf[0], size);
        return true;
    }
    catch (std::ios_base::failure& ex) {
        error() << "reading file '" << path << "' failed: " << ex.what() << "\n";
        return false;
    }
}

std::vector<char> loadGrammar(const std::filesystem::path& path)
{
    std::vector<char> buf{};
    if (path.empty() || !std::filesystem::exists(path)) {
        // use built in grammar
        std::copy(BUILTIN_GRAMMAR.begin(), BUILTIN_GRAMMAR.end(), std::back_inserter(buf));
    }
    else {
        readFile(path, buf);
    }

    return buf;
}

}

namespace scc {

    bool Parser::load(const std::filesystem::path& path)
    {
        auto grammar = loadGrammar(path);
        if (grammar.empty()) {
            error() << "grammar is empty\n";
            return false;
        }

        P = std::make_shared<peg::parser>(&grammar[0], grammar.size());
        if (!(*P)) {
            error() << "loading parser grammar failed";
            return false;
        }
        P->enable_ast();
        return true;
    }

    void Parser::repl()
    {
        if (*(P)) {
            P->log = [](size_t line, size_t col, const std::string& msg) {
                error() << "<stdin>:" << line << ": " << msg << std::endl;
            };
        }

        while ((P != nullptr) && (*P)) {
            char *line;
            size_t len{0};
            info() << "> ";
            if (getline(&line, &len, stdin) < 0) {
                // error reading input
                error() << "failed to read input: " << strerror(errno);
                break;
            }

            if (strncmp("exit", line, 4) == 0) {
                /* exit requested */
                free(line);
                break;
            }

            std::shared_ptr<peg::Ast> ast;
            if (P->parse(line, ast)) {
                ast = peg::AstOptimizer(true).optimize(ast);
                std::cout << peg::ast_to_s(ast) << "\n";
            }
            free(line);
        };
    }

    Program Parser::parse(const std::filesystem::path& path)
    {
        if (!std::filesystem::exists(path)) {
            throw Exception("source '", path, "' does not exist");
        }
        std::vector<char> content{};
        if (!readFile(path, content)) {
            return {};
        }
        P->log = [path](size_t line, size_t col, const std::string& msg) {
            error() << path.string() << ":" << line << ": " << msg << std::endl;
        };

        std::shared_ptr<peg::Ast> ast;
        if (P->parse_n(content.data(), content.size(), ast, path.c_str())) {
            ast = peg::AstOptimizer(true).optimize(ast);
            return Program{ast};
        }
        else {
            return {};
        }
    }
}