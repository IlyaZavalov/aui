#pragma once

#include <chrono>
#include <AUI/Platform/AProcess.h>
#include <AUI/Util/kAUI.h>
#include <AUI/Json/AJson.h>
#include <boost/test/unit_test.hpp>
#include <AUI/IO/FileInputStream.h>
#include <AUI/IO/FileOutputStream.h>

namespace aui::perf {
    using namespace std::chrono;
    using namespace std::chrono_literals;

    inline time_t get_previous_result(const AString& test) {
        try {
            return (time_t) AJson::read(_new<FileInputStream>("tests.json"))[test].asInt();
        } catch (...) {
            return 0;
        }
    }
    inline void set_result(const AString& test, time_t value) {
        AJsonObject r;
        try {
            r = AJson::read(_new<FileInputStream>("tests.json")).asObject();
        } catch (...) {
        }
        r[test] = value;
        AJson::write(_new<FileOutputStream>("tests.json"), r);
    }
    template <typename Callable>
    inline time_t measure(Callable c) {

        auto begin = AProcess::current().getCpuTime().user;

        const auto times = 100;

        repeat(times) {
            c();
        }

        auto end = AProcess::current().getCpuTime().user;
        return (end - begin) / 100;
    }

}

#define AUI_PERF_CASE(name, code) BOOST_AUTO_TEST_CASE(name) {                                                         \
                                    using namespace std::chrono_literals;                                              \
                                    auto runTime = aui::perf::measure([]() code);                                      \
                                    BOOST_TEST_MESSAGE("[" << #name << "]: " << runTime << "us");                      \
                                    auto previous = aui::perf::get_previous_result(#name);                             \
                                    if (previous == 0) {                                                               \
                                        aui::perf::set_result(#name, runTime);                                         \
                                        BOOST_TEST_MESSAGE("[" << #name << "]: No previous results present");          \
                                    } else {                                                                           \
                                        if (previous + 1000 < runTime) {                                               \
                                            BOOST_TEST_ERROR("[" << #name << "]: Became slower by "                    \
                                                              << (runTime - previous) << "us");                        \
                                        } else if (previous > runTime + 100) {                                         \
                                            BOOST_TEST_MESSAGE("[" << #name << "]: Became faster by "                  \
                                                               << (previous - runTime) << "us");                       \
                                            aui::perf::set_result(#name, runTime);                                     \
                                        } else {                                                                       \
                                            BOOST_TEST_MESSAGE("[" << #name <<                                         \
                                                               "]: No significant performance influence");             \
                                        }                                                                              \
                                    }                                                                                  \
                                  }