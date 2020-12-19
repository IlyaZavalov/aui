//
// Created by alex2 on 19.12.2020.
//
#include <boost/test/unit_test.hpp>
#include <AUI/Crypt/ARsa.h>
#include <AUI/Util/ARandom.h>
#include <AUI/Common/AString.h>
#include <AUI/Crypt/AHash.h>
#include <AUI/IO/ByteBufferInputStream.h>

#include <AUI/Test/Measure.h>
#include <AUI/Render/Stylesheet.h>


using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE(Performance)

    AUI_PERF_CASE(CSS, {
        Stylesheet s;
    })

BOOST_AUTO_TEST_SUITE_END()