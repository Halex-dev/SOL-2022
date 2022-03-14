#ifndef _CLIENT_H
#define _CLIENT_H

#include "util/util.h"
#include "util/log.h"

#include "util/data/rbt.h"
#include "util/data/LinkedList.h"
#include "util/data/dict.h"

#include "api.h"

#define LOG_PATH "system/logs/client"

#define DEFAULT_PROGNAME "./client"
#define OPTSTR ":hf:w:W:D:r:R:d:t:l:u:c:p"

typedef enum {
    ACT_TIME,
    ACT_WRITE_DIR,
    ACT_WRITE,
    ACT_READ,
    ACT_READ_N,
    ACT_LOCK,
    ACT_UNLOCK,
    ACT_REMOVE,
    ACT_D,
    ACT_d
} action_c;
//**____________________________________________________  PARSING FUNCTION  ____________________________________________________ **//

typedef struct {
  char * sock;
  bool print;
  int time;
} options_client;

extern options_client opt_c;
extern LinkedList* operation;

int parsing(int argc, char *argv[]);
int check_everything_right();
int convert_absolute_path();

#define TIME_BETWEEN_CONN 500
#endif