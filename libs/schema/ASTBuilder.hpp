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

#ifndef _hpp_ASTBuilder_
#define _hpp_ASTBuilder_

#include "AST.hpp"

#include <klib/rc.h>

struct KSymbol;
struct BSTree;
struct STypeExpr;
struct SFunction;
struct SFormParmlist;
struct SIndirectType;
struct SIndirectConst;

namespace ncbi
{
    namespace SchemaParser
    {
        class ASTBuilder
        {
        public:
            ASTBuilder ();
            ~ASTBuilder ();

            void DebugOn ();

            AST* Build ( const ParseTree& p_root );

            const KSymbol* Resolve ( const AST_FQN& p_fqn, bool p_reportUnknown = true );
            const KSymbol* Resolve ( const char* p_ident, bool p_reportUnknown = true );

            uint32_t IntrinsicTypeId ( const char * p_type ) const;

            void ReportError ( const char* p_fmt, ... );
            void ReportError ( const char* p_msg, const AST_FQN& p_fqn );
            void ReportError ( const char* p_msg, rc_t );

            // error list is cleared by a call to Build
            uint32_t GetErrorCount() const { return VectorLength ( & m_errors ); }
            const char* GetErrorMessage ( uint32_t p_idx ) const { return ( const char * ) VectorGet ( & m_errors, p_idx ); }

            // uses malloc(); reports allocation failure
            template < typename T > T* Alloc ( size_t p_size = sizeof ( T ) );

            const VSchema * GetSchema () const { return m_schema; }

        public:
            // AST node creation methods for use from bison
            AST * TypeDef ( const Token*, AST_FQN* baseType, AST* newTypes );
            AST * TypeSet ( const Token*, AST_FQN* name, AST* typeSpecs );
            AST * FmtDef  ( const Token*, AST_FQN* name, AST_FQN* super_opt );
            AST * ConstDef  ( const Token*, AST* type, AST_FQN* name, AST_Expr* expr );
            AST * AliasDef  ( const Token*, AST_FQN* name, AST_FQN* newName );
            AST * UntypedFunctionDecl ( const Token*, AST_FQN* name );
            AST * RowlenFunctionDecl ( const Token*, AST_FQN* name );
            AST * FunctionDecl ( const Token*, AST * schema, AST * returnType, AST_FQN* name, AST_ParamSig* fact, AST_ParamSig* params, AST* prologue );

        private:
            bool Init();

            const KSymbol* CreateFqnSymbol ( const AST_FQN& fqn, uint32_t type, const void * obj );

            void DeclareType ( const AST_FQN& fqn, const KSymbol& super, const AST_Expr* dimension_opt );
            void DeclareTypeSet ( const AST_FQN& fqn, const BSTree& types, uint32_t typeCount );
            void DeclareFunction ( const AST_FQN&           fqn,
                                   uint32_t                 type,
                                   struct STypeExpr *       retType,
                                   SFormParmlist *          factory,
                                   SFormParmlist *          formals,
                                   const BSTree *           sscope,
                                   const Vector *           stypes,
                                   const Vector *           sparams,
                                   const BSTree *           fscope,
                                   bool                     canOverload );

            struct STypeExpr * MakeTypeExpr ( const AST & p_type );

            struct SFormParmlist * MakeFactoryParams ( const AST_ParamSig & );
            void AddFactoryParams ( Vector& p_sig, const AST & p_params );

            struct SFormParmlist * MakeFormalParams ( const AST_ParamSig & );
            void AddFormalParams ( Vector& p_sig, const AST & p_params );

            void MakeSchemaParams ( const AST & p_sig, Vector& p_types, Vector & p_values );
            struct SIndirectType * MakeSchemaParamType ( const AST_FQN & p_name );
            struct SIndirectConst * MakeSchemaParamConst ( const AST_FQN & p_name );

            uint64_t EvalConstExpr ( const AST_Expr &expr );
            bool HandleOverload ( struct SFunction * fn, const KSymbol * priorDecl);

            // false - failed, error reported
            rc_t VectorAppend ( Vector *self, uint32_t *idx, const void *item );

            void VectorMove ( Vector & p_dest, const Vector * p_source );

            void SFormParmlistMove ( SFormParmlist & p_dest, SFormParmlist * p_source );
            struct KSymbol * CreateParamSymbol ( const char* p_name, int p_type, void * p_obj );

        private:
            bool m_debug;

            VSchema*    m_intrinsic;
            VSchema*    m_schema;
            KSymTable   m_symtab;

            Vector      m_errors;
        };


        template < typename T >
        T*
        ASTBuilder :: Alloc ( size_t p_size )
        {
            T * ret = static_cast < T * > ( malloc ( p_size ) ); // VSchema's tables dispose of objects with free()
            if ( ret == 0 )
            {
                ReportError ( "malloc failed: %R", RC ( rcVDB, rcSchema, rcParsing, rcMemory, rcExhausted ) );
            }
            return ret;
        }

    }
}

#endif
