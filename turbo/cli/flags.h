// Copyright (c) 2024, Kumo inc. and its affiliates.
//
// Inspired by gflags (Google Flags), this file provides a gflags-compatible
// flag definition and registration system, with namespacing and naming
// adjusted to avoid conflicts with other gflags-using libraries.
//
// XDEFINE_bool / XDEFINE_int32 / XDEFINE_uint32 / XDEFINE_int64 / XDEFINE_uint64 / XDEFINE_double / XDEFINE_string
// XDECLARE_bool / XDECLARE_int32 / XDECLARE_uint32 / XDECLARE_int64 / XDECLARE_uint64 / XDECLARE_double / XDECLARE_string
// XFLAGS_<name>  — access the flag value directly as a global variable.

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <cctype>
#include <map>
#include <string>
#include <vector>

// ============================================================
// Integer type aliases (mirror gflags but use standard names)
// ============================================================
// Users write XDEFINE_int32 / XDEFINE_uint32 / XDEFINE_int64 / XDEFINE_uint64
// The underlying types are int32_t / uint32_t / int64_t / uint64_t.

// ============================================================
// Macros to suppress "unused" warnings
// ============================================================
#if defined(__GNUC__) || defined(__clang__)
#define XCLI_MAYBE_UNUSED __attribute__((unused))
#else
#define XCLI_MAYBE_UNUSED
#endif

// ============================================================
// Forward declarations
// ============================================================
namespace xcli {
namespace detail {

// Portable case-insensitive string comparison
inline bool iequal(const char *a, const char *b) {
    while (*a && *b) {
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b)))
            return false;
        ++a; ++b;
    }
    return *a == *b;
}

enum class FlagType : uint8_t {
    Bool = 0,
    Int32 = 1,
    UInt32 = 2,
    Int64 = 3,
    UInt64 = 4,
    Double = 5,
    String = 6,
};

class FlagValue;
class CommandFlag;
class FlagRegistry;

// ============================================================
// FlagValue — type-erased value storage
// ============================================================
class FlagValue {
    void *const storage_;
    FlagType const type_;
    bool const owns_;

  public:
    template <typename T>
    FlagValue(T *storage, bool owns)
        : storage_(storage), type_(flag_type_for<T>()), owns_(owns) {}

    ~FlagValue();

    FlagType type() const { return type_; }

    bool parse_from(const std::string &value);
    std::string flags_to_string() const;
    bool equal(const FlagValue &other) const;

    void *storage() const { return storage_; }

  private:
    template <typename T>
    static FlagType flag_type_for();
};

// ============================================================
// CommandFlag — one registered flag
// ============================================================
class CommandFlag {
    std::string name_;
    std::string help_;
    std::string file_;
    FlagValue *current_;
    FlagValue *defvalue_;

  public:
    CommandFlag(const char *name, const char *help, const char *file,
                FlagValue *current, FlagValue *defvalue);

    const std::string &name() const { return name_; }
    const std::string &help() const { return help_; }
    const std::string &file() const { return file_; }

    std::string current_value() const;
    std::string default_value() const;
    FlagType type() const;

    bool set_value(const std::string &value);
    std::string flags_type_name() const;
};

// ============================================================
// FlagRegistry — singleton map of name → flag
// ============================================================
class FlagRegistry {
    std::map<std::string, CommandFlag *> flags_;

    FlagRegistry() = default;
    ~FlagRegistry();

  public:
    FlagRegistry(const FlagRegistry &) = delete;
    FlagRegistry &operator=(const FlagRegistry &) = delete;

    static FlagRegistry &global_registry();

    void flags_register(CommandFlag *flag);
    CommandFlag *find(const std::string &name) const;

    bool set_flag(const std::string &name, const std::string &value);
    CommandFlag *set_flag(const std::string &name, const std::string &value,
                         std::string &msg);

    std::vector<const CommandFlag *> get_all_flags() const;
};

// ============================================================
// FlagRegisterer — static constructor helper
// ============================================================
class FlagRegisterer {
  public:
    template <typename T>
    FlagRegisterer(const char *name,
                   const char *help,
                   const char *file,
                   T *current_storage,
                   T *defvalue_storage) {
        FlagValue *current = new FlagValue(current_storage, false);
        FlagValue *defvalue = new FlagValue(defvalue_storage, false);
        CommandFlag *flag = new CommandFlag(name, help, file, current, defvalue);
        FlagRegistry::global_registry().flags_register(flag);
    }
};

// ============================================================
// Free functions
// ============================================================
bool set_flag(const std::string &name, const std::string &value);
bool set_flag(const std::string &name, const std::string &value,
             std::string &msg);

// ============================================================
// FlagValue template specializations
// ============================================================
template <> inline FlagType FlagValue::flag_type_for<bool>() { return FlagType::Bool; }
template <> inline FlagType FlagValue::flag_type_for<int32_t>() { return FlagType::Int32; }
template <> inline FlagType FlagValue::flag_type_for<uint32_t>() { return FlagType::UInt32; }
template <> inline FlagType FlagValue::flag_type_for<int64_t>() { return FlagType::Int64; }
template <> inline FlagType FlagValue::flag_type_for<uint64_t>() { return FlagType::UInt64; }
template <> inline FlagType FlagValue::flag_type_for<double>() { return FlagType::Double; }
template <> inline FlagType FlagValue::flag_type_for<std::string>() { return FlagType::String; }

// ============================================================
// FlagValue implementation
// ============================================================
inline FlagValue::~FlagValue() {
    if (owns_) {
        switch (type_) {
            case FlagType::Bool:    delete reinterpret_cast<bool*>(storage_); break;
            case FlagType::Int32:   delete reinterpret_cast<int32_t*>(storage_); break;
            case FlagType::UInt32:  delete reinterpret_cast<uint32_t*>(storage_); break;
            case FlagType::Int64:   delete reinterpret_cast<int64_t*>(storage_); break;
            case FlagType::UInt64:  delete reinterpret_cast<uint64_t*>(storage_); break;
            case FlagType::Double:  delete reinterpret_cast<double*>(storage_); break;
            case FlagType::String:  delete reinterpret_cast<std::string*>(storage_); break;
        }
    }
}

inline bool FlagValue::parse_from(const std::string &value) {
    switch (type_) {
        case FlagType::Bool: {
            const char *kTrue[]  = {"1", "t", "true", "y", "yes"};
            const char *kFalse[] = {"0", "f", "false", "n", "no"};
            for (auto *s : kTrue)
                if (iequal(value.c_str(), s)) {
                    *reinterpret_cast<bool*>(storage_) = true;
                    return true;
                }
            for (auto *s : kFalse)
                if (iequal(value.c_str(), s)) {
                    *reinterpret_cast<bool*>(storage_) = false;
                    return true;
                }
            return false;
        }
        case FlagType::String: {
            *reinterpret_cast<std::string*>(storage_) = value;
            return true;
        }
        default: {
            if (value.empty()) return false;
            char *end = nullptr;
            int base = 10;
            if (value.size() > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                base = 16;
            std::string v = value;
            const char *cstr = v.c_str();
            errno = 0;

            switch (type_) {
                case FlagType::Int32: {
                    int64_t r = strtoll(cstr, &end, base);
                    if (errno != 0 || end != cstr + v.size()) return false;
                    if (static_cast<int32_t>(r) != r) return false;
                    *reinterpret_cast<int32_t*>(storage_) = static_cast<int32_t>(r);
                    return true;
                }
                case FlagType::UInt32: {
                    const char *p = cstr;
                    while (*p == ' ') p++;
                    if (*p == '-') return false;
                    uint64_t r = strtoull(p, &end, base);
                    if (errno != 0 || end != p + strlen(p)) return false;
                    if (static_cast<uint32_t>(r) != r) return false;
                    *reinterpret_cast<uint32_t*>(storage_) = static_cast<uint32_t>(r);
                    return true;
                }
                case FlagType::Int64: {
                    int64_t r = strtoll(cstr, &end, base);
                    if (errno != 0 || end != cstr + v.size()) return false;
                    *reinterpret_cast<int64_t*>(storage_) = r;
                    return true;
                }
                case FlagType::UInt64: {
                    const char *p = cstr;
                    while (*p == ' ') p++;
                    if (*p == '-') return false;
                    uint64_t r = strtoull(p, &end, base);
                    if (errno != 0 || end != p + strlen(p)) return false;
                    *reinterpret_cast<uint64_t*>(storage_) = r;
                    return true;
                }
                case FlagType::Double: {
                    double r = strtod(cstr, &end);
                    if (errno != 0 || end != cstr + v.size()) return false;
                    *reinterpret_cast<double*>(storage_) = r;
                    return true;
                }
                default:
                    return false;
            }
        }
    }
}

inline std::string FlagValue::flags_to_string() const {
    char buf[64];
    switch (type_) {
        case FlagType::Bool:
            return *reinterpret_cast<const bool*>(storage_) ? "true" : "false";
        case FlagType::Int32:
            snprintf(buf, sizeof(buf), "%" PRId32, *reinterpret_cast<const int32_t*>(storage_));
            return buf;
        case FlagType::UInt32:
            snprintf(buf, sizeof(buf), "%" PRIu32, *reinterpret_cast<const uint32_t*>(storage_));
            return buf;
        case FlagType::Int64:
            snprintf(buf, sizeof(buf), "%" PRId64, *reinterpret_cast<const int64_t*>(storage_));
            return buf;
        case FlagType::UInt64:
            snprintf(buf, sizeof(buf), "%" PRIu64, *reinterpret_cast<const uint64_t*>(storage_));
            return buf;
        case FlagType::Double:
            snprintf(buf, sizeof(buf), "%.17g", *reinterpret_cast<const double*>(storage_));
            return buf;
        case FlagType::String:
            return *reinterpret_cast<const std::string*>(storage_);
    }
    return {};
}

inline bool FlagValue::equal(const FlagValue &other) const {
    if (type_ != other.type_) return false;
    switch (type_) {
        case FlagType::Bool:    return *reinterpret_cast<const bool*>(storage_) == *reinterpret_cast<const bool*>(other.storage_);
        case FlagType::Int32:   return *reinterpret_cast<const int32_t*>(storage_) == *reinterpret_cast<const int32_t*>(other.storage_);
        case FlagType::UInt32:  return *reinterpret_cast<const uint32_t*>(storage_) == *reinterpret_cast<const uint32_t*>(other.storage_);
        case FlagType::Int64:   return *reinterpret_cast<const int64_t*>(storage_) == *reinterpret_cast<const int64_t*>(other.storage_);
        case FlagType::UInt64:  return *reinterpret_cast<const uint64_t*>(storage_) == *reinterpret_cast<const uint64_t*>(other.storage_);
        case FlagType::Double:  return *reinterpret_cast<const double*>(storage_) == *reinterpret_cast<const double*>(other.storage_);
        case FlagType::String:  return *reinterpret_cast<const std::string*>(storage_) == *reinterpret_cast<const std::string*>(other.storage_);
    }
    return false;
}

// ============================================================
// CommandFlag implementation
// ============================================================
inline CommandFlag::CommandFlag(const char *name, const char *help, const char *file,
                                 FlagValue *current, FlagValue *defvalue)
    : name_(name), help_(help ? help : ""), file_(file ? file : ""),
      current_(current), defvalue_(defvalue) {}

inline std::string CommandFlag::current_value() const {
    return current_->flags_to_string();
}

inline std::string CommandFlag::default_value() const {
    return defvalue_->flags_to_string();
}

inline FlagType CommandFlag::type() const {
    return defvalue_->type();
}

inline bool CommandFlag::set_value(const std::string &value) {
    return current_->parse_from(value);
}

inline std::string CommandFlag::flags_type_name() const {
    switch (type()) {
        case FlagType::Bool:   return "bool";
        case FlagType::Int32:  return "int32";
        case FlagType::UInt32: return "uint32";
        case FlagType::Int64:  return "int64";
        case FlagType::UInt64: return "uint64";
        case FlagType::Double: return "double";
        case FlagType::String: return "string";
    }
    return {};
}

// ============================================================
// FlagRegistry implementation
// ============================================================
inline FlagRegistry &FlagRegistry::global_registry() {
    static FlagRegistry instance;
    return instance;
}

inline FlagRegistry::~FlagRegistry() {
    for (auto &pair : flags_)
        delete pair.second;
}

inline void FlagRegistry::flags_register(CommandFlag *flag) {
    if (flag == nullptr) return;
    auto result = flags_.emplace(flag->name(), flag);
    if (!result.second) {
        fprintf(stderr, "ERROR: flag '%s' defined more than once\n", flag->name().c_str());
        std::_Exit(1);
    }
}

inline CommandFlag *FlagRegistry::find(const std::string &name) const {
    auto it = flags_.find(name);
    if (it != flags_.end()) return it->second;
    // Also try with dashes replaced by underscores
    std::string alt = name;
    for (auto &c : alt)
        if (c == '-') c = '_';
    if (alt != name) {
        it = flags_.find(alt);
        if (it != flags_.end()) return it->second;
    }
    return nullptr;
}

inline bool FlagRegistry::set_flag(const std::string &name,
                                   const std::string &value) {
    CommandFlag *flag = find(name);
    if (flag == nullptr) return false;
    return flag->set_value(value);
}

inline CommandFlag *FlagRegistry::set_flag(const std::string &name,
                                           const std::string &value,
                                           std::string &msg) {
    CommandFlag *flag = find(name);
    if (flag == nullptr) {
        msg.clear();
        return nullptr;
    }
    if (flag->set_value(value)) {
        msg = flag->name() + " set to " + value;
    } else {
        msg.clear();
    }
    return flag;
}

inline std::vector<const CommandFlag *> FlagRegistry::get_all_flags() const {
    std::vector<const CommandFlag *> result;
    result.reserve(flags_.size());
    for (const auto &pair : flags_)
        result.push_back(pair.second);
    return result;
}

// ============================================================
// Free function implementations
// ============================================================
inline bool set_flag(const std::string &name, const std::string &value) {
    return FlagRegistry::global_registry().set_flag(name, value);
}

inline bool set_flag(const std::string &name, const std::string &value,
                     std::string &msg) {
    return FlagRegistry::global_registry().set_flag(name, value, msg) != nullptr;
}

}  // namespace detail
}  // namespace xcli

// ============================================================
// XDECLARE macros — declare a flag from another file
// ============================================================
// The type-specific namespace (fLB, fLI, etc.) ensures that
// XDECLARE_bool(name) and XDECLARE_int32(name) refer to different
// variables, catching type mismatches at link time.

#define XDECLARE_bool(name)                                   \
    namespace xcli { namespace _fLB { extern bool XFLAGS_##name; } } \
    using xcli::_fLB::XFLAGS_##name

#define XDECLARE_int32(name)                                   \
    namespace xcli { namespace _fLI { extern int32_t XFLAGS_##name; } } \
    using xcli::_fLI::XFLAGS_##name

#define XDECLARE_uint32(name)                                   \
    namespace xcli { namespace _fLU { extern uint32_t XFLAGS_##name; } } \
    using xcli::_fLU::XFLAGS_##name

#define XDECLARE_int64(name)                                   \
    namespace xcli { namespace _fLI64 { extern int64_t XFLAGS_##name; } } \
    using xcli::_fLI64::XFLAGS_##name

#define XDECLARE_uint64(name)                                   \
    namespace xcli { namespace _fLU64 { extern uint64_t XFLAGS_##name; } } \
    using xcli::_fLU64::XFLAGS_##name

#define XDECLARE_double(name)                                   \
    namespace xcli { namespace _fLD { extern double XFLAGS_##name; } } \
    using xcli::_fLD::XFLAGS_##name

#define XDECLARE_string(name)                                   \
    namespace xcli { namespace _fLS { extern std::string XFLAGS_##name; } } \
    using xcli::_fLS::XFLAGS_##name

// ============================================================
// XDEFINE macros — define a flag
// ============================================================
// Each XDEFINE creates:
//   1. A global XFLAGS_<name> variable
//   2. A static default-value copy
//   3. A static FlagRegisterer that auto-registers with the global
//      FlagRegistry at static-init time.

#define XDEFINE_bool(name, val, desc)                                          \
    static const bool _XFLAGS_sv_##name = val;                                 \
    namespace xcli { namespace _fLB { bool XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLB::XFLAGS_##name;                                            \
    static bool _XFLAGS_def_##name = _XFLAGS_sv_##name;                         \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_int32(name, val, desc)                                         \
    static const int32_t _XFLAGS_sv_##name = val;                               \
    namespace xcli { namespace _fLI { int32_t XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLI::XFLAGS_##name;                                            \
    static int32_t _XFLAGS_def_##name = _XFLAGS_sv_##name;                      \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_uint32(name, val, desc)                                        \
    static const uint32_t _XFLAGS_sv_##name = val;                              \
    namespace xcli { namespace _fLU { uint32_t XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLU::XFLAGS_##name;                                            \
    static uint32_t _XFLAGS_def_##name = _XFLAGS_sv_##name;                     \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_int64(name, val, desc)                                         \
    static const int64_t _XFLAGS_sv_##name = val;                               \
    namespace xcli { namespace _fLI64 { int64_t XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLI64::XFLAGS_##name;                                          \
    static int64_t _XFLAGS_def_##name = _XFLAGS_sv_##name;                      \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_uint64(name, val, desc)                                        \
    static const uint64_t _XFLAGS_sv_##name = val;                              \
    namespace xcli { namespace _fLU64 { uint64_t XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLU64::XFLAGS_##name;                                          \
    static uint64_t _XFLAGS_def_##name = _XFLAGS_sv_##name;                     \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_double(name, val, desc)                                        \
    static const double _XFLAGS_sv_##name = val;                                \
    namespace xcli { namespace _fLD { double XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLD::XFLAGS_##name;                                            \
    static double _XFLAGS_def_##name = _XFLAGS_sv_##name;                       \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)

#define XDEFINE_string(name, val, desc)                                        \
    static const std::string _XFLAGS_sv_##name{val};                           \
    namespace xcli { namespace _fLS { std::string XFLAGS_##name = _XFLAGS_sv_##name; } } \
    using xcli::_fLS::XFLAGS_##name;                                            \
    static std::string _XFLAGS_def_##name = _XFLAGS_sv_##name;                  \
    static ::xcli::detail::FlagRegisterer _XFLAGS_reg_##name XCLI_MAYBE_UNUSED( \
        #name, desc, __FILE__, &XFLAGS_##name, &_XFLAGS_def_##name)
