//
// Created by Alex2772 on 12/5/2021.
//

#pragma once


struct ViewAssertionExists {
    using IGNORE_VISIBILITY = std::true_type;

    bool exists = false;
    bool operator()(const _<AView>& v) {
        exists = true;
        return true;
    }
    ~ViewAssertionExists() {
        BOOST_REQUIRE_MESSAGE(exists, "view does not exist");
    }
};

using exists = ViewAssertionExists;