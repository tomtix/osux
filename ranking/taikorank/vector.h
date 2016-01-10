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
#ifndef VECTOR_H
#define VECTOR_H

/**
 * Vector used to construct function. For example: interpolation.
*/
struct vector {
  int len;
  double ** t;
};

struct vector * cst_vect(struct hash_table * ht, const char * key);
void vect_free(struct vector * v);

// 2pt
double vect_exp(struct vector * v, double x);
double vect_poly2(struct vector * v, double x);

#endif //VECTOR_H
