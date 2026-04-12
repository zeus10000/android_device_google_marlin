/*
 * Permissive malloc for legacy vendor binaries with heap corruption bugs.
 *
 * Each allocation gets extra padding to absorb buffer overflows that would
 * otherwise corrupt adjacent chunk headers. Large allocations get their own
 * mmap with a guard page. Small allocations come from an arena with 64-byte
 * padding between each allocation.
 */
#include <sys/mman.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#define HEADER_MAGIC 0x504D414C4C4F4355ULL
#define GUARD_SIZE 256
#define LARGE_THRESHOLD 2048

struct alloc_header {
    uint64_t magic;
    size_t usable_size;
    size_t total_size;
    uint64_t pad;
};

static pthread_mutex_t arena_lock = PTHREAD_MUTEX_INITIALIZER;
static char *arena_base;
static char *arena_ptr;
static char *arena_end;
#define ARENA_SIZE (16 * 1024 * 1024)

static void arena_init(void) {
    if (arena_base) return;
    arena_base = (char *)mmap(NULL, ARENA_SIZE, PROT_READ | PROT_WRITE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena_base == MAP_FAILED) { arena_base = NULL; return; }
    arena_ptr = arena_base;
    arena_end = arena_base + ARENA_SIZE;
}

static void *arena_alloc(size_t size) {
    size_t total = sizeof(struct alloc_header) + size + GUARD_SIZE;
    total = (total + 31) & ~31;
    pthread_mutex_lock(&arena_lock);
    arena_init();
    if (!arena_base || arena_ptr + total > arena_end) {
        pthread_mutex_unlock(&arena_lock);
        return NULL;
    }
    struct alloc_header *hdr = (struct alloc_header *)arena_ptr;
    arena_ptr += total;
    pthread_mutex_unlock(&arena_lock);
    hdr->magic = HEADER_MAGIC;
    hdr->usable_size = size;
    hdr->total_size = total;
    return (void *)(hdr + 1);
}

void *malloc(size_t size) {
    if (size == 0) size = 1;
    if (size < LARGE_THRESHOLD) {
        void *p = arena_alloc(size);
        if (p) return p;
    }
    size_t total = sizeof(struct alloc_header) + size + GUARD_SIZE;
    total = (total + 4095) & ~4095;
    void *ptr = mmap(NULL, total + 4096, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) return NULL;
    mprotect((char *)ptr + total, 4096, PROT_NONE);
    struct alloc_header *hdr = (struct alloc_header *)ptr;
    hdr->magic = HEADER_MAGIC | 1;
    hdr->usable_size = size;
    hdr->total_size = total + 4096;
    return (void *)(hdr + 1);
}

void free(void *ptr) {
    if (!ptr) return;
    struct alloc_header *hdr = ((struct alloc_header *)ptr) - 1;
    if ((hdr->magic & ~1ULL) != HEADER_MAGIC) return;
    if (hdr->magic & 1) {
        munmap(hdr, hdr->total_size);
    }
}

void *calloc(size_t n, size_t size) {
    size_t total = n * size;
    if (n && total / n != size) return NULL;
    void *p = malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (!size) { free(ptr); return NULL; }
    struct alloc_header *hdr = ((struct alloc_header *)ptr) - 1;
    size_t old_size = size;
    if ((hdr->magic & ~1ULL) == HEADER_MAGIC)
        old_size = hdr->usable_size;
    void *new_ptr = malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size < size ? old_size : size);
        free(ptr);
    }
    return new_ptr;
}

size_t malloc_usable_size(void *ptr) {
    if (!ptr) return 0;
    struct alloc_header *hdr = ((struct alloc_header *)ptr) - 1;
    if ((hdr->magic & ~1ULL) != HEADER_MAGIC) return 0;
    return hdr->usable_size + GUARD_SIZE;
}

void *memalign(size_t alignment, size_t size) {
    (void)alignment;
    return malloc(size);
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    (void)alignment;
    *memptr = malloc(size);
    return *memptr ? 0 : 12;
}
