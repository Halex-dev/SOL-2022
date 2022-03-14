#include "client.h"

#define SOCKETNAME  "./system/bin/SOL-SOCKET.sk"

LinkedList* operation = NULL;
options_client opt_c = {
    .sock = NULL,
    .time = 0,
    .print = false
};

int main(int argc, char* argv[]){ 
    
    if(argc < 2) {
        log_error("You must provide at least one parameter. Use -h to show all command");
        return -1;
    }

    if(parsing(argc, argv) == -1){
        return -1;
    }

    init_log_file(LOG_PATH, WRITE);

    /*
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    if(openConnection(SOCKETNAME, TIME_BETWEEN_CONN, abstime) == -1){
        perror("Connection failed");
        return -1;
    } 

    setCurrent(SOCKETNAME);
    
    //nsleep(2000);

    openFile("./test.txt", O_ALL);

    writeFile("./test.txt", NULL);

    //nsleep(2000);

    closeFile("./test.txt");

    // ------- CLOSING CONNECTION ------ //
    if(closeConnection(SOCKETNAME) == -1){
        perror("Couldn't close connection");
        return -1;
    }*/

    List_destroy(operation,FULL);

    //LOG
    close_logger();
    return 0;
}