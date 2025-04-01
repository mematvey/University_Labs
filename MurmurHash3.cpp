#include <cstdint>
#include <cstring>

inline uint32_t rotl(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

/**
 * MurmurHash3
 * 
 * @param key   - указатель на данные для хеширования
 * @param len   - длина данных в байтах
 * @param seed  - начальное значение (seed) хеша
 * @param out   - указатель на переменную, куда будет записан 32-битный хеш
 * 
 * @solve
 */
void MurmurHash3 ( const void * key, int len, uint32_t seed, void * out ) {
    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    // Напрямую делаем тип данных key uint8_t для побайтовой обработки
    const uint8_t* data = (const uint8_t*) key;
    const int nblocks = len / 4;

    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);
    
    // Используем цикл в обратном порядке для данных
    for (int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        // Выполняем циклический сдвиг влево
        k1 = rotl(k1, 15);
        k1 *= c2;
        h1 ^= k1;
        h1 = rotl(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    // Обработка "хвоста" 
    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
    uint32_t k1 = 0;
    
    int tail_len = len % 4;
    if (tail_len == 3) {
        k1 ^= tail[2] << 16;
        k1 ^= tail[1] << 8;
        k1 ^= tail[0];
    } else if (tail_len == 2) {
        k1 ^= tail[1] << 8;
        k1 ^= tail[0];
    } else if (tail_len == 1) {
        k1 ^= tail[0];
    }

    if (tail_len != 0) {
        k1 *= c1;
        k1 = rotl(k1, 15);
        k1 *= c2;
        h1 ^= k1;
    }

    // Снова перемешаем
    h1 ^= len;
    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    // Вручную укажем тип для out, так как он void
    *(uint32_t*)out = h1;
}
