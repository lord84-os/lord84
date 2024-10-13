#include <stdint.h>
#include <stddef.h>

void *memset(void *dest, int c, uint64_t n){
    for(uint64_t i = 0; i < n; i++){
        *((uint8_t*)dest+i) = c;
    }

    return dest;
}

void *memcpy(void *dest, const void *src, uint64_t n){
    for(uint64_t i = 0; i < n; i++){
        *((uint8_t*)dest+i) =  *((uint8_t*)src+i);
    }

    return dest;
}

/* stolen from limine c template */
void *memmove(void *dest, const void *src, uint64_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, uint64_t n){
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;

}

uint64_t strlen(const char* str){
    uint64_t i = 0;

    while (str[i] != '\0') {
        i++;
    }

    return i;
}

