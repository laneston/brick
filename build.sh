#!/bin/sh

current_path=$(cd $(dirname $0) && pwd)

echo "Makefile execute..."


# compile mqtt target.
echo "compile mqtt target..."
cd $current_path/build/libraries_path/paho.mqtt.c-1.3.12
make

sleep 1s

# compile mqtt target.
echo "compile mqtt target..."
cd $current_path/build/libraries_path/nng-1.5.2
make

sleep 1s

# compile all.
echo "compile all...."
cd $current_path/build/
make

