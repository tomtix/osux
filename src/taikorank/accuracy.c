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
#include <math.h>

#include "osux.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_object.h"
#include "cst_yaml.h"
#include "linear_fun.h"
#include "print.h"
#include "accuracy.h"
#include "spacing_count.h"

#define TIME_EQUAL_MS 12

static osux_yaml *yw_acc;
static GHashTable *ht_cst_acc;

static int equal_i(int x, int y);

static double tro_slow(const struct tr_object *o);
static double tro_spacing_influence(const struct tr_object *o1,
                                    const struct tr_object *o2);
static struct spacing_count *
tro_spacing_init(const struct tr_object *objs, int i);
static double tro_spacing(const struct tr_object *o,
                          const struct spacing_count *spc);

static void trm_set_slow(struct tr_map *map);
static void trm_set_hit_window(struct tr_map *map);
static void trm_set_spacing(struct tr_map *map);
static void trm_set_accuracy_star(struct tr_map *map);

//-----------------------------------------------------

#define ACCURACY_FILE  "accuracy_cst.yaml"

#define MS_GREAT       48
#define MS_COEFF_GREAT 3
#define MS_GOOD        108
#define MS_COEFF_GOOD  6
#define MS_MISS        500 // arbitrary
#define MS_COEFF_MISS  0   // arbitrary

static struct linear_fun *SLOW_LF;
static struct linear_fun *HIT_WINDOW_LF;
static struct linear_fun *SPC_FREQ_LF;
static struct linear_fun *SPC_INFLU_LF;

static double ACCURACY_STAR_SLOW_POW;
static double ACCURACY_STAR_SPACING_POW;
static double ACCURACY_STAR_HIT_WINDOW_POW;
static struct linear_fun *ACCURACY_SCALE_LF;

//-----------------------------------------------------

static void accuracy_global_init(GHashTable *ht_cst)
{
    SLOW_LF       = cst_lf(ht_cst, "vect_slow");
    HIT_WINDOW_LF = cst_lf(ht_cst, "vect_hit_window");
    SPC_FREQ_LF   = cst_lf(ht_cst, "vect_spacing_frequency");
    SPC_INFLU_LF  = cst_lf(ht_cst, "vect_spacing_influence");
    ACCURACY_SCALE_LF = cst_lf(ht_cst, "vect_accuracy_scale");

    ACCURACY_STAR_SLOW_POW       = cst_f(ht_cst, "star_slow");
    ACCURACY_STAR_SPACING_POW    = cst_f(ht_cst, "star_spacing");
    ACCURACY_STAR_HIT_WINDOW_POW = cst_f(ht_cst, "star_hit_window");
}

//-----------------------------------------------------

static void ht_cst_exit_accuracy(void)
{
    osux_yaml_free(yw_acc);
    lf_free(SLOW_LF);
    lf_free(SPC_FREQ_LF);
    lf_free(SPC_INFLU_LF);
    lf_free(HIT_WINDOW_LF);
    lf_free(ACCURACY_SCALE_LF);
}

void tr_accuracy_initialize(void)
{
    yw_acc = cst_get_yw(ACCURACY_FILE);
    ht_cst_acc = yw_extract_ht(yw_acc);
    if (ht_cst_acc != NULL)
        accuracy_global_init(ht_cst_acc);
    atexit(ht_cst_exit_accuracy);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static int equal_i(int x, int y)
{
    return abs(x - y) < TIME_EQUAL_MS;
}

//-----------------------------------------------------

double *trm_get_ggm_val(const struct tr_map *map)
{
    double ggm_ms[3];
    ggm_ms[0] = (map->od_hit_window_mult *
                 (MS_GREAT - (MS_COEFF_GREAT * map->od)));
    ggm_ms[1] = (map->od_hit_window_mult *
                 (MS_GOOD  - (MS_COEFF_GOOD  * map->od)));
    ggm_ms[2] = (map->od_hit_window_mult *
                 (MS_MISS  - (MS_COEFF_MISS  * map->od)));
    double *ggm_val = malloc(sizeof(double) * 3);
    for (int i = 0; i < 3; i++)
        ggm_val[i] = lf_eval(HIT_WINDOW_LF, ggm_ms[i]);
    return ggm_val;
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static double tro_slow(const struct tr_object *o)
{
    return lf_eval(SLOW_LF, o->bpm_app);
}

//-----------------------------------------------------

static double tro_spacing_influence(const struct tr_object *o1,
                                    const struct tr_object *o2)
{
    int diff = o2->offset - o1->offset;
    return lf_eval(SPC_INFLU_LF, diff);
}

//-----------------------------------------------------

static struct spacing_count *
tro_spacing_init(const struct tr_object *objs, int i)
{
    struct spacing_count *spc = spc_new(equal_i);
    for (int j = i; j >= 0; j--) {
        if (objs[j].ps == MISS)
            continue;
        double influ = tro_spacing_influence(&objs[j], &objs[i]);
        if (influ == 0)
            break; /* j-- influence won't increase */
        spc_add(spc, objs[j].rest, influ);
    }
    return spc;
}

//-----------------------------------------------------

static double tro_spacing(const struct tr_object *o,
                          const struct spacing_count *spc)
{
    double total = spc_get_total(spc);
    double nb = spc_get_nb(spc, o->rest);
    double freq = 1;
    if (total != 0) // avoid error when only miss
        freq = nb / total;
    return lf_eval(SPC_FREQ_LF, freq);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void tro_set_hit_window(struct tr_object *o, const double *ggm_val)
{
    switch (o->ps) {
    case GREAT:
        o->hit_window = ggm_val[0];
        break;
    case GOOD:
        o->hit_window = ggm_val[1];
        break;
    default: // MISS & bonus
        o->hit_window = ggm_val[2];
        break;
    }
}

//-----------------------------------------------------

void tro_set_slow(struct tr_object *o)
{
    if (o->ps == MISS) {
        o->slow = 1;
        return;
    }
    o->slow = tro_slow(o);
}

//-----------------------------------------------------

void tro_set_spacing(struct tr_object *o, int i)
{
    struct spacing_count *spc = tro_spacing_init(o->objs, i);
    o->spacing = tro_spacing(o, spc);
    spc_free(spc);
}

//-----------------------------------------------------

void tro_set_accuracy_star(struct tr_object *o)
{
    o->accuracy_star = lf_eval
        (ACCURACY_SCALE_LF,
         (pow(o->slow,       ACCURACY_STAR_SLOW_POW) *
          pow(o->spacing,    ACCURACY_STAR_SPACING_POW) *
          pow(o->hit_window, ACCURACY_STAR_HIT_WINDOW_POW)));
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

static void trm_set_slow(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_slow(&map->object[i]);
}

//-----------------------------------------------------

static void trm_set_hit_window(struct tr_map *map)
{
    double *ggm_val = trm_get_ggm_val(map);
    for (int i = 0; i < map->nb_object; i++)
        tro_set_hit_window(&map->object[i], ggm_val);
    free(ggm_val);
}

//-----------------------------------------------------

static void trm_set_spacing(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_spacing(&map->object[i], i);
}

//-----------------------------------------------------

static void trm_set_accuracy_star(struct tr_map *map)
{
    for (int i = 0; i < map->nb_object; i++)
        tro_set_accuracy_star(&map->object[i]);
}

//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

void trm_compute_accuracy(struct tr_map *map)
{
    if (ht_cst_acc == NULL) {
        tr_error("Unable to compute accuracy stars.");
        return;
    }

    /*
      Compuatation is in three parts:
      - hit window, the time given to hit correctly the object
      - spacing, based on spacing frequency
      - slow, when object are very slow they are harder to acc'
      */
    trm_set_hit_window(map);
    trm_set_spacing(map);
    trm_set_slow(map);

    trm_set_accuracy_star(map);
}
