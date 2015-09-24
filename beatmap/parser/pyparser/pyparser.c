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

#include <string.h>
#include <stdint.h>

#include "beatmap/beatmap.h"

#include <beatmap/timingpoint/timing_point.h>
#include <beatmap/timingpoint/pytimingpoint.h>

#include <beatmap/hitobject/hit_object.h>
#include <beatmap/hitobject/pyhitobject.h>

#include <beatmap/combocolor/combocolor.h>

#include "pyparser.h"

#include <embed/python.h>
#include <python2.7/Python.h>

#include <embed/module.h>


__attribute__((constructor))
static void pyparser_init(void)
{
    module_inc_usecount("py_parser", "parser");
    module_inc_usecount("py_parser", "embed_python");
    
}

__attribute__((destructor))
static void pyparser_exit(void)
{

    module_dec_usecount("py_parser", "embed_python");
    module_dec_usecount("py_parser", "parser");
}


#define READ_VALUE(map, fieldName, pyObj, pyMethod, convMethod)		\
    ({									\
	PyObject *pyTmp;						\
	pyTmp = PyDict_GetItemString(pyObj, #fieldName);		\
	if (pyTmp) {							\
	    map->fieldName = convMethod(pyMethod(pyTmp));		\
	    if (PyErr_Occurred())					\
	    { puts("\n");PyErr_Print();	puts("\n");};			\
	}								\
	else								\
	    map->fieldName = DEFAULT_MAP.fieldName;			\
    })									\


#define READ_STRING(map, fieldName, pyObj)				\
    READ_VALUE(map, fieldName, pyObj, PyString_AsString, strdup)	\
    
#define READ_DOUBLE(map, fieldName, pyObj)				\
    READ_VALUE(map, fieldName, pyObj, PyFloat_AsDouble,)		\
    
#define READ_INT(map, fieldName, pyObj)					\
    READ_VALUE(map, fieldName, pyObj, PyInt_AsLong, (uint32_t))		\

static void map_parse_General(PyObject *d, struct map *m)
{
    if (!d)
	return;
    
    READ_STRING(m, AudioFilename,        d);
    READ_DOUBLE(m, AudioLeadIn,          d);
    READ_DOUBLE(m, PreviewTime,          d);
    READ_DOUBLE(m, Countdown,            d);
    READ_STRING(m, SampleSet,            d);
    READ_DOUBLE(m, StackLeniency,        d);
    READ_INT(   m, Mode,                 d);
    READ_INT(   m, LetterboxInBreaks,    d);
    READ_INT(   m, WidescreenStoryboard, d);

    // UseSkinSprites
    // StoryfireInFront
}

static void map_parse_Editor(PyObject *d, struct map *m)
{
    if (!d)
	return;
    PyObject *v = NULL;

    v = PyDict_GetItemString(d, "Bookmarks");
    if (v) {
	m->bkmkc = (uint32_t) PyList_Size(v);
	m->Bookmarks = malloc(sizeof (*m->Bookmarks) * m->bkmkc);
	for (int i = 0; i < m->bkmkc; ++i)
	    m->Bookmarks[i] = (uint32_t) PyInt_AsLong(PyList_GetItem(v, i));
    }
    
    READ_DOUBLE(m, DistanceSpacing, d);
    READ_INT(m,    BeatDivisor, d);
    READ_INT(m,    GridSize, d);
    READ_DOUBLE(m, TimelineZoom, d);
    
}

static void map_parse_Metadata(PyObject *d, struct map *m)
{
    if (!d)
	return;

    READ_STRING(m, Title, d);
    READ_STRING(m, TitleUnicode, d);
    READ_STRING(m, Artist, d);
    READ_STRING(m, ArtistUnicode, d);
    READ_STRING(m, Creator, d);
    READ_STRING(m, Version, d);
    READ_STRING(m, Source, d);

    READ_INT(m, BeatmapID, d);
    READ_INT(m, BeatmapSetID, d);


    // tags
    PyObject *v = PyDict_GetItemString(d, "Tags");
    if (v) {
	m->tagc = (uint32_t) PyList_Size(v);
	m->Tags = malloc(sizeof (*m->Tags) * m->tagc);
	for (int i = 0; i < m->tagc; ++i)
	    m->Tags[i] = strdup(PyString_AsString(PyList_GetItem(v, i)));
    }
}

static void map_parse_Colours(PyObject *d, struct map *m)
{
    if (!d)
	return;

    size_t size = PyList_Size(d);
    m->Colours = malloc(sizeof(*m->Colours) * size);
    m->colc = size;
    for (int i = 0; i < size; ++i) {
	PyObject *p = PyList_GetItem(d, i);
	col_parse(&m->Colours[i], p);
    }
}

static void map_parse_Difficulty(PyObject *d, struct map *m)
{
    if (!d)
	return;
    READ_DOUBLE(m, HPDrainRate, d);
    READ_DOUBLE(m, CircleSize, d);
    READ_DOUBLE(m, OverallDifficulty, d);
    READ_DOUBLE(m, ApproachRate, d);
    READ_DOUBLE(m, SliderMultiplier, d);
    READ_DOUBLE(m, SliderTickRate, d);
}

static void map_parse_TimingPoints(PyObject *d, struct map *m)
{
    if (!d)
	return;
    m->tpc = (uint32_t) PyList_Size(d);
    m->TimingPoints = malloc(sizeof (*m->TimingPoints) * m->tpc);
    for (int i = 0; i < m->tpc; ++i)
	tp_py_parse(&m->TimingPoints[i], PyList_GetItem(d, i));
}

static void map_parse_HitObjects(PyObject *d, struct map *m)
{
    if (!d)
	return;
    m->hoc = (uint32_t) PyList_Size(d);
    m->HitObjects = malloc(sizeof (*m->HitObjects) * m->hoc);
    for (int i = 0; i < m->hoc; ++i)
	ho_py_parse(&m->HitObjects[i], PyList_GetItem(d, i));
}

static void map_parse_Events(PyObject *d, struct map *m)
{
    if (!d)
	return;
}


#define PARSE_SECTION(sect, pyobjsrc, map)			\
    ({								\
	PyObject *pyobjtmp;					\
	pyobjtmp = PyDict_GetItemString(pyobjsrc, #sect);	\
	map_parse_##sect(pyobjtmp, map);			\
    })


struct map *osux_py_parse_beatmap(const char *filename)
{
    
    PyObject *data =
	embed_python_funcall("./scripts/python", "omp", "parse",
			     1, (const char*[]) { filename });
    if (!data) {
	fprintf(stderr, "Error parsing with omp python module");
	return NULL;
    }
    struct map *m = calloc(sizeof(*m), 1);

    m->version = PyInt_AsLong(PyDict_GetItemString(data, "version"));
    m->bom = PyInt_AsLong(PyDict_GetItemString(data, "BOM"));
    
    PARSE_SECTION( General,      data, m);
    PARSE_SECTION( Editor,       data, m);
    PARSE_SECTION( Difficulty,   data, m);
    PARSE_SECTION( Metadata,     data, m);
    PARSE_SECTION( Events,       data, m);
    PARSE_SECTION( TimingPoints, data, m);
    PARSE_SECTION( Colours,      data, m);
    PARSE_SECTION( HitObjects,   data, m);

    Py_XDECREF(data);

    return  m;
}