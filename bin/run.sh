#!/bin/sh

docker run -it -v ${PWD}/build:/usr/src/binaries ncbi_blast_arm
