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

/**
* Unit tests for Kdb interface
*/

#include <ktst/unit_test.hpp>

#include <sysalloc.h>

#include <kdb/manager.h>
#include <kdb/database.h>
#include <kdb/index.h>
#include <kdb/table.h>

#include <vfs/manager.h>

using namespace std;

TEST_SUITE(KdbTestSuite);

#define KDB_MANAGER_MAKE(mgr, wd) KDBManagerMakeRead((const KDBManager **)mgr, (struct KDirectory const *)wd)
#include "remote_open_test.cpp"

TEST_CASE(KDBManagerVPathType)
{
    string path;
    { // convert accession "SRR000123" into a file system path
        VFSManager* vfsmgr;
        REQUIRE_RC(VFSManagerMake(&vfsmgr));
        {
            VPath * vpath;
            const struct KFile *dummy1;
            const struct VPath *dummy2;
            REQUIRE_RC(VFSManagerResolveSpec ( vfsmgr, "SRR000123", &vpath, &dummy1, &dummy2, true));
            
            path = ToString(vpath);
            
            REQUIRE_RC(KFileRelease(dummy1));
            REQUIRE_RC(VPathRelease(dummy2));
            REQUIRE_RC(VPathRelease(vpath));
        }
        REQUIRE_RC(VFSManagerRelease(vfsmgr));
    }

//cout << path << endl;

    const KDBManager* mgr;
    REQUIRE_RC(KDBManagerMakeRead(&mgr, NULL));

    REQUIRE_EQ((int)kptTable, KDBManagerPathType(mgr, path.c_str()));
    
    REQUIRE_RC(KDBManagerRelease(mgr));
    
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/args.h>
#include <kfg/config.h>

ver_t CC KAppVersion ( void )
{
    return 0x1000000;
}
rc_t CC UsageSummary (const char * progname)
{
    return 0;
}

rc_t CC Usage ( const Args * args )
{
    return 0;
}

const char UsageDefaultName[] = "test-kdb";

rc_t CC KMain ( int argc, char *argv [] )
{
    KConfigDisableUserSettings();
    rc_t rc=KdbTestSuite(argc, argv);
    return rc;
}

}
