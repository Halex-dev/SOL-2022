#include "client.h"

#define SOCKETNAME  "./system/bin/SOL-SOCKET.sk"

int main(int argc, char* argv[]){ 
    
    /**if(argc < 2) {
        log_error("You must provide at least one parameter. Use -h to show all command\n");
        return -1;
    }*/

    init_log_file(LOG_PATH, WRITE);

    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    if(openConnection(SOCKETNAME, TIME_BETWEEN_CONN, abstime) == -1){
        perror("Connection failed");
        return -1;
    } 

    setCurrent(SOCKETNAME);
    
    //nsleep(2000);

    openFile("home/test/ciaone", O_ALL);

    //nsleep(2000);

    // ------- CLOSING CONNECTION ------ //
    if(closeConnection(SOCKETNAME) == -1){
        perror("Couldn't close connection");
        return -1;
    }

    //LOG
    close_logger();
    return 0;
}