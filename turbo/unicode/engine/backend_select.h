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

#include <cstddef>
#include <turbo/unicode/engine/interface.h>

namespace turbo::internal {


         /// The list of available implementations compiled into simdutf.
        class AvailableImplementationList {
        public:
            /// Get the list of available implementations compiled into simdutf
            KUMO_FORCE_INLINE AvailableImplementationList() { }
            /// Number of implementations
            size_t size() const noexcept;
            /// STL const begin() iterator
            const implementation* const* begin() const noexcept;
            /// STL const end() iterator
            const implementation* const* end() const noexcept;

            /// Get the implementation with the given name.
            ///
            /// Case sensitive.
            ///
            ///     const implementation *impl =
            /// turbo::available_implementations["westmere"]; if (!impl) { exit(1); } if
            /// (!imp->supported_by_runtime_system()) { exit(1); }
            ///     turbo::active_implementation = impl;
            ///
            /// @param name the implementation to find, e.g. "westmere", "haswell", "arm64"
            /// @return the implementation, or nullptr if the parse failed.
            const implementation* operator[](std::string_view name) const noexcept {
                for (const implementation* impl : *this) {
                    if (impl->name() == name) {
                        return impl;
                    }
                }
                return nullptr;
            }

            /// Detect the most advanced implementation supported by the current host.
            ///
            /// This is used to initialize the implementation on startup.
            ///
            ///     const implementation *impl =
            /// turbo::available_implementation::detect_best_supported();
            ///     turbo::active_implementation = impl;
            ///
            /// @return the most advanced supported implementation for the current host, or
            /// an implementation that returns UNSUPPORTED_ARCHITECTURE if there is no
            /// supported implementation. Will never return nullptr.
            const implementation* detect_best_supported() const noexcept;
        };

    template <typename T>
  class atomic_ptr {
    public:
        atomic_ptr(T* _ptr)
            : ptr { _ptr } { }

#if defined(SIMDUTF_NO_THREADS)
        operator const T*() const { return ptr; }
        const T& operator*() const { return *ptr; }
        const T* operator->() const { return ptr; }

        operator T*() { return ptr; }
        T& operator*() { return *ptr; }
        T* operator->() { return ptr; }
        atomic_ptr& operator=(T* _ptr) {
            ptr = _ptr;
            return *this;
        }

#else
        operator const T*() const { return ptr.load(); }
        const T& operator*() const { return *ptr; }
        const T* operator->() const { return ptr.load(); }

        operator T*() { return ptr.load(); }
        T& operator*() { return *ptr; }
        T* operator->() { return ptr.load(); }
        atomic_ptr& operator=(T* _ptr) {
            ptr = _ptr;
            return *this;
        }

#endif

    private:
#if defined(SIMDUTF_NO_THREADS)
        T* ptr;
#else
        std::atomic<T*> ptr;
#endif
    };

    class detect_best_supported_implementation_on_first_use;


} // namespace turbo::internal

namespace turbo {
    /// The list of available implementations compiled into simdutf.
    extern KUMO_DLL const internal::AvailableImplementationList&
    get_available_implementations();

    const implementation* get_default_implementation();


    /// The active implementation.
    ///
    /// Automatically initialized on first use to the most advanced implementation
    /// supported by this hardware.
    extern KUMO_DLL internal::atomic_ptr<const implementation>&
    get_active_implementation();

}