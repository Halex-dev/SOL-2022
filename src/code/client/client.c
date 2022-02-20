#include "client.h"

int main(int argc, char* argv[]){ 
    
    /**if(argc < 2) {
        log_error("You must provide at least one parameter. Use -h to show all command\n");
        return -1;
    }*/

    init_log_file(LOG_PATH, WRITE);

    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    if(openConnection("./system/bin/SOL-SOCKET.sk", TIME_BETWEEN_CONN, abstime) == -1){
        perror("Connection failed");
        return -1;
    } 

    usleep(5000);

    // ------- CLOSING CONNECTION ------ //
    if(closeConnection("./system/bin/SOL-SOCKET.sk") == -1){
        perror("Couldn't close connection");
        return -1;
    }

    //LOG
    close_logger();
    return 0;
}