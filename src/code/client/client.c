#include "client.h"

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
    log_setConsole(LOG_INFO, opt_c.print);
    log_setConsole(LOG_ERROR, opt_c.print);
    log_setConsole(LOG_WARN, opt_c.print);

    //log_setColor(LOG_INFO, GRN_BACK);
    //log_setColor(LOG_ERROR, RED_BACK);
    //log_setColor(LOG_WARN, YEL_BACK);

    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);

    if(openConnection(opt_c.sock, TIME_BETWEEN_CONN, abstime) == -1){
        perror("Connection failed");
        return -1;
    } 
    setCurrent(opt_c.sock);
    
    execute();

    // ------- CLOSING CONNECTION ------ //
    if(closeConnection(opt_c.sock) == -1){
        perror("Couldn't close connection");
        return -1;
    }

    free(opt_c.sock);
    List_destroy(operation,KEY);

    //LOG
    close_logger();
    return 0;
}