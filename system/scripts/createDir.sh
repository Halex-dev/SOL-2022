#!/bin/bash

#Objs directory
if  [ ! -d src/objs/server ]            \
    || [ ! -d src/objs/api ]            \
    || [ ! -d src/objs/client ]         \
    || [ ! -d src/objs/util ]           \
    || [ ! -d src/objs/util/data ]      \
    || [ ! -d src/objs/log ]            \
    || [ ! -d src/objs/server/file ]    
then
    echo "Creating directories objs..."
    mkdir -p src/objs/server src/objs/server/file src/objs/api src/objs/client src/objs/util src/objs/util/data src/objs/log
    echo "Directories created!"
fi

#System directory
if [ ! -d system/bin ]                  \
    || [ ! -d system/libs ]             \
    || [ ! -d system/config ]           \
    || [ ! -d system/logs ]             \
    || [ ! -d system/logs/client ]      \
    || [ ! -d system/logs/server  ]     \
    || [ ! -d src/objs/scripts ]
then
    echo "Creating directories server..."
    mkdir -p system/bin system/libs system/config system/logs src/objs/scripts system/logs/client system/logs/server 
    echo "Directories created!"
fi

#________________________________________ TEST CONTROL ________________________________________

if [ ! -d test ]                        \
    || [ ! -d test/test1 ]              \
    || [ ! -d test/test1/EXPELLED ]     \
    || [ ! -d test/test1/READ ]         \
    || [ ! -d test/test1/READN ]        \
    || [ ! -d test/test1/WRITE ]        
then
    echo "Creating test1 directories..."
    mkdir -p test test/test1 test/test1/EXPELLED test/test1/READ test/test1/READN test/test1/WRITE
    echo "Directories created!"
fi

if [ ! -d test ]                        \
    || [ ! -d test/test2 ]              \
    || [ ! -d test/test2/EXPELLED ]     \
    || [ ! -d test/test2/READ ]         \
    || [ ! -d test/test2/READN ]        \
    || [ ! -d test/test2/WRITE ]        
then
    echo "Creating test2 directories..."
    mkdir -p test test/test2 test/test2/EXPELLED test/test2/READ test/test2/READN test/test2/WRITE
    echo "Directories created!"
fi

if [ ! -d test ]                        \
    || [ ! -d test/test3 ]              \
    || [ ! -d test/test3/EXPELLED ]     \
    || [ ! -d test/test3/READ ]         \
    || [ ! -d test/test3/READN ]        \
    || [ ! -d test/test3/WRITE ]        
then
    echo "Creating test3 directories..."
    mkdir -p test test/test3 test/test3/EXPELLED test/test3/READ test/test3/READN test/test3/WRITE
    echo "Directories created!"
fi