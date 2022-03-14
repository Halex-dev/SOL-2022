#include "client.h"

void usage(char *progname, int opt) {
   printf("A client application for the File Storage System.\n");
   printf("Usage: %s [options]\n", progname?progname:DEFAULT_PROGNAME);
   printf("Possible options:\n");
   printf("\t-h \t\t\tPrints this helper.\n");
   printf("\t-f <sock> \t\tSets socket name to <sock>. \033[0;31m This option must be set once and only once. \033[0m\n");
   printf("\t-w <dir>[,<n>] \tSends the content of the directory <dir> (and its subdirectories) to the server. If <n> is specified, at most <n> files will be sent to the server.\n");
   printf("\t-W <file>[,<files>]\tSends the files passed as arguments to the server.\n");
   printf("\t-D <dir>\t\tWrites into directory <dir> all the files expelled by the server app. \033[0;31m This option must follow one of -w or -W. \033[0m\n");
   printf("\t-r <file>[,<files>]\tReads the files specified in the argument list from the server.\n");
   printf("\t-R[<n>] \t\tReads <n> files from the server. If <n> is not specified, reads all files from the server. \033[0;31m There must be no space bewteen -R and <n>.\033[0m\n");
   printf("\t-d <dir> \t\tWrites into directory <dir> the files read from server. If it not specified, files read from server will be lost. \033[0;31m This option must follow one of -r or -R. \033[0m\n");
   printf("\t-t <time> \t\tSets the waiting time (in milliseconds) between requests. Default is 0.\n"); 
   printf("\t-l <file>[,<files>] \tLocks all the files given in the file list.\n");
   printf("\t-u <file>[,<files>] \tUnlocks all the files given in the file list.\n");
   printf("\t-c <file>[,<files>] \tDeletes from server all the files given in the file list, if they exist.\n");
   printf("\t-p \t\t\tIf set, every operation will be printed to stdout. \033[0;31m This option must be set at most once. \033[0m\n");
   printf("\n");
   List_destroy(operation,FULL);
   exit(EXIT_FAILURE);
}

int parsing(int argc, char *argv[]){
   int opt;

   opterr = 0;
   operation = create_List();
   
   while ((opt = getopt(argc, argv, OPTSTR)) != EOF){
      switch(opt) {
         case 'f':{
            opt_c.sock = absolute_path(optarg);
            break;
         }
         case 'p':{
            opt_c.print = true;
            log_setConsole(LOG_INFO, opt_c.print);
            break;
         }
         case 'w':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_WRITE_DIR;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'W':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_WRITE;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'r':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_READ;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'R':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_READ_N;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 't':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_TIME;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'l':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_LOCK;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'u':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_UNLOCK;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'c':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_REMOVE;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'D':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_D;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case 'd':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_d;
            int size = strlen(optarg);
            char* str = safe_calloc(size, sizeof(char));

            strcpy(str,optarg);
            List_add(operation, action, str);
            break;
         }
         case '?':
         case 'h':
         default:
            usage(argv[0], opt);
            break;
      }
   }

   //Check if d or D is setted right
   if(opt_c.sock == NULL){
      log_error("You must set a valid socket with -f <sock>");
      List_destroy(operation,FULL);
      return -1;
   }

   if(convert_absolute_path() == -1){
      List_destroy(operation,FULL);
      return -1;
   }

   if (check_everything_right() != EXIT_SUCCESS) {
      log_error("You have used unconsciously the option -d or -D. Do -h to help");
      List_destroy(operation,FULL);
      return -1;
   }
   
   return 0;
}


int convert_absolute_path(){
   return 0;
}

int check_everything_right() {

   Node* tmp = (Node*) List_getHead(operation);

   if(tmp == NULL)
      return EXIT_SUCCESS;

   Node* next = tmp->next;

   if(next == NULL && ACT_D != *(tmp->key) && ACT_d != *(tmp->key))
      return EXIT_SUCCESS;

   while(tmp != NULL && next != NULL){

      //-D solo con -w o -W
      if(ACT_D == *(next->key)){
         if(ACT_WRITE != *(tmp->key) && ACT_WRITE_DIR != *(tmp->key))
            return EXIT_FAILURE;
      }

      //-d solo con -r o -R
      if(ACT_d == *(next->key)){
         if(ACT_READ != *(tmp->key) && ACT_READ_N != *(tmp->key))
            return EXIT_FAILURE;
      }

      tmp = next;
      next = tmp->next;
      continue;
   }
   return EXIT_SUCCESS;
}