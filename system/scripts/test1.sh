#!/bin/bash

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
# x         FILE DI TEST 1          x
# x                                 x
# x Autore: Alessio Vito D'angelo   x
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

clear
SOCK_NAME=./system/bin/SOL-2022.sk
CONFIG=./system/config/test1.txt

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./system/bin/server 
CLIENT=./system/bin/client

# Preparo il file di configurazione
touch ${CONFIG}
echo -e "WORKERS = 1\nSOCK_PATH = ${SOCK_NAME}\n\
MAX_FILES = 10000\nMAX_SPACE = 128\nLOG_PATH = system/logs/server\n\
POLICY = MFU\nDEBUG = no\nSTORAGE = HASH" > ${CONFIG}

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
valgrind --leak-check=full ${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 3 # just to make valgrind print stuff

echo -e "${GREEN}\n\t[CLIENT 0] Prints helper message (-h).${NORMC}\n";
${CLIENT} -h
sleep 2 # wait 2 sec before execute other command

echo -e "${GREEN}\n\t[CLIENT 1] Tests -W and -c. (Upload two file and after delete one of this) ${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -W test/test1/WRITE/help.txt,test/test1/WRITE/help2.txt -D test/test1/EXPELLED -c test/test1/WRITE/help2.txt #ok
sleep 2 # wait 2 sec before execute other command

echo -e "${GREEN}\n\t[CLIENT 2] Tests -w. (Upload 8 file from test/test1/WRITE/)${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w test/test1/WRITE/,8 -D test/test1/EXPELLED
sleep 2 # wait 2 sec before execute other command

echo -e "${GREEN}\n\t[CLIENT 3] Tests -R (all file) and -r (help.text).${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -R 0 -d test/test1/READN/ -r test/test1/WRITE/help.txt -d test/test1/READ/
sleep 2 # wait 2 sec before execute other command

echo -e "${GREEN}\n\t[CLIENT 4] Tests -l and -u, unlock the file after 5000 second.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -l test/test1/WRITE/help.txt -t 5000 -u test/test1/WRITE/help.txt &
sleep 1 # wait 1 sec before execute other command

echo -e "${GREEN}\n\t[CLIENT 5] Tests -l and -u, when user has already locked file.${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -l test/test1/WRITE/help.txt -u test/test1/WRITE/help.txt
sleep 2 # wait 2 sec before execute other command

kill -SIGHUP ${SERVER_PID}
rm -f ${CONFIG}
sleep 3 # once again to make valgrind print stuff

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"