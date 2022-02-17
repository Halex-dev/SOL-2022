#include "util/config.h"

config_file* init_config(const char* path){
    FILE* fp;
    config_file* cf = NULL;

    if ((fp = fopen(path, "r+")) == NULL) {
        perror("[CONFIG] fopen()");
        exit(EXIT_FAILURE);
    }

    if ((cf = calloc(1, sizeof(config_file))) == NULL){
        perror("[CONFIG] calloc()");
        exit(EXIT_FAILURE);
    }   

    cf->size = 0;
    cf->head = NULL;
    cf->tail = NULL;

    while(1) {
        List_c* el = NULL;
        if ((el = calloc(1, sizeof(List_c))) == NULL){
            perror("[CONFIG] calloc()");
            exit(EXIT_FAILURE);
        }
            
        memset(el, 0, sizeof(List_c));

        if (fscanf(fp, "%s = %s", el->key, el->data) != 2) { //Read node
            if (feof(fp)) {
                free(el);
                break;
            }
            if (el->key[0] == '#') { //Key can't be like a comment
                while (fgetc(fp) != '\n') {
                    // Do nothing (to move the cursor to the end of the line).
                }
                free(el);
                continue;
            }
            //perror("[CONFIG] fscanf()");
            
            free(el);
            continue;
        }     

        if(cf->size == 0){ //Empty config node
            cf->head = el;
            cf->tail = el;
        }
        else{
            cf->tail->next = el;
            cf->tail = el;
        }
        cf->size++;
    }

    fclose(fp);
    return cf;
}

void printConfig(config_file* cf){

    if(cf == NULL)
        return;

    List_c* el = cf->head;

    while(el != NULL){
        printf("- Key: %s Value: %s\n", el->key, el->data);
        el = el->next;
    }
}

void free_config(config_file* cf){

    if(cf == NULL)
        return;

    List_c* el = cf->head;

    if(el == NULL)
        return;
    
    while(el != NULL){
        List_c* tmp = el;
        el = el->next;
        free(tmp);
    }
    
    free(cf);
}