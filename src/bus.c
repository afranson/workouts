/* Defines function related to 'pure' bus activity */

#include <errno.h>		/* errno */
#include <string.h>
#include <libgen.h> 		/* basename, dirname */
#include "workouts.h"
#include <zlib.h>


struct bus bus_default = {0, NULL, broken, NULL, NULL, 0, NULL, NULL, 0};

/* Main object that gets passed around during the execution of the program.
   Stores CLI args, parsed CLI args, the filename, number of workouts, the workouts themselves, recent workouts, and number of unique workouts.*/
struct bus
bus_init(int argc, char **const argv, const char *const filename)
{
    struct bus mainbus = bus_default;
    mainbus.argc = argc;
    mainbus.argv = argv;
    /* mainbus.method = broken; */ 

    char filepath[FILENAME_MAX];
    snprintf(filepath, FILENAME_MAX, "%s/%s", dirname(argv[0]), filename);
    mainbus.filename = strdup(filepath);
    return mainbus;
}


struct string_method_pair
{
    char* string;
    enum methods method;
};


/* Iterate over  */
enum methods
check_argv_against_pairs(const struct string_method_pair *smp, size_t smp_len, char** argv)
{
    for ( size_t i=0; i<smp_len; i++ ) {
	    
	if ( !strcmp(argv[1], smp[i].string) || argv[1][0] == smp[i].string[0] ) { /* Check is strings match or if first letters match */
	    return smp[i].method;
	}
    }
    fprintf(stderr, "Error: Failed to find '%s'. Either invalid option or wrong number of inputs.\n", argv[1]);
    return broken;
}


/* Parses the command line args for the program
   If 1 (no args) - default to show
   If 2 - Check for create, help, all, and show
   If 3 - Check for list, progress, edit, and rm. */
enum methods
bus_parse_argv(struct bus *const mainbus)
{
    const struct string_method_pair two_args_checks[] = { {"backup", backup}, { "create", create }, { "help", help }, { "all", all }, { "show", show } };
    const struct string_method_pair three_args_checks[] = { { "list", list_wid }, {"detail", detail_wid}, { "progress", progress_wid }, { "edit", edit_wid }, { "rm", rm_wid } };
    enum methods method = mainbus->method;
    int argc = mainbus->argc;
    char **argv = mainbus->argv;
    if ( argc == 1 ) { 		/* No arguments given */
        method = show;
    } else if ( argc == 2 ) { 	/* Single argument given */
	method = check_argv_against_pairs(two_args_checks, sizeof(two_args_checks)/sizeof(two_args_checks[0]), argv);
    } else if ( argc == 3 ) {	/* 2 arguments given */
	method = check_argv_against_pairs(three_args_checks, sizeof(three_args_checks)/sizeof(three_args_checks[0]), argv);
    } else {			/* Too many arguments */
        printf("\t** Invalid argument combination **\n");
        method = broken;
    }
    return method;
}

/* Prints the result of the user asking for help or using the command line args incorrectly. */
void
bus_do_broken_help_create_backup(struct bus *const mainbus)
{
    if ( mainbus->method == create )
    {
        bus_create_and_add_workout(mainbus);
        mainbus->method = show;
        return;
    } else if ( mainbus->method == backup ) {
	bus_backup_workout_file(mainbus);
	return;
    } else if ( mainbus->method == help || mainbus->method == broken) {
        puts("usage:  workouts [[o]ption] [id]");
        puts("___options___");
	printf("%-12s --  %s\n", "help", "Display this help message");
        printf("%-12s --  %s\n", "show", "Default, list all active, recent workouts and ids");
        printf("%-12s --  %s\n", "all", "Show all active and disactive, recent workouts");
	printf("%-12s --  %s\n", "backup", "Create a gzip backup of the workout datafile");
        printf("%-12s --  %s\n", "create", "Create a new workout and add to active workouts");
	printf("%-9s %s --  %s\n", "list", "id", "Show history of one particular workout");
	printf("%-9s %s --  %s\n", "detail", "id", "Show full detail for long entries");
        printf("%-9s %s --  %s\n", "progress", "id", "Advance an active workout to its next form");
        printf("%-9s %s --  %s\n", "edit", "id", "Edit any recent workout in-place");
        printf("%-9s %s --  %s\n", "rm", "id", "Deactivate an active workout");
        exit(EXIT_SUCCESS);
    } else {
	return;
    }
}


/* Backs up the workout file with the date */
void
bus_backup_workout_file(struct bus *const mainbus)
{
    char *today = get_todays_date_yymmdd();
    /* Allocate space for base filename/path, '_', date, and '.gz' '\0'  */
    char *compressed_filename = malloc(strlen(mainbus->filename) + 1 + strlen(today) + 4);
    sprintf(compressed_filename, "%s_%s.gz", mainbus->filename, today); /* TODO, I would like the date to be in front */
    printf("Backing up '%s' to gzipped file '%s'.\n", mainbus->filename, compressed_filename);
    mainbus->workoutFile = bus_open_workoutfile(mainbus);
    FILE *compressed_file = fopen(compressed_filename, "wb");
    free(today);
    free(compressed_filename);
    if ( !compressed_file ) {
	fprintf(stderr, "Failed to open compressed file, %s\n", compressed_filename);
	fclose(mainbus->workoutFile);
	return;
    }
    int ret = def(mainbus->workoutFile, compressed_file, -1);
    if ( ret != Z_OK ) {
	zerr(ret);
    }
    fclose(mainbus->workoutFile);
    fclose(compressed_file);
    return;
}


/* Creates a new workout and adds it to the workout file and buses workouts list */
int
bus_create_and_add_workout(struct bus *const mainbus)
{
    // generate the new workout from user input
    char *todays_date = get_todays_date_yymmdd();
    struct workout workout_to_add = workouts_generate_workout((char *[7]){"", "", "", "", "", todays_date, ""});

    // open file for appending
    mainbus->workoutFile = bus_open_workoutfile_append(mainbus);

    // write the new workout
    bus_write_workout(mainbus, workout_to_add);

    // close file
    bus_close_workoutfile(mainbus);

    // reassure user
    printf("'%s' added to workouts.\n", workout_to_add.exercise);
    free(todays_date);
    return EXIT_SUCCESS;
}


/* Writes a workout to the workout file */
int
bus_write_workout(struct bus *const mainbus, const struct workout workout_to_add)
{
    char *workout_string = workout_to_string(workout_to_add);
    fputs(workout_string, mainbus->workoutFile);
    free(workout_string);
    return EXIT_SUCCESS;
}



/* Opens the workout file for reading with errors if it fails to open */
FILE *
bus_open_workoutfile(struct bus *const mainbus)
{
    FILE *workoutFile;
    if ( (workoutFile = fopen(mainbus->filename, "r")) ) { // All is good
	return workoutFile;
    } else { // File pointer is NULL
	if ( errno == 2 ) { // File does not exist
	    if ( (workoutFile = fopen(mainbus->filename, "a+")) ) { // Created file successfully
		printf("Created new file '%s'.\n", mainbus->filename);
		return workoutFile;
	    } else {		// Unable to create file
		fprintf(stderr, "Error creating file '%s' for appending. Errno: %d.\n", mainbus->filename, errno);
		exit(EXIT_FAILURE);
	    }
	} else { 		/* Errno is not 2, Needs further investigation. */
	    fprintf(stderr, "Error opening file '%s' for reading. Errno: %d.\n", mainbus->filename, errno);
	    exit(EXIT_FAILURE);
	}
    }
}


/* Opens the workout file for appending with errors if it fails to open */
FILE *
bus_open_workoutfile_append(struct bus *const mainbus)
{
    FILE *workoutFile;
    if ( (workoutFile = fopen(mainbus->filename, "a")) == NULL ) {
        perror("Error opening workouts file");
        exit(EXIT_FAILURE);
    }
    return workoutFile;
}


/* Counts number of newlines in the file as a safe (over)estimate of the number of workouts */
size_t
bus_get_num_workouts(struct bus *const mainbus)
{
    char c;
    size_t count = 0;
    for (c = getc(mainbus->workoutFile); c != EOF; c = getc(mainbus->workoutFile)) {
        if (c == '\n') {
            count++;
        }
    }
    // reset pointer to start of file
    if ( fseek(mainbus->workoutFile, 0, SEEK_SET) ) {
        perror("Error returning to start of file");
        exit(EXIT_FAILURE);
    }
    return count;
}


/* Allocate space for workouts based on length of workout file (num_workouts) */
void
bus_malloc_workouts(struct bus *const mainbus)
{
    /* +1s account for potential added or modified workouts */
    mainbus->workouts = malloc((mainbus->num_workouts+1) * sizeof(*mainbus->workouts));
    mainbus->recent_workouts = malloc((mainbus->num_workouts+1) * sizeof(*mainbus->recent_workouts));
    return;
}


/* Read all lines from the workout file into the bus
   Normal workouts are read in full, rm's just add a
   nonactive headstone workout */
void
bus_read_workoutfile(struct bus *const mainbus)
{
    char buffer[MAX_WORKOUT_SIZE];
    size_t i = 0;
    while ( fgets(buffer, MAX_WORKOUT_SIZE, mainbus->workoutFile) ) {
	buffer[strlen(buffer)-1] = '\0'; /* Remove \n at end of each line */
	struct workout buffer_workout = string_to_workout(buffer);
	mainbus->workouts[i] = buffer_workout;
	bus_update_recent_workouts(mainbus, mainbus->workouts[i]);
	i++;
    }
    return;
}


/* If workout is active (i.e. not a rm entry), check for it in recent_workouts
   and if it's there, update the pointer. If not, add it to the list. If it is
   not active (rm entry), remove the active status from the recent_workouts entry. */
void
bus_update_recent_workouts(struct bus *const mainbus, struct workout workout)
{
    /* Find the index of the entry in recent_workouts that matches input workout */
    int match_i = -1;
    for (size_t i=0; i < mainbus->num_uniques; i++) {
        if( !strcmp(mainbus->recent_workouts[i].id, workout.id) ) {
            match_i = i;
        }
    }
    if ( match_i == -1 ) {	/* Workout not found in recents */
        mainbus->recent_workouts[mainbus->num_uniques++] = workout;
    } else if ( workout.active ) { /* Workout found and active */
        mainbus->recent_workouts[match_i] = workout;
    } else {			/* workout found and rm entry */
	mainbus->recent_workouts[match_i].active = false;
    }
    return;
}


/* Close the workout file with error reporting */
void
bus_close_workoutfile(struct bus *const mainbus)
{
   if ( fclose(mainbus->workoutFile) ) {
        perror("Error closing workouts file");
        exit(EXIT_FAILURE);
    }
    return;
}


/* Completely frees the bus at the end of the program */
void
free_bus(struct bus *const mainbus)
{
    free(mainbus->filename);
    for (size_t i=0; i<mainbus->num_workouts; i++) {
	free(mainbus->workouts[i].id);
	free(mainbus->workouts[i].exercise); /* Frees all fields due to how split_strings work */
    }
    free(mainbus->workouts);
    free(mainbus->recent_workouts);
    return;
}
