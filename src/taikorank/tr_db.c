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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <mysql/mysql.h>

#include "osux.h"

#include "taiko_ranking_map.h"
#include "taiko_ranking_score.h"
#include "taiko_ranking_object.h"

#include "tr_db.h"
#include "config.h"
#include "tr_mods.h"
#include "print.h"

#ifdef USE_TR_MYSQL_DB

#define TR_DB_NAME   "taiko_rank"
#define TR_DB_USER   "tr_user"
#define TR_DB_MAPSET "tr_mapset"
#define TR_DB_DIFF   "tr_diff"
#define TR_DB_MOD    "tr_mod"
#define TR_DB_SCORE  "tr_score"

static MYSQL *sql;

static int new_rq(MYSQL *sql, const char *rq, ...);
static int tr_db_get_id(MYSQL *sql, char *table, char *cond);
static char *tr_db_escape_str(MYSQL *sql, const char *src);

static int tr_db_insert_user(const struct tr_map *map);
static int tr_db_insert_mapset(const struct tr_map *map, int user_id);
static int tr_db_insert_diff(const struct tr_map *map, int mapset_id);
static int tr_db_insert_mod(const struct tr_map *map);
static int tr_db_insert_update_score(const struct tr_map *map,
                                     int diff_id, int mod_id);

//-------------------------------------------------

static void tr_db_exit(void)
{
    if (sql != NULL)
        mysql_close(sql);
}

void tr_db_init(void)
{
    sql = mysql_init(NULL);

    if (sql == NULL) {
        tr_error("Error: mysql init");
        return;
    }

    if (NULL == mysql_real_connect(
            sql, GLOBAL_CONFIG->db_ip, GLOBAL_CONFIG->db_login,
            GLOBAL_CONFIG->db_passwd, NULL, 0, NULL, 0)) {
        tr_error("%s", mysql_error(sql));
        mysql_close(sql);
        sql = NULL;
        return;
    }
    new_rq(sql, "USE %s;", TR_DB_NAME);
    atexit(tr_db_exit);
}


//-------------------------------------------------

static int new_rq(MYSQL *sql, const char *rq, ...)
{
    va_list va;
    va_start(va, rq);
    char *buf = NULL;
    vasprintf(&buf, rq, va);
    va_end(va);

    if (mysql_query(sql, buf)) {
        tr_error("'%s' request: '%s'", mysql_error(sql), buf);
        free(buf);
        return -1;
    }

    free(buf);
    return 0;
}

//-------------------------------------------------

static int tr_db_get_id(MYSQL *sql, char *table, char *cond)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    #pragma omp critical
    {
        new_rq(sql, "SELECT * FROM %s WHERE %s;", table, cond);
        result = mysql_store_result(sql);
        row = mysql_fetch_row(result);
    }
    int id = -1;
    if (row != NULL)
        id = atoi(row[0]);
    return id;
}

//-------------------------------------------------

static char *tr_db_escape_str(MYSQL *sql, const char *src)
{
    unsigned int l = strlen(src);
    char *dst = malloc(sizeof(char) * (2 * l + 1));
    mysql_real_escape_string(sql, dst, src, l);
    return dst;
}

//-------------------------------------------------

static int tr_db_insert_user(const struct tr_map *map)
{
    char *map_creator = tr_db_escape_str(sql, map->creator);
    char *cond = NULL;
    asprintf(&cond, "name = '%s'", map_creator);

    int user_id = tr_db_get_id(sql, TR_DB_USER, cond);
    if (user_id < 0) {
        #pragma omp critical
        new_rq(sql,"INSERT INTO %s(name, density_star, reading_star,"
               "pattern_star, accuracy_star, final_star)"
               "VALUES('%s', 0, 0, 0, 0, 0);",
               TR_DB_USER, map_creator);
        user_id = tr_db_get_id(sql, TR_DB_USER, cond);
        fprintf(OUTPUT_INFO, "New user: %s, ID: %d\n",
                map_creator, user_id);
    }
    free(cond);
    free(map_creator);
    return user_id;
}

//-------------------------------------------------

static int tr_db_insert_mapset(const struct tr_map *map, int user_id)
{
    char *map_title  = tr_db_escape_str(sql, map->title);
    char *map_artist = tr_db_escape_str(sql, map->artist);
    char *map_source = tr_db_escape_str(sql, map->source);
    char *map_artist_uni = tr_db_escape_str(sql, map->artist_uni);
    char *map_title_uni  = tr_db_escape_str(sql, map->title_uni);
    char *cond = NULL;
    asprintf(&cond, "creator_ID = %d and artist = '%s' and "
             "title = '%s'",
             user_id, map_artist, map_title);

    int mapset_id = tr_db_get_id(sql, TR_DB_MAPSET, cond);
    if (mapset_id < 0) {
        #pragma omp critical
        new_rq(sql, "INSERT INTO %s(artist, title, source, "
               "creator_ID, artist_uni, title_uni, osu_mapset_ID)"
               "VALUES('%s', '%s', '%s', %d, '%s', '%s', %d);",
               TR_DB_MAPSET, map_artist, map_title, map_source, user_id,
               map_artist_uni, map_title_uni, map->mapset_osu_ID);
        mapset_id = tr_db_get_id(sql, TR_DB_MAPSET, cond);
        fprintf(OUTPUT_INFO, "New mapset: %s - %s ID: %d\n",
                map_artist, map_title, mapset_id);
    }
    free(cond);
    free(map_title);
    free(map_artist);
    free(map_source);
    free(map_artist_uni);
    free(map_title_uni);
    return mapset_id;
}

//-------------------------------------------------

static int tr_db_insert_diff(const struct tr_map *map, int mapset_id)
{
    char *map_diff = tr_db_escape_str(sql, map->diff);
    char *cond = NULL;
    asprintf(&cond, "mapset_ID = %d and diff_name = '%s'",
             mapset_id, map_diff);

    int diff_id = tr_db_get_id(sql, TR_DB_DIFF, cond);
    if (diff_id < 0) {
        #pragma omp critical
        new_rq(sql, "INSERT INTO %s(diff_name, mapset_ID, osu_diff_ID,"
               "max_combo, bonus, hash)"
               "VALUES('%s', %d, %d, %d, %d, '%s');",
               TR_DB_DIFF, map_diff, mapset_id, map->diff_osu_ID,
               map->max_combo, map->bonus, map->hash);
        diff_id = tr_db_get_id(sql, TR_DB_DIFF, cond);
        fprintf(OUTPUT_INFO, "New diff: %s ID: %d\n",
                map_diff, diff_id);
    }
    free(cond);
    free(map_diff);
    return diff_id;
}

//-------------------------------------------------

static int tr_db_insert_mod(const struct tr_map *map)
{
    char *mod_str = trm_mods_to_str(map);
    char *cond = NULL;
    asprintf(&cond, "mod_name = '%s'", mod_str);

    int mod_id = tr_db_get_id(sql, TR_DB_MOD, cond);
    if (mod_id < 0) {
        #pragma omp critical
        new_rq(sql, "INSERT INTO %s(mod_name) VALUES('%s');",
               TR_DB_MOD, mod_str);
        mod_id = tr_db_get_id(sql, TR_DB_MOD, cond);
        fprintf(OUTPUT_INFO, "New mod: %s ID: %d\n",
                mod_str, mod_id);
    }
    free(cond);
    free(mod_str);
    return mod_id;
}

//-------------------------------------------------

static int tr_db_insert_update_score(const struct tr_map *map,
                                     int diff_id, int mod_id)
{
    char *cond = NULL;
    asprintf(&cond, "diff_ID = %d and mod_ID = %d and great = %d"
             " and good = %d and miss = %d",
             diff_id, mod_id, map->great, map->good, map->miss);

    int score_id = tr_db_get_id(sql, TR_DB_SCORE, cond);
    if (score_id < 0) {
        #pragma omp critical
        new_rq(sql, "INSERT INTO %s(diff_ID, mod_ID, accuracy, "
               "combo, great, good, miss, "
               "density_star, pattern_star, reading_star, "
               "accuracy_star, final_star)"
               "VALUES(%d, %d, %.4g, %d, %d, %d, %d, "
               "%.4g, %.4g, %.4g, %.4g, %.4g);",
               TR_DB_SCORE, diff_id, mod_id, map->acc,
               map->combo, map->great, map->good, map->miss,
               map->density_star, map->pattern_star,
               map->reading_star, map->accuracy_star,
               map->final_star);
        score_id = tr_db_get_id(sql, TR_DB_SCORE, cond);
        fprintf(OUTPUT_INFO, "New score: (%g%%) ID: %d\n",
                map->acc, score_id);
    } else {
        #pragma omp critical
        new_rq(sql, "UPDATE %s SET combo = %d, density_star = %.4g, "
               "reading_star = %.4g, pattern_star = %.4g,"
               "accuracy_star = %.4g, final_star = %.4g "
               "WHERE ID = %d;",
               TR_DB_SCORE, map->combo, map->density_star,
               map->reading_star, map->pattern_star, map->accuracy_star,
               map->final_star, score_id);
        fprintf(OUTPUT_INFO, "Updated score: (%g%%) ID: %d\n",
                map->acc, score_id);
    }
    free(cond);
    return score_id;
}

//-------------------------------------------------

void trm_db_insert(const struct tr_map *map)
{
    if (sql == NULL) {
        tr_error("Couldn't connect to DB. Data won't be stored.");
        return;
    }

    int user_id = tr_db_insert_user(map);
    int mod_id = tr_db_insert_mod(map);
    int mapset_id = tr_db_insert_mapset(map, user_id);
    int diff_id = tr_db_insert_diff(map, mapset_id);
    tr_db_insert_update_score(map, diff_id, mod_id);
}

//-------------------------------------------------
//-------------------------------------------------
//-------------------------------------------------

#else // USE_TR_MYSQL_DB

#include "taiko_ranking_map.h"

void tr_db_init(void)
{

}

void trm_db_insert(const struct tr_map *map UNUSED)
{
    tr_error("Database was not compiled!");
}

#endif // USE_TR_MYSQL_DB
