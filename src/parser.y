
%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <errno.h>

  #include "data.h"
  #include "settings.h"

  #include <net-snmp/agent/snmp_agent.h>

  int yylex(void);
  int yyerror(char *pErrorMsg);

  extern FILE* yyin;
  extern int yylineno;
  extern char *yytext;
  extern char *c_file;
%}

%union {
  char *str;
  int   i;
  snmp_column_t col;
  snmp_table_t  col_list;
  snmp_type_t snmp_type;
  char bool;
}

%token START_VALUE_SECTION
%token NAME_VALUE
%token OID_VALUE
%token <str> STRING
%token <i> INT
%token END_VALUE_SECTION
%token START_TABLE_SECTION
%token END_TABLE_SECTION
%token COLUMN_VALUE
%token TYPE_VALUE
%token INDEX_VALUE
%token PTYPE_INTEGER32
%token PTYPE_COUNTER32
%token PTYPE_GAUGE32
%token PTYPE_COUNTER64
%token START_SETTINGS_SECTION
%token END_SETTINGS_SECTION
%token COLLECTD_ADDRESS_SETTING
%token MASTER_AGENT_SETTING
%token AGENTX_ADDRESS_SETTING
%token T_TRUE
%token T_FALSE

%type <col> column;
%type <col_list> column_list;
%type <snmp_type> snmp_type_entry;
%type <bool> boolean;

%start entire_file

%%
 
snmp_type_entry:
PTYPE_INTEGER32 
{ $$ = ASN_INTEGER; } |
PTYPE_COUNTER32 
{ $$ = ASN_COUNTER; } |
PTYPE_GAUGE32 
{ $$ = ASN_GAUGE; } |
PTYPE_COUNTER64 
{$$ = ASN_COUNTER64; } 

boolean:
T_TRUE { $$ = 1; } |
T_FALSE { $$ = 0; }

setting:
COLLECTD_ADDRESS_SETTING STRING
{
  user_settings.collectd_address = $2;
} |
MASTER_AGENT_SETTING boolean
{
  user_settings.is_master_agent = $2;
} |
AGENTX_ADDRESS_SETTING STRING
{
  user_settings.agentx_address = $2;
}

settings_list:
settings_list setting {} | setting {}

settings_block:
START_SETTINGS_SECTION
settings_list
END_SETTINGS_SECTION

value_block:
START_VALUE_SECTION
NAME_VALUE STRING
OID_VALUE STRING
TYPE_VALUE snmp_type_entry
END_VALUE_SECTION
{
  snmp_value_t new_value;

  new_value.stat_name = $3;
  new_value.oid_str = $5;
  new_value.type = $7;

  value_list.num_values++;
  value_list.values = (snmp_value_t *) realloc(value_list.values, value_list.num_values * sizeof(snmp_value_t));
  value_list.values[value_list.num_values-1] = new_value;
};

column:
COLUMN_VALUE INT STRING snmp_type_entry
{
  $$.col_num = $2;
  $$.name    = $3;
  $$.type    = $4;
}

column_list:
  column_list column 
{
  $$ = $1;
  $$.num_columns++;
  $$.columns = (snmp_column_t*) ( realloc((void*) $$.columns, sizeof(snmp_column_t)*$$.num_columns) );
  $$.columns[$$.num_columns-1] = $2;
}
  | column
{
	bzero(&$$, sizeof($$));
	$$.num_columns = 1;
	$$.columns = malloc(sizeof(snmp_column_t));
	$$.columns[0] = $1;
}

table_block:
START_TABLE_SECTION
OID_VALUE STRING
column_list
END_TABLE_SECTION
{
  table_list.num_tables++;
  table_list.tables = (snmp_table_t *) realloc((void*) table_list.tables, table_list.num_tables * sizeof(snmp_table_t));
  table_list.tables[table_list.num_tables-1] = $4;
  table_list.tables[table_list.num_tables-1].table_oid_str = $3;
};

statement:
value_block | table_block | settings_block {}

statement_list:
statement_list statement | statement {}


entire_file:
statement_list

%%

int yyerror(char *errmsg)
{
  char *text;

  if (*yytext == '\n')
    text = "<newline>";
  else
    text = yytext;


  fprintf (stderr, "Parse error on line %i near `%s': %s\n",
	   yylineno, text, errmsg);

  return 0;
}

int parse_config_file(const char *filename)
{
  FILE *config_file;
  int result = -1;

  config_file = fopen(filename, "r");

  if (config_file == NULL) {
    fprintf(stderr,"Can't open config file %s (%s)\n",filename, strerror(errno));
  }
  else {
    yyin = config_file;
    result = yyparse();
    fclose(config_file);
  }
  
  return result;
}

