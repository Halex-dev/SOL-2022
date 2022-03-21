#include "server.h"

sigset_t sig_mask;
int* sig_handler_pipe = NULL;
pthread_t sig_handler_tid = -1;

void* sig_handler_thread(void* arg){
    int* pipe = (int*)arg;

    log_info("[SIG-HANDLER] Started signal handler thread.");

    while(true){
        int sig;
        int err = sigwait(&sig_mask, &sig);
        if(err != 0){
            errno = err;
            perror("Error in sigwait");
            return NULL;
        }

        switch (sig) {
            // closing server immediately
            case SIGINT:
                log_info("Received signal to force close server.");
                server.socket.mode = CLOSE_SERVER; //CHANGE TO FORCE_CLOSE_SERVER

                // signaling to main thread
                close(pipe[WEND]);
                pipe[WEND] = -1;
                return NULL;
            case SIGQUIT:
                log_info("Received signal to close server.\n");
                server.socket.mode = CLOSE_SERVER;

                // signaling to main thread
                close(pipe[WEND]);
                pipe[WEND] = -1;

                return NULL;
                
            // blocking new connections
            case SIGHUP:
                log_info("Received signal to refuse incoming connections");
                server.socket.mode = REFUSE_CONN; //CHANGE TO REFUSE_CONN

                // signaling to main thread
                close(pipe[WEND]);
                pipe[WEND] = -1;
                return NULL;

            default: ;
        }
    }
    return NULL;
}

int init_sig_handler(){

    sig_handler_pipe = safe_calloc(2, sizeof(int));
    pipe_init(sig_handler_pipe);

    SYSTEM_CALL_EXIT(pipe(sig_handler_pipe),"Error in creating sig_handler_pipe");

    server.socket.fd_max = sig_handler_pipe[REND];
    
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGINT);
    sigaddset(&sig_mask, SIGTERM);
    sigaddset(&sig_mask, SIGQUIT);
    sigaddset(&sig_mask, SIGHUP);
    
    SYSTEM_CALL_EXIT(pthread_sigmask(SIG_BLOCK, &sig_mask, NULL),"Error while trying to ignore SIGPIPE");

    // Ignoring SIGPIPE
    struct sigaction sig_ignore;
    memset(&sig_ignore, 0, sizeof(sig_ignore));
    sig_ignore.sa_handler = SIG_IGN;

    SYSTEM_CALL_EXIT(sigaction(SIGPIPE, &sig_ignore, NULL),"Error while trying to ignore SIGPIPE");
    SYSTEM_CALL_EXIT(pthread_create(&sig_handler_tid, NULL, sig_handler_thread, sig_handler_pipe), "Error to create signal thread");

    return 0;
}

void cleen_handler(shutdown_mode_t flags){

    if(flags == immediate_shutdown ){
        if(sig_handler_tid != -1){
            SYSTEM_CALL_EXIT(pthread_detach(sig_handler_tid), "Error to deatch thread");
            sig_handler_tid = -1;
        }  
    }
    else{
        if(sig_handler_tid != -1){
            SYSTEM_CALL_EXIT(pthread_join(sig_handler_tid, NULL), "Error to join thread");
            sig_handler_tid = -1;
        }  
    }
    
    //Close pipe
    if(sig_handler_pipe != NULL) {
        for(int j = 0; j < 2; j++)
            if(sig_handler_pipe[j] != -1)
                close(sig_handler_pipe[j]);
    }

    if(sig_handler_pipe != NULL) {
        free(sig_handler_pipe);
        sig_handler_pipe = NULL;
    }  
    
}