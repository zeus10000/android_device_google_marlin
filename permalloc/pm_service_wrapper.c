/*
 * Wrapper for pm-service that preloads libpermalloc.so before exec.
 *
 * Android linker strips LD_PRELOAD when AT_SECURE=1 (uid-changing processes).
 * This wrapper dlopens the allocator library first — dlopen loads the symbols
 * into the process before exec, making them available to the linker as
 * RTLD_GLOBAL overrides.
 */
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    /* Load permalloc with RTLD_GLOBAL so its malloc/free symbols override libc */
    void* handle = dlopen("/vendor/lib64/libpermalloc.so", RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "pm-service-wrapper: failed to load libpermalloc: %s\n", dlerror());
    }
    /* exec the real pm-service — the preloaded symbols persist across exec...
     * actually they dont. dlopen in the wrapper process doesnt survive exec.
     *
     * Different approach: just run pm-service in THIS process by calling its main.
     * Since were already loaded with permalloc symbols, they take effect.
