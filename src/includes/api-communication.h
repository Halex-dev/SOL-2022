// _____________________________________ COMMUNICATION FUNCTION  _____________________________________ //

/**
* REQUESTCLIENT 20-50
*/
typedef enum {
    REQ_DATA = 20,
    REQ_OPEN_FILE = 30,
    REQ_CLOSE_FILE = 31,
    REQ_READ_FILE = 32,
    REQ_WRITE_FILE = 33,
    REQ_READ_N_FILES = 34,
    REQ_LOCK_FILE = 35,
    REQ_UNLOCK_FILE = 36,
    REQ_REMOVE_FILE = 37,
    REQ_APPEND_TO_FILE = 38,
    REQ_NULL = 39
} api_op;

typedef enum {
    /**
     *  GENERIC RESPONSE SERVER -9 - 0
     */
    RES_SUCCESS = 0,
    RES_CLOSE = -1,
    RES_ERROR = -2,
    RES_NULL = -3,
    RES_DATA = -4,
    RES_ERROR_DATA = -5,
    RES_NO_DATA = -6,

    /**
     *  OPEN/CREATING RESPONSE SERVER -19 - -10
     */
    RES_EXIST = -10,
    RES_NOT_EXIST = -11,
    RES_NOT_OPEN = -12,
    RES_DELETE = -13,
    RES_ALREADY_OPEN = -14,

    /**
     *  LOCK/UNLOCK RESPONSE SERVER -29 - -20
     */
    RES_IS_LOCKED = -20,
    RES_NOT_LOCKED = -21,
    RES_NOT_YOU_LOCKED = -22,
    RES_YOU_LOCKED = -23, //Maybe not use

    /**
     *  FILE RESPONSE SERVER -39 - -30
     */
    RES_TOO_BIG = -31,
    RES_NOT_EMPTY = -32,
    RES_EMPTY = -33,

    /**
     *  SERVER COMMUNICATION -39
     */
    RES_SERVER_EMPTY = -40
} api_res;

typedef enum {
    O_CREATE = 1,
    O_LOCK = 2,
    O_ALL = 3,
    O_NULL = 0 
} api_flags;

typedef struct {
    int data_length;
    void* data;
    api_op  operation;
    api_res response;
    api_flags flags;
} api_msg;