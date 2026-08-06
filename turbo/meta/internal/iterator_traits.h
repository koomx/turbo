// Copyright 2025 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: internal/iterator_traits.h
// -----------------------------------------------------------------------------
//
// Helpers for querying traits of iterators, for implementing containers, etc.

#pragma once

#include <iterator>
#include <type_traits>

#include <turbo/macros/config.h>
#include <turbo/meta/type_traits.h>

namespace turbo::meta_internal {
    template<typename Iterator, typename = void>
    struct IteratorCategory {
    };

    template<typename Iterator>
    struct IteratorCategory<
                Iterator,
                std::void_t<typename std::iterator_traits<Iterator>::iterator_category> > {
        using type = typename std::iterator_traits<Iterator>::iterator_category;
    };

    template<typename Iterator, typename = void>
    struct IteratorConceptImpl : IteratorCategory<Iterator> {
    };

    template<typename Iterator>
    struct IteratorConceptImpl<
                Iterator,
                std::void_t<typename std::iterator_traits<Iterator>::iterator_concept> > {
        using type = typename std::iterator_traits<Iterator>::iterator_concept;
    };

    // The newer `std::iterator_traits<Iterator>::iterator_concept` if available,
    // else `std::iterator_traits<Iterator>::iterator_category`.
    template<typename Iterator>
    using IteratorConcept = typename IteratorConceptImpl<Iterator>::type;

    template<typename IteratorTag, typename Iterator>
    using IsAtLeastIterator =
    std::is_convertible<IteratorConcept<Iterator>, IteratorTag>;

    template<typename Iterator>
    using IsAtLeastInputIterator =
    IsAtLeastIterator<std::input_iterator_tag, Iterator>;

    template<typename Iterator>
    using IsAtLeastForwardIterator =
    IsAtLeastIterator<std::forward_iterator_tag, Iterator>;
} // namespace turbo::meta_internal
