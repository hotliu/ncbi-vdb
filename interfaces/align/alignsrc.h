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

#ifndef _h_align_alignsrc_
#define _h_align_alignsrc_

#ifndef _h_align_extern_
 #include <align/extern.h>
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


/*--------------------------------------------------------------------------
 * AlignSrc
 *  an alignment guy
 */
typedef struct AlignSrc AlignSrc;


/* Make
 *  create an alignment source object from BAM file
 *
 *  "as" [ OUT ] - return parameter for AlignSrc object
 *
 *  "bam" [ IN ] - NUL terminated path to BAM file
 *
 *  "bai" [ IN, NULL OKAY ] - optional NUL terminated path to BAM index file
 */
ALIGN_EXTERN int CC AlignSrcMakeFromBAM ( const AlignSrc **as, const char *bam, const char *bai );


/* Dispose
 *  release all resources associated with alignment source
 */
ALIGN_EXTERN int CC AlignSrcDispose ( const AlignSrc *self );


#ifdef __cplusplus
}
#endif

#endif /* _h_align_alignsrc_ */
