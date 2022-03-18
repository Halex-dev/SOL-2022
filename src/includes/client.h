#ifndef _CLIENT_H
#define _CLIENT_H

#include "util/util.h"

#include "util/data/rbt.h"
#include "util/data/LinkedList.h"
#include "util/data/dict.h"

#include "api.h"

#define LOG_PATH "system/logs/client"

#define DEFAULT_PROGNAME "./client"
#define OPTSTR ":hf:w:W:D:r:R:d:t:l:u:c:p"

#define TIME_BETWEEN_CONN 500

#define NEXT                            \
        if(exp_dir == NULL)             \
            curr = curr->next;          \
        else curr = curr->next->next;   \

typedef enum {
    ACT_TIME,
    ACT_WRITE_DIR, //-w
    ACT_WRITE, //-W
    ACT_READ, //r
    ACT_READ_N, //R
    ACT_LOCK, //l
    ACT_UNLOCK, //u
    ACT_REMOVE, //c
    ACT_D,
    ACT_d
} action_c;

typedef enum {
    DIREC,
    N_WRITE,
    N_READ,
    FILES,
} parsing_o;

//**____________________________________________________  PARSING FUNCTION  ____________________________________________________ **//

typedef struct {
  char * sock;
  bool print;
  long time;
} options_client;

extern options_client opt_c;
extern LinkedList* operation;

/**
 * @brief Function to parsing command-line options
 * 
 * @param argc 
 * @param argv 
 * @return -1 on error, 0 otherwise
 */
int parsing(int argc, char *argv[]);

/**
 * Check if d or D is setted right
 * 
 * @return -1 on error, 0 otherwise
 */
int check_everything_right();

/**
 * @brief Prepare the data to execute
 * 
 * @param action 
 * @param parameters 
 * @return LinkedList* List of operations that are valid, on error exits the program 
 */
LinkedList* convert_absolute_path(action_c action, char* parameters);

/**
 * @brief Execute all command in the order in which they were passed
 * 
 * @return 0 on success, on error exits the program 
 */
int execute();

/**
 * @brief Reads recursively the folder and writes N files
 * 
 * @param dirname 
 * @param numFiles 
 * @param exp_dir 
 * @return number of files written
 */
int rec_write_dir(const char* dirname, int* numFiles, const char* exp_dir);

/**
 * @brief compare key and key2
 * 
 * @param key 
 * @param key2 
 * @return true is equal, false otherwise
 */
bool compare_action(void* key, void* key2);

/**
 * @brief compare key and key2
 * 
 * @param key 
 * @param key2 
 * @return true is equal, false otherwise
 */
bool compare_parsing(void* key, void* key2);

//**____________________________________________________  DEBUG FUNCTION  ____________________________________________________ **//
void List_print_act(LinkedList *list);
void List_print_parsing(LinkedList *list);
char * print_action(action_c* act);
char * print_parsing(parsing_o* pars);

#endif