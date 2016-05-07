/*
 *  Copyright (©) 2015-2016 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include <stdio.h>

#include "util/hash_table.h"
#include "freq_counter.h"

struct counter {
    struct hash_table * ht;
    double total;
};

struct counter_entry {
    void * data;
    double nb;
};

struct heriter {
    struct counter_entry * e;
    int (*herit)(void *, void *);
    double nb;
};

struct bundle {
    struct counter * c;
    int (*herit)(void *, void *);
};

static struct counter_entry * cnte_new(void * d, double nb);
static void cnte_free(const char * key __attribute__((unused)), 
		      struct counter_entry * e, 
		      void * args __attribute__((unused)));
static void cnte_print(const char * key, struct counter_entry * e,
		       struct bundle * b);
static void cnte_herit(const char * key __attribute__((unused)), 
		       struct counter_entry * e, struct heriter * h);

typedef void (*ht_fun)(const char*,void*,void*);

//--------------------------------------------------

static struct counter_entry * cnte_new(void * d, double nb)
{
    struct counter_entry * e = malloc(sizeof(*e));
    e->data = d;
    e->nb = nb;
    return e;
}

static void cnte_free(const char * key __attribute__((unused)), 
		      struct counter_entry * e, 
		      void * args __attribute__((unused)))
{
    if (e == NULL)
	return;
    free(e);
}

//--------------------------------------------------

static void cnte_print(const char * key, struct counter_entry * e,
		       struct bundle * b)
{
    if (b == NULL) {
	printf("Entry:\t%s\t%g\n", key, e->nb);
    } else {
	double d = cnt_get_nb_compressed(b->c, key, b->herit);
	printf("Entry:\t%s\t%g\t%g\n", key, e->nb, d);
    }
}

//--------------------------------------------------

static void cnte_herit(const char * key __attribute__((unused)), 
		       struct counter_entry * e, struct heriter * h)
{
    if (h->herit(h->e->data, e->data))
	h->nb += e->nb;
}

//--------------------------------------------------

struct counter * cnt_new(void)
{
    struct counter * c = malloc(sizeof(*c));
    c->ht = ht_create(0, NULL);
    c->total = 0;
    return c;
}

void cnt_free(struct counter * c)
{
    if (c == NULL)
	return;
    ht_for_each(c->ht, (ht_fun) cnte_free, NULL);
    ht_free(c->ht);
    free(c);
}

//--------------------------------------------------

void cnt_add(struct counter * c, void * data, const char * key, 
	     double val)
{
    struct counter_entry * e = NULL;
    ht_get_entry(c->ht, key, &e);
    if (e != NULL) {
	e->nb += val;
    } else {
	e = cnte_new(data, val);
	ht_add_entry(c->ht, key, e);
    }
    c->total += val;
}

//--------------------------------------------------

double cnt_get_nb_compressed(struct counter * c, const char * key,
			     int (*herit)(void *, void *))
{
    struct counter_entry * e = NULL;
    ht_get_entry(c->ht, key, &e);
    if (e == NULL)
	return 0;
    struct heriter total = {e, herit, 0};
    ht_for_each(c->ht, (ht_fun) cnte_herit, &total);
    return total.nb;
}

double cnt_get_nb(struct counter * c, const char * key)
{
    struct counter_entry * e = NULL;
    ht_get_entry(c->ht, key, &e);
    if (e == NULL)
	return 0;
    return e->nb;
}

double cnt_get_total(struct counter * c)
{
    return c->total;
}

//--------------------------------------------------

void cnt_print(struct counter * c)
{
    printf("Counter: (%g)\n", c->total);
    ht_for_each(c->ht, (ht_fun) cnte_print, NULL);
}

void cnt_print_compressed(struct counter * c,
			  int (*herit)(void *, void *))
{
    printf("Counter: (%g)\n", c->total);
    struct bundle b = {c, herit};
    ht_for_each(c->ht, (ht_fun) cnte_print, &b);
}