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
RUN cd ReleaseMT/build && make all_r

CMD ["bash"]
