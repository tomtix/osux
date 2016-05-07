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

#include <math.h>
#include <stdio.h>

#include "util/hash_table.h"
#include "util/list.h"
#include "util/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "stats.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"

#include "density.h"

static struct yaml_wrap * yw_dst;
static struct hash_table * ht_cst_dst;

static double tro_get_coeff_density(struct tr_object * obj);
static double tro_density(struct tr_object * obj1,
			  struct tr_object * obj2);

static void tro_set_density_raw(struct tr_object * objs, int i);
static void trm_set_density_raw(struct tr_map * map);

static void tro_set_density_color(struct tr_object * objs, int i);
static void trm_set_density_color(struct tr_map * map);

static void tro_set_density_star(struct tr_object * obj);
static void trm_set_density_star(struct tr_map * map);

//--------------------------------------------------

#define DENSITY_FILE  "density_cst.yaml"

// coeff for density
static struct linear_fun * DENSITY_VECT;

// coefficient for object type, 1 is the maximum
static double DENSITY_NORMAL;
static double DENSITY_BIG;
static double DENSITY_BONUS;

// coefficient for length weighting in density
static double DENSITY_LENGTH;

// coeff for star
static double DENSITY_STAR_COEFF_COLOR;
static double DENSITY_STAR_COEFF_RAW;
static struct linear_fun * DENSITY_SCALE_VECT;

//-----------------------------------------------------

static void density_global_init(struct hash_table * ht_cst)
{
    DENSITY_VECT       = cst_lf(ht_cst, "vect_density");
    DENSITY_SCALE_VECT = cst_lf(ht_cst, "vect_scale");

    DENSITY_NORMAL = cst_f(ht_cst, "density_normal");
    DENSITY_BIG    = cst_f(ht_cst, "density_big");
    DENSITY_BONUS  = cst_f(ht_cst, "density_bonus");

    DENSITY_LENGTH = cst_f(ht_cst, "density_length");
  
    DENSITY_STAR_COEFF_COLOR = cst_f(ht_cst, "star_color");
    DENSITY_STAR_COEFF_RAW   = cst_f(ht_cst, "star_raw");
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_cst_init_density(void)
{
    yw_dst = cst_get_yw(DENSITY_FILE);
    ht_cst_dst = yw_extract_ht(yw_dst);
    if(ht_cst_dst != NULL)
	density_global_init(ht_cst_dst);
}

__attribute__((destructor))
static void ht_cst_exit_density(void)
{
    yaml2_free(yw_dst);
    lf_free(DENSITY_VECT);
    lf_free(DENSITY_SCALE_VECT);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_get_coeff_density(struct tr_object * o)
{
    if (tro_is_bonus(o))
	return DENSITY_BONUS;
    else if (tro_is_big(o))
	return DENSITY_BIG;
    else
	return DENSITY_NORMAL;
}

//-----------------------------------------------------

static double tro_density(struct tr_object * obj1, 
			  struct tr_object * obj2)
{
    double value  = lf_eval(DENSITY_VECT, 
			    ((double) obj2->end_offset - obj1->offset) + 
			    DENSITY_LENGTH * obj1->length);
    return tro_get_coeff_density(obj1) * value;
}

//-----------------------------------------------------

#define TRO_SET_DENSITY_TYPE(TYPE, TRO_TEST)			\
    static void tro_set_density_##TYPE(struct tr_object * objs,	\
				       int i)			\
    {								\
	if(objs[i].ps == MISS) {				\
	    objs[i].density_##TYPE = 0;				\
	    return;						\
	}							\
								\
	double sum = 0;						\
	for(int j = 0; j < i; j++) {				\
	    if(objs[j].ps == MISS)				\
		continue;					\
	    if(TRO_TEST(&objs[i], &objs[j]))			\
		sum += tro_density(&objs[j], &objs[i]);		\
	}							\
	sum *= tro_get_coeff_density(&objs[i]);			\
	objs[i].density_##TYPE = sum;				\
    }								\
    								\
    static void trm_set_density_##TYPE(struct tr_map * map)	\
    {								\
	map->object[0].density_##TYPE = 0;			\
	for(int i = 1; i < map->nb_object; i++)			\
	    tro_set_density_##TYPE (map->object, i);		\
    }

static int tro_true(struct tr_object * o1 __attribute__((unused)),
		    struct tr_object * o2 __attribute__((unused)))
{
    return 1;
}

TRO_SET_DENSITY_TYPE(raw,   tro_true)
TRO_SET_DENSITY_TYPE(color, tro_are_same_density)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void tro_set_density_star(struct tr_object * obj)
{
    obj->density_star = lf_eval
	(DENSITY_SCALE_VECT,
	 (DENSITY_STAR_COEFF_COLOR * obj->density_color +
	  DENSITY_STAR_COEFF_RAW   * obj->density_raw));

}

//-----------------------------------------------------

static void trm_set_density_star(struct tr_map * map)
{
    for(int i = 0; i < map->nb_object; i++)
	tro_set_density_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_density(struct tr_map * map)
{
    if(ht_cst_dst == NULL) {
	tr_error("Unable to compute density stars.");
	return;
    }

    trm_set_density_raw(map);
    trm_set_density_color(map);

    trm_set_density_star(map);
}