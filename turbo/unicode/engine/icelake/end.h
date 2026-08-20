#if UNICODE_CAN_ALWAYS_RUN_ICELAKE
// nothing needed.
#else
UNICODE_UNTARGET_REGION
#endif

#undef UNICODE_IMPLEMENTATION

#if UNICODE_GCC11ORMORE // workaround for
                        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105593
KUMO_PRAGMA_DIAG_POP
#endif // end of workaround
