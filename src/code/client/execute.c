#include "client.h"

int execute(){

    //debugging function for building functions, do not use or it blows up everything
    //List_print_act(operation);

    Node* curr = (Node*) List_getHead(operation);

    while(curr != NULL){
        
        if(nsleep(opt_c.time) == -1){
            log_error("Something went wrong, try again.");
            exit(EXIT_FAILURE);
        }

        action_c* act = (action_c*) curr->key;

        switch (*act){
            case ACT_TIME:{
                char* exp_dir = NULL;
                long* num = curr->data;
                opt_c.time = *num;
                log_info("Change time execute to %d", opt_c.time);
                free(num);
                NEXT;
                break;
            }
            case ACT_WRITE_DIR:{ //w
                char* exp_dir = NULL;
                action_c* opt_d = NULL;

                if(curr->next != NULL)
                    opt_d = (action_c*) curr->next->key; 

                //Get expelled dir if exist
                if(opt_d != NULL && *opt_d == ACT_D){
                    exp_dir = (char*)curr->next->data;
                    log_info("Writing expelled files in folder %s (option -D was set).", exp_dir);
                } 

                //Get dir for write and num file
                LinkedList* list = curr->data;
                
                parsing_o find;
                find = DIREC;
                char* dirPath = (char*) List_get(list, (void*) &find);

                find = N_WRITE;
                long* num = (long*) List_get(list, (void*) &find);
                int files = (int) *num;

                //setting -1 to send all file
                if(files == 0)
                    files = -1;

                int writen;
                if((writen = rec_write_dir(dirPath, &files, exp_dir)) == -1){
                    log_error("Could not open directory %s: %s.", dirPath, strerror(errno));
                }
                
                if(exp_dir != NULL)
                    free(exp_dir);

                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_WRITE:{
                char* exp_dir = NULL;
                action_c* opt_d = NULL;

                if(curr->next != NULL)
                    opt_d = (action_c*) curr->next->key;

                //Get expelled dir if exist
                if(curr->next != NULL && *opt_d == ACT_D){
                    exp_dir = (char*)curr->next->data;
                    log_info("Writing expelled files in folder %s (option -D was set).", exp_dir);
                } 

                //Get dir for write and num file
                LinkedList* list = curr->data;
                Node* files = List_getHead(list);

                while(files != NULL){
                    
                    parsing_o* prs = (parsing_o *) files->key;
                    char* file = files->data;

                    if(*prs != FILES)
                        log_error("This was not supposed to happen. Contact a programmer.");


                    //TODO APPEND CON errno
                    log_info("Write file %s on server", file);

                    if(openFile(file, O_ALL) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    if(writeFile(file, exp_dir) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    } 

                    if(closeFile(file) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    log_info("File %s has been written to the server", file);

                    files = files->next;
                }

                //if -D exist delete exp_dir
                if(exp_dir != NULL)
                    free(exp_dir);
                    
                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_REMOVE:{
                //log_debug("REMOVE");
                char* exp_dir = NULL;

                //Get dir for write and num file
                LinkedList* list = curr->data;
                Node* files = List_getHead(list);

                while(files != NULL){
                    
                    parsing_o* prs = (parsing_o *) files->key;
                    char* file = files->data;

                    if(*prs != FILES)
                        log_error("This was not supposed to happen. Contact a programmer.");

                    log_info("Remove file %s on server", file);

                    if(openFile(file, O_LOCK) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    if(removeFile(file) == -1){
                        log_warn("\t\\-----> REMOVE FAILED, close file (%s)", strerror(errno));

                        if(closeFile(file) == -1){
                            files=files->next;
                            continue;
                        }        
                        continue;
                    } 

                    log_info("File %s has been removed from the server ", file);

                    files = files->next;
                }

                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_LOCK:{
                //log_debug("REMOVE");
                char* exp_dir = NULL;

                //Get dir for write and num file
                LinkedList* list = curr->data;
                Node* files = List_getHead(list);

                while(files != NULL){
                    
                    parsing_o* prs = (parsing_o *) files->key;
                    char* file = files->data;

                    if(*prs != FILES)
                        log_error("This was not supposed to happen. Contact a programmer.");

                    log_info("Lock file %s on server", file);

                    if(lockFile(file) == -1){
                        log_warn("\t\\-----> OPEN Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    log_info("File %s has been locked from the server", file);

                    files = files->next;
                }

                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_UNLOCK:{
                char* exp_dir = NULL;

                //Get dir for write and num file
                LinkedList* list = curr->data;
                Node* files = List_getHead(list);

                while(files != NULL){
                    
                    parsing_o* prs = (parsing_o *) files->key;
                    char* file = files->data;

                    if(*prs != FILES)
                        log_error("This was not supposed to happen. Contact a programmer.");

                    log_info("Lock file %s on server", file);

                    if(unlockFile(file) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    log_info("File %s has been unlocked from the server", file);

                    files = files->next;
                }

                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_READ:{
                //log_debug("READ");

                char* exp_dir = NULL;
                action_c* opt_d = NULL;

                if(curr->next != NULL)
                    opt_d = (action_c*) curr->next->key; 

                //Get expelled dir if exist
                if(opt_d != NULL && *opt_d == ACT_d){
                    exp_dir = (char*)curr->next->data;
                    log_info("Writing expelled files in folder %s (option -D was set).", exp_dir);
                } 

                //Get dir for read and num file
                LinkedList* list = curr->data;
                Node* files = List_getHead(list);

                while(files != NULL){
                    
                    parsing_o* prs = (parsing_o *) files->key;
                    char* file = files->data;

                    if(*prs != FILES)
                        log_error("This was not supposed to happen. Contact a programmer.");

                    log_info("Read file %s from server", file);

                    if(openFile(file, O_NULL) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    void* buff = NULL;
                    size_t size;

                    if(readFile(file, &buff, &size) == -1){
                        log_warn("\t\\-----> Go to next file read (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    } 

                    char* fileName = basename(file);

                    if(exp_dir != NULL){
                       
                        int sizePath = strlen(exp_dir)+strlen(fileName)+2; //1 size for '/'
                        char* path = safe_calloc(sizePath,sizeof(char*));
                        strcat(path, exp_dir);
                        strcat(path, "/");
                        strcat(path, fileName);

                        if(file_write(path, buff, size) == -1){
                            log_error("An error occurred while writing %s: %s", fileName, strerror(errno));
                        }

                        log_info("Read file %s in the server and writed it in %s", fileName, path);
                        free(path);
                        free(exp_dir);
                    }
                    free(buff);

                    log_info("Read file %s in the server", fileName);

                    if(closeFile(file) == -1){
                        log_warn("\t\\-----> Go to next file (%s)", strerror(errno));
                        files=files->next;
                        continue;
                    }

                    files = files->next;
                }

                List_destroy(list, FULL);
                NEXT;
                break;
            }
            case ACT_READ_N:{
                //log_debug("READ_N");

                char* exp_dir = NULL;
                action_c* opt_d = NULL;

                if(curr->next != NULL)
                    opt_d = (action_c*) curr->next->key; 

                //Get expelled dir if exist
                if(opt_d != NULL && *opt_d == ACT_d){
                    exp_dir = (char*)curr->next->data;
                    log_info("Writing expelled files in folder %s (option -D was set).", exp_dir);
                } 

                //Get num file
                long files = *((long*)curr->data);

                //setting -1 to send all file
                if(files <= 0)
                    files = -1;

                int readed;

                if((readed = readNFiles(files, exp_dir)) == -1){
                    log_error("Error on read %d files on server: %s.",files, strerror(errno));
                }

                if(exp_dir != NULL)
                    free(exp_dir);

                if(curr->data != NULL)
                    free(curr->data);
                    
                NEXT;
                break;
            }
            default:{
                char* exp_dir = NULL;
                NEXT;
                break;
            }
        }
    }

    return 0;
}


int rec_write_dir(const char* dirname, int* numFiles, const char* exp_dir){

    int write = *numFiles;
    int tot = write;

    if(write < -1)
        return -1;

    DIR* folder;
    struct dirent *entry;

    if((folder = opendir(dirname)) == NULL){
        return -1;
    }

    while((entry = readdir(folder)) != NULL && (write > 0 || write == -1)) {
        char path[PATH_MAX];
        if(dirname[strlen(dirname)-1] != '/')
            snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);
        else
            snprintf(path, sizeof(path), "%s%s", dirname, entry->d_name);
        
        struct stat properties;

        if( stat(path, &properties) == -1){
            perror("\tError in stat");
            return -1;
        }

        if(S_ISDIR(properties.st_mode)) {
            // skipping . and ..
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            //Recall recursive fucntion with updated numfile
            rec_write_dir(path, &write, exp_dir);
        } 
        else { // It's a file

            log_info("Write file %s on server", path);

            if(write != -1)
                write--;

            if(openFile(path, O_ALL) == -1){
                log_warn("\t\\-----> Go to next file");
                continue;
            }

            if(writeFile(path, exp_dir) == -1){
                log_warn("\t\\-----> Go to next file");
                continue;
            } 

            if(closeFile(path) == -1){
                log_warn("\t\\-----> Go to next file");
                continue;
            }

            log_info("File %s has been written to the server", path);
            /** I don't count files with error
             * if(write != -1)
             *   write--;
            */
            
        }
    }

    closedir(folder);

    return tot-write;
}