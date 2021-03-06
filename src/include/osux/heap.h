#ifndef OSUX_HEAP_H
#define OSUX_HEAP_H

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

#include <stdlib.h>
#include <glib.h>

G_BEGIN_DECLS

struct heap;

struct heap *heap_new(unsigned int buffer_size, void *buffer,
		      int (*cmp)(void*, void*));
void heap_free(struct heap *heap);
size_t heap_size(struct heap *heap);
void *heap_extract_max(struct heap *heap);
void *heap_max(struct heap *heap);
void heap_insert(struct heap *heap, void *k);


G_END_DECLS

#endif // OSUX_HEAP_H
