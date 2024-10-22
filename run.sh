#!/bin/bash

SERVER_PID_FILE="/tmp/my_test_server.pid"
CLIENT_PID_FILE="/tmp/my_test_client.pid"

start() {
    if [ -f $SERVER_PID_FILE ]; then
        echo "Server already running."
    else
        ./build/src/server/server &
        SERVER_PID=$!
        echo $SERVER_PID > $SERVER_PID_FILE
    fi
    
    if [ -f $CLIENT_PID_FILE]; then
        echo "Client already running."
    else
        ./build/src/client/client extra/file1.txt extra/file2.txt extra/file3.txt extra/large.txt
        CLIENT_PID=$!
        echo $CLIENT_PID > $CLIENT_PID_FILE
    fi
}

stop() {
    if [ -f $CLIENT_PID_FILE ]; then
        CLIENT_PID=$(cat $CLIENT_PID_FILE)
        if ps -p $CLIENT_PID > /dev/null; then
            kill $CLIENT_PID
        fi
        rm $CLIENT_PID_FILE
    fi

    if [ -f $SERVER_PID_FILE ]; then
        SERVER_PID=$(cat $SERVER_PID_FILE)
        if ps -p $SERVER_PID > /dev/null; then
            kill $SERVER_PID
        fi
        rm $SERVER_PID_FILE
    fi
}

help() {
    echo "This is a test program script to help running server and cleint."
    echo "Type \"./run start\" to build-compile only client code."
    echo "Type \"./build stop\" to build-compile only server code."
}

case $1 in
    "start" ) start ;;
    "stop" ) stop ;;
    * ) help ;;
esac
