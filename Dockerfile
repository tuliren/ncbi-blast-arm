FROM arm32v7/ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    zlib1g-dev \
    liblmdb-dev

COPY "./c++" "/usr/src/c++"

WORKDIR "/usr/src/c++"

# Configure and build NCBI BLAST
RUN rm -rf ReleaseMT/*
RUN ./configure --build=arm-linux
RUN cd ReleaseMT/build && make all_r

# Create a directory to store the binaries
RUN mkdir /usr/src/binaries
RUN cp ReleaseMT/build/bin/* /usr/src/binaries/

CMD ["bash"]
