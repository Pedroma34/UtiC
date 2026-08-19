#ifdef _WIN32
#include <UtiC/memory/allocator_system.h>
#include <Windows.h>

typedef struct Win32AllocatorHeader {
        void* base;
        usz total_size;
} Win32AllocatorHeader;

static uptr _align_forward(uptr value, usz alignment) {
    return (value + alignment - 1) & ~(uptr)(alignment - 1);
}

static void* _win32_alloc(usz size, usz alignment, void* user) {
        (void)user;

        if (size == 0)
            return NULL;

        if (alignment == 0)
            alignment = _Alignof(void*);
        if ((alignment & (alignment - 1)) != 0) /* if not power of two */
            return NULL;

        //     base             header             returned block
        //       |                 |                     |
        //       v                 v                     v
        //     [ unused padding ][ Win32AllocatorHeader ][ user bytes ]
        const usz padding  = alignment - 1;
        const usz overhead = sizeof(Win32AllocatorHeader) + padding;
        
        if (padding > (usz)-1 - sizeof(Win32AllocatorHeader))
            return NULL;
        if (size > (usz)-1 - overhead)
            return NULL;

        usz total_size = size + overhead;
        void* base = HeapAlloc(GetProcessHeap(), 0, total_size);
        if (!base)
            return NULL;

        uptr raw = (uptr)base;
        uptr aligned = _align_forward(raw + sizeof(Win32AllocatorHeader), alignment);
        Win32AllocatorHeader* header = (Win32AllocatorHeader*)(aligned - sizeof(Win32AllocatorHeader));
        header->base = base;
        header->total_size = total_size;
        return (void*)aligned;
    }
    
    static void _win32_free(void* block, usz size, usz alignment, void* user) {
        (void)size;
        (void)alignment;
        (void)user;

        if (!block)
            return;

        uptr aligned = (uptr)block;
        Win32AllocatorHeader* header = (Win32AllocatorHeader*)(aligned - sizeof(Win32AllocatorHeader));
        HeapFree(GetProcessHeap(), 0, header->base);
    }

Allocator system_allocator(void) {
    return (Allocator) {
        .alloc = _win32_alloc,
        .free  = _win32_free,
        .user  = NULL
    };
}

#endif
