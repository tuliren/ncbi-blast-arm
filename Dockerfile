FROM arm64v8/ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
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

# Create a directory to store the binaries
RUN mkdir /usr/bin/blast
RUN cd app/blast && cp blastn blastp blastx tblastn tblastx /usr/bin/blast

CMD ["bash"]
