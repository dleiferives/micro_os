#ifndef LRU_CACHE_H_
#define LRU_CACHE_H_
#include <stdint.h>



typedef struct{
	uint32_t *keys;
	uint8_t *values;
	uint32_t *key_rets;
	uint32_t ret_num;
	uint32_t capacity;
	uint32_t size;
	uint32_t value_capacity;
}Cache_t;
typedef uint8_t *(*Cache_get_t)(Cache_t *cache, uint32_t key);
uint8_t *Cache_get(Cache_t *cache, uint32_t key);
uint8_t Cache_contains(Cache_t *cache, uint32_t key);











#endif // LRU_CACHE_H_

