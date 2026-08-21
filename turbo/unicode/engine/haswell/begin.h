#define UNICODE_IMPLEMENTATION haswell
#define UNICODE_SIMD_HAS_BYTEMASK 1

#include <turbo/macros/macros/pragma/pragma.h>

#if UNICODE_IMPLEMENTATION_HASWELL
UNICODE_TARGET_HASWELL
#endif

#if UNICODE_GCC11ORMORE // workaround for
                        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105593
KUMO_PRAGMA_DIAG_PUSH
KUMO_PRAGMA_DIAG_IGNORED("-Wmaybe-uninitialized")
#endif // end of workaround
