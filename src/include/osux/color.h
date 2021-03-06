#ifndef OSUX_COLOR_H
#define OSUX_COLOR_H
/*
 * Copyright (c) 2015 Lucas Maugère, Thomas Mijieux
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <glib.h>

G_BEGIN_DECLS

#define COLOR_TYPES(COLOR)                                              \
    COLOR(COMBO,                     N_("Combo"))                       \
    COLOR(MENU_GLOW,                 N_("MenuGlow"))                    \
    COLOR(SLIDER_BORDER,             N_("SliderBorder"))                \
    COLOR(SPINNER_APPROACH_CIRCLE,   N_("SpinnerApproachCircle"))       \
    COLOR(SONG_SELECT_ACTIVE_TEXT,   N_("SongSelectActiveText"))        \
    COLOR(SONG_SELECT_INACTIVE_TEXT, N_("SongSelectInactiveText"))      \
    COLOR(STAR_BREAK_ADDITIVE,       N_("StarBreakAdditive"))           \
    COLOR(SLIDER_TRACK_OVERRIDE,     N_("SliderTrackOverride"))         \


#define COLOR_TYPE_TO_ENUM(capital_, pretty_)   \
    COLOR_##capital_,

enum color_type {
    COLOR_TYPES(COLOR_TYPE_TO_ENUM)
    MAX_COLOR_TYPE,
};

typedef struct osux_color_ {
    int type;
    uint32_t id;
    int32_t r;
    int32_t g;
    int32_t b;
    int32_t a;
} osux_color;

typedef struct osux_combo_ {
    uint32_t id;
    uint32_t pos;
    GList *colours;
    GList *current;
} osux_combo;

char const *osux_color_type_get_name(int type);
int osux_color_init(osux_color *c, char *line, uint32_t osu_version);
void osux_color_free(osux_color *c);
void osux_color_print(FILE *f, osux_color *c);
void osux_color_copy(osux_color *from, osux_color *to);
void osux_color_move(osux_color *from, osux_color *to);
bool osux_color_array_contains_type(osux_color array[],
                                    uint32_t size, int type);

G_END_DECLS

#endif // OSUX_COLOR_H
