#include "lru_cache.h"

// int main(){
// 	uint32_t a[10] = {0};
// 	uint8_t values[10 * 1024] = {0};
// 	uint32_t key_rets[10] = {0};
// 	Cache_t cache = {
// 		.capacity = 10,
// 		.size = 0,
// 		.keys = &a[0],
// 		.values = &values[0],
// 		.key_rets = &key_rets[0],
// 		.value_capacity = 1024,
// 	};
// }

uint8_t *Cache_get(Cache_t *cache, uint32_t key){
	cache->ret_num++;
	// check if key already exists
	for(int i = 0; i < cache->size; i++){
		if(cache->keys[i] == key){
			cache->key_rets[i] = cache->ret_num;
			return &cache->values[i * cache->value_capacity];
		}
	}
	// check if cache is full
	if(cache->size < cache->capacity){
		cache->keys[cache->size] = key;
		cache->key_rets[cache->size] = cache->ret_num;
		cache->size++;
		return &cache->values[(cache->size-1) * cache->value_capacity];
	}
	// cache eviction needed, find the least recently used key
	uint32_t lru_index = 0;
	uint32_t min_ret = cache->ret_num;
	for(int i = 0; i < cache->size; i++){
		if(cache->key_rets[i] < min_ret){
			min_ret = cache->key_rets[i];
			lru_index = i;
		}
	}
	cache->keys[lru_index] = key;
	cache->key_rets[lru_index] = cache->ret_num;
	return &cache->values[lru_index * cache->value_capacity];
}

uint8_t Cache_contains(Cache_t *cache, uint32_t key){
	for(int i = 0; i < cache->size; i++){
		if(cache->keys[i] == key){
			return 1;
		}
	}
	return 0;
}
