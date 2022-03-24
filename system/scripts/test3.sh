#!/bin/bash

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
# x         FILE DI TEST 3          x
# x                                 x
# x Autore: Alessio Vito D'angelo   x
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

clear
SOCK_NAME=./system/bin/SOL-2022.sk
CONFIG=./system/config/test2.txt

WRITE_DIR=./test/test3/WRITE

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./system/bin/server 
CLIENT=./system/bin/client

FILES_CREATED=50
CLIENT_TO_OPEN=25

function createClient {

    for (( i=1; i <= ${CLIENT_TO_OPEN}; i++ ))
    do

        FILES_WRITES="test/test3/WRITE/file0.dat"
        
        for (( j=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j <= $((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j++ ))
        do
            FILES_WRITES=${FILES_WRITES},test/test3/WRITE/file${j}.dat
        done

        FILES_READ="test/test3/WRITE/file0.dat"
        for (( j=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j <= $((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j++ ))
        do
            FILES_READ=${FILES_READ},test/test3/WRITE/file${j}.dat
        done

        LOCK=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 ))
        ${CLIENT} -f ${SOCK_NAME} -W ${FILES_WRITES} -D test/test3/EXPELLED -R $((1+ $RANDOM % 100)) -d test/test3/READN -w test/test3/WRITE/ -D test/test3/EXPELLED -r ${FILES_READ} -l test/test3/WRITE/file${LOCK}.dat -t 500 -u test/test3/WRITE/file${LOCK}.dat &
        #${CLIENT} -f ${SOCK_NAME} -W ${FILES_WRITES} -D test/test3/EXPELLED -R $((1+ $RANDOM % 100)) -d test/test3/READN -w test/test3/WRITE/ -D test/test3/EXPELLED -r ${FILES_READ} &
    done
}


echo -e "${GREEN}\t TEST 3: Stress-test on server ${NORMC}\n"

# Preparo il file di configurazione
touch ${CONFIG}
echo -e "WORKERS = 8\nSOCK_PATH = ${SOCK_NAME}\n\
MAX_FILES = 100\nMAX_SPACE = 32\nLOG_PATH = system/logs/server\n\
POLICY = MFU\nDEBUG = no\nSTORAGE = HASH" > ${CONFIG}

echo -e "${GREEN}\t TEST 3: Config file write in ${CONFIG}${NORMC}\n"

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
valgrind --leak-check=full --show-leak-kinds=all ${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 2 # Wait 1 second to server print

echo -e "\n"
echo -e "${GREEN}\tTEST 3: Creating ${FILES_CREATED} files random size in ${WRITE_DIR}${NORMC}\n"
sleep 2 # Wait 2 second to read echo

for (( i=0; i < ${FILES_CREATED}; i++ ))
do
    dd if=/dev/zero of=${WRITE_DIR}/file${i}.dat  bs=1b  count=$((1 + $RANDOM % 3200)) #max file 1.6MB
done

sleep 1
echo -e "\n"

echo -e "${GREEN}\tTEST 3: Create connection every 0,5 sec. After 30 seconds force to close server. ${NORMC}\n"

for (( i=0; i < 60; i++ )) # wait 30 second
do
    createClient &
    sleep 0.5
done

echo -e "${GREEN}\tTEST 3: Close server.${NORMC}\n";
kill -SIGINT ${SERVER_PID}
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"