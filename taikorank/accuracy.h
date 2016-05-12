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

#ifndef ACCURACY_H
#define ACCURACY_H


// work independently, use trm_get_ggm_m to get second parameter
void tro_set_hit_window(struct tr_object * o, const int * ggm_val);

// work independently
void tro_set_slow(struct tr_object * o);
void tro_set_spacing(struct tr_object * o, int i);

// work independently, must be done after all others
void tro_set_accuracy_star(struct tr_object * o);

// all
void trm_compute_accuracy(struct tr_map * map);

#endif //ACCURACY_H
