#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "cache.h"
#include "jbod.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int num_queries = 0;
static int num_hits = 0;
static int cache_amount = 0;

int cache_create(int num_entries) {
  if (cache != NULL) {
    return -1;
  }else if (num_entries < 2) {
    return -1;
  }else if (num_entries > 4096) {
    return -1;
  } else {
    cache = malloc(num_entries*sizeof(cache_entry_t));
    for (int i = 0; i < cache_size; i++) {
      cache[i].valid = 0;
    }
    cache_size = num_entries;
    num_queries = 0;
    num_hits = 0;
    cache_amount = 0;
    return 1;
  }
}

int cache_destroy(void) {
  if (cache == NULL) {
    return -1;
  } else {
    cache = NULL;
    cache_size = 0;
    num_queries = 0;
    num_hits = 0;
    cache_amount = 0;
    return 1;
  }
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if (cache == NULL) {
    return -1;
  } else if (buf == NULL ) {
    return -1;
  }

  if (cache_amount == 0) {
    return -1;
  }

  if (disk_num > JBOD_NUM_DISKS) {
    return -1;
  } else if (disk_num < 0) {
    return -1;
  }

  if (block_num > JBOD_DISK_SIZE) {
    return -1;
  } else if (block_num < 0) {
    return -1;
  }

  for (int i = 0; i < cache_size; i++) {
    if (cache[i].disk_num == disk_num) {
      if (cache[i].block_num == block_num) {
	memcpy(buf, cache[i].block, JBOD_BLOCK_SIZE);
	num_hits++;
	num_queries++;
	return 1;
      }
    }
  }
  num_queries++;
  printf("%d, %d \n", num_queries, num_hits);
  return -1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  for (int i = 0; i < cache_size; i++) {
    if (cache[i].disk_num == disk_num) {
      if (cache[i].block_num == block_num) {
	memcpy(cache[i].block, buf, JBOD_BLOCK_SIZE);
	cache[i].num_accesses++;
      }
    }
  }
}

int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  if (cache == NULL) {
    return -1;
  } else if (buf == NULL ) {
    return -1;
  }

  if (disk_num > JBOD_NUM_DISKS) {
    return -1;
  } else if (disk_num < 0) {
    return -1;
  }
  
  if (block_num > JBOD_DISK_SIZE) {
    return -1;
  } else if (block_num < 0) {
    return -1;
  }

  for (int i = 0; i < cache_size; i++) {
    if (cache[i].disk_num == disk_num) {
      if (cache[i].block_num == block_num) {
	if(cache[i].valid == 1) {
	  return -1;
	}
      }
    }
  }

  if (cache_amount == cache_size) {
    int least_amount_used_position = 0;
    int current_least_value = cache[0].num_accesses;
    for(int i = 0; i < cache_size; i++) {
      if (cache[i].num_accesses < current_least_value) {
	least_amount_used_position = i;
	current_least_value = cache[i].num_accesses;
      }
    }
    cache[least_amount_used_position].valid = 1;
    cache[least_amount_used_position].disk_num = disk_num;
    cache[least_amount_used_position].block_num = block_num;
    memcpy(cache[least_amount_used_position].block, buf, JBOD_BLOCK_SIZE);
    cache[least_amount_used_position].num_accesses = 1;
  } else {
    int avaliable_position = 0;
    for(int i = 0; i < cache_size; i++) {
      if(cache[i].valid != 1) {
	avaliable_position = i;
	break;
      }
    }
    cache[avaliable_position].valid = 1;
    cache[avaliable_position].disk_num = disk_num;
    cache[avaliable_position].block_num = block_num;
    memcpy(cache[avaliable_position].block, buf, JBOD_BLOCK_SIZE);
    cache[avaliable_position].num_accesses = 1;
    cache_amount++;
  }
  return 1;
}
bool cache_enabled(void) {
	return cache != NULL && cache_size > 0;
}

void cache_print_hit_rate(void) {
	fprintf(stderr, "num_hits: %d, num_queries: %d\n", num_hits, num_queries);
	fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
