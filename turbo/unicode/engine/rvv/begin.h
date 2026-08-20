#define UNICODE_IMPLEMENTATION rvv

#if UNICODE_CAN_ALWAYS_RUN_RVV
// nothing needed.
#else
UNICODE_TARGET_RVV
#endif
