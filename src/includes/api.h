#ifndef _SERVER_API_H
#define _SERVER_API_H

#include "util/util.h"
#include "util/log.h"

#include "util/data/rbt.h"
#include "util/data/hashmap.h"

#include "util/data/dict.h"
#include "util/data/LinkedList.h"
#include "util/data/node.h"

#define SYSTEM_CALL0_EXIT(sys_call, str) if(sys_call != 0) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }

#define SYSTEM_CALL0(sys_call, str) if(sys_call != 0) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        return -1; \
    }

#define SYSTEM_CALL_EXIT(sys_call, str) if(sys_call == -1) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }

#define SYSTEM_CALL(sys_call, str) if(sys_call == -1) {\
        log_error("%s %s (codice %d)\n", str, strerror(errno), errno); \
        exit(EXIT_FAILURE); \
    }


// _____________________________________ PROJECT FUNCTION  _____________________________________ //
/**
 * Takes a socket name, a time in milliseconds and an absolute time.
 * Tries to connect to sockname every msec milliseconds and gives up
 * abstime is reached.
 * Returns 0 on success and -1 on error, setting errno.
 */
int openConnection(const char* sockname, int msec, const struct timespec abstime);

/** 
 * Takes a socket name and closes the connection.
 * Returns 0 on success and -1 on error, setting errno.
 */
int closeConnection(const char* sockname);

/**
 * Given a file name and flags sends a request to open the given file
 * with the given flags.
 * Returns 0 on success and -1 on error, setting errno.
 */
int openFile(const char* pathname, int flags);

/**
 * Given a file name, a pointer to a buffer and a pointer to a size variable
 * sends a request to read the contents of the given file and writes them into
 * the buffer.
 * Returns 0 on success and -1 on error, setting errno.
 */
int readFile(const char* pathname, void** buf, size_t* size);

/**
 * Takes an integer N and a directory name: if N is greater than 0 
 * reads N files from server and writes them into dirname, whereas if
 * N is 0 reads all files from server and writes them into dirname.
 * If dirname == NULL files read are destroyed.
 * Returns 0 on success and -1 on error, setting errno.
 */
int readNFiles(int N, const char* dirname);

/**
 * Takes a pathname for a file and a directory name and writes 
 * the contents of file pathname into the (already created) file
 * in the server. If the server expells some files as a result of
 * the replacement algorithm, those files are written in dirname.
 * If dirname == NULL the expelled files are destroyed.
 * Returns 0 on success, -1 on error and sets errno.
 */
int writeFile(const char* pathname, const char* dirname);

/**
 * Given a file name, a buffer and its size and a directory name,
 * appends the contents of the buffer to the file with the given filename
 * in the server. If the server expells some files as a result of
 * the replacement algorithm, those files are written in dirname.
 * If dirname == NULL the expelled files are destroyed.
 * Returns 0 on success, -1 on error and sets errno.
 */
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

/**
 * Given a pathname, closes the file.
 * Returns 0 on success (also when the file was not opened by client) 
 * and if pathname was locked, removes the lock.
 * On error returns -1 and sets errno.
 */
int closeFile(const char* pathname);

/**
 * Given a pathname, removes the given file from the server.
 * This file must be locked by the client before trying to remove it.
 * On success returns 0, on error -1 and sets errno.
 */
int removeFile(const char* pathname);

/**
 * Given a pathname, Try to lock the file on server.
 * On success returns 0, on error -1 and sets errno.
 */
int lockFile(const char* pathname);

/**
 * Given a pathname, Try to unlock the file on server.
 * On success returns 0, on error -1 and sets errno.
 */
int unlockFile(const char* pathname);

//extern Dict* socket_m;

// _____________________________________ SOCKET FUNCTION  _____________________________________ //

typedef struct SocketConnection{
    int fd;
    char* name;
    int failed;
    int success;
} SocketConnection;

extern SocketConnection* current_socket; //for operation
extern Dict* socket_m;

/**
 * Given a socket and reset it.
 */
void resetSocket(SocketConnection* socket);

/**
 * Given a socket return if it is connected.
 * Return true if connected, false otherwise
 */
bool isConnect(SocketConnection* socket);

/**
 * Given a socketname add it for future operations.
 * Return the create SocketConnection, on error return NULL
 */
SocketConnection* addSocket(char* key);

/**
 * Given a socketname add it for future operations.
 * Return the SocketConnection if exist, NULL otherwise
 */
SocketConnection* getSocket(char* key);

/**
 * Remove socket if not connected.
 */
bool removeSocket(char* key);

/**
 * Function to set the actual socket for operation.
 */
void setCurrent(char* key);
#endif