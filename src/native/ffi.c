#include <lean/lean.h>
#include <yoga/Yoga.h>

/// @param sz must be divisible by `LEAN_OBJECT_SIZE_DELTA`
static inline void* lean_yoga_alloc(size_t sz) {
#ifdef LEAN_YOGA_ALLOC_NATIVE
    return malloc(sz);
#else
    return (void*)lean_alloc_small_object(sz);
#endif
}

/// @param p pointer to memory allocated with `lean_raylib_alloc`
static inline void lean_yoga_free(void* p) {
#ifdef LEAN_YOGA_ALLOC_NATIVE
    free(p);
#else
    lean_free_small_object((lean_object*)p);
#endif
}
