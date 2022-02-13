# Compilation options
CC 		= gcc
CFLAGS 	+= -std=c99 -Wall -pedantic -g -I./src/includes -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE -DLOG_USE_COLOR -fPIC

# Directories

# ------------------- SYSTEM ------------------- #
LIB_DIR			= ./system/libs
BIN_DIR			= ./system/bin
LOG_DIR_SERVER 	= ./system/logs/server
LOG_DIR_CLIENT 	= ./system/logs/client
SCRIPT_DIR  	= ./system/scripts
STORAGE_DIR 	= ./storage

# ------------------- OTHER ------------------- #
CODE_DIR		= ./src/code
OBJ_DIR			= ./src/objs
INC_DIR			= ./src/includes
TEST1_DIR		= ./test/test1
TEST2_DIR		= ./test/test2
TEST3_DIR		= ./test/test3

# Dynamic linking
DYN_LINK = -L$(LIB_DIR) -Wl,-rpath,$(LIB_DIR)

# 	---------------- Default rule ----------------	#

all : createDir $(BIN_DIR)/server #$(BIN_DIR)/client

# 	---------------- Debug macro -----------------  #

debug : CFLAGS += -D DEBUG
debug : all

#	--------------- Directory utils --------------	#

.PHONY: createDir
createDir:
	@bash $(SCRIPT_DIR)/createDir.sh

#	-------------- Other Library -------------  #
OTHER_SRC := $(wildcard $(CODE_DIR)/other/*.c)
OTHER_OBJ := $(patsubst $(CODE_DIR)/other/%.c, $(OBJ_DIR)/other/%.o, $(OTHER_SRC))
OTHER_INC := $(patsubst $(CODE_DIR)/other/%.c, $(INC_DIR)/other/%.h, $(OTHER_SRC))

$(LIB_DIR)/libhalex.so : $(OTHER_SRC)
	$(CC) $(CFLAGS) -shared $^ -o $@

$(OBJ_DIR)/other/log.o : $(CODE_DIR)/other/log.c $(INC_DIR)/other/log.h
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

$(OBJ_DIR)/other/config.o : $(CODE_DIR)/other/config.c $(INC_DIR)/other/config.h
	$(CC) $(CFLAGS) $< -c -fPIC -o $@

# 	------------------- Server ------------------- 	#

SERVER_SRC := $(wildcard $(CODE_DIR)/server/*.c) $(wildcard $(CODE_DIR)/server/file/*.c)
SERVER_OBJ := $(patsubst $(CODE_DIR)/server/%.c, $(OBJ_DIR)/server/%.o, $(SERVER_SRC))
SERVER_INC := $(INC_DIR)/server.h

SERVER_DEPS := $(LIB_DIR)/libhalex.so #$(LIB_DIR)/libutil.so
SERVER_LIBS := $(DYN_LINK) -lpthread -lhalex #-lutil 

$(BIN_DIR)/server : $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(SERVER_LIBS)

$(OBJ_DIR)/server/file/%.o : $(CODE_DIR)/server/file/%.c $(SERVER_INC) $(SERVER_DEPS)
	$(CC) $(CFLAGS) $< -c -o $@

$(OBJ_DIR)/server/%.o : $(CODE_DIR)/server/%.c $(SERVER_INC) $(SERVER_DEPS)
	$(CC) $(CFLAGS) $< -c -o $@

# 	-------------------------------------------------------------- COMMANDS --------------------------------------------------------------	#

# 	------------------ Cleaning ------------------	#
.PHONY: clean
clean: cleanTest cleanlog cleanStorage
	@echo "Removing object files, libraries, logs, storage and executables..."
	@rm -f vgcore.* 
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(BIN_DIR)/*
	@rm -rf $(LIB_DIR)/*
	@echo "Cleaning complete!"

.PHONY: cleanStorage
cleanStorage:
	@echo "Removing storage files...."
	@rm -f vgcore.* 
	@rm -rf $(STORAGE_DIR)/*
	@echo "Cleaning complete!"

.PHONY: cleanlog
cleanlog:
	@echo "Removing logs files...."
	@rm -f vgcore.* 
	@rm -rf $(LOG_DIR_SERVER)/*
	@rm -rf $(LOG_DIR_CLIENT)/*
	@echo "Cleaning complete!"


.PHONY: cleanTest1
cleanTest1:
	@echo "Removing tests 1 files...."
	@rm -f vgcore.* 
	@rm -rf $(TEST1_DIR)/EXPELLED/*
	@rm -rf $(TEST1_DIR)/READ/*
	@rm -rf $(TEST1_DIR)/READN/*
	@echo "Cleaning complete!"

.PHONY: cleanTest2
cleanTest2:
	@echo "Removing tests 2 files...."
	@rm -f vgcore.* 
	@rm -rf $(TEST2_DIR)/EXPELLED/*
	@rm -rf $(TEST2_DIR)/READ/*
	@rm -rf $(TEST2_DIR)/READN/*
	@rm -rf $(TEST2_DIR)/WRITE/files/*
	@echo "Cleaning complete!"

.PHONY: cleanTest3
cleanTest3:
	@echo "Removing tests 3 files...."
	@rm -f vgcore.* 
	@rm -rf $(TEST3_DIR)/EXPELLED/*
	@rm -rf $(TEST3_DIR)/READ/*
	@rm -rf $(TEST3_DIR)/READN/*
	@rm -rf $(TEST3_DIR)/WRITE/*
	@echo "Cleaning complete!"

.PHONY: cleanTest
cleanTest: cleanTest1 cleanTest2 cleanTest3 cleanlog

# 	------------------ Script ------------------	#

.PHONY: stats
stats:
	@bash $(SCRIPT_DIR)/stats.sh

.PHONY: test1
test1: cleanTest1 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test1.sh

.PHONY: test2
test2: cleanTest2 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test2.sh MFU

test2FIFO: cleanTest2 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test2.sh FIFO
	
test2LFU: cleanTest2 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test2.sh LFU

test2LRU: cleanTest2 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test2.sh LRU

.PHONY: test3
test3: cleanTest3 cleanlog cleanStorage
	@bash $(SCRIPT_DIR)/test3.sh