/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
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
#include "interpolation.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "util/hashtable/hashtable.h"
#include "yaml/yaml2.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "sum.h"
#include "stats.h"
#include "print.h"

#include "pattern.h"

//--------------------------------------------------

struct pattern
{
  double * d;
};

static struct hash_table * ht_pattern;

//--------------------------------------------------

static void add_new_pattern(const char * s, ...);
static void remove_pattern(const char * s);

static double tro_singletap_proba(struct tr_object * obj);
static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt);

static void trm_pattern_alloc(struct tr_map * map);
static void trm_compute_pattern_proba(struct tr_map * map);
static void trm_compute_pattern_full_alt(struct tr_map * map);
static void trm_compute_pattern_singletap(struct tr_map * map);

static void trm_compute_pattern_star(struct tr_map * map);

//--------------------------------------------------

#define MAX_PATTERN_LENGTH  4
#define LENGTH_PATTERN_USED 2

// values between 0. and 10.
// "." -> all must but double!
// 1 pattern
#define D    1.,  0.
#define K    1.,  0.
// 2 pattern
#define DD   2.0, 2.2
#define DK   2.0, 3.0
#define KD   2.5, 3.5
#define KK   2.0, 2.2
// 3 pattern
#define DDD  2.2, 2.5
#define DDK  2.5, 3.0
#define DKD  2.5, 3.5
#define DKK  2.5, 4.2
#define KDD  2.5, 3.5
#define KDK  2.5, 3.5
#define KKD  2.5, 3.0
#define KKK  2.2, 2.5
// 4 pattern
#define DDDD 3.0, 3.5
#define DDDK 3.0, 5.0
#define DDKD 3.0, 4.0
#define DDKK 3.0, 3.5
#define DKDD 4.0, 6.5
#define DKDK 3.5, 4.0
#define DKKD 4.5, 6.0
#define DKKK 3.5, 4.5

#define KDDD 3.5, 4.5
#define KDDK 5.5, 5.0
#define KDKD 4.0, 3.5
#define KDKK 4.5, 7.0
#define KKDD 3.0, 3.5
#define KKDK 3.0, 4.0
#define KKKD 3.0, 5.0
#define KKKK 3.0, 3.5

//--------------------------------------------------

// coeff for singletap proba
#define SINGLETAP_MIN 0.
#define SINGLETAP_MAX 1.
#define TIME_MIN 0.
#define TIME_MAX 500.
// 160ms ~ 180bpm 1/2
// 125ms = 240bpm 1/2

#define PROBA_START 0.2  
#define PROBA_END   1.  // <= 
#define PROBA_STEP  0.2

// coeff for star
#define PATTERN_STAR_COEFF_ALT 2.5
#define PATTERN_STAR_COEFF_SIN 1.

// coeff for stats
#define PATTERN_COEFF_MEDIAN 0.5
#define PATTERN_COEFF_MEAN   0.3
#define PATTERN_COEFF_D1     0.
#define PATTERN_COEFF_D9     0.2
#define PATTERN_COEFF_Q1     0.
#define PATTERN_COEFF_Q3     0.

// scaling
#define PATTERN_STAR_SCALING 5.0

// stats module
TRM_STATS_HEADER(pattern_star, PATTERN)

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

#define PATTERN_FILE "pattern_cst.yaml"

struct hash_table * ht_cst;
static void pattern_cst_init()
{
  struct yaml_wrap * yw = NULL;
  int r = yaml2_parse_file(&yw, PATTERN_FILE);
  if (r != 0)
    {
      
      return;
    }
  yaml2_dump(stdout, yw);
  if(yw->type == YAML_MAPPING)
    {
      ht_cst = yw->content.mapping;
      struct yaml_wrap * tmp = NULL;
      ht_get_entry(ht_cst, "MAX_PATTERN_LENGTH", &tmp);
      if(tmp->type == YAML_SCALAR)
	printf("%s\n", tmp->content.scalar);
    }
}

//-----------------------------------------------------

static void add_new_pattern(const char * s, ...)
{
  struct pattern * p = calloc(sizeof(*p), 1);
  p->d = calloc(sizeof(double), LENGTH_PATTERN_USED);
  
  va_list vl;
  va_start(vl, s);
  for (int i = 0; i < LENGTH_PATTERN_USED; ++i)
    p->d[i] = va_arg(vl, double);
  va_end(vl);
  
  ht_add_entry(ht_pattern, s, p);
}

static void remove_pattern(const char * s)
{
  struct pattern * p = NULL;
  ht_get_entry(ht_pattern, s, &p);
  free(p->d);
  free(p);
}

//-----------------------------------------------------

__attribute__((constructor))
static void ht_pattern_init()
{  
  ht_pattern = ht_create(0, NULL);
  // 1 pattern list
  add_new_pattern("d", D);
  add_new_pattern("k", K);
  // 2 pattern list
  add_new_pattern("dd", DD);
  add_new_pattern("dk", DK);
  add_new_pattern("kd", KD);
  add_new_pattern("kk", KK);
  // 3 pattern list
  add_new_pattern("ddd", DDD);
  add_new_pattern("ddk", DDK);
  add_new_pattern("dkd", DKD);
  add_new_pattern("dkk", DKK);
  add_new_pattern("kdd", KDD);
  add_new_pattern("kdk", KDK);
  add_new_pattern("kkd", KKD);
  add_new_pattern("kkk", KKK);
  // 4 pattern list
  add_new_pattern("dddd", DDDD);
  add_new_pattern("dddk", DDDK);
  add_new_pattern("ddkd", DDKD);
  add_new_pattern("ddkk", DDKK);
  add_new_pattern("dkdd", DKDD);
  add_new_pattern("dkdk", DKDK);
  add_new_pattern("dkkd", DKKD);
  add_new_pattern("dkkk", DKKK);
  
  add_new_pattern("kddd", KDDD);
  add_new_pattern("kddk", KDDK);
  add_new_pattern("kdkd", KDKD);
  add_new_pattern("kdkk", KDKK);
  add_new_pattern("kkdd", KKDD);
  add_new_pattern("kkdk", KKDK);
  add_new_pattern("kkkd", KKKD);
  add_new_pattern("kkkk", KKKK);
}
  
//--------------------------------------------------

__attribute__((destructor))
static void ht_pattern_free()
{
  // 1 pattern
  remove_pattern("d");
  remove_pattern("k");
  // 2 pattern
  remove_pattern("dd");
  remove_pattern("dk");
  remove_pattern("kd");
  remove_pattern("kk");
  // 3 pattern
  remove_pattern("ddd");
  remove_pattern("ddk");
  remove_pattern("dkd");
  remove_pattern("dkk");
  remove_pattern("kdd");
  remove_pattern("kdk");
  remove_pattern("kkd");
  remove_pattern("kkk");
  // 3 pattern
  remove_pattern("dddd");
  remove_pattern("dddk");
  remove_pattern("ddkd");
  remove_pattern("ddkk");
  remove_pattern("dkdd");
  remove_pattern("dkdk");
  remove_pattern("dkkd");
  remove_pattern("dkkk");
  remove_pattern("kddd");
  remove_pattern("kddk");
  remove_pattern("kdkd");
  remove_pattern("kdkk");
  remove_pattern("kkdd");
  remove_pattern("kkdk");
  remove_pattern("kkkd");
  remove_pattern("kkkk");

  ht_free(ht_pattern);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_singletap_proba(struct tr_object * obj)
{
  double time = obj->rest;
  if (time > TIME_MAX)
    time = TIME_MAX;

  return POLY_2_PT(time,
		   TIME_MIN, SINGLETAP_MIN,
		   TIME_MAX, SINGLETAP_MAX);
}

//-----------------------------------------------------

static struct pattern * trm_get_pattern(struct tr_map * map,
					int i, double proba_alt)
{
  char * s = calloc(sizeof(char), MAX_PATTERN_LENGTH + 1);
  for (int j = 0; (j < MAX_PATTERN_LENGTH &&
		   i + j < map->nb_object); j++)
    {
      if (tro_is_bonus(&map->object[i+j]))
	{
	  s[j] = 0;
	  break; // continue ?
	}
      else if (tro_is_don(&map->object[i+j]))
	s[j] = 'd';
      else // if (tro_is_kat(&map->object[i+j]))
	s[j] = 'k';
      
      if (tro_is_big(&map->object[i+j]) ||
	  map->object[i+j].proba > proba_alt)
	{
	  s[j+1] = 0;
	  break;
	}
    }

  struct pattern * p = NULL;
  int ret = ht_get_entry(ht_pattern, s, &p);
  if (ret != 0) // when s[0] = 0 (bonus) or not found (error)
    {
      if (s[0] != 0)
	fprintf(OUTPUT_ERR, "Could not find pattern :%s\n", s);
      p = NULL;
    }
  free(s);
  return p;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_proba(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].proba = tro_singletap_proba(&map->object[i]);
    }  
}

//-----------------------------------------------------

static void trm_compute_pattern_full_alt(struct tr_map * map)
{ 
  for(int i = 0; i < map->nb_object; i++)
    {
      struct pattern * p = trm_get_pattern(map, i, 1);
      if (p == NULL)
	continue;

      for (int j = 0; (j < LENGTH_PATTERN_USED &&
		       i + j < map->nb_object); j++)
	map->object[i+j].alt[j] = p->d[j];
    }
}

//-----------------------------------------------------

static void trm_compute_pattern_singletap(struct tr_map * map)
{
  float proba;
  for(proba = PROBA_START; proba <= PROBA_END; proba += PROBA_STEP)
    for(int i = 0; i < map->nb_object; i++)
      {
	struct pattern * p = trm_get_pattern(map, i, proba);
	if (p == NULL)
	  continue;
	
	for (int j = 0; (j < LENGTH_PATTERN_USED &&
			 i + j < map->nb_object); j++)
	  map->object[i+j].singletap[j] += proba * p->d[j];
      }
}


//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_compute_pattern_star(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].pattern_star = 0;
      for (int j = 0; j < LENGTH_PATTERN_USED; j++)
	{
	  map->object[i].pattern_star +=
	    PATTERN_STAR_COEFF_ALT * map->object[i].alt[j] +
	    PATTERN_STAR_COEFF_SIN * map->object[i].singletap[j];
	}
    }

  map->pattern_star = trm_stats_compute_pattern_star(map); 
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_pattern_alloc(struct tr_map * map)
{
  for (int i = 0; i < map->nb_object; i++)
    {
      map->object[i].alt =
	calloc(sizeof(double), LENGTH_PATTERN_USED);
      map->object[i].singletap =
	calloc(sizeof(double), LENGTH_PATTERN_USED);
    }  
}

//-----------------------------------------------------

void trm_compute_pattern (struct tr_map * map)
{
  trm_pattern_alloc(map);
  trm_compute_pattern_proba(map);
  trm_compute_pattern_full_alt(map);
  trm_compute_pattern_singletap(map);
  
  trm_compute_pattern_star(map);
}
