#if UNICODE_IMPLEMENTATION_HASWELL
KUMO_UNTARGET_REGION
#endif

#undef UNICODE_IMPLEMENTATION
#undef UNICODE_SIMD_HAS_BYTEMASK

#if UNICODE_GCC11ORMORE // workaround for
                        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105593
KUMO_PRAGMA_DIAG_POP
#endif // end of workaround
