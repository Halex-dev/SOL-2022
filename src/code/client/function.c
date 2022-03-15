#include "client.h"

void List_print_parsing(LinkedList *list) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* tmp = list->head;

    while(tmp != NULL){  
        parsing_o* key = (parsing_o*)tmp->key;

        if(*key == N_WRITE || *key == N_READ){
            long* num = (long *) tmp->data;
            printf("- Key: %s -Data: %ld\n", print_parsing(key), *num);
        }  
        else
            printf("- Key: %s -Data: %s\n", print_parsing(key), (char *) tmp->data);

        tmp = tmp->next;
    }
}


void List_print_act(LinkedList *list) {
    if (list == NULL){
        errno = EINVAL;
        return;
    }

    Node* tmp = list->head;

    while(tmp != NULL){  
        printf("- Key: %s\n", print_action((action_c*) tmp->key));
        List_print_parsing((LinkedList *)tmp->data);
        tmp = tmp->next;
    }
}

bool compare_action(void* key, void* key2){
    action_c* tmp1 = (action_c*) key;
    action_c* tmp2 = (action_c*) key2;

    return tmp1 == tmp2;
}

bool compare_parsing(void* key, void* key2){
    parsing_o* tmp1 = (parsing_o*) key;
    parsing_o* tmp2 = (parsing_o*) key2;
    
    return *tmp1 == *tmp2;
}

char * print_action(action_c* act){
    switch (*act){
        case ACT_TIME:
            return "ACT_TIME";
            break;
        case ACT_WRITE_DIR:
            return "ACT_WRITE_DIR";
            break;
        case ACT_WRITE:
            return "ACT_WRITE";
            break;
        case ACT_READ:
            return "ACT_READ";
            break;
        case ACT_READ_N:
            return "ACT_READ_N";
            break;
        case ACT_LOCK:
            return "ACT_LOCK";
            break;
        case ACT_UNLOCK:
            return "ACT_UNLOCK";
            break;
        case ACT_REMOVE:
            return "ACT_REMOVE";
            break;
        case ACT_D:
            return "ACT_D";
            break;   
        case ACT_d:
            return "ACT_d";
            break;   
        default:
            return "IDK";
            break;
    }
}

char * print_parsing(parsing_o* pars){
    switch (*pars){
        case DIREC:
            return "DIREC";
            break;
        case N_WRITE:
            return "N_WRITE";
            break;
        case N_READ:
            return "N_READ";
            break;
        case FILES:
            return "FILES";
            break;
        default:
            return "IDK";
            break;
    }
}