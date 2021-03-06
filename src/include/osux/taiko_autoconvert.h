#ifndef OSUX_TAIKO_AUTOCONVERT_H
#define OSUX_TAIKO_AUTOCONVERT_H

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

#include <glib.h>

G_BEGIN_DECLS

/*
 * return 0 on success
 * return -1 on failure
 */
int osux_beatmap_taiko_autoconvert(osux_beatmap *bm);

G_END_DECLS

#endif // OSUX_TAIKO_AUTOCONVERT_H
