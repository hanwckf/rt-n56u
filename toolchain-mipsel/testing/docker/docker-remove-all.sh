#!/bin/bash

for c in `docker ps -a -q`; do
    docker rm $c
done
for i in `docker images -q`; do
    docker rmi --force $i
done
