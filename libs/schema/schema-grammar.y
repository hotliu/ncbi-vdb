/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

%{
    #define YYDEBUG 1

    #include <stdio.h>

    #include "ParseTree.hpp"
    using namespace ncbi::SchemaParser;

    #include "schema-tokens.h"
    #include "schema-lex.h"
    #define Schema_lex SchemaScan_yylex

    void Schema_error ( YYLTYPE *llocp, void* parser, struct SchemaScanBlock* sb, const char* msg )
    {
        /*TODO: send message to the C++ parser for proper display and recovery */
        printf("Line %i pos %i: %s\n", llocp -> first_line, llocp -> first_column, msg);
    }

    extern "C"
    {
        extern enum yytokentype SchemaScan_yylex ( YYSTYPE *lvalp, YYLTYPE *llocp, SchemaScanBlock* sb );
    }

    static
    ParseTree*
    P ( YYSTYPE & p_prod )
    {
        assert ( p_prod . subtree );
        return ( ParseTree * ) p_prod . subtree;
    }

    static
    ParseTree*
    T ( YYSTYPE & p_term )
    {
        assert ( p_term . subtree == 0 );
        return new ParseTree ( p_term );
    }

    /* Create production node */
    static
    ParseTree *
    MakeTree ( int p_token,
               ParseTree * p_ch1 = 0,
               ParseTree * p_ch2 = 0,
               ParseTree * p_ch3 = 0,
               ParseTree * p_ch4 = 0,
               ParseTree * p_ch5 = 0,
               ParseTree * p_ch6 = 0,
               ParseTree * p_ch7 = 0,
               ParseTree * p_ch8 = 0
             )
    {
        SchemaToken v = { p_token, NULL, 0, NULL, NULL };
        ParseTree * ret = new ParseTree ( v );
        if ( p_ch1 != 0 ) ret -> AddChild ( p_ch1 );
        if ( p_ch2 != 0 ) ret -> AddChild ( p_ch2 );
        if ( p_ch3 != 0 ) ret -> AddChild ( p_ch3 );
        if ( p_ch4 != 0 ) ret -> AddChild ( p_ch4 );
        if ( p_ch5 != 0 ) ret -> AddChild ( p_ch5 );
        if ( p_ch6 != 0 ) ret -> AddChild ( p_ch6 );
        if ( p_ch7 != 0 ) ret -> AddChild ( p_ch7 );
        if ( p_ch8 != 0 ) ret -> AddChild ( p_ch8 );
        return ret;
    }

    /* Create a flat list */
    static
    ParseTree *
    MakeList ( YYSTYPE & p_prod )
    {
        SchemaToken v = { PT_ASTLIST, NULL, 0, NULL, NULL };
        ParseTree * ret = new ParseTree ( v );
        ret -> AddChild ( P ( p_prod ) );
        return ret;
    }

    /* Add to a flat list node */
    static
    ParseTree *
    AddToList ( ParseTree * p_root, ParseTree * p_br1, ParseTree * p_br2 = 0 )
    {
        assert ( p_br1 != 0 );
        p_root -> AddChild ( p_br1 );
        if ( p_br2 != 0 )
        {
            p_root -> AddChild ( p_br2 );
        }
        return p_root;
    }


%}

%name-prefix "Schema_"
%parse-param { ParseTree** root }
%param { struct SchemaScanBlock* sb }

%define api.value.type {SchemaToken}

%define parse.error verbose
%locations

%define api.pure full


 /* !!! Keep token declarations in synch with schema-ast.y */

%token END_SOURCE 0 "end of source"

%token UNRECOGNIZED

%token ELLIPSIS
%token INCREMENT

%token DECIMAL
%token OCTAL
%token HEX
%token FLOAT
%token EXP_FLOAT
%token STRING
%token ESCAPED_STRING

%token IDENTIFIER_1_0
%token PHYSICAL_IDENTIFIER_1_0
%token VERSION

/* unterminated strings are dealt with by flex */
%token UNTERM_STRING
%token UNTERM_ESCAPED_STRING

%token VERS_1_0 /* recognized under special flex state */

%token KW___no_header
%token KW___row_length
%token KW___untyped
%token KW_alias
%token KW_column
%token KW_const
%token KW_control
%token KW_database
%token KW_decode
%token KW_default
%token KW_encode
%token KW_extern
%token KW_false
%token KW_fmtdef
%token KW_function
%token KW_include
%token KW_limit
%token KW_physical
%token KW_read
%token KW_readonly
%token KW_return
%token KW_schema
%token KW_static
%token KW_table
%token KW_template
%token KW_trigger
%token KW_true
%token KW_type
%token KW_typedef
%token KW_typeset
%token KW_validate
%token KW_version
%token KW_view
%token KW_virtual
%token KW_void
%token KW_write

    /* Parse tree nodes */
%token PT_ASTLIST
%token PT_PARSE
%token PT_SOURCE
%token PT_VERSION_1_0
%token PT_VERSION_2
%token PT_SCHEMA_1_0
%token PT_INCLUDE
%token PT_TYPEDEF
%token PT_FQN
%token PT_IDENT
%token PT_UINT
%token PT_TYPESET
%token PT_TYPESETDEF
%token PT_FORMAT
%token PT_CONST
%token PT_ALIAS
%token PT_EXTERN
%token PT_FUNCTION
%token PT_UNTYPED
%token PT_ROWLENGTH
%token PT_FUNCDECL
%token PT_EMPTY
%token PT_SCHEMASIG
%token PT_SCHEMAFORMAL
%token PT_RETURNTYPE
%token PT_FACTSIG
%token PT_FUNCSIG
%token PT_FUNCPARAMS
%token PT_FORMALPARAM
%token PT_ELLIPSIS
%token PT_FUNCPROLOGUE
%token PT_RETURN
%token PT_PRODSTMT
%token PT_SCHEMA
%token PT_VALIDATE
%token PT_PHYSICAL
%token PT_PHYSPROLOGUE
%token PT_PHYSSTMT
%token PT_PHYSBODYSTMT
%token PT_TABLE
%token PT_TABLEPARENTS
%token PT_TABLEBODY
%token PT_FUNCEXPR
%token PT_FACTPARMS
%token PT_COLUMN
%token PT_COLDECL
%token PT_TYPEDCOL
%token PT_COLMODIFIER
%token PT_COLSTMT
%token PT_DFLTVIEW
%token PT_PHYSMBR
%token PT_PHYSCOL
%token PT_PHYSCOLDEF
%token PT_COLSCHEMAPARMS
%token PT_COLSCHEMAPARAM
%token PT_COLUNTYPED
%token PT_DATABASE
%token PT_TYPEEXPR
%token PT_DBBODY
%token PT_DBDAD
%token PT_DATABASEMEMBER
%token PT_DBMEMBER
%token PT_TABLEMEMBER
%token PT_TBLMEMBER
%token PT_NOHEADER
%token PT_CASTEXPR
%token PT_CONSTVECT
%token PT_NEGATE
%token PT_UNARYPLUS
%token PT_FACTPARMNAMED
%token PT_VERSNAME
%token PT_ARRAY

 /* !!! Keep token declarations above in synch with schema-ast.y */


%start parse

%%

parse
    : END_SOURCE                { *root =  MakeTree ( PT_PARSE, T ( $1 ) ); }
    | source END_SOURCE         { *root =  MakeTree ( PT_PARSE, P ( $1 ), T ( $2 ) ); }
    ;

source
    : schema_1_0                { $$ . subtree = MakeTree ( PT_SOURCE, P ( $1 ) ); }
    | version_1_0 schema_1_0    { $$ . subtree = MakeTree ( PT_SOURCE, P ( $1 ), P ( $2 ) ); }
    | version_2_x schema_2_x    { $$ . subtree = MakeTree ( PT_SOURCE, P ( $1 ), P ( $2 ) ); }
    ;

version_1_0
    : KW_version VERS_1_0 ';'   { $$ . subtree = MakeTree ( PT_VERSION_1_0, T ( $1 ), T ( $2 ), T ( $3 ) ); }
    ;

version_2_x
    : KW_version FLOAT ';'      { $$ . subtree = MakeTree ( PT_VERSION_2, T ( $1 ), T ( $2 ), T ( $3 ) ); } /* later checked for valid version number */
    ;

schema_2_x
    : '$'                       { $$ . subtree = T ( $1 ); }   /* TBD */
    ;

/* schema-1.0
 */
schema_1_0
    : schema_1_0_decl              { $$ . subtree = MakeTree ( PT_SCHEMA_1_0, P ( $1 ) ); }
    | schema_1_0 schema_1_0_decl   { $$ . subtree = AddToList ( P ( $1 ) , P ( $2 ) ); }
    ;

schema_1_0_decl
    : typedef_1_0_decl      { $$ = $1; }
    | typeset_1_0_decl      { $$ = $1; }
    | format_1_0_decl       { $$ = $1; }
    | const_1_0_decl        { $$ = $1; }
    | alias_1_0_decl        { $$ = $1; }
    | function_1_0_decl     { $$ = $1; }
    | extern_1_0_decl       { $$ = $1; }
    | script_1_0_decl       { $$ = $1; }
    | validate_1_0_decl     { $$ = $1; }
    | physical_1_0_decl     { $$ = $1; }
    | table_1_0_decl        { $$ = $1; }
    | database_1_0_decl     { $$ = $1; }
    | include_directive     { $$ = $1; }
    | ';'                   { $$ . subtree = T ( $1 ); }  /* for lots of reasons, we tolerate stray semicolons */
    ;

script_1_0_decl
    : KW_schema func_1_0_decl                                   /* MUST have a function body      */
                            { $$ . subtree = MakeTree ( PT_SCHEMA, T ( $1 ), P ( $2 ) ); }
    | KW_schema KW_function func_1_0_decl                       /* "function" keyword is optional */
                            { $$ . subtree = MakeTree ( PT_SCHEMA, T ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

validate_1_0_decl
    : KW_validate function_1_0_decl                             /* has exactly 2 parameters and is not inline */
                            { $$ . subtree = MakeTree ( PT_VALIDATE, T ( $1 ), P ( $2 ) ); }
    ;

include_directive
    : KW_include STRING     { $$ . subtree = MakeTree ( PT_INCLUDE, T ( $1 ), T ( $2 ) ); }
    ;

/* typedef-1.0
 */
typedef_1_0_decl
    :   KW_typedef
        fqn_1_0 /* must be from 'typedef' or schema template */
        typedef_1_0_new_name_list
        ';'
                            { $$ . subtree = MakeTree ( PT_TYPEDEF, T ( $1 ), P ( $2 ), P ( $3 ), T ( $4 ) ); }
    ;

typedef_1_0_new_name_list
    : typespec_1_0                                  { $$ . subtree = MakeList ( $1 ); }
    | typedef_1_0_new_name_list ',' typespec_1_0    { $$ . subtree = AddToList ( P($1), T($2), P($3) ); }
    ;

/* typeset-1.0
 */
typeset_1_0_decl
    : KW_typeset typeset_1_0_new_name typeset_1_0_def ';'
                            { $$ . subtree = MakeTree ( PT_TYPESET, T ( $1 ), P ( $2 ), P ( $3 ), T ( $4 ) ); }
    ;

typeset_1_0_new_name
    : fqn_1_0               { $$ = $1; }                /* we allow duplicate redefinition here... */
    ;

typeset_1_0_def
    : '{' typespec_1_0_list '}'                                 /* ...obviously, the set contents must match */
            { $$ . subtree = MakeTree ( PT_TYPESETDEF, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

/* typespec-1.0
 */
typespec_1_0_list
    : typespec_1_0                          { $$ . subtree = MakeList ( $1 ); }
    | typespec_1_0_list ',' typespec_1_0    { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

typespec_1_0
    : fqn_1_0                   { $$ = $1; }
    | fqn_1_0 '[' dim_1_0 ']'   { $$ . subtree = MakeTree ( PT_ARRAY, P ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ) ); }
    ;

dim_1_0
    : expression_1_0    { $$ = $1; }        /* expects unsigned integer constant expression */
    | '*'               { $$ . subtree = T ( $1 ); }
    ;

/* format-1.0
 */
format_1_0_decl
    : KW_fmtdef format_1_0_new_name ';'
                                    { $$ . subtree = MakeTree ( PT_FORMAT, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    | KW_fmtdef format_1_0_name format_1_0_new_name ';'       /* creates a derived format */
                                    { $$ . subtree = MakeTree ( PT_FORMAT, T ( $1 ), P ( $2 ), P ( $3 ), T ( $4 ) ); }
    ;

format_1_0_new_name
    : fqn_1_0                       { $$ = $1; }   /* allow for redefinition - current code seems to be wrong */
    ;

format_1_0_name
    : fqn_1_0                       { $$ = $1; }   /* must name a format */
    ;


/* const-1.0
 */
const_1_0_decl
    : KW_const typespec_1_0 fqn_1_0 '=' expression_1_0 ';'
            { $$ . subtree = MakeTree ( PT_CONST, T ( $1 ), P ( $2 ), P ( $3 ), T ( $4 ), P ( $5 ), T ( $6 ) ); }
    ;

/* alias-1.0
 */
alias_1_0_decl
    : KW_alias fqn_1_0 alias_1_0_new_name ';'
            { $$ . subtree = MakeTree ( PT_ALIAS, T ( $1 ), P ( $2 ), P ( $3 ), T ( $4 ) ); }
    ;

alias_1_0_new_name
    : fqn_1_0                       { $$ = $1; }    /* for some reason, does not allow duplicate redefinition */
    ;


/* extern-1.0
 */
extern_1_0_decl
    : KW_extern ext_func_1_0_decl                               /* supposed to be an extern C function */
            { $$ . subtree = MakeTree ( PT_EXTERN, T ( $1 ), P ( $2 ) ); }
    ;

ext_func_1_0_decl
    : function_1_0_decl             { $$ = $1; }    /* there are restrictions and potentially additions */
    ;


/* function-1.0
 */
function_1_0_decl
    : KW_function func_1_0_decl      { $$ . subtree = MakeTree ( PT_FUNCTION, T ( $1 ), P ( $2 ) ); }
    ;

func_1_0_decl
    : untyped_func_1_0_decl          { $$ = $1; }   /* cannot be inline or a validation function */
    | row_length_func_1_0_decl       { $$ = $1; }   /* cannot be inline or a validation function */
    | opt_func_1_0_schema_sig
      func_1_0_return_type
      fqn_opt_vers
      opt_func_1_0_fact_sig
      func_1_0_param_sig
      func_1_0_prologue
            { $$ . subtree = MakeTree ( PT_FUNCDECL, P ( $1 ), P ( $2 ), P ( $3 ), P ( $4 ), P ( $5 ), P ( $6 ) ); }
    ;

untyped_func_1_0_decl
    : KW___untyped fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( PT_UNTYPED, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ) ); }
    ;

row_length_func_1_0_decl
    : KW___row_length fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( PT_ROWLENGTH, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ) ); }
    ;

opt_func_1_0_schema_sig
    : empty                     { $$ = $1; }
    | func_1_0_schema_sig       { $$ = $1; }
    ;

func_1_0_schema_sig
    : '<' func_1_0_schema_formals '>'       { $$ . subtree = MakeTree ( PT_SCHEMASIG, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

func_1_0_schema_formals
    : func_1_0_schema_formal                                { $$ . subtree = MakeList ( $1 ); }
    | func_1_0_schema_formals ',' func_1_0_schema_formal    { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

func_1_0_schema_formal
    : KW_type ident_1_0         { $$ . subtree = MakeTree ( PT_SCHEMAFORMAL, T ( $1 ), P ( $2 ) ); }
    | type_expr_1_0 ident_1_0   { $$ . subtree = MakeTree ( PT_SCHEMAFORMAL, P ( $1 ), P ( $2 ) ); } /* type-expr must be uint */
    ;

func_1_0_return_type
    : KW_void           { $$ . subtree = MakeTree ( PT_RETURNTYPE, T ( $1 ) ); }
    | type_expr_1_0     { $$ . subtree = MakeTree ( PT_RETURNTYPE, P ( $1 ) ); }
    ;

opt_func_1_0_fact_sig
    : empty                     { $$ = $1; }
    | func_1_0_fact_sig         { $$ = $1; }
    ;

func_1_0_fact_sig
    : '<' func_1_0_fact_signature '>'  { $$ . subtree = MakeTree ( PT_FACTSIG, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

func_1_0_fact_signature
    : empty                 { $$ = $1; }
    | func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), P ( $2 ) ); }
    | '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, T ( $1 ), P ( $2 ), P ( $3 ) ); }
    | func_1_0_fact_params '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), T ( $2 ), P ( $3 ), P ( $4 ) ); }
    | func_1_0_fact_params ',' '*' func_1_0_fact_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ), P ( $5 ) ); }
    ;

func_1_0_fact_params
    : fact_param_1_0                                { $$ . subtree = MakeList ( $1 ); }
    | func_1_0_fact_params ',' fact_param_1_0       { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

fact_param_1_0
    : typespec_1_0 IDENTIFIER_1_0       { $$ . subtree = MakeTree ( PT_FORMALPARAM, P ( $1 ), T ( $2 ) ); }
    ;

func_1_0_param_sig
    : '(' func_1_0_param_signature ')'  { $$ . subtree = MakeTree ( PT_FUNCSIG, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

func_1_0_param_signature
    : empty                 { $$ = $1; }
    | func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), P ( $2 ) ); }
    | '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, T ( $1 ), P ( $2 ), P ( $3 ) ); }
    | func_1_0_formal_params '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), T ( $2 ), P ( $3 ), P ( $4 ) ); }
    | func_1_0_formal_params ',' '*' func_1_0_formal_params func_1_0_vararg_formals
                            { $$ . subtree = MakeTree ( PT_FUNCPARAMS, P ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ), P ( $5 ) ); }
    ;

func_1_0_formal_params
    : formal_param_1_0                                  { $$ . subtree = MakeList ( $1 ); }
    | func_1_0_formal_params ',' formal_param_1_0       { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

formal_param_1_0
    : typespec_1_0 IDENTIFIER_1_0       { $$ . subtree = MakeTree ( PT_FORMALPARAM, P ( $1 ), T ( $2 ) ); }
    | KW_control typespec_1_0 IDENTIFIER_1_0
                                        { $$ . subtree = MakeTree ( PT_FORMALPARAM, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

func_1_0_vararg_formals
    : empty                     { $$ = $1; }
    | ',' ELLIPSIS              { $$ . subtree = MakeTree ( PT_ELLIPSIS, T ( $1 ), T ( $2 ) ); }
    ;

func_1_0_prologue
    : ';'                                                       /* this is a simple external function declaration    */
                                { $$ . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( $1 ) ); }
    | '=' fqn_1_0 ';'                                          /* rename the function declaration with fqn          */
                                { $$ . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    | '{' script_1_0_stmt_seq '}'                              /* this is a "script" function - cannot be extern!   */
                                { $$ . subtree = MakeTree ( PT_FUNCPROLOGUE, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

script_1_0_stmt_seq
    : script_1_0_stmt                       { $$ . subtree = MakeList ( $1 ); }
    | script_1_0_stmt_seq script_1_0_stmt   { $$ . subtree = AddToList ( P ( $1 ), P ( $2 ) ); }
    ;

script_1_0_stmt
    : KW_return cond_expr_1_0 ';'   { $$ . subtree = MakeTree ( PT_RETURN, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    | production_1_0_stmt           { $$ = $1; }
    ;

production_1_0_stmt
    : typespec_1_0 IDENTIFIER_1_0 '=' cond_expr_1_0 ';'    /* cannot have format */
                                     { $$ . subtree = MakeTree ( PT_PRODSTMT, P ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ), T ( $5 ) ); }
    | KW_trigger    IDENTIFIER_1_0 '=' cond_expr_1_0 ';'
                                     { $$ . subtree = MakeTree ( PT_PRODSTMT, T ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ), T ( $5 ) ); }
    ;

/* physical encoding
 */
physical_1_0_decl
    :   KW_physical
        opt_func_1_0_schema_sig
        phys_1_0_return_type
        fqn_vers
        opt_func_1_0_fact_sig
        phys_1_0_prologue
            { $$ . subtree = MakeTree ( PT_PHYSICAL, T ( $1 ), P ( $2 ), P ( $3 ), P ( $4 ), P ( $5 ), P ( $6 ) ); }
    ;

phys_1_0_return_type
    : func_1_0_return_type                  { $$ = $1; }
    | KW___no_header func_1_0_return_type   { $$ . subtree = MakeTree ( PT_NOHEADER, T ( $1 ), P ( $2 ) ); }  /* not supported with schema signature */
    ;

phys_1_0_prologue
    : '=' phys_1_0_stmt                                         /* shorthand for decode-only rules */
            { $$ . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( $1 ), P ( $2 ) ); }
    | '{' phys_1_0_body '}'
            { $$ . subtree = MakeTree ( PT_PHYSPROLOGUE, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

phys_1_0_body
    : phys_1_0_body_stmt                    { $$ . subtree = MakeList ( $1 ); }
    | phys_1_0_body phys_1_0_body_stmt      { $$ . subtree = AddToList ( P ( $1 ), P ( $2 ) ); }
    ;

phys_1_0_body_stmt
    : ';'
            { $$ . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( $1 ) ); }
    | KW_decode phys_1_0_stmt
            { $$ . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( $1 ), P ( $2 ) ); }
    | KW_encode phys_1_0_stmt
            { $$ . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( $1 ), P ( $2 ) ); }
    | KW___row_length '=' fqn_1_0 '(' ')'
            { $$ . subtree = MakeTree ( PT_PHYSBODYSTMT, T ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ), T ( $5 ) ); }
    ;

phys_1_0_stmt
    : '{' script_1_0_stmt_seq '}'                              /* with caveat that magic parameter "@" is defined */
            { $$ . subtree = MakeTree ( PT_PHYSSTMT, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

/* table
 */
table_1_0_decl
    :   KW_table
        fqn_vers
        opt_tbl_1_0_parents
        tbl_1_0_body
            { $$ . subtree = MakeTree ( PT_TABLE, T ( $1 ), P ( $2 ), P ( $3 ), P ( $4 ) ); }
    ;

opt_tbl_1_0_parents
    : empty                     { $$ = $1; }
    | '=' tbl_1_0_parents       { $$ . subtree = MakeTree ( PT_TABLEPARENTS, T ( $1 ), P ( $2 ) ); }
    ;

tbl_1_0_parents
    : fqn_opt_vers                        { $$ . subtree = MakeList ( $1 ); }
    | tbl_1_0_parents ',' fqn_opt_vers    { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

tbl_1_0_body
    : '{' tbl_1_0_stmt_seq '}'
            { $$ . subtree = MakeTree ( PT_TABLEBODY, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    | '{' '}'
            { $$ . subtree = MakeTree ( PT_TABLEBODY, T ( $1 ), T ( $2 ) ); }
    ;

tbl_1_0_stmt_seq
    : tbl_1_0_stmt                      { $$ . subtree = MakeList ( $1 ); }
    | tbl_1_0_stmt_seq tbl_1_0_stmt     { $$ . subtree = AddToList ( P ( $1 ), P ( $2 ) ); }
    ;

tbl_1_0_stmt
    : production_1_0_stmt                       { $$ = $1; }
    | column_1_0_decl                           { $$ = $1; }
    | default_view_1_0_decl                     { $$ = $1; }
    | KW_static physmbr_1_0_decl                { $$ . subtree = MakeTree ( PT_PHYSCOL, T ( $1 ), P ( $2 ) ); }
    | KW_physical physmbr_1_0_decl              { $$ . subtree = MakeTree ( PT_PHYSCOL, T ( $1 ), P ( $2 ) ); }
    | KW_static KW_physical physmbr_1_0_decl    { $$ . subtree = MakeTree ( PT_PHYSCOL, T ( $1 ), T ( $2 ), P ( $3 ) ); }
    | KW___untyped '=' fqn_1_0 '(' ')' ';'      { $$ . subtree = MakeTree ( PT_COLUNTYPED, T ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ), T ( $5 ), T ( $6 ) ); }
    | ';'                                       { $$ . subtree = T ( $1 ); }
    ;

column_1_0_decl
    :   opt_col_1_0_modifier_seq KW_column col_1_0_decl
            { $$ . subtree = MakeTree ( PT_COLUMN, P ( $1 ), T ( $2 ), P ( $3 ) ); }
    |   opt_col_1_0_modifier_seq KW_column KW_default col_1_0_decl
            { $$ . subtree = MakeTree ( PT_COLUMN, P ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ) ); }
    |   opt_col_1_0_modifier_seq KW_column KW_limit '=' expression_1_0 ';'
            { $$ . subtree = MakeTree ( PT_COLUMN, P ( $1 ), T ( $2 ), T ( $3 ), T ( $4 ), P ( $5 ), T ( $6 ) ); }
    |   opt_col_1_0_modifier_seq KW_column KW_default KW_limit '=' expression_1_0 ';'
            { $$ . subtree = MakeTree ( PT_COLUMN, P ( $1 ), T ( $2 ), T ( $3 ), T ( $4 ), T ( $5 ), P ( $6 ), T ( $7 ) ); }
    ;

opt_col_1_0_modifier_seq
    : empty                     { $$ = $1; }
    | col_1_0_modifier_seq      { $$ = $1; }
    ;

col_1_0_modifier_seq
    : col_1_0_modifier                          { $$ . subtree = MakeList ( $1 ); }
    | col_1_0_modifier_seq col_1_0_modifier     { $$ . subtree = AddToList ( P ( $1 ), P ( $2 ) ); }
    ;

col_1_0_modifier
    : KW_default        { $$ . subtree = MakeTree ( PT_COLMODIFIER, T ( $1 ) ); }
    | KW_extern         { $$ . subtree = MakeTree ( PT_COLMODIFIER, T ( $1 ) ); }
    | KW_readonly       { $$ . subtree = MakeTree ( PT_COLMODIFIER, T ( $1 ) ); }
    ;

col_1_0_decl
    : KW_physical '<' schema_parms_1_0 '>' fqn_opt_vers opt_factory_parms_1_0 typed_column_decl_1_0
            { $$ . subtree = MakeTree ( PT_COLDECL, T ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ), P ( $5 ), P ( $6 ), P ( $7 ) ); }
    |             '<' schema_parms_1_0 '>' fqn_opt_vers opt_factory_parms_1_0 typed_column_decl_1_0
            { $$ . subtree = MakeTree ( PT_COLDECL, T ( $1 ), P ( $2 ), T ( $3 ), P ( $4 ), P ( $5 ), P ( $6 ) ); }
    | fqn_vers opt_factory_parms_1_0 typed_column_decl_1_0
            { $$ . subtree = MakeTree ( PT_COLDECL, P ( $1 ), P ( $2 ), P ( $3 ) ); }
    | fqn_1_0 '<' factory_parms_1_0 '>' typed_column_decl_1_0
            { $$ . subtree = MakeTree ( PT_COLDECL, P ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ), P ( $5 ) ); }
    | typespec_1_0 typed_column_decl_1_0
            { $$ . subtree = MakeTree ( PT_COLDECL, P ( $1 ), P ( $2 ) ); }
    ;

typed_column_decl_1_0
    : col_ident '{' column_body_1_0 '}'
            { $$ . subtree = MakeTree ( PT_TYPEDCOL, P ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ) ); }
    | col_ident '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( PT_TYPEDCOL, P ( $1 ), T ( $2 ), P ( $3 ), T ( $4 ) ); }
    | col_ident ';'
            { $$ . subtree = MakeTree ( PT_TYPEDCOL, P ( $1 ), T ( $2 ) ); }
    ;

col_ident
    : ident_1_0                     { $$ = $1; }
    | phys_ident                    { $$ = $1; }
    ;

phys_ident
    : PHYSICAL_IDENTIFIER_1_0       { $$ . subtree = MakeTree ( PT_IDENT, T ( $1 ) ); }     /* starts with a '.' */
    ;

column_body_1_0
    : column_stmt_1_0                       { $$ . subtree = MakeList ( $1 ); }
    | column_body_1_0 ';' column_stmt_1_0   { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

column_stmt_1_0
    : KW_read '=' cond_expr_1_0             { $$ . subtree = MakeTree ( PT_COLSTMT, T ( $1 ), T ( $2 ), P ( $3 ) ); }
    | KW_validate '=' cond_expr_1_0         { $$ . subtree = MakeTree ( PT_COLSTMT, T ( $1 ), T ( $2 ), P ( $3 ) ); }
    | KW_limit '=' uint_expr_1_0            { $$ . subtree = MakeTree ( PT_COLSTMT, T ( $1 ), T ( $2 ), P ( $3 ) ); }
    | empty                                 { $$ = $1; }
    ;

default_view_1_0_decl
    :   KW_default KW_view STRING ';'
            { $$ . subtree = MakeTree ( PT_DFLTVIEW, T ( $1 ), T ( $2 ), T ( $3 ), T ( $4 ) ); }
    ;

physmbr_1_0_decl
    : phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( PT_PHYSMBR, P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( PT_PHYSMBR, P ( $1 ), T ( $2 ), T ( $3 ), P ( $4 ), T ( $5 ) ); }
    | KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( PT_PHYSMBR, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ) ); }
    | KW_column phys_coldef_1_0 PHYSICAL_IDENTIFIER_1_0 '=' cond_expr_1_0 ';'
            { $$ . subtree = MakeTree ( PT_PHYSMBR, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ), P ( $5 ), T ( $6 ) ); }
    ;

phys_coldef_1_0
    : opt_col_schema_parms_1_0 fqn_opt_vers opt_factory_parms_1_0
            { $$ . subtree = MakeTree ( PT_PHYSCOLDEF, P ( $1 ), P ( $2 ), P ( $3 ) ); }
    ;

opt_col_schema_parms_1_0
    : empty                             { $$ = $1; }
    | '<' col_schema_parms_1_0 '>'      { $$ . subtree = MakeTree ( PT_COLSCHEMAPARMS, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

col_schema_parms_1_0
    : col_schema_parm_1_0                           { $$ . subtree = MakeList ( $1 ); }
    | col_schema_parms_1_0 ',' col_schema_parm_1_0  { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

col_schema_parm_1_0
    : fqn_1_0 '=' col_schema_value_1_0      { $$ . subtree = MakeTree ( PT_COLSCHEMAPARAM, P ( $1 ), T ( $2 ), P ( $3 ) ); }
    | col_schema_value_1_0                  { $$ = $1; }
    ;

col_schema_value_1_0
    : fqn_1_0                               { $$ = $1; }
    | uint_expr_1_0                         { $$ = $1; }
    ;

/* expression
 */

cond_expr_1_0
    : expression_1_0                        { $$ . subtree = MakeList ( $1 ); }
    | cond_expr_1_0 '|' expression_1_0      { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

expression_1_0
    : primary_expr_1_0                      { $$ = $1; }
    | '(' type_expr_1_0 ')' expression_1_0  { $$ . subtree = MakeTree ( PT_CASTEXPR, T ( $1 ), P ( $2 ), T ( $3 ), P ( $4 ) ); }
    ;

primary_expr_1_0
    : fqn_1_0                   { $$ = $1; }
    | phys_ident                { $$ = $1; }
    | '@'                       { $$ . subtree = T ( $1 ); }
    | func_expr_1_0             { $$ = $1; }
    | uint_expr_1_0             { $$ = $1; }
    | float_expr_1_0            { $$ = $1; }
    | string_expr_1_0           { $$ = $1; }
    | const_vect_expr_1_0       { $$ = $1; }
    | bool_expr_1_0             { $$ = $1; }
    | negate_expr_1_0           { $$ = $1; }
    | '+' expression_1_0        { $$ . subtree = MakeTree ( PT_UNARYPLUS, T ( $1 ), P ( $2 ) ); }
    ;

func_expr_1_0
    :   '<'
        schema_parms_1_0
        '>'
        fqn_opt_vers
        opt_factory_parms_1_0
        '('
        opt_func_1_0_parms
        ')'
             { $$ . subtree = MakeTree ( PT_FUNCEXPR, T ( $1 ), P ( $2 ), T ( $3 ), P ( $4 ), P ( $5 ), T ( $6 ), P ( $7 ), T ( $8 ) ); }
    |   fqn_opt_vers
        opt_factory_parms_1_0
        '('
        opt_func_1_0_parms
        ')'
             { $$ . subtree = MakeTree ( PT_FUNCEXPR, P ( $1 ), P ( $2 ), T ( $3 ), P ( $4 ), T ( $5 ) ); }
    ;

schema_parms_1_0
    : schema_parm_1_0                       { $$ . subtree = MakeList ( $1 ); }
    | schema_parms_1_0 ',' schema_parm_1_0  { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

schema_parm_1_0
    : type_expr_1_0                         { $$ = $1; }
    | uint_expr_1_0                         { $$ = $1; }
    ;

opt_factory_parms_1_0
    : empty                                 { $$ = $1; }
    | '<' factory_parms_1_0 '>'             { $$ . subtree = MakeTree ( PT_FACTPARMS, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

factory_parms_1_0
    : factory_parm_1_0                          { $$ . subtree = MakeList ( $1 ); }
    | factory_parms_1_0 ',' factory_parm_1_0    { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

factory_parm_1_0
    : expression_1_0                        { $$ = $1; }
    | IDENTIFIER_1_0 '=' expression_1_0     { $$ . subtree = MakeTree ( PT_FACTPARMNAMED, T ( $1 ), T ( $2 ), P ( $2 ) ); }
    ;

opt_func_1_0_parms
    : empty                                 { $$ = $1; }
    | func_1_0_parms                        { $$ = $1; }
    ;

func_1_0_parms
    : expression_1_0                        { $$ . subtree = MakeList ( $1 ); }
    | func_1_0_parms ',' expression_1_0     { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

uint_expr_1_0
    : DECIMAL                               { $$ . subtree = MakeTree ( PT_UINT, T ( $1 ) ); }
    | HEX                                   { $$ . subtree = MakeTree ( PT_UINT, T ( $1 ) ); }
    | OCTAL                                 { $$ . subtree = MakeTree ( PT_UINT, T ( $1 ) ); }
    ;

float_expr_1_0
    : FLOAT                     { $$ . subtree = T ( $1 ); }
    | EXP_FLOAT                 { $$ . subtree = T ( $1 ); }
    ;

string_expr_1_0
    : STRING                    { $$ . subtree = T ( $1 ); }
    | ESCAPED_STRING            { $$ . subtree = T ( $1 ); }
    ;

const_vect_expr_1_0
    : '[' opt_const_vect_exprlist_1_0 ']'      { $$ . subtree = MakeTree ( PT_CONSTVECT, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

opt_const_vect_exprlist_1_0
    : empty                     { $$ = $1; }
    | const_vect_exprlist_1_0   { $$ = $1; }
    ;

const_vect_exprlist_1_0
    : expression_1_0                                { $$ . subtree = MakeList ( $1 ); }
    | const_vect_exprlist_1_0 ',' expression_1_0    { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    ;

bool_expr_1_0
    : KW_true                   { $$ . subtree = T ( $1 ); }
    | KW_false                  { $$ . subtree = T ( $1 ); }
    ;

negate_expr_1_0
    : '-' fqn_1_0               { $$ . subtree = MakeTree ( PT_NEGATE, T ( $1 ), P ( $2 ) ); }
    | '-' phys_ident            { $$ . subtree = MakeTree ( PT_NEGATE, T ( $1 ), P ( $2 ) ); }
    | '-' uint_expr_1_0         { $$ . subtree = MakeTree ( PT_NEGATE, T ( $1 ), P ( $2 ) ); }
    | '-' float_expr_1_0        { $$ . subtree = MakeTree ( PT_NEGATE, T ( $1 ), P ( $2 ) ); }
    ;

type_expr_1_0
    : typespec_1_0              { $$ = $1; } /* datatype, typeset, schematype */
    | fqn_1_0 '/' fqn_1_0       { $$ . subtree = MakeTree ( PT_TYPEEXPR, P ( $1 ), T ( $2), P ( $3 ) ); } /* format /  ? */
    ;

 /* database */

database_1_0_decl
    :   KW_database
        fqn_vers
        opt_database_dad_1_0
        database_body_1_0
            { $$ . subtree = MakeTree ( PT_DATABASE, T ( $1 ), P ( $2), P ( $3 ), P ( $4 ) ); }
    ;

opt_database_dad_1_0
    : empty                         { $$ = $1; }
    | '=' fqn_opt_vers              { $$ . subtree = MakeTree ( PT_DBDAD, T ( $1 ), P ( $2 ) ); }
    ;

database_body_1_0
    : '{' '}'                       { $$ . subtree = MakeTree ( PT_DBBODY, T ( $1 ), T ( $2 ) ); }
    | '{' database_members_1_0 '}'  { $$ . subtree = MakeTree ( PT_DBBODY, T ( $1 ), P ( $2 ), T ( $3 ) ); }
    ;

database_members_1_0
    : database_member_1_0                       { $$ . subtree = MakeList ( $1 ); }
    | database_members_1_0 database_member_1_0  { $$ . subtree = AddToList ( P ( $1 ), P ( $2 ) ); }
    ;

database_member_1_0
    : opt_template_1_0 db_member_1_0            { $$ . subtree = MakeTree ( PT_DATABASEMEMBER, P ( $1 ), P ( $2 ) ); }
    | opt_template_1_0 table_member_1_0         { $$ . subtree = MakeTree ( PT_TABLEMEMBER, P ( $1 ), P ( $2 ) ); }
    | ';'
    ;

opt_template_1_0
    : empty                     { $$ = $1; }
    | KW_template               { $$ . subtree = T ( $1 ); }
    ;

db_member_1_0
    : KW_database fqn_opt_vers IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( PT_DBMEMBER, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ) ); }
    ;

table_member_1_0
    : KW_table fqn_opt_vers IDENTIFIER_1_0 ';'
            { $$ . subtree = MakeTree ( PT_TBLMEMBER, T ( $1 ), P ( $2 ), T ( $3 ), T ( $4 ) ); }
    ;

/* other stuff
 */
fqn_1_0
    : ident_1_0                                     { $$ . subtree = MakeTree ( PT_FQN, P ( $1 ) ); }
    | fqn_1_0 ':' ident_1_0                         { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), P ( $3 ) ); }
    /* a hack to handle keywords used as namespace identifiers in existing 1.0 schemas */
    | fqn_1_0 ':' KW_database                       { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_decode                         { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_encode                         { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_read                           { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_table                          { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_type                           { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_view                           { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    | fqn_1_0 ':' KW_write                          { $$ . subtree = AddToList ( P ( $1 ), T ( $2 ), T ( $3 ) ); }
    ;

ident_1_0
    : IDENTIFIER_1_0    { $$ . subtree = MakeTree ( PT_IDENT, T ( $1 ) ); }     /* this is just a C identifier */
    ;

empty
    : %empty    { $$ . subtree = MakeTree ( PT_EMPTY ); }
    ;

fqn_vers
    :   fqn_1_0 VERSION     { $$ . subtree = MakeTree ( PT_VERSNAME, P ( $1 ), T ( $2 ) ); }
    ;

fqn_opt_vers
    :   fqn_1_0      { $$ = $1; }
    |   fqn_vers     { $$ = $1; }
    ;