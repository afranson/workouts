// .h file for the whole workouts program collective

#ifndef WORKOUTS_H
#define WORKOUTS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_WORKOUT_SIZE 1000

enum methods { METHOD_MIN=-1, broken, help, show, all, create, progress_wid, edit_wid, rm_wid, list_wid, detail_wid, backup, METHOD_MAX };

struct workout
{
    bool active;
    char* id;
    char* exercise;
    char* weights;
    char* sets;
    char* reps;
    char* days;
    char* date;
    char* notes;
};

struct workouts
{
    struct workout *all_workouts; /* Gets rid of the rm's */
    struct workout *recent_workouts; /* Only the active and most recent version of workouts */
    char **unique_workout_names;     /* For determining if a workout is new or not */
};

struct split_string
{
    char **str_p_array;
    size_t num_elements;
};

struct bus
{
    int argc;
    char **argv;
    enum methods method;
    char *filename;
    FILE *workoutFile;
    size_t num_workouts;
    struct workout *workouts;
    struct workout *recent_workouts;
    size_t num_uniques;
};

struct size_t_w_error
{
    size_t value;
    bool error;
};

extern struct workout workout_default;
extern struct bus bus_default;

/* Compression */
void zerr(int ret);
int def(FILE *src, FILE *dst, int level);

/* Utilities */
void print_buffer(char* buff, size_t len);
void remove_end_newline(char **bufferp);
char *get_todays_date_yymmdd(void);

/* Splitting Strings */
struct split_string strsplit(char * string_to_split, char delim);
void free_split_string(struct split_string);
void print_split_string(struct split_string);
int test_strsplit(void);

/* Workout struct functions */
void workout_pprint_header();
void workout_pprint(struct workout workout);
void workout_detail_print(struct workout workout);
size_t workout_get_num_chars(struct workout workout);
char *workout_to_string(struct workout workout_in);
struct split_string workout_to_split_string(struct workout workout_in);
struct workout create_rm_workout(char *exercise);
struct workout string_to_workout(char* string);
int workout_compare(struct workout workout_a, struct workout workout_b);
char *workout_get_id(char *name);

/* Bus Functions */
struct bus bus_init(int argc, char **argv, const char *filename);
enum methods bus_parse_argv(struct bus *);
FILE *bus_open_workoutfile(struct bus *);
FILE *bus_open_workoutfile_append(struct bus *);
void bus_do_broken_help_create_backup(struct bus *);
void bus_backup_workout_file(struct bus *);
int bus_create_and_add_workout(struct bus *);
size_t bus_get_num_workouts(struct bus *);
void bus_malloc_workouts(struct bus *);
void bus_read_workoutfile(struct bus *);
void bus_close_workoutfile(struct bus *);
int bus_write_workout(struct bus *, struct workout);
void bus_update_recent_workouts(struct bus *, struct workout workout);
void free_bus(struct bus *);

/* Workouts Functions */
struct size_t_w_error workouts_get_most_recent_workout(struct bus *, char *workout_name, size_t i);
void workouts_print_workouts(struct bus *);
int compare_date(const void *a, const void *b);
void workouts_wid_actions(struct bus *);
 int workouts_progress_wid_workout(struct bus *, char *id);
 int workouts_detail_wid_workout(struct bus *, char *id);
 int workouts_rm_wid_workout(struct bus *, char *id);
  int workouts_write_rm_workout(FILE *, struct workout);
 int workouts_list_wid_workout(struct bus *, char *id);
 void workouts_edit_wid_workout(struct bus *main, char *temp);
  int workouts_write_edited_workout(struct bus *main, struct bus *temp, struct workout original, struct workout edited);
  struct workout workouts_generate_workout(char **default_options);

#endif
