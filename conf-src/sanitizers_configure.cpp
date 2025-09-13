#ifndef __has_feature
// GCC does not have __has_feature...
#define __has_feature(feature) 0
#endif

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
#ifdef __cplusplus
extern "C"
#endif
        const char*
        __asan_default_options()
{
    return "detect_invalid_pointer_pairs=2";
}
#endif
