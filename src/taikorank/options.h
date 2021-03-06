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
#ifndef TR_OPTIONS_H
#define TR_OPTIONS_H

#define LOCAL_OPT_PREFIX  "-"
#define GLOBAL_OPT_PREFIX "+"

void tr_options_initialize(void);
int local_opt_set(int argc, const char **argv);
int global_opt_set(int argc, const char **argv);

void print_help(void);

#endif // TR_OPTIONS_H
