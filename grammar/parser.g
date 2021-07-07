program         <- _ before? namespace? after?
namespace       <- namespacekey _ scoped _ '{' _ nscontent? _ '}' _
namespacekey    <- 'namespace'
nscontent       <- (variable / class / struct / native / comment / invoke)+
class           <- classkey _ attribs? _ ident (_ ':' _ bases)? _ '{' _ members? _ '}' sp ';' _
classkey        <- 'class'
struct          <- structkey _ attribs? _ ident _ '{' _ fields? _ '}' sp ';' _
structkey       <- 'struct' / 'union'
members         <- (modifier / constructor / method / field / comment / native)+
fields          <- (field / comment / native)+
bases           <- base (_ ',' _ base)*
base            <- encapsul? sp generic
modifier        <- encapsul sp ':' _
encapsul        <- 'public' / 'protected' / 'private'
method          <- attribs? _ (const sp)? generic typemode? _ ident _ '(' (_ params)? _ ')' (sp const)? sp ';' _
constructor     <- attribs? _ ident _ '(' (_ params)? _ ')' sp ';' _
params          <- param (_ ',' _ param)*
param           <- (const sp)? generic typemode? _ ident
field           <- attribs? _ const? sp generic typemode? sp ident (sp fieldvalue)? sp ';' _
fieldvalue      <- '{' literal '}'
generic         <- scoped '<' sp generic? (sp ',' sp generic)* '>' / scoped
attribs         <- attrib (_ attrib)*
attrib          <- '[[' attribname ('(' sp attribparams? sp ')')? ']]'
attribname      <- ident ('::' ident)?
attribparams    <- ident (sp ',' sp ident)*
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
kvps            <- '{' sp kvp (sp ',' sp kvp)* sp '}'
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
comment         <- linecomment / blockcomment
blockcomment    <- startcomment commentblock endcomment
linecomment     <- '//' lcommentdetails _
lcommentdetails <- (!nl .)*
startcomment    <- '/*' (!nl .)*
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