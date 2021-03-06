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
#ifndef TR_FINAL_STAR_H
#define TR_FINAL_STAR_H

void tr_final_star_initialize(void);

/* First arg is all objects
   Use:
   - ps
   - offset
   - density_star
   - reading_star
   - pattern_star
   - accuracy_star
*/
void tro_set_influence(struct tr_object *objs, int i, int nb);

/* Use:
   - ps
   - density_star
   - reading_star
   - pattern_star
   - accuracy_star
*/
void tro_set_final_star(struct tr_object *o);

// all
void trm_compute_final_star(struct tr_map *map);

#endif // TR_FINAL_STAR_H
