#define UNICODE_IMPLEMENTATION icelake

#include <turbo/macros/macros/pragma/pragma.h>

#if UNICODE_IMPLEMENTATION_ICELAKE
UNICODE_TARGET_ICELAKE
#endif

#if UNICODE_GCC11ORMORE // workaround for
                        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105593
KUMO_PRAGMA_DIAG_PUSH
KUMO_PRAGMA_DIAG_IGNORED("-Wmaybe-uninitialized")
#endif // end of workaround
