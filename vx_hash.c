/**
 * vx_hash.h Copyright Voxaris Inc, George Howitt 2008
 */

#include <vx_hash.h>

typedef uint32_t (*vx_hash_func_t) (const void * key, size_t key_size);
typedef void (*vx_hash_free_func_t) (void * value);
typedef int (*vx_hash_cmp_func_t) (const void * foo, const void * bar, size_t key_size);

typedef struct vx_node
{
   void *key;
   void *value;
   uint32_t hashval;
   struct vx_node *link;
   struct vx_node *next;
   struct vx_node *prev;
} vx_node_t;

struct vx_hash
{
   size_t key_size;
   size_t size;
   size_t count;
   vx_node_t **bins;
   vx_node_t *sentry;
   vx_hash_func_t hash_func;
   vx_hash_cmp_func_t cmp_func;
   vx_hash_free_func_t free_func;
};

vx_hash_t * vx_hash_new (size_t size, size_t key_size)
{
   vx_hash_t *hash;
   hash = calloc (1, sizeof(vx_hash_t));
   assert (hash != NULL);
   hash->key_size = key_size;
   hash->size = hash_size(size);
   hash->bins = calloc (hash->size, sizeof(vx_node_t *));
   assert (hash->bins != NULL);
   hash->sentry = calloc (1, sizeof(vx_node_t));
   assert (hash->sentry != NULL);
   hash->sentry->next = hash->sentry->prev = hash->sentry;
   hash->count = 0;
   hash->hash_func = vx_hash_func;
   hash->cmp_func = vx_hash_cmp_func;
   return (hash);
}

void * vx_hash_put (vx_hash_t *hash, void *key, void *value)
{
   uint32_t hash_value, bindex;
   vx_node_t *node;

   if (hash->count > hash->size)
      vx_hash_rehash (hash);

   hash_value = hash->hash_func (key, hash->key_size);
   bindex = hash_value & (hash->size - 1);

   for (node = hash->bins[bindex]; node ; node = node->link)
   {
      if (hash->cmp_func (node->key, key, hash->key_size))
      {
         if (hash->free_func)
            hash->free_func (node->value);
         node->value = value;
         return (value);
      }
   }

   node = (vx_node_t *) calloc (1, sizeof(vx_node_t));
   node->hashval = hash_value;
   node->key = vx_hash_key_dup (hash, key);
   node->value = value;

   if (hash->bins[bindex] != NULL)
      node->link = hash->bins[bindex];
   else
      node->link = NULL;

   hash->bins[bindex] = node;

   node->next = hash->sentry;
   node->prev = hash->sentry->prev;
   hash->sentry->prev->next = node;
   hash->sentry->prev = node;

   hash->count++;

   //printf ("created node: %p  bin = %d key (%p) -> %s value (%p) -> %s\n", 
   //   node, bindex, node->key, (char *)node->key, node->value, (char *)node->value);
   
   return (value);
}

void * vx_hash_get (vx_hash_t *hash, void *key)
{
   uint32_t hash_value;
   uint32_t bindex;
   vx_node_t *node;
   if (hash == NULL)
      return (NULL);
   hash_value = hash->hash_func (key, hash->key_size);
   bindex = hash_value & (hash->size - 1);

   assert (bindex < hash->size);

   for (node = hash->bins[bindex]; node ; node = node->link)
   {
      //printf ("checking node %p bin %d key %s node (key (%p) -> %s, value (%p) -> %s)\n", 
      //   node, bindex, (char *)key, node->key, (char *)node->key, node->value, (char *)node->value);
      if (hash->cmp_func (node->key, key, hash->key_size))
      {
         //printf ("found it\n");
         return (node->value);
      }
   }
   return (NULL);
}

void * vx_hash_delete (vx_hash_t *hash, void *key)
{
   uint32_t hash_value;
   uint32_t bindex;
   vx_node_t *node, *prev;
   void * value;
   if (hash == NULL) return (NULL);

   hash_value = hash->hash_func (key, hash->key_size);
   bindex = hash_value & (hash->size - 1);

   assert (bindex < hash->size);

   for (node = hash->bins[bindex], prev = NULL; node ; prev = node, node = node->link)
   {
      if (hash->cmp_func (node->key, key, hash->key_size))
      {
         value = node->value;
         if (prev)
            prev->link = node->link;
         else
            hash->bins[bindex] = node->link;

         node->prev->next = node->next;
         node->next->prev = node->prev;
   
         free (node->key);
         free (node);
         hash->count--;
         return (value);
      }
   }
   return (NULL);
}

int vx_hash_get_next (vx_hash_t *hash, void **key, void **value, void **ptr)
{
   vx_node_t *node = (vx_node_t *) (*ptr);

   if (hash == NULL) 
      return (0);

   if (node == NULL) 
      node = hash->sentry->next;

   if (node == hash->sentry)
   {
      *key = *value = *ptr = NULL;
      return (0);
   }

   *key = node->key;
   *value = node->value;
   *ptr = (void *) node->next;

   return (1);
}

void vx_hash_rehash (vx_hash_t *hash)
{
   vx_node_t **newbins;
   vx_node_t *node;
   size_t bindex;

   hash->size *= 2;
   newbins = calloc (hash->size, sizeof (vx_node_t *));
   assert (newbins != NULL);

   node = hash->sentry;
   while ((node = node->next) != hash->sentry)
   {
      bindex = node->hashval & (hash->size -1);
      if (newbins[bindex] != NULL)
         node->link = newbins[bindex];
      else
         node->link = NULL;

      newbins[bindex] = node;
   }
   free(hash->bins);
   hash->bins = newbins;
}

uint32_t vx_hash_func (const void *key, size_t key_size)
{
   register const unsigned char *str = (unsigned char *) key;
   register uint32_t hash = 0;

   if (key_size == 0)
      key_size = strlen ((char *) key);

   while (key_size--)
   {
      hash += *str++;
      hash += (hash << 10);
      hash ^= (hash >> 6);
   }
   hash += (hash << 3);
   hash ^= (hash >> 11);
   hash += (hash << 15);

   return (hash);
}

int vx_hash_cmp_func (const void * foo, const void * bar, size_t key_size)
{
   if (key_size == 0)
      return (strcmp((char *) foo, (char *) bar) == 0);
   else
      return (memcmp(foo, bar, key_size)) == 0;
}

void * vx_hash_key_dup (vx_hash_t *hash, void *key)
{
   void *ptr = NULL;
   if (hash->key_size == 0)
      ptr = (void *) strdup((const char *) key);
   else
   {
      ptr = malloc (hash->key_size);
      memcpy(ptr, key, hash->key_size);
   }

   return (ptr);
}

static uint32_t hash_size (uint32_t size)
{
   uint32_t foo = 1;
   while (foo < size)
      foo <<= 1;
   return foo;
}

size_t vx_hash_count (vx_hash_t *hash)
{
   return (hash->count);
}

size_t vx_hash_size (vx_hash_t *hash)
{
   return (hash->size);
}

void vx_hash_destroy(vx_hash_t *hash)
{
   free (hash->bins);
   free (hash->sentry);
   free (hash);
}
   
int main (int argc, char *argv[])
{
   char key[64];
   char *foo;
   char bar[64];
   int ndx;
   vx_hash_t *hash;
   void *ptr;
   char *k, *v;

   hash = vx_hash_new(3, 0);
   printf ("hash count: %d size: %d\n", vx_hash_count(hash), hash->size);
   for (ndx = 0; ndx < 32; ndx++)
   {
      sprintf (key, "key%03d", ndx);
      sprintf (bar, "value%03d", ndx);
      foo = strdup (bar);
      vx_hash_put (hash, key, foo);
      printf ("vx_hash_put: %s -> %s\n", key, foo); 
   }
   printf ("hash count: %d\n", vx_hash_count(hash));
   for (ndx = 0; ndx < 32; ndx++)
   {
      sprintf (key, "key%03d", ndx);
      foo = (char *) vx_hash_get (hash, key);
      printf ("vx_hash_get: %s -> %s\n", key, foo); 
   }
   printf ("hash count: %d\n", vx_hash_count(hash));
   
   ptr = NULL;
   while (vx_hash_get_next(hash, (void **) &k, (void **) &v, &ptr))
   {
      printf ("vx_hash_get_next: %s -> %s\n", k, v); 
   }

   for (ndx = 0; ndx < 32; ndx++)
   {
      sprintf (key, "key%03d", ndx);
      foo = (char *) vx_hash_delete (hash, key);
      printf ("vx_hash_delete: deleted %s -> %s\n", key, foo); 
      free (foo);
   }
   printf ("hash count: %d\n", vx_hash_count(hash));

   vx_hash_destroy (hash);

   return(0);
}
