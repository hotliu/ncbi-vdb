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
* A test for C interface to JWT library
*/

#include <klib/rc.h>

#include <cfloat>

#include <jwt/jwt-string.hpp>
#include <../libs/jwt/jwt-vector-impl.hpp>
#include <jwt/jwt-pair.hpp>
#include <../libs/jwt/jwt-map-impl.hpp>

using namespace ncbi;
// this is compiled by a c++ compiler but will be linked without C++ std library (using gcc)

//TODO: remove this when done removing exceptions from JWT
void *__gxx_personality_v0;

extern "C"
{

#define THROW return RC(rcText,rcString,rcProcessing,rcError,rcUnexpected)

rc_t JWT_String()
{
    {   // empty
        JwtString str;
        if (! str.empty()) THROW;
        if (str.size() != 0) THROW;
        if (strcmp ( str.data(), "" ) != 0) THROW;
        if (str.c_str() != str.data()) THROW;
    }

    {
        JwtString str("123");
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {
        JwtString str("123", 3);
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {   // copy ctor
        JwtString src("123");
        JwtString str(src);
        if (str.empty()) THROW;
        if (str.size() != 3) THROW;
        if (strcmp(str.data(), "123") != 0) THROW;
        if (strcmp(str.c_str(), "123") != 0) THROW;
    }

    {   // find_first_of(char)
        JwtString str("123");
        if ( str.find_first_of('2', 0) != 1) THROW;
        if ( str.find_first_of('2', 3) != JwtString::npos) THROW;
        if ( str.find_first_of('4', 0) != JwtString::npos) THROW;
    }

    {   // find_first_of(char*)
        JwtString str("123");
        if ( str.find_first_of("32", 0) != 1) THROW;
        if ( str.find_first_of("23", 3) != JwtString::npos) THROW;
    }

    {   // substr
        JwtString str("1234");
        if ( strcmp("23", str.substr(1, 2).data()) != 0 ) THROW;
    }
    {   // clear
        JwtString str("1234");
        str.clear();
        if (! str.empty()) THROW;
    }

    {   // compare
        JwtString str("1234");
        if ( str.compare("23") >= 0 ) THROW;
        if ( str.compare("0123") <= 0 ) THROW;
        if ( str.compare("1234") != 0 ) THROW;

        if ( str.compare( JwtString("23") ) >= 0 ) THROW;
        if ( str.compare( JwtString("0123") ) <= 0 ) THROW;
        if ( str.compare( JwtString("1234") ) != 0 ) THROW;

        if ( str.compare( 1, 2, JwtString("01234"), 2, 2 ) != 0 ) THROW;
        if ( str.compare( 1, 2, "23" ) != 0 ) THROW;
        if ( str.compare( 1, 2, "234", 2 ) != 0 ) THROW;
    }

    {   // find
        JwtString str("1234");
        if ( str.find("23") != 1 ) THROW;
        if ( str.find("32") != JwtString::npos ) THROW;
    }

    {   // +=
        JwtString str("1234");
        str += JwtString("56");
        if ( str.compare("123456") != 0 ) THROW;
        str += '7';
        if ( str.compare("1234567") != 0 ) THROW;
    }

    {   // []
        JwtString str("1234");
        if ( str[1] != '2' ) THROW;
    }

    {   // <
        if ( ! ( JwtString("123") < JwtString("23") ) ) THROW;
    }

    {   // +
        if ( strcmp( ( JwtString("123") + "45" ) . data(), JwtString("12345").data() ) != 0 ) THROW;
        if ( strcmp( ( JwtString("123") + JwtString("45") ) . data(), JwtString("12345").data() ) != 0 ) THROW;
        if ( strcmp( ( JwtString("123") + '4' ) . data(), JwtString("1234").data() ) != 0 ) THROW;
    }

    {   // stoi
        if ( stoi ( JwtString("-123"), nullptr, 10 ) != -123 ) THROW;
        size_t idx = 0;
        if ( stoi ( JwtString("123fx"), &idx, 16 ) != 0x123F || idx != 4 ) THROW;
    }

    {   // stoll
        if ( stoll ( JwtString("-123"), nullptr, 10 ) != -123 ) THROW;
        size_t idx = 0;
        if ( stoll ( JwtString("123fx"), &idx, 16 ) != 0x123F || idx != 4 ) THROW;
    }

    {   // stold
        if ( stold ( JwtString("-12.3"), nullptr ) - -12.3L > DBL_EPSILON ) THROW;
        size_t idx = 0;
        if ( stold ( JwtString("1.23fx"), &idx ) - 1.23L > DBL_EPSILON || idx != 4 ) THROW;
    }

    {   // to_string
        if ( strcmp ( to_string ( 123u ) . data (), JwtString ( "123" ) . data () ) != 0 ) THROW;
        if ( strcmp ( to_string ( 123LL ) . data (), JwtString ( "123" ) . data () ) != 0 ) THROW;
    }

    return 0;
}

rc_t JWT_Vector_POD()
{   // Vector of Plain Old Data
    {   // ctor, dtor, size()
        JwtVector<int> v;
        if ( v.size() != 0 ) THROW;
    }
    {   // push_back(), size(), const []
        JwtVector<int> v;
        v.push_back(2);
        if ( v.size() != 1 ) THROW;
        const JwtVector<int>& rv = v;
        if ( rv [ 0 ] != 2 ) THROW;
    }
    {   // empty()
        JwtVector<int> v;
        if ( ! v.empty() ) THROW;
        v.push_back(2);
        if ( v.empty() ) THROW;
    }
    {   // copy ctor
        JwtVector<int> v1;
        v1.push_back(2);
        JwtVector<int> v2 ( v1 );
        if ( v2 [ 0 ] != 2 ) THROW;
    }
    {   // non-const []
        JwtVector<int> v;
        v.push_back(2);
        v[0] = 1;
        if ( v [ 0 ] != 1 ) THROW;
    }
    {   // clear()
        JwtVector<int> v;
        v.push_back(2);
        v.clear();
        if ( v.size() != 0 ) THROW;
    }
    {   // const back()
        JwtVector<int> v;
        v.push_back(2);
        v.push_back(3);
        const JwtVector<int>& rv = v;
        if ( rv . back () != 3 ) THROW;
    }
    {   // non-const back()
        JwtVector<int> v;
        v.push_back(2);
        v.push_back(3);
        v.back() = 4;
        if ( v . back () != 4 ) THROW;
    }
    {   // pop_back()
        JwtVector<int> v;
        v.push_back(2);
        v.push_back(3);
        v.push_back(4);
        v.pop_back();
        if ( v . size () != 2 ) THROW;
        if ( v . back () != 3 ) THROW;
    }
    return 0;
}

rc_t JWT_Vector_JwtString()
{   // Vector of Plain Old Data
    {   // ctor, dtor, size()
        JwtVector<JwtString> v;
        if ( v.size() != 0 ) THROW;
    }
    {   // push_back(), size(), const []
        JwtVector<JwtString> v;
        v.push_back("2");
        if ( v.size() != 1 ) THROW;
        const JwtVector<JwtString>& rv = v;
        if ( rv [ 0 ] != JwtString ( "2" ) ) THROW;
    }
    {   // empty()
        JwtVector<JwtString> v;
        if ( ! v.empty() ) THROW;
        v.push_back(JwtString ( "2" ));
        if ( v.empty() ) THROW;
    }
    {   // copy ctor
        JwtVector<JwtString> v1;
        v1.push_back(JwtString ( "2" ));
        JwtVector<JwtString> v2 ( v1 );
        if ( v2 [ 0 ] != JwtString ( "2" ) ) THROW;
    }
    {   // non-const []
        JwtVector<JwtString> v;
        v.push_back(JwtString ( "2" ));
        v[0] = JwtString ( "1" );
        if ( v [ 0 ] != JwtString ( "1" ) ) THROW;
    }
    {   // clear()
        JwtVector<JwtString> v;
        v.push_back(JwtString ( "2" ));
        v.clear();
        if ( v.size() != 0 ) THROW;
    }
    {   // const back()
        JwtVector<JwtString> v;
        v.push_back(JwtString ( "2" ));
        v.push_back(JwtString ( "3" ));
        const JwtVector<JwtString>& rv = v;
        if ( rv . back () != JwtString ( "3" ) ) THROW;
    }
    {   // non-const back()
        JwtVector<JwtString> v;
        v.push_back(JwtString ( "2" ));
        v.push_back(JwtString ( "3" ));
        v.back() = JwtString ( "4" );
        if ( v . back () != JwtString ( "4" ) ) THROW;
    }
    {   // pop_back()
        JwtVector<JwtString> v;
        v.push_back(JwtString ( "2" ));
        v.push_back(JwtString ( "3" ));
        v.push_back(JwtString ( "4" ));
        v.pop_back();
        if ( v . size () != 2 ) THROW;
        if ( v . back () != JwtString ( "3" ) ) THROW;
    }
    return 0;
}

rc_t JWT_Pair()
{
    JwtPair<bool, int> p(true, 1);
    if ( ! p . first ) THROW;
    if ( p . second != 1 ) THROW;
    return 0;
}

rc_t JWT_Map()
{
    {   // ctor/dtor
        JwtMap < JwtString, int > m;
    }
    {   // empty() true
        JwtMap < JwtString, int > m;
        if ( ! m . empty() ) THROW;
    }
    {   // emplace() success, empty() false
        JwtMap < JwtString, int > m;
        auto it = m . emplace ( "1", 1 );
        if ( it . first -> first != JwtString ( "1" ) ) THROW;
        if ( it . first -> second != 1 ) THROW;
        if ( ! it . second ) THROW;
        if ( m . empty() ) THROW;
    }
    {   // emplace() fail
        JwtMap < JwtString, int > m;
        m . emplace ( "1", 1 );
        auto it = m . emplace ( "1", 2 ); // no effect, the existing node pointed to by it
        if ( it . first -> first != JwtString ( "1" ) ) THROW;
        if ( it . first -> second != 1 ) THROW;
        if ( it . second ) THROW;
    }
    {   // size()
        JwtMap < JwtString, int > m;
        if ( m . size () != 0 ) THROW;
        m . emplace ( "1", 1 );
        if ( m . size () != 1 ) THROW;
        m . emplace ( "2", 2 );
        if ( m . size () != 2 ) THROW;
    }
    {   // end()
        JwtMap < JwtString, int > m;
        if ( * m . end ()  != nullptr ) THROW;
    }
    {   // non-const find()
        JwtMap < JwtString, int > m;
        m . emplace ( "2", 2 );
        m . emplace ( "1", 1 );

        auto it = m . find ( JwtString ( "1" ) );
        if ( it == m . end () ) THROW;
        if ( it -> first != JwtString ( "1" ) ) THROW;
        if ( it -> second != 1 ) THROW;

        it = m . find ( JwtString ( "2" ) );
        if ( it == m . end () ) THROW;
        if ( it -> first != JwtString ( "2" ) ) THROW;
        if ( it -> second != 2 ) THROW;

        it = m . find ( JwtString ( "3" ) );
        if ( it != m . end() ) THROW;
    }
    {   // const find()
        JwtMap < JwtString, int > m;
        m . emplace ( "1", 1 );

        const JwtMap < JwtString, int > & c ( m );
        auto it = c . find ( JwtString ( "1" ) );
        if ( it == c . end () ) THROW;
        if ( it -> first != JwtString ( "1" ) ) THROW;
    }

    {   // non-const begin()
        JwtMap < JwtString, int > m;
        if ( m.begin() != m . end () ) THROW;
        m . emplace ( "1", 1 );
        auto it = m . begin();
        if ( it == m . end () ) THROW;
        if ( it -> first != JwtString ( "1" ) ) THROW;
        if ( it -> second != 1 ) THROW;
    }
    {   // const begin()
        JwtMap < JwtString, int > m;
        if ( m.begin() != m . end () ) THROW;
        m . emplace ( "1", 1 );
        const JwtMap < JwtString, int > & c ( m );
        auto it = c . begin();
        if ( it == c . end () ) THROW;
        if ( it -> first != JwtString ( "1" ) ) THROW;
        if ( it -> second != 1 ) THROW;
    }
    {   // cend()
        JwtMap < JwtString, int > m;
        if ( * m . cend ()  != nullptr ) THROW;
    }
    {   // cbegin()
        JwtMap < JwtString, int > m;
        if ( m.cbegin() != m . cend () ) THROW;
        m . emplace ( "1", 1 );
        auto it = m . cbegin();
        if ( it == m . cend () ) THROW;
        if ( it -> first != JwtString ( "1" ) ) THROW;
        if ( it -> second != 1 ) THROW;
    }
    {   // erase(), const_iterator(iterator)
        JwtMap < JwtString, int > m;
        m . emplace ( "1", 1 );
        m . emplace ( "2", 2 );
        m . emplace ( "3", 3 );

        auto it = m . erase ( m . find ( JwtString ( "2" ) ) );
        if ( it == m . end () ) THROW;
        if ( it -> first != JwtString ( "3" ) ) THROW; // the following element
        if ( it -> second != 3 ) THROW;
        if ( m . size () != 2 ) THROW;

        it = m . begin();
        if ( it -> first != JwtString ( "1" ) ) THROW;
        if ( it -> second != 1 ) THROW;
    }
    {   // non-const ++
        JwtMap < JwtString, int > m;
        m . emplace ( "1", 1 );
        m . emplace ( "2", 2 );
        m . emplace ( "3", 3 );
        auto it = m . begin();
        if ( it -> first != JwtString ( "1" ) ) THROW;
        ++ it;
        if ( it -> first != JwtString ( "2" ) ) THROW;
        ++ it;
        if ( it -> first != JwtString ( "3" ) ) THROW;
        ++ it;
        if ( it != m . end () ) THROW;
    }
    {   // const ++
        JwtMap < JwtString, int > m;
        m . emplace ( "1", 1 );
        m . emplace ( "2", 2 );
        m . emplace ( "3", 3 );
        const JwtMap < JwtString, int > & c ( m );
        auto it = c . begin();
        if ( it -> first != JwtString ( "1" ) ) THROW;
        ++ it;
        if ( it -> first != JwtString ( "2" ) ) THROW;
        ++ it;
        if ( it -> first != JwtString ( "3" ) ) THROW;
        ++ it;
        if ( it != c . end () ) THROW;
    }
    return 0;
}

//////////////////////////////////////////// Main

#include <kapp/args.h>
#include <kfg/config.h>
#include <stdio.h>

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

const char UsageDefaultName[] = "test-jwt-c";

rc_t CC KMain ( int argc, char *argv [] )
{
    rc_t rc = JWT_String();
    if ( rc == 0 ) rc = JWT_Vector_POD();
    if ( rc == 0 ) rc = JWT_Vector_JwtString();
    if ( rc == 0 ) rc = JWT_Pair();
    if ( rc == 0 ) rc = JWT_Map();

    if ( rc == 0 )
    {
        printf("No errors detected\n");
    }
    return rc;
}

}