#!/bin/bash

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
# x         FILE DI TEST 3          x
# x                                 x
# x Autore: Alessio Vito D'angelo   x
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

clear
SOCK_NAME=./system/bin/SOL-2022.sk
CONFIG=./system/config/test3.txt

WRITE_DIR=./test/test3/WRITE

GREEN="\033[0;32m"
NORMC="\033[0m"

SERVER=./system/bin/server 
CLIENT=./system/bin/client

WORKERS=8
SECOND=30
MAX_FILES=100
FILES_CREATED=${MAX_FILES}+10
CLIENT_TO_OPEN=5

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

        OTHER_FILES_WRITES="test/test3/WRITE/file0.dat"
        
        for (( j=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j <= $((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j++ ))
        do
            OTHER_FILES_WRITES=${FILES_WRITES},test/test3/WRITE/file${j}.dat
        done

        FILES_DELETE="test/test3/WRITE/file0.dat"
        for (( j=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j <= $((( $RANDOM % ($FILES_CREATED - 1) ) + 1 )); j++ ))
        do
            FILES_DELETE=${FILES_WRITES},test/test3/WRITE/file${j}.dat
        done

        LOCK=$((( $RANDOM % ($FILES_CREATED - 1) ) + 1 ))
        FILES_LOCK="test/test3/WRITE/file${LOCK}.dat"

        ${CLIENT} -f ${SOCK_NAME} -W ${FILES_WRITES} -D test/test3/EXPELLED -R $((1+ $RANDOM % 20)) -d test/test3/READN -r ${FILES_READ} -d test/test3/READ -R $((1+ $RANDOM % 20)) -W ${OTHER_FILES_WRITES} -c ${FILES_DELETE} -l ${FILES_LOCK} -t 100 -u ${FILES_LOCK} &
    done
}


echo -e "${GREEN}\t TEST 3: Stress-test on server ${NORMC}\n"

# Preparo il file di configurazione
touch ${CONFIG}
echo -e "WORKERS = ${WORKERS}\nSOCK_PATH = ${SOCK_NAME}\n\
MAX_FILES = ${MAX_FILES}\nMAX_SPACE = 32\nLOG_PATH = system/logs/server\n\
POLICY = MFU\nDEBUG = yes\nSTORAGE = HASH" > ${CONFIG}

echo -e "${GREEN}\t TEST 3: Config file write in ${CONFIG}${NORMC}\n"

echo -e "${GREEN}\n\tOpening server process.${NORMC}\n";
#valgrind --leak-check=full --show-leak-kinds=all ${SERVER} ${CONFIG} & # opening server
${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 2 # Wait 1 second to server print

echo -e "\n"
echo -e "${GREEN}\tTEST 3: Creating ${FILES_CREATED} files random size in ${WRITE_DIR}${NORMC}\n"
sleep 2 # Wait 2 second to read echo

for (( i=0; i < ${FILES_CREATED}; i++ ))
do
    dd if=/dev/zero of=${WRITE_DIR}/file${i}.dat  bs=1b  count=$((1 + $RANDOM % 1600)) #max file 1.6MB (3200)
done

sleep 1
echo -e "\n"

echo -e "${GREEN}\tTEST 3: Create connection every 0,5 sec. After ${SECOND} seconds force to close server. ${NORMC}\n"

for (( i=0; i < ${SECOND}*2; i++ )) # wait 30 second
do
    createClient &
    sleep 0.5
done

echo -e "${GREEN}\tTEST 3: Close server.${NORMC}\n";
kill -SIGINT ${SERVER_PID}
rm -f ${CONFIG}
sleep 2
echo -e "${GREEN}\n\tEnded test!${NORMC}\n"