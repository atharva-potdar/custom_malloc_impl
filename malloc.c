#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define WSIZE 4
#define DSIZE 8

typedef uint32_t block_header;

void *heap_start;

void mm_init() {
    heap_start = sbrk(0);
}

size_t get_size(void *block_ptr) {
    uint32_t *header = (uint32_t *)((char *)block_ptr - WSIZE);
    return *header & ~0x7;
}

// size in bytes
// round up to nearest WSIZE
void *mm_malloc(size_t size) {
    
    size_t reqdSize = size + 2 * WSIZE;
    reqdSize += (DSIZE - (reqdSize % DSIZE)) % DSIZE;
    
    for (void *ptr = heap_start; ptr < sbrk(0); ) {
        uint32_t header = *(uint32_t *) ptr;
        size_t block_size = header & ~0x7;
        int allocated = header & 0x1;
        if (!allocated && block_size >= reqdSize) {
            *(uint32_t *) ptr |= 0x1;
            *(uint32_t *)((char *) ptr + block_size - WSIZE) |= 0x1;
            return (uint32_t *)((char *)ptr + WSIZE);
        }
        ptr = (uint32_t *)((char *)ptr + block_size);
    }

    void *p = sbrk(reqdSize);
    *(uint32_t *)p = (reqdSize | 0x1);
    
    *(uint32_t *)((char *) p + reqdSize - WSIZE) = (reqdSize | 0x1);
    
    return (uint32_t *)((char *)p + WSIZE);
}

void mm_free(void *block_ptr) {
    size_t reqdSize = get_size(block_ptr);
    uint32_t *header = (uint32_t *)((char *)block_ptr - WSIZE);
    uint32_t *footer = (uint32_t *)((char *)block_ptr + reqdSize - 2 * WSIZE);
    *header &= ~0x1;
    *footer &= ~0x1;
    bool canCoalescePrev = false;
    uint32_t *prev_header;
    size_t prev_size;
    bool canCoalesceNext = false;
    uint32_t *next_header;
    size_t next_size;
    if (header != heap_start) {
        uint32_t *prev_footer = (uint32_t *)((char *)block_ptr - 2 * WSIZE);
        int allocated = *prev_footer & 0x1;
        if (!allocated) {
            canCoalescePrev = true;
            prev_size = *prev_footer & ~0x7;
            prev_header = (uint32_t *)((char *)block_ptr - WSIZE - prev_size);
        }
    }
    if ((char *)block_ptr + reqdSize - WSIZE < (char *)sbrk(0)) {
        uint32_t *nh = (uint32_t *)((char *)block_ptr + reqdSize - WSIZE);
        int allocated = *nh & 0x1;
        if (!allocated) {
            canCoalesceNext = true;
            next_size = *nh & ~0x7;
            next_header = nh;
        }
    }
    
    if (canCoalescePrev && !canCoalesceNext) {
        *prev_header = (reqdSize + prev_size);
        *prev_header &= ~0x1;
        *footer = (reqdSize + prev_size);
        *footer &= ~0x1;
    }
    else if (!canCoalescePrev && canCoalesceNext) {
        *header = (reqdSize + next_size);
        *header &= ~0x1;
        uint32_t *new_footer = (uint32_t *)((char*)next_header + next_size - WSIZE);
        *new_footer = (reqdSize + next_size);
        *new_footer &= ~0x1;
    }
    else if (canCoalesceNext && canCoalescePrev) {
        *prev_header = (reqdSize + prev_size + next_size);
        *prev_header &= ~0x1;
        uint32_t *new_footer = (uint32_t *)((char*)next_header + next_size - WSIZE);
        *new_footer = (reqdSize + prev_size + next_size);
        *new_footer &= ~0x1;
    }
}

int main(void) {
    mm_init();
    char *a = mm_malloc(10);
    mm_free(a);
    char *b = mm_malloc(10);
    printf("a=%p b=%p same=%d\n", a, b, a == b);
    mm_free(b);
    char *c = mm_malloc(12);
    printf("a=%p b=%p c=%p reused=%d\n", a, b, c, c == a);
    return 0;
}
