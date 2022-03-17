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
   bool socket = false;

   opterr = 0;
   operation = create_List(compare_action);
   
   while ((opt = getopt(argc, argv, OPTSTR)) != EOF){
      switch(opt) {
         case 'f':{ //Socket

            if(socket){
               log_error("You can set one socket");
               exit(EXIT_FAILURE);
            }

            opt_c.sock = absolute_path(optarg);
            if(opt_c.sock == NULL){
               log_error("Error to convert path '%s' into absolute", optarg);
               exit(EXIT_FAILURE);
            }
            socket = true;
            break;
         }
         case 'p':{ //Printable mode
            opt_c.print = true;
            break;
         }
         case 'w':{ // Dirname, n file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_WRITE_DIR;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'W':{ //List of file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_WRITE;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'r':{ //List of file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_READ;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'R':{ //Num of read
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_READ_N;

            long* n = safe_calloc(1, sizeof(long));

            if(optarg != NULL) {
               *n = atoi(optarg);
            }

            List_add(operation, action, (void*) n);
            break;
         }
         case 'l':{ //List of file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_LOCK;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'u':{ //List of file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_UNLOCK;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'c':{ //List of file
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_REMOVE;

            LinkedList* act = convert_absolute_path(*action, optarg);
            List_add(operation, action, (void*) act);
            break;
         }
         case 'D':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_D;

            char * dirPath = absolute_path(optarg);
            if(dirPath == NULL){
               log_error("Error to convert path '%s' into absolute", optarg);
               exit(EXIT_FAILURE);
            }

            List_add(operation, action, (void*) dirPath);
            break;
         }
         case 'd':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_d;

            char * dirPath = absolute_path(optarg);
            if(dirPath == NULL){
               log_error("Error to convert path '%s' into absolute", optarg);
               exit(EXIT_FAILURE);
            }

            List_add(operation, action, (void*) dirPath);
            break;
         }
         case 't':{
            action_c* action = safe_calloc(1, sizeof(action_c));
            *action = ACT_TIME;

            long* files = safe_calloc(1, sizeof(long));
            if(optarg != NULL && (sscanf(optarg, "%ld", files) == EOF || *files < 0)){
               log_error("Error in parsing option -t: couldn't read number of files.\n");
               exit(EXIT_FAILURE);
            }

            List_add(operation, action, (void*) files);
            break;
         }
         case '?':
         case 'h':
         default:
            usage(argv[0], opt);
            break;
      }
   }

   //Check socket
   if(opt_c.sock == NULL){
      log_error("You must set a valid socket with -f <sock>");
      List_destroy(operation,FULL);
      return -1;
   }

   //Check if d or D is setted right
   if (check_everything_right() != EXIT_SUCCESS) {
      log_error("You have used unconsciously the option -d or -D. Do -h to help");
      List_destroy(operation,FULL);
      return -1;
   }
   
   return 0;
}


LinkedList* convert_absolute_path(action_c action, char* parameters){

   char* p;
   LinkedList* act = create_List(compare_parsing);

   switch(action) {
      case ACT_WRITE_DIR: { //-w

         char* dirPath;
         long* files;

         //Take dir and get absolute
         dirPath = strtok_r(parameters, ",", &p);
         dirPath = absolute_path(dirPath);

         if(dirPath == NULL){
            log_error("Error to convert path '%s' into absolute", optarg);
            exit(EXIT_FAILURE);
         }

         struct stat properties;

         if(stat(dirPath, &properties) == -1){
            perror("\tError in stat");
            exit(EXIT_FAILURE);
         }

         //Check if dir exist
         if(S_ISDIR(properties.st_mode)) {
            log_error("Folder specified  '%s' does not exist.", dirPath); 
            List_destroy(act,FULL);
            exit(EXIT_FAILURE);
         } 
         
         
         parsing_o* par = safe_calloc(1, sizeof(parsing_o));
         *par = DIREC;
         List_add(act, par, (void*) dirPath);

         //Check get number files
         files = safe_calloc(1, sizeof(long));
         *files = 0;
         
         char* num = strtok_r(NULL, ",", &p);
         if(num != NULL && (sscanf(num, "%ld", files) == EOF || *files < 0)){
            log_error("Error in parsing option -w: couldn't read number of files.\n");
            List_destroy(act,FULL);
            exit(EXIT_FAILURE);;
         }

         parsing_o* par2 = safe_calloc(1, sizeof(parsing_o));
         *par2 = N_WRITE;

         List_add(act, par2, (void*) files);
         return act;
         break;
      }
      case ACT_WRITE: { //-W

         char* token = strtok_r(parameters, ",", &p);

         if(token == NULL){
            List_destroy(act,FULL);
            return NULL;
         }
            
         while(token) {
            char* filePath = absolute_path(token);
            // converting token into absolute path if necessary
            if(filePath == NULL){
               log_error("Error to convert path '%s' into absolute", token);
               exit(EXIT_FAILURE);
            }

            parsing_o* par = safe_calloc(1, sizeof(action_c));
            *par = FILES;

            struct stat properties;

            if(stat(filePath, &properties) == -1){
               perror("\tError in stat");
               exit(EXIT_FAILURE);
            }

            if(S_ISDIR(properties.st_mode)) {
               log_error("You can only send files on option -W");
               exit(EXIT_FAILURE);
            } 

            List_add(act,par, (void*) filePath);
            token = strtok_r(NULL, ",", &p);
         }

         return act;
         break;
      }
      case ACT_READ: { //r
         char* token = strtok_r(parameters, ",", &p);

         if(token == NULL){
            List_destroy(act,FULL);
            return NULL;
         }
            
         while(token) {
            char* filePath = absolute_path(token);
            // converting token into absolute path if necessary
            if(filePath == NULL){
               log_error("Error to convert path '%s' into absolute", token);
               exit(EXIT_FAILURE);
            }

            parsing_o* par = safe_calloc(1, sizeof(action_c));
            *par = FILES;

            List_add(act,par, (void*) filePath);
            token = strtok_r(NULL, ",", &p);
         }

         return act;
         break;
      }
      case ACT_LOCK: { //l
         char* token = strtok_r(parameters, ",", &p);

         if(token == NULL){
            List_destroy(act,FULL);
            return NULL;
         }
            
         while(token) {
            char* filePath = absolute_path(token);
            // converting token into absolute path if necessary
            if(filePath == NULL){
               log_error("Error to convert path '%s' into absolute", token);
               exit(EXIT_FAILURE);
            }

            parsing_o* par = safe_calloc(1, sizeof(action_c));
            *par = FILES;

            List_add(act,par, (void*) filePath);
            token = strtok_r(NULL, ",", &p);
         }

         return act;
         break;
      }
      case ACT_UNLOCK: { //u
         char* token = strtok_r(parameters, ",", &p);

         if(token == NULL){
            List_destroy(act,FULL);
            return NULL;
         }
            
         while(token) {
            char* filePath = absolute_path(token);
            // converting token into absolute path if necessary
            if(filePath == NULL){
               log_error("Error to convert path '%s' into absolute", token);
               exit(EXIT_FAILURE);
            }

            parsing_o* par = safe_calloc(1, sizeof(action_c));
            *par = FILES;

            List_add(act,par, (void*) filePath);
            token = strtok_r(NULL, ",", &p);
         }

         return act;
         break;
      }
      case ACT_REMOVE: { //c
         char* token = strtok_r(parameters, ",", &p);

         if(token == NULL){
            List_destroy(act,FULL);
            return NULL;
         }
            
         while(token) {
            char* filePath = absolute_path(token);
            // converting token into absolute path if necessary
            if(filePath == NULL){
               log_error("Error to convert path '%s' into absolute", token);
               exit(EXIT_FAILURE);
            }

            parsing_o* par = safe_calloc(1, sizeof(action_c));
            *par = FILES;

            List_add(act,par, (void*)filePath);
            token = strtok_r(NULL, ",", &p);
         }

         return act;
         break;
      }
      default: {
         return NULL;
         break;
      }
   }
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