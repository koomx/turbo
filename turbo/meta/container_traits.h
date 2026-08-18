// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <turbo/meta/type_traits.h>

namespace turbo {

      template <typename T>
    struct IsByteLike
        : std::bool_constant<std::is_same_v<T, std::byte> || std::is_same_v<T, char> || std::is_same_v<T, signed char> || std::is_same_v<T, unsigned char>
#ifdef __cpp_char8_t
              || std::is_same_v<T, char8_t>
#endif
              > {
    };

    template <typename T>
    inline constexpr bool is_byte_like_v = IsByteLike<remove_cvref_t<T>>::value;

    template <typename T>
    inline constexpr bool is_mutable_v = !std::is_const_v<std::remove_reference_t<T>>;

    //////////////////////////////////////////
    /// detect that type
    ///     - has value_type
    ///     - has size_t size() const
    ///     - has const T* data() const

    template <typename Container, typename = void>
        struct HasPointerData : std::false_type { };

    template <typename Container>
    struct HasPointerData<Container,
        std::void_t<decltype(std::declval<Container&>().data())>>
        : std::is_pointer<decltype(std::declval<Container&>().data())> { };

    template <typename Container, typename = void>
    struct HasConstPointerData : std::false_type { };

    template <typename Container>
    struct HasConstPointerData<Container,
        std::void_t<decltype(std::declval<const Container&>().data())>>
        : std::is_pointer<decltype(std::declval<const Container&>().data())> { };

    //
    // HasSize<Container> inherets from true_type if Container has a size() member.
    // false_type otherwise.
    //

    template <typename, typename = void>
    struct HasSize : public std::false_type { };

    template <class Container>
    struct HasSize<Container,
        std::void_t<decltype(std::declval<Container&>().size())>>
        : public std::true_type { };


    //
    // TypeOfData<Container>::type is the return type of data() if Container has a
    // data() member. It is NoData otherwise.
    //

    struct NoData { };

    template <typename, typename = void>
    struct TypeOfData {
        using type = NoData;
    };

    template <class Container>
    struct TypeOfData<Container,
        std::void_t<decltype(std::declval<Container&>().data())>> {
            using type = decltype(std::declval<Container&>().data());
        };

    // Element type of container based on operator[].
    template <class Container>
    using ElementType = std::remove_reference_t<decltype(std::declval<Container&>()[0])>;

}  // namespace turbo
