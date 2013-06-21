/**
 * vx_hash.h Copyright Voxaris Inc, George Howitt 2008
 */

#ifndef _VX_HASH_H_
#define _VX_HASH_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum hash_flag
{
   VX_HASH_COPY_KEYS = 1,
   VX_HASH_FREE_VALUE = 1<<1
} vx_hash_flag_t;

/**
 * vx_hash_func_t: function prototype for hashing functions
 */
typedef uint32_t (*vx_hash_func_t) (const void * key, size_t key_size);

/**
 * vx_hash_free_func_t: function prototype for hash value memory free
 */
typedef void (*vx_hash_free_func_t) (void * value);

/**
 * vx_hash_cmp_func_t: function prototype for key cmp
 */
typedef int (*vx_hash_cmp_func_t) (const void * foo, const void * bar, size_t key_size);

struct vx_hash;
/**
 * vx_hash_t: opaque pointer to a hash table
 */
typedef struct vx_hash vx_hash_t;

/**
 *
 */
uint32_t vx_hash_func (const void * key, size_t key_size);

/**
 *
 */
int vx_hash_cmp_func (const void * foo, const void * bar, size_t key_size);

/**
 *
 */
void vx_hash_free_func (const void * value);

/**
 *
 */
static size_t hash_size (size_t size);

/**
 *
 */
void * vx_hash_key_dup (vx_hash_t *hash, void *key);

/**
 *
 */
void vx_hash_rehash (vx_hash_t *hash);


/**
 *
 */
vx_hash_t * vx_hash_new(void);

/**
 *
 */
vx_hash_t * vx_hash_create (size_t size, size_t key_size, int flags);

/**
 *
 */
void vx_hash_set_hash_func (vx_hash_t *hash, vx_hash_func_t hash_func);

/**
 *
 */
void vx_hash_set_free_func (vx_hash_t *hash, vx_hash_free_func_t free_func);

/**
 *
 */
void vx_hash_destroy(vx_hash_t *hash);

/**
 *
 */
void * vx_hash_put (vx_hash_t *hash, void *key, void *value);

/**
 *
 */
void * vx_hash_get (vx_hash_t *hash, void *key);

/**
 *
 */
void * vx_hash_delete (vx_hash_t *hash, void *key);

/**
 *
 */
int vx_hash_get_next (vx_hash_t *hash, void **key, void **value, void **ptr);

/**
 *
 */
size_t vx_hash_count (vx_hash_t *hash);

/**
 *
 */
size_t vx_hash_size (vx_hash_t *hash);

#ifdef __cplusplus
}
#endif

#endif
