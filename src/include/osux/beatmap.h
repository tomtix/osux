#ifndef OSUX_BEATMAP_H
#define OSUX_BEATMAP_H

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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <openssl/md5.h>

#include "osux/hitobject.h"
#include "osux/game_mode.h"
#include "osux/hash_table.h"

#include "osux/color.h"
#include "osux/timingpoint.h"
#include "osux/event.h"


typedef struct osux_beatmap osux_beatmap;

struct osux_beatmap {
    uint32_t beatmap_id;
    uint32_t beatmap_set_id;

    uint32_t osu_forum_thrd;

    char *md5_hash;
    char *osu_filename;
    char *file_path;

    time_t last_modification;
    time_t last_checked;

    // stats
    uint16_t circles;
    uint16_t sliders;
    uint16_t spinners;

    int64_t drain_time;
    int64_t total_time;
    double bpm_avg;
    double bpm_max;
    double bpm_min;
    bool already_played;
    time_t last_played;

    //offsets
    uint16_t local_offset;
    uint16_t online_offset;

    // misc
    bool ignore_hitsound;
    bool ignore_skin;
    bool disable_sb;
    bool disable_video;
    bool visual_override;
    uint8_t mania_scroll_speed;

    // .osu  SECTION :
    uint32_t osu_version;

    // general info
    char *AudioFilename;
    int64_t AudioLeadIn;
    int64_t PreviewTime;
    int64_t Countdown;

    char *SampleSet;  // sample type !
    int64_t sample_type;

    double StackLeniency;

    union {
        int64_t Mode; // game mode
        int64_t game_mode;
    };

    int64_t LetterboxInBreaks;
    int64_t WidescreenStoryboard;

    // Editor settings
    double DistanceSpacing;
    double TimelineZoom;
    int64_t GridSize;
    int64_t BeatDivisor;

    uint32_t bookmark_count;
    uint32_t bookmark_bufsize;
    int64_t *bookmarks;

    // Metadata
    char *Title;
    char *TitleUnicode;
    char *Artist;
    char *ArtistUnicode;
    char *Creator;

    union {
        char *Version; // difficulty name
        char *difficulty_name;
    };

    char *Source;

    uint32_t tag_count;
    char *tags_orig;
    char **tags;

    int64_t BeatmapID;
    int64_t BeatmapSetID;

    // Difficulty
    double HPDrainRate;
    double CircleSize;
    double OverallDifficulty;
    double ApproachRate;
    double SliderMultiplier;
    double SliderTickRate;

    uint32_t color_count;
    uint32_t color_bufsize;
    osux_color *colors;

    uint32_t event_count;
    uint32_t event_bufsize;
    osux_event *events;

    uint32_t timingpoint_count;
    uint32_t timingpoint_bufsize;
    osux_timingpoint *timingpoints;

    uint32_t hitobject_count;
    uint32_t hitobject_bufsize;
    osux_hitobject *hitobjects;

    osux_hashtable *sections;
};

int osux_beatmap_init(osux_beatmap *beatmap, char const *filename);
int osux_beatmap_free(osux_beatmap *beatmap);
char *osux_beatmap_default_filename(const osux_beatmap *bm);
int osux_beatmap_prepare(osux_beatmap *beatmap);
int osux_beatmap_print(osux_beatmap const *m, FILE *f);
int osux_beatmap_save(osux_beatmap const *beatmap, char const *path);
int osux_beatmap_save_full(osux_beatmap const *beatmap,
                           char const *dirpath, char const *filename,
                           bool use_default_filename);


#endif // OSUX_BEATMAP_H
