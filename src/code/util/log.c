/**
 * Copyright (c) 2022 Halex
 * MIT License
 * 
 */

#include "util/log.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t s_mutex;

/** Log file. */
FILE* log_file;

log_config config ={
    .thread_safe = false,
    .file = false
};

//__________________________________ SETTING LOG _________________________________//

static const char *level_strings[] = {
  "STATS", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  BLU, CYN, GRN, YEL, RED, MAG
};
#endif

bool level_line[] = {
  false, true, false, false, false, false
};

bool level_console[] = {
  true, true, true, true, true, true
};

//__________________________________ MUTEX _________________________________//

void mutex_lock_file(){
    int err;
    if( (err = pthread_mutex_lock(&s_mutex)) != 0){
        errno = err;
        perror("[LOG] Error in pthread_mutex_lock");
        exit(EXIT_FAILURE);
    }
}

void mutex_unlock_file(){
    int err;
    if( (err = pthread_mutex_unlock(&s_mutex)) != 0){
        errno = err;
        perror("[LOG] Error in pthread_mutex_unlock");
        exit(EXIT_FAILURE);
    }
}

//__________________________________ FUNCTION _________________________________//

void* log_malloc(size_t size){
    void* buf;
    if( (buf = malloc(size)) == NULL){
        perror("[LOG] Error in memory allocation (malloc)");
        exit(EXIT_FAILURE);
    }
    return buf;
}

void log_threadsafe(bool value){
    mutex_lock_file();
    config.thread_safe = value;
    mutex_unlock_file();
}

void log_setColor(int level, const char* color){

    #ifdef LOG_USE_COLOR
    if( level > SIZEOF_ARRAY(level_colors)){
            fprintf(stderr, "[LOG] Fatal error, This log level does not exist.\n");
            return;
    }
    level_colors[level] = color;
    #endif
    
}

void log_setConsole(int level, bool cond){

    if( level > SIZEOF_ARRAY(level_strings)){
            fprintf(stderr, "[LOG] Fatal error, This log level does not exist.\n");
            return;
    }

    level_console[level] = cond;
    
}

void log_setWriteLine(int level, bool cond){

    if( level > SIZEOF_ARRAY(level_strings)){
            fprintf(stderr, "[LOG] Fatal error, This log level does not exist.\n");
            return;
    }

    level_line[level] = cond;
}

void init_log_file(const char* path, const char* mode){
    //Init the mutex, even if i don't use it
    if (pthread_mutex_init(&s_mutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        exit(EXIT_FAILURE);
    }

    struct tm curr_time;
    time_t curr_time_abs = time(NULL);
    localtime_r(&curr_time_abs, &curr_time);

    char* log_file_path;
    int len_path = strlen(path);

    log_file_path = log_malloc((len_path + 30) * sizeof(char));
    memset(log_file_path, '\0', sizeof(char)*(len_path + 30));

    snprintf(
        log_file_path, len_path + 30,
        "%s/log-%4d-%02d-%02d.txt", 
        path,
        curr_time.tm_year + 1900,
        curr_time.tm_mon + 1,
        curr_time.tm_mday
    );

    if((log_file = fopen(log_file_path, mode)) == NULL){
        fprintf(stderr, "\x1b[41m [LOG] Fatal error in creating or open log file. Aborting.\x1B[0m\n");
        exit(EXIT_FAILURE);
    }
    config.file = true;
    free(log_file_path);
}

static void stdout_callback(log_event *ev, int level) {

    if( level > SIZEOF_ARRAY(level_strings)){
            fprintf(stderr, "[LOG] Fatal error, This log level does not exist.\n");
            return;
    }

    char buf[32];
    time_t t = time(NULL);

    buf[strftime(buf, sizeof(buf), "%d-%m-%Y %H:%M:%S", localtime(&t))] = '\0';

    #ifdef LOG_USE_COLOR

    //ON TERMINAL BUT WHIT COLOR
    if(level_console[level]){
        if(level_line[level])
            fprintf(ev->udata, "[%s%s\x1b[0m] [%s] [%s:%d:] ",level_colors[level],level_strings[level], buf, ev->file, ev->line);
        else
            fprintf(ev->udata, "[%s%s\x1b[0m] [%s] ",level_colors[level],level_strings[level], buf);
    }

    #else

    //ON TERMINAL
    if(level_console[level]){
        if(level_line[level])
            fprintf(ev->udata, "[%s] [%s] [%s:%d:] ",level_strings[level], buf, ev->file, ev->line);
        else
            fprintf(ev->udata, "[%s] [%s] ",level_strings[level], buf);
    }

    #endif

    
    //ON FILE
    if(config.file){
        if(level_line[level])
            fprintf(log_file, "[%s] [%s] [%s:%d:] ",level_strings[level], buf, ev->file, ev->line);
        else
            fprintf(log_file, "[%s] [%s] ",level_strings[level], buf);
    }

    va_list aq;

    va_copy(aq, ev->ap);

    if(level_console[level]){
        vfprintf(ev->udata, ev->fmt, ev->ap); //write on console
        fprintf(ev->udata, "\n");
        fflush(ev->udata);
    }

    //Write on file
    if(config.file){
        vfprintf(log_file, ev->fmt, aq); //write on file
        fprintf(log_file, "\n");
        fflush(log_file);
    }
}

void mylog(int level, const char *file, int line, const char *fmt, ...) {
        
    log_event ev = {
        .fmt   = fmt,
        .file  = file,
        .line  = line,
        .udata = stderr
    };

    va_start(ev.ap, fmt);

    if(config.thread_safe){
        mutex_lock_file();
        stdout_callback(&ev, level);
        mutex_unlock_file();
    }
    else
    stdout_callback(&ev, level);

    va_end(ev.ap);
}

void close_logger(){
    pthread_mutex_destroy(&s_mutex);
    fclose(log_file);
}