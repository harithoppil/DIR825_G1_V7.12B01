#define  USER_LEVEL_HASH
#ifdef   USER_LEVEL_HASH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <mem.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <lib/syscalls.h>
#endif

#include "hash.h"

struct hashentry *hashentry_new(void *key, void *data)
{
#ifdef USER_LEVEL_HASH
    struct hashentry *new_entry = (struct hashentry *)malloc(sizeof(struct hashentry));
#else
		struct hashentry *new_entry = kmalloc(sizeof(struct hashentry),GFP_KERNEL);
#endif

    new_entry->key = key;
    new_entry->data = data;
    new_entry->next = NULL;

    return new_entry;
}

struct hashentry *hashentry_free(struct hashentry *h)
{
    struct hashentry *next = h->next;

#ifdef USER_LEVEL_HASH
    free(h->key);
    free(h->data);
    free(h);
#else
    kfree(h->key);
    kfree(h->data);
    kfree(h);
#endif
    h = NULL;
    return (next);
}

void hlist_append(struct hashentry **root, void *key, void *data)
{
    struct hashentry *l, *pos;
    l = hashentry_new(key, data);
    if (*root == NULL) {
        *root = l;
    } else {
        for(pos = *root; pos->next != NULL; pos = pos->next);
            pos->next = l;
    }
}

int hlist_update(struct hashentry *root, void *key, void *data,
        int (*compare)(void *, void *))
{
    struct hashentry *pos;
    for(pos = root; pos != NULL; pos = pos->next ) {
        if ( compare(key, pos->key) ) {

#ifdef USER_LEVEL_HASH
            free(pos->data);
            free(key);
#else
            kfree(pos->data);
            kfree(key);
#endif
            pos->data = data;

            return 0;
        }
    }
    return -1;
}

int hlist_remove(struct hashentry **root, void *key,
                 int (*compare)(void *,void *))
{
    struct hashentry *pos ,*prev;

    if (NULL == *root) return -1;

    if (compare( key,(*root)->key)) {
        *root = hashentry_free(*root);
        return 0;
    }

    prev = *root;
    for (pos = prev->next; NULL != pos; pos = pos->next) {
        if (compare( key,pos->key)) {
            prev->next = hashentry_free(pos);
            return 0;
        }
        prev = pos;
    }
    return -1;
}

hashtable *hash_create(unsigned int (*keyfunc)(void *),
                       int (*comparefunc)(void *, void *),
                       int size)
{
    int len = sizeof(struct hashentry *) * size;
    int i;
#ifdef USER_LEVEL_HASH
    hashtable *tab = malloc( sizeof(hashtable) );
#else
    hashtable *tab = kmalloc(sizeof(hashtable),GFP_KERNEL);
#endif
    memset(tab, 0, sizeof(hashtable));

#ifdef USER_LEVEL_HASH
    tab->hashlist = malloc(len);
#else
    tab->hashlist = kmalloc(len,GFP_KERNEL);
#endif

    if (tab->hashlist == NULL) {
#ifdef USER_LEVEL_HASH
        free(tab);
#else
	kfree(tab);
#endif
        return NULL;
    }

    memset(tab->hashlist, 0, len );
    for (i = 0; i < size; i++)
        tab->hashlist[i] = NULL ;

    tab->compare = comparefunc;
    tab->gethash = keyfunc;
    tab->hashsize = size;
    tab->count    = 0;
    return tab;
}

void hash_free(hashtable *tab)
{
    int i;
    struct hashentry *pos;

    for (i = 0; i < tab->hashsize; i++)
        for (pos = tab->hashlist[i]; NULL != pos; pos = hashentry_free(pos));

#ifdef USER_LEVEL_HASH
    free(tab->hashlist);
    free(tab);
#else
		kfree(tab->hashlist);
		kfree(tab);
#endif

    tab =NULL;
}

void hash_insert(void *key, void *data, hashtable *tab)
{

    unsigned int index = hashindex(key, tab);
//    struct hashentry *root = tab->hashlist[index];

//    if ( hlist_update(root, key, data, tab->compare ) != 0 )
    {
        hlist_append(&(tab->hashlist[index]), key, data );
        tab->count++;
    }
}

//(1) 查看Hash Table中是否存在键值为key的项，如果有则替换该键值所对应的value，否则调用hlist_append为key, data生成新的hashentry并插入相应的队列中。

void hash_remove(void *key, hashtable *tab)
{
    unsigned int index = hashindex(key, tab);
    if (hlist_remove(&(tab->hashlist[index]), key, tab->compare) == 0) {
        tab->count--;
    }
}

void *hash_value(void *key, hashtable *tab)
{
    struct hashentry *pos;
    unsigned int index = hashindex(key, tab);
    for (pos = tab->hashlist[index]; NULL != pos; pos = pos->next) {
        if (tab->compare(key, pos->key)) {
            return (pos->data);
        }
    }
    return NULL;
}


void hash_for_each_do(hashtable *tab, int(cb)(void *, void *))
{
    int i = 0;
    struct hashentry *pos;
     for (i = 0; i < tab->hashsize; i++) {
         for (pos = tab->hashlist[i]; NULL != pos; pos = pos->next ) {
            cb(pos->key, pos->data);
        }
    }
}

inline int hash_count(hashtable *tab)
{
    return tab->count;
}

