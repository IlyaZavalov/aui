//
// Created by Alex2772 on 12/5/2021.
//

#pragma once

#include <AUI/Util/AMetric.h>
#include <AUI/Common/ASide.h>
#include "Traits.h"

namespace uitest::impl {
    template<ASide side>
    struct side_value {};

    template<>
    struct side_value<ASide::LEFT> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().x; }
    };

    template<>
    struct side_value<ASide::TOP> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().y; }
    };

    template<>
    struct side_value<ASide::RIGHT> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().x + view->getSize().x; }
    };

    template<>
    struct side_value<ASide::BOTTOM> {
        int operator()(const _<AView>& view) { return view->getPositionInWindow().y + view->getSize().y; }
    };


    template<ASide side>
    struct align {
        std::optional<int> value;

        bool operator()(const _<AView>& v) {
            int current = side_value<side>{}(v);
            if (!value) {
                value = current;
                return true;
            }
            return current == *value;
        }
    };

    template<ASide lhs, ASide rhs, typename Comparator>
    struct positional {
        Matcher other;

        positional(const Matcher& other) : other(other) {}

        bool operator()(const _<AView>& v) {
            int l = side_value<lhs>{}(v);
            for (auto& view : other.toSet()) {
                int r = side_value<rhs>{}(view);
                if (!Comparator{}(l, r)) return false;
            }
            return true;
        }
    };
}

using leftAligned = uitest::impl::align<ASide::LEFT>;
using topAligned = uitest::impl::align<ASide::TOP>;
using rightAligned = uitest::impl::align<ASide::RIGHT>;
using bottomAligned = uitest::impl::align<ASide::BOTTOM>;

using leftRightAligned = uitest::impl::both<uitest::impl::align<ASide::LEFT>, uitest::impl::align<ASide::RIGHT>>;
using topBottomAligned = uitest::impl::both<uitest::impl::align<ASide::TOP>, uitest::impl::align<ASide::BOTTOM>>;

using below = uitest::impl::positional<ASide::TOP, ASide::BOTTOM, std::greater<>>;
using above = uitest::impl::positional<ASide::BOTTOM, ASide::TOP, std::less<>>;
