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

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "cst_yaml.h"
#include "vector.h"
#include "interpolation.h"

#define VECT_DIM 2

//--------------------------------------------------

struct vector * cst_vect(struct hash_table * ht, const char * key)
{
  struct vector * v = malloc(sizeof(*v));
  char * s = NULL;
  asprintf(&s, "%s_length", key);
  v->len = cst_i(ht, s);
  free(s);

  v->t = malloc(sizeof(double *) * v->len);
  for(int i = 0; i < v->len; i++)
    {
      v->t[i] = malloc(sizeof(double) * VECT_DIM);
      for(int j = 0; j < v->len; j++)
	{
	  asprintf(&s, "%s_%c%d", key, 'x'+j, i+1);
	  v->t[i][j] = cst_f(ht, s);
	  free(s);
	}
    }
  return v;
}

//--------------------------------------------------

void vect_free(struct vector * v)
{
  for(int i = 0; i < v->len; i++)
    free(v->t[i]);
  free(v->t);
  free(v);
}

//--------------------------------------------------

double vect_exp(struct vector * v, double x)
{
  return EXP_2_PT(x, 
		  v->t[0][0], v->t[0][1],
		  v->t[1][0], v->t[1][1]);
}

double vect_poly2(struct vector * v, double x)
{
  return POLY_2_PT(x, 
		   v->t[0][0], v->t[0][1],
		   v->t[1][0], v->t[1][1]);
}
