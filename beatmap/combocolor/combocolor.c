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

#include <stdio.h>

#include <python2.7/Python.h>
#include "combocolor.h"

void col_print(FILE *f, struct color *c, int id)
{
    fprintf(f, "Combo%d : %hhu,%hhu,%hhu\r\n", id,  c->r, c->g, c->b);
}

void col_parse(struct color *c, PyObject *p)
{
    PyObject *q = PyList_GetItem(p, 1);
    
    c->r = PyInt_AsLong(PyList_GetItem(q, 0));
    c->g = PyInt_AsLong(PyList_GetItem(q, 1));
    c->b = PyInt_AsLong(PyList_GetItem(q, 2)) ;
}