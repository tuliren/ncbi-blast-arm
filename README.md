# NCBI BLAST+ for ARM

The latest NCBI BLAST+ (`2.14.0`) does not have ARM64 build. This repo aims to build the binaries for ARM64.

The source code is downloaded from [here](https://ftp.ncbi.nlm.nih.gov/blast/executables/blast+/LATEST/). No changes have been made to any of the source code.

## Build (ARM MacOS)

To build the source in docker:

```sh
bin/build.sh
```

This approach works on MacBook Pro with M2 Pro.

To build the source natively:

```sh
cd c++
rm -rf ReleaseMT/*
./configure --build=aarch64-linux
cd ReleaseMT/build
make all_r
```

Dependency management is not fully sorted out under native MacOS.

## Run

```sh
bin/run.sh
```

## Publish

```sh
docker tag ncbi_blast_arm:latest tuliren/ncbi_blast_arm:latest
docker push tuliren/ncbi_blast_arm:latest
```

## License
The source code belongs to NCBI. See the original [LICENSE](https://github.com/ncbi/ncbi-cxx-toolkit-public/blob/master/doc/public/LICENSE).
