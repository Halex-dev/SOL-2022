#!/bin/bash

# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
# x         FILE DI TEST 2          x
# x                                 x
# x Autore: Alessio Vito D'angelo   x
# xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

clear
SOCK_NAME=./system/bin/SOL-2022.sk
CONFIG=./system/config/test1.txt

GREEN="\033[0;32m"
NORMC="\033[0m"

POLICY=$1

SERVER=./system/bin/server 
CLIENT=./system/bin/client

# Preparo il file di configurazione
touch ${CONFIG}
echo -e "WORKERS = 4\nSOCK_PATH = ${SOCK_NAME}\n\
MAX_FILES = 10\nMAX_SPACE = 1\nLOG_PATH = system/logs/server\n\
POLICY = ${POLICY}\nDEBUG = yes\nSTORAGE = HASH" > ${CONFIG}

echo -e "${GREEN}\n\tOpening server process whith with policy ${POLICY}.${NORMC}\n";
${SERVER} ${CONFIG} & # opening server
SERVER_PID=$! # getting server PID
sleep 1 # just to make valgrind print stuff

echo -e "${GREEN}\n\tCreating 8 file (max 36Kb)${NORMC}\n";

for (( i=1; i < 10; i++ ))
do
    dd if=/dev/zero of=./test/test2/WRITE/files/file${i}.dat  bs=1b  count=$((1 + $RANDOM % 70))
done

FILES_WRITES="-W test/test2/WRITE/files/file1.dat"
for (( i=2; i < 10; i++ ))
do
    FILES_WRITES="${FILES_WRITES} -W test/test2/WRITE/files/file${i}.dat"
done

echo -e "${GREEN}\n\t[CLIENT 1] Upload 9 files, one every second ${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 1000 ${FILES_WRITES}

echo -e "${GREEN}\n\t[CLIENT 1] READ 2 files (file1.dat and file2.dat without writing) (for MFU or LFU)${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 1000 -r test/test2/WRITE/files/file1.dat,test/test2/WRITE/files/file2.dat

echo -e "${GREEN}\n\t[CLIENT 2] Upload 3 img expelled in test/test2/EXPELLED/NUMBER ${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -w test/test2/WRITE/img/ -D test/test2/EXPELLED/NUMBER/

echo -e "${GREEN}\n\t[CLIENT 3] Upload 0.8MB file.dat expelled in test/test2/EXPELLED/SIZE ${NORMC}\n";
${CLIENT} -f ${SOCK_NAME} -p -t 200 -W test/test2/WRITE/mega/file.dat -D test/test2/EXPELLED/SIZE/

sleep 1
kill -SIGHUP ${SERVER_PID}
rm -f ${CONFIG}

echo -e "${GREEN}\n\tEnded test!${NORMC}\n"