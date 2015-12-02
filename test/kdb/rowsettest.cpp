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

// allow g++ to find INT64_MAX in stdint.h
#define __STDC_LIMIT_MACROS

#include <ktst/unit_test.hpp>

#include <klib/log.h>
#include <klib/out.h>
#include <klib/rc.h>
#include <kdb/rowset.h>

#include <stdlib.h>
#include <time.h>

#include <vector>
#include <set>

TEST_SUITE(KRowSetTestSuite);

void vector_inserter ( int64_t row_id, void *data )
{
	std::vector<int64_t> * rows = (std::vector<int64_t> *) data;
	rows->push_back ( row_id );
}

int64_t generate_id ( int64_t range_start, int64_t range_count )
{
	if ( range_start == -1 )
		range_start = 0;

	if ( range_count == -1 || range_start + range_count < 0 )
		range_count = INT64_MAX - range_start;

	int64_t generated_id = ((int64_t)rand() << 32) | rand();
	generated_id &= INT64_MAX; // make sure it is positive

	generated_id = generated_id % range_count;
	generated_id += range_start;

	return generated_id;
}

TEST_CASE(KRowSetWalk)
{
	KRowSet * rowset;
	uint64_t num_rows;
	std::set<int64_t> inserted_rows_set;
	std::vector<int64_t> inserted_rows;
	std::vector<int64_t> returned_rows;

	srand ( time(NULL) );

	REQUIRE_RC ( KCreateRowSet ( &rowset ) );
	for ( int i = 0; i < 10000; ++i )
	{
		int64_t row_id = generate_id ( -1, -1 );
		if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
		{
			REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
			inserted_rows_set.insert( row_id );
		}
		else
			--i;
	}

	std::copy(inserted_rows_set.begin(), inserted_rows_set.end(), std::back_inserter(inserted_rows));

	REQUIRE_RC ( KRowSetWalkRows( rowset, false, vector_inserter, (void *)&returned_rows ) );

	REQUIRE_RC ( KRowSetGetNumRows ( rowset, &num_rows ) );

	REQUIRE_EQ ( inserted_rows.size(), returned_rows.size() );
	REQUIRE_EQ ( num_rows, (uint64_t)returned_rows.size() );
	REQUIRE ( inserted_rows == returned_rows );

	REQUIRE_RC ( KRowSetRelease( rowset ) );
}

TEST_CASE(KRowSetWalkReverse)
{
	KRowSet * rowset;
	uint64_t num_rows;
	std::set<int64_t> inserted_rows_set;
	std::vector<int64_t> inserted_rows;
	std::vector<int64_t> returned_rows;

	srand ( time(NULL) );

	REQUIRE_RC ( KCreateRowSet ( &rowset ) );
	for ( int i = 0; i < 10000; ++i )
	{
		int64_t row_id = generate_id ( -1, -1 );
		if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
		{
			REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
			inserted_rows_set.insert( row_id );
		}
		else
			--i;
	}

	std::copy(inserted_rows_set.rbegin(), inserted_rows_set.rend(), std::back_inserter(inserted_rows));

	KRowSetWalkRows( rowset, true, vector_inserter, (void *)&returned_rows );

	KRowSetGetNumRows ( rowset, &num_rows );

	REQUIRE_EQ ( inserted_rows.size(), returned_rows.size() );
	REQUIRE_EQ ( num_rows, (uint64_t)returned_rows.size() );
	REQUIRE ( inserted_rows == returned_rows );

	REQUIRE_RC ( KRowSetRelease( rowset ) );
}

TEST_CASE(KRowSetDenseRowsWalk)
{
	KRowSet * rowset;
	uint64_t num_rows;
	std::set<int64_t> inserted_rows_set;
	std::vector<int64_t> inserted_rows;
	std::vector<int64_t> returned_rows;

	srand ( time(NULL) );

	REQUIRE_RC ( KCreateRowSet ( &rowset ) );
	for ( int i = 0; i < 10000; ++i )
	{
		int64_t row_id = generate_id ( 0, 131072 ); // row ids will only go to first two leaves
		if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
		{
			REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
			inserted_rows_set.insert( row_id );
		}
		else
			--i;
	}

	std::copy(inserted_rows_set.begin(), inserted_rows_set.end(), std::back_inserter(inserted_rows));

	KRowSetWalkRows( rowset, false, vector_inserter, (void *)&returned_rows );

	KRowSetGetNumRows ( rowset, &num_rows );

	REQUIRE_EQ ( inserted_rows.size(), returned_rows.size() );
	REQUIRE_EQ ( num_rows, (uint64_t)returned_rows.size() );
	REQUIRE ( inserted_rows == returned_rows );

	REQUIRE_RC ( KRowSetRelease( rowset ) );
}

TEST_CASE(KRowSetDenseRowsWalkReverse)
{
    KRowSet * rowset;
    uint64_t num_rows;
    std::set<int64_t> inserted_rows_set;
    std::vector<int64_t> inserted_rows;
    std::vector<int64_t> returned_rows;

    srand ( time(NULL) );

    REQUIRE_RC ( KCreateRowSet ( &rowset ) );
    for ( int i = 0; i < 10000; ++i )
    {
        int64_t row_id = generate_id ( 0, 131072 ); // row ids will only go to first two leaves
        if ( inserted_rows_set.find( row_id ) ==  inserted_rows_set.end() )
        {
            REQUIRE_RC ( KRowSetInsertRow ( rowset, row_id ) );
            inserted_rows_set.insert( row_id );
        }
        else
            --i;
    }

    std::copy(inserted_rows_set.rbegin(), inserted_rows_set.rend(), std::back_inserter(inserted_rows));

    KRowSetWalkRows( rowset, true, vector_inserter, (void *)&returned_rows );

    KRowSetGetNumRows ( rowset, &num_rows );

    REQUIRE_EQ ( inserted_rows.size(), returned_rows.size() );
    REQUIRE_EQ ( num_rows, (uint64_t)returned_rows.size() );
    REQUIRE ( inserted_rows == returned_rows );

    REQUIRE_RC ( KRowSetRelease( rowset ) );
}

//////////////////////////////////////////// Main
extern "C"
{

#include <kapp/main.h>
#include <kapp/args.h>

ver_t CC KAppVersion ( void )
{
    return 0;
}


const char UsageDefaultName[] = "test-rowset";

rc_t CC UsageSummary ( const char *progname )
{
    return KOutMsg ( "\n"
                     "Usage:\n"
                     "  %s [Options] <target>\n"
                     "\n"
                     "Summary:\n"
                     "  test the rowset.\n"
                     , progname
        );
}

rc_t CC Usage ( const Args *args )
{
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;
    rc_t rc;

    if (args == NULL)
        rc = RC (rcApp, rcArgv, rcAccessing, rcSelf, rcNull);
    else
        rc = ArgsProgram (args, &fullpath, &progname);
    if (rc)
        progname = fullpath = UsageDefaultName;

    UsageSummary (progname);

    KOutMsg ("Options:\n");

    HelpOptionsStandard ();

    HelpVersion (fullpath, KAppVersion());

    return rc;
}
rc_t CC KMain ( int argc, char *argv [] )
{
	rc_t rc = KRowSetTestSuite(argc, argv);
    return rc;
}

}

