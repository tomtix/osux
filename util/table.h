#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>

struct table {
    void ** t;
    int len;
    int max;
};

static inline struct table * table_new(int max)
{
    struct table * res = malloc(sizeof(*res));
    res->len = 0;
    res->max = max;
    res->t = malloc(sizeof(void *) * max);
    return res;
}

static inline void table_add(struct table * t, void * o)
{
    t->t[t->len] = o;
    t->len++;
}

static inline void table_free(struct table * t)
{
    if(t == NULL)
	return;
    free(t->t);
    free(t);
}

static inline int table_len(struct table * t)
{
    return t->len;
}

static inline int table_max(struct table * t)
{
    return t->max;
}

static inline void * table_get(struct table * t, int i)
{
    return t->t[i];
}

static inline void table_set(struct table * t, int i, void * o)
{
    t->t[i] = o;
}

#endif //TABLE_H