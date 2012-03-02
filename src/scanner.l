

%{
#include <string.h>
#include "data.h"
#include "parser.h"
%}

%option yylineno
%option noyywrap

%%
[ \t\n]       ;
"<Settings>"      { return START_SETTINGS_SECTION; }
"</Settings>"     { return END_SETTINGS_SECTION; }
"<Value>"         { return START_VALUE_SECTION; }
"</Value>"        { return END_VALUE_SECTION; }
"<Table>"         { return START_TABLE_SECTION; }
"</Table>"        { return END_TABLE_SECTION; }
"Type"            { return TYPE_VALUE; }
"Name "           { return NAME_VALUE;}
"OID"             { return OID_VALUE;}
"Column"          { return COLUMN_VALUE; }
"Integer32"       { return PTYPE_INTEGER32; }
"Counter32"       { return PTYPE_COUNTER32; }
"Gauge32"         { return PTYPE_GAUGE32; }
"Counter64"       { return PTYPE_COUNTER64; }
"CollectdAddress" { return COLLECTD_ADDRESS_SETTING; }
"AgentxAddress"   { return AGENTX_ADDRESS_SETTING; }
"MasterAgent"     { return MASTER_AGENT_SETTING; }
"true"       { return T_TRUE; }
"false"      { return T_FALSE; }
[0-9]+            { yylval.i = atoi(yytext); return INT; }
[a-zA-Z0-9_\*\.\/-]+    { yylval.str = strdup(yytext); return STRING; }
#.*           ;
.           ;
%%


