#!/bin/sh

version=$(cat VERSION | tr -d '\n')
docker tag ncbi_blast_arm:${version} tuliren/ncbi_blast_arm:${version}
docker push tuliren/ncbi_blast_arm:${version}
