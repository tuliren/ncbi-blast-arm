#!/bin/sh

version=$(cat VERSION | tr -d '\n')
docker run -it -v ${PWD}/build:/usr/src/binaries ncbi_blast_arm:${version}
