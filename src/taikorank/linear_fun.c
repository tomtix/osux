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
#include <math.h>

#include "osux.h"

#include "print.h"
#include "cst_yaml.h"
#include "vector.h"
#include "linear_fun.h"

#define ERROR_VAL INFINITY

// piecewise linear function
// f(x) = a*x+b
struct linear_fun {
    char *name;
    int len;
    double *x; // len
    double *a; // len - 1
    double *b; // len - 1

    int has_error;
};
/*
  x = [x0, x1, x2, ...]
  a = [a0, a1, a2, ...]
  b = [b0, b1, b2, ...]
  in [xi, xi+1] use ai, bi
*/

//--------------------------------------------------

struct linear_fun *lf_new(struct vector *v)
{
    struct linear_fun *lf = malloc(sizeof(*lf));
    lf->has_error = 0;
    lf->len = v->len;
    lf->x = malloc(sizeof(double) * lf->len);
    for (int i = 0; i < lf->len; i++) {
        lf->x[i] = v->t[i][0];
    }
    lf->a = malloc(sizeof(double) * lf->len - 1);
    lf->b = malloc(sizeof(double) * lf->len - 1);
    for (int i = 0; i < lf->len - 1; i++) {
        lf->a[i] = ((v->t[i][1] - v->t[i+1][1]) /
                    (v->t[i][0] - v->t[i+1][0]));
        lf->b[i] = v->t[i][1] - lf->a[i] * v->t[i][0];
    }
    return lf;
}

void lf_free(struct linear_fun *lf)
{
    if (lf == NULL)
        return;
    free(lf->x);
    free(lf->a);
    free(lf->b);
    free(lf);
}

//--------------------------------------------------

static inline int find_interval_binary(const double *array,
                                       int l, int r, double x)
{
    if (r < l)
        return -1;
    int m = (l + r) / 2;
    if (x < array[m])
        return find_interval_binary(array, l, m-1, x);
    if (array[m+1] < x)
        return find_interval_binary(array, m+1, r, x);
    return m;
}

static inline int find_interval_linear(const double *array, int len,
                                       double x)
{
    if (x < array[0])
        return -1;
    for (int i = 1; i < len; i++)
        if (x <= array[i])
            return i-1;
    return -1;
}

static inline double lf_eval_interval(struct linear_fun *lf,
                                      double x, int i)
{
    return lf->a[i] * x + lf->b[i];
}

double lf_eval(struct linear_fun *lf, double x)
{
    // As array are small, binary search is not really faster
    int i = find_interval_linear(lf->x, lf->len, x);
    //int i = find_interval_binary(lf->x, 0, lf->len-1, x);
    if (i < 0) {
        if (!lf->has_error) {
            tr_error("Out of bounds value (%g) for linear_fun (%s)",
                     x, lf->name);
            lf_print(lf);
            lf->has_error = 1;
        }
        return ERROR_VAL;
    }
    return lf_eval_interval(lf, x, i);
}

//--------------------------------------------------

struct linear_fun *cst_lf(GHashTable *ht, const char *key)
{
    struct vector *v = cst_vect(ht, key);
    struct linear_fun *lf = lf_new(v);
    vect_free(v);
    lf->name = (char*) key;
    return lf;
}

//--------------------------------------------------

void lf_print(struct linear_fun *lf)
{
    fprintf(stderr, "linear_fun: %s\nx:", lf->name);
    for (int i = 0; i < lf->len; i++)
        fprintf(stderr, "\t%.4g", lf->x[i]);
    fprintf(stderr, "\na:");
    for (int i = 0; i < lf->len-1; i++)
        fprintf(stderr, "\t%.4g", lf->a[i]);
    fprintf(stderr, "\nb:");
    for (int i = 0; i < lf->len-1; i++)
        fprintf(stderr, "\t%.4g", lf->b[i]);
    fprintf(stderr, "\n");
}

//--------------------------------------------------

static void append_string(char **str, char const *format, ...)
{
    va_list ap;
    char *append, *new_str;

    va_start(ap, format);
    append = g_strdup_vprintf(format, ap);
    va_end(ap);

    new_str = g_strdup_printf("%s%s", *str, append);
    g_free(*str); g_free(append);
    *str = new_str;
}

static char *lf_array_to_str(double *t, int len, char const *var)
{
    char *values = g_strdup_printf("%g", t[0]);
    for (int i = 1; i < len; i++)
        append_string(&values, ", %g", t[i]);

    char *array_str;
    array_str = g_strdup_printf("double %s[%d] = {%s};\n", var, len, values);
    g_free(values);
    return array_str;
}

void lf_dump(struct linear_fun *lf)
{
    char *x_name = g_strdup_printf("%s_x", lf->name);
    char *x = lf_array_to_str(lf->x, lf->len,   x_name);

    char *a_name = g_strdup_printf("%s_a", lf->name);
    char *a = lf_array_to_str(lf->a, lf->len-1, a_name);

    char *b_name = g_strdup_printf("%s_b", lf->name);
    char *b = lf_array_to_str(lf->b, lf->len-1, b_name);

    char *var, *all;
    var = g_strdup_printf(
        "struct linear_fun lf_%s = {\n"
        "\t.name = \"%s\",\n"
        "\t.len = %d,\n"
        "\t.x = %s,\n"
        "\t.a = %s,\n"
        "\t.b = %s,\n"
        "};\n",
        lf->name, lf->name, lf->len,
        x_name, a_name, b_name
    );
    all = g_strdup_printf("%s%s%s%s", x, a, b, var);
    fprintf(stderr, "%s", all);

    g_free(x_name);  g_free(x);
    g_free(a_name);  g_free(a);
    g_free(b_name);  g_free(b);
    g_free(var);
    g_free(all);
}
