#!/bin/bash

build_client() {
    cmake -B build
    cmake --build build --target client core
}

build_server() {
    cmake -B build
    cmake --build build --target server core
}

build_all() {
    cmake -B build
    cmake --build build
}

help() {
    echo "This is a test program script to help building source files."
    echo "Type \"./build clent\" to build-compile only client code."
    echo "Type \"./build server\" to build-compile only server code."
    echo "Type \"./build all\" to build-compile only server code."
}

case $1 in
    "client" ) build_client ;;
    "server" ) build_server ;;
    "all" ) build_all ;;
    * ) help ;;
esac