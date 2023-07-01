FROM arm64v8/ubuntu:latest as builder

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    zlib1g-dev \
    liblmdb-dev

COPY "./c++" "/usr/src/c++"

WORKDIR "/usr/src/c++"

# Configure and build NCBI BLAST
RUN rm -rf ReleaseMT/*
RUN ./configure --build=aarch64-linux

WORKDIR "/usr/src/c++/ReleaseMT/build"
RUN make all_r


FROM arm64v8/ubuntu:latest as final

WORKDIR "/usr/bin/blast"

COPY --from=builder "/usr/src/c++/ReleaseMT/build/app/blast/blastn" .
COPY --from=builder "/usr/src/c++/ReleaseMT/build/app/blast/blastp" .
COPY --from=builder "/usr/src/c++/ReleaseMT/build/app/blast/blastx" .
COPY --from=builder "/usr/src/c++/ReleaseMT/build/app/blast/tblastn" .
COPY --from=builder "/usr/src/c++/ReleaseMT/build/app/blast/tblastx" .

CMD ["bash"]
