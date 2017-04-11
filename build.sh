#!/bin/bash

if [ $# -ge 1 ] && [ $1 = "clean" ]
then
    cd ./WebConsoleCore/obj/ && rm *.o && cd ../../WebConsole/obj/ && rm *.o
    echo "Clean";
else
    cd WebConsoleCore && make && cp bin/libWebConsoleCore.so ../WebConsole/bin/ && cd ../WebConsole && make && cd bin && build="ok"
    if [ $# -ge 1 ] && [ $1 = "exec" ] && [ $build = "ok" ]
    then
        terminology -e "LD_LIBRARY_PATH=. ./WebConsole"
    fi
fi

