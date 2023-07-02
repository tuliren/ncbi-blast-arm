#!/bin/sh

# run under the root directory
version=$(cat VERSION | tr -d '\n')
docker build -t ncbi_blast_arm:${version} . > build.log
