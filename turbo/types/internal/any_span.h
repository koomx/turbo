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

// -----------------------------------------------------------------------------
// File: internal/any_span.h
// -----------------------------------------------------------------------------
//
// Helper types and functions used by AnySpan. This file should only be
// included by any_span.h.
#ifndef TURBO_TYPES_INTERNAL_ANY_SPAN_H_
#define TURBO_TYPES_INTERNAL_ANY_SPAN_H_

#include <algorithm>
#include <cstddef>
#include <functional>
#include <type_traits>

#include <turbo/base/internal/raw_logging.h>
#include <turbo/macros/config.h>
#include <turbo/meta/container_traits.h>

namespace turbo {

    template <typename T>
    class AnySpan;

    namespace any_span_transform {
        struct IdentityT;
        struct DerefT;
    } // namespace any_span_transform

    namespace any_span_internal {
        //
        // IsAnySpan inherits from true_type if T is an instance of AnySpan.
        //

        template <typename T>
        struct IsAnySpan : public std::false_type { };

        template <typename T>
        struct IsAnySpan<AnySpan<T>> : public std::true_type { };

        template <typename T>
        struct IsFlatSpan : public std::false_type { };

        // A type suitable for storing any function pointer. The standard allows
        // function pointers to round-trip through other function pointers, but void* is
        // implementation defined.
        using FunPtr = void (*)();

        // Whether a transform will be copied and therefore does not have to outlive
        // the AnySpan.
        template <typename Transform>
        constexpr bool kIsTransformCopied = std::is_function_v<std::remove_pointer_t<Transform>>;

        // Type to pass as extra argument to TransformPtr to ensure that our
        // assumption about Transform copyability is valid.
        template <typename Transform>
        using IsTransformCopied = std::bool_constant<kIsTransformCopied<Transform>>;

        // A pointer to the transform function or functor that should be applied to
        // elements of the container.
        class TransformPtr {
        public:
            TransformPtr() = default;

            // Construct from ptr to function.
            template <typename R, typename... Args, typename CopiedTransform>
            explicit TransformPtr(R (*f)(Args...),
                CopiedTransform copied_transform [[maybe_unused]])
                : fun_ptr_(reinterpret_cast<FunPtr>(f)) {
                static_assert(CopiedTransform::value);
            }

            // Construct from any other invokable object.
            template <typename T, typename CopiedTransform>
            explicit TransformPtr(const T& t,
                CopiedTransform copied_transform [[maybe_unused]])
                : ptr_(&t) {
                static_assert(!CopiedTransform::value);
            }

            // Casts the pointer to the given type.
            template <typename Transform>
            auto get() const {
                if constexpr (std::is_function_v<Transform>) {
                    return reinterpret_cast<const Transform*>(fun_ptr_);
                } else if constexpr (std::is_function_v<std::remove_pointer_t<Transform>>) {
                    return reinterpret_cast<const Transform>(fun_ptr_);
                } else {
                    return static_cast<const Transform*>(ptr_);
                }
            }

        private:
            union {
                const void* ptr_ = nullptr;
                FunPtr fun_ptr_;
            };
        };

        // A void* pointer to the container and transform functor. These are always cast
        // to the correct type since we create this object and the GetterFunction at the
        // same time (from one of the Make*Getter functions below).
        struct TransformedContainer {
            // The container or array.
            void* ptr;

            // The transform being applied to the container.
            TransformPtr transform;
        };

        // Applies transform (at the correct type) to the argument and returns the
        // result. Does some validity checking to make sure the result is not a
        // temporary (proxy containers are not allowed).
        template <typename T, typename Transform, typename U>
        T& apply_transform(TransformPtr transform, U& u) { // NOLINT(runtime/references)
            const auto t = transform.get<Transform>();
            TURBO_RAW_DCHECK(t != nullptr, "pointer cannot be null");

            // If compilation fails here due to dropping a const qualifier, it usually
            // means you tried to wrap a const container with a non-const AnySpan.
            //
            // If compilation fails here due to a reference to a temporary, it usually
            // means that the container or transform cannot be converted to a reference to
            // T. AnySpan requires that a valid instance of T exists, conversion to a
            // value type (such as string -> StringPiece or int -> float) is not
            // supported.
            //
            // If compilation fails here due to taking the address of a temporary object,
            // it means that the transform is returning a temporary. This is disallowed.
            // Transforms must return a reference to T, or a reference to an object that
            // can be safely converted to a reference to T (such as a reference_wrapper or
            // a class that inherits from T).
            return *&std::invoke(*t, u); // Failed compilation? See above.
        }

        // The return type of GetterFunction<T>. GetterFunctions return non-const
        // references to support mutable -> const conversion of spans without additional
        // indirection.
        template <typename T>
        using GetterFunctionResult = std::remove_const_t<T>&;

        // A type of function pointer that can return an element of a
        // TransformedContainer.  This is used so that we can type-erase with a simple
        // function pointer instead of a vtable. Doing things this way has less pointer
        // indirection.
        template <typename T>
        using GetterFunction = GetterFunctionResult<T> (*)(const TransformedContainer&,
            std::size_t);

        // A GetterFunction that works on arrays.
        template <typename T, typename Element, typename Transform>
        GetterFunctionResult<T> get_from_array(const TransformedContainer& container,
            std::size_t i) {
            TURBO_RAW_DCHECK(container.ptr != nullptr, "cannot dereference null pointer");
            auto* array = static_cast<Element*>(container.ptr);
            return const_cast<GetterFunctionResult<T>>(
                apply_transform<T, Transform>(container.transform, array[i]));
        }

        // A GetterFunction that works on containers.
        template <typename T, typename Container, typename Transform>
        GetterFunctionResult<T> get_from_container(const TransformedContainer& container,
            std::size_t i) {
            TURBO_RAW_DCHECK(container.ptr != nullptr, "cannot dereference null pointer");
            Container& c = *static_cast<Container*>(container.ptr);
            return const_cast<GetterFunctionResult<T>>(
                apply_transform<T, Transform>(container.transform, c[i]));
        }

        // A GetterFunction that crashes, indicating an invalid AnySpan has been
        // accessed..
        template <typename T>
        GetterFunctionResult<T> get_from_uninitialized(const TransformedContainer&,
            std::size_t) {
            TURBO_RAW_LOG(FATAL, "Uninitialized AnySpan access.");
        }

        //
        // array_tag and ptr_array_tag are GetterFunctions that are never called. They are
        // used as a tag so Getter::Get can access flat arrays without a function
        // pointer.
        //

        template <typename T>
        GetterFunctionResult<T> array_tag(const TransformedContainer&, std::size_t) {
            TURBO_RAW_LOG(FATAL, "array_tag should never be called.");
        }

        template <typename T>
        GetterFunctionResult<T> ptr_array_tag(const TransformedContainer&, std::size_t) {
            TURBO_RAW_LOG(FATAL, "ptr_array_tag should never be called.");
        }



        // Element type of container when elements are dereferenced.
        template <class Container>
        using DerefElementType = std::remove_reference_t<decltype(*std::declval<ElementType<Container>>())>;

        // DataIsValid is true_type if Container has a data() member that returns a
        // pointer to the type returned by operator[], false_type if there is no data()
        // member or if the types disagree.
        template <class Container>
        using DataIsValid = std::is_same<ElementType<Container>*, typename TypeOfData<Container>::type>;

        // Used to access elements of a container or array.
        template <typename T>
        struct Getter {
            Getter() { }

            // Handle mutable -> const conversion.
            template <typename LazyT = T,
                typename = std::enable_if_t<std::is_const_v<LazyT>>>
            explicit Getter(const Getter<std::remove_const_t<T>>& other) {
                using MutableT = std::remove_const_t<T>;
                if (other.fun == &array_tag<MutableT>) {
                    TURBO_RAW_DCHECK(other.offset == 0u, "offset must be zero");
                    fun = &array_tag<T>;
                    array = other.array;
                    offset = 0;
                } else if (other.fun == &ptr_array_tag<MutableT>) {
                    TURBO_RAW_DCHECK(other.offset == 0u, "offset must be zero");
                    fun = &ptr_array_tag<T>;
                    ptr_array = other.ptr_array;
                    offset = 0;
                } else {
                    fun = other.fun;
                    container = other.container;
                    offset = other.offset;
                }
            }

            // Returns the element at the given index.
            T& Get(std::size_t index) const {
                TURBO_RAW_DCHECK(fun != nullptr, "pointer cannot be null");
                if (KUMO_LIKELY(fun == &array_tag<T>)) {
                    TURBO_RAW_DCHECK(array != nullptr, "pointer cannot be null");
                    return array[index];
                }
                if (fun == &ptr_array_tag<T>) {
                    TURBO_RAW_DCHECK(ptr_array != nullptr, "pointer cannot be null");
                    return *ptr_array[index];
                }
                return fun(container, index + offset);
            }

            // Returns a Getter offset into this one by pos.
            Getter Offset(std::size_t pos) const {
                // Special case when offset is zero. This safely handles empty spans and
                // empty containers where data() can be null.
                if (pos == 0) {
                    return *this;
                }
                TURBO_RAW_DCHECK(fun != nullptr, "pointer cannot be null");
                Getter result;
                result.fun = fun;
                if (fun == &array_tag<T>) {
                    TURBO_RAW_DCHECK(array != nullptr, "pointer cannot be null");
                    TURBO_RAW_DCHECK(offset == 0u, "offset must be zero");
                    result.array = array + pos;
                    result.offset = 0;
                } else if (fun == &ptr_array_tag<T>) {
                    TURBO_RAW_DCHECK(ptr_array != nullptr, "pointer cannot be null");
                    TURBO_RAW_DCHECK(offset == 0u, "offset must be zero");
                    result.ptr_array = ptr_array + pos;
                    result.offset = 0;
                } else {
                    TURBO_RAW_DCHECK(container.ptr != nullptr, "pointer cannot be null");
                    result.container = container;
                    result.offset = offset + pos;
                }
                return result;
            }

            // A pointer to a function (or tag function) that specifies how to get an
            // element from the array or container.
            GetterFunction<T> fun = &get_from_uninitialized<T>;

            union {
                T* array; // Active if fun == array_tag<T>.
                T* const* ptr_array; // Active if fun == ptr_array_tag<T>.
                TransformedContainer container; // Active for all other fun.
            };

            // Offset into container. Always 0 for array or ptr_array.
            std::size_t offset = 0;
        };

        //
        // make_array_getter returns a Getter for an array.
        //
        // MakeArrayGetterImpl is specialised to use array_tag<T> when Element == T and
        // Transform == IdentityT, and to use ptr_array_tag<T> when Element == T* and
        // Transform == DerefT.
        //

        template <typename SpanElement, typename ArrayElement, typename Transform>
        struct MakeArrayGetterImpl {
            template <typename U>
            static Getter<U> Make(ArrayElement* array, const Transform& transform) {
                Getter<U> result;
                result.fun = &get_from_array<U, ArrayElement, Transform>;
                result.container.ptr = const_cast<void*>(static_cast<const void*>(array));
                result.container.transform = TransformPtr(transform, IsTransformCopied<Transform> { });
                return result;
            }
        };

        // When the span and the array are the same type.
        template <typename T>
        struct MakeArrayGetterImpl<T, T, any_span_transform::IdentityT> {
            template <typename U>
            static Getter<U> Make(T* array, const any_span_transform::IdentityT&) {
                Getter<U> result;
                result.fun = &array_tag<U>;
                result.array = array;
                return result;
            }
        };

        // If we are dereferencing an array of mutable elements (T*), it is safe to add
        // constness (const T*).
        template <typename T>
        struct MakeArrayGetterImpl<const T, T, any_span_transform::IdentityT>
            : public MakeArrayGetterImpl<const T, const T,
                  any_span_transform::IdentityT> { };

        // When the array is of pointers to elements of the same type as the span.
        template <typename T>
        struct MakeArrayGetterImpl<T, T*, any_span_transform::DerefT> {
            template <typename U>
            static Getter<U> Make(T* const* ptr_array,
                const any_span_transform::DerefT&) {
                Getter<U> result;
                result.fun = &ptr_array_tag<U>;
                result.ptr_array = ptr_array;
                return result;
            }
        };

        // If we are dereferencing an array that is mutable along any extent, it is safe
        // to add constness.
        template <typename T>
        struct MakeArrayGetterImpl<const T, T*, any_span_transform::DerefT>
            : public MakeArrayGetterImpl<const T, const T*,
                  any_span_transform::DerefT> { };

        template <typename T>
        struct MakeArrayGetterImpl<const T, T* const, any_span_transform::DerefT>
            : public MakeArrayGetterImpl<const T, const T*,
                  any_span_transform::DerefT> { };

        template <typename T>
        struct MakeArrayGetterImpl<const T, const T* const, any_span_transform::DerefT>
            : public MakeArrayGetterImpl<const T, const T*,
                  any_span_transform::DerefT> { };

        template <typename T, typename Element, typename Transform>
        Getter<T> make_array_getter(Element* array, const Transform& transform) {
            return MakeArrayGetterImpl<T, Element, Transform>::template Make<T>(
                array, transform);
        }

        //
        // make_container_getter returns a Getter for a given container. It will pass
        // container.data() to make_array_getter if possible, which allows element access
        // to be inline.
        //
        // The first argument to MakeArrayGetterImpl is expected to be of type
        // DataIsValid<container>.
        //

        template <typename T, typename Container, typename Transform>
        Getter<T> make_container_getter_impl(
            std::true_type /* DataIsValid<Container> */,
            Container& container, // NOLINT(runtime/references)
            const Transform& transform) {
            return make_array_getter<T, ElementType<Container>, Transform>(container.data(),
                transform);
        }

        template <typename T, typename Container, typename Transform>
        Getter<T> make_container_getter_impl(
            std::false_type /* DataIsValid<Container> */,
            Container& container, // NOLINT(runtime/references)
            const Transform& transform) {
            Getter<T> result;
            result.fun = &get_from_container<T, Container, Transform>;
            result.container.ptr = const_cast<void*>(static_cast<const void*>(&container));
            result.container.transform = TransformPtr(transform, IsTransformCopied<Transform> { });

            return result;
        }

        template <typename T, typename Container, typename Transform>
        Getter<T> make_container_getter(
            Container& container, // NOLINT(runtime/references)
            const Transform& transform) {
            static_assert(std::is_reference_v<decltype(container[0])>,
                "AnySpan only works with containers that return a reference "
                "(no vector<bool>, or containers that return by value).");
            return make_container_getter_impl<T>(DataIsValid<Container>(), container,
                transform);
        }

        // Used for testing. Returns true if the given AnySpan performs inline element
        // access.
        template <typename T>
        bool IsCheap(AnySpan<T> s) {
            return s.getter_.fun == &array_tag<T> || s.getter_.fun == &ptr_array_tag<T>;
        }


        template <typename T>
        bool EqualImpl(AnySpan<T> a, AnySpan<T> b) {
            static_assert(std::is_const_v<T>, "");
            return std::equal(a.begin(), a.end(), b.begin(), b.end());
        }

    } // namespace any_span_internal

} // namespace turbo

#endif // TURBO_TYPES_INTERNAL_ANY_SPAN_H_
