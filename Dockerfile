FROM ubuntu:22.04

# Avoid prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libsasl2-dev \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Install MongoDB C driver
RUN cd /tmp && \
    wget https://github.com/mongodb/mongo-c-driver/releases/download/1.23.5/mongo-c-driver-1.23.5.tar.gz && \
    tar xzf mongo-c-driver-1.23.5.tar.gz && \
    cd mongo-c-driver-1.23.5 && \
    mkdir cmake-build && cd cmake-build && \
    cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --target install && \
    ldconfig

# Install MongoDB C++ driver
RUN cd /tmp && \
    wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.7.0/mongo-cxx-driver-r3.7.0.tar.gz && \
    tar xzf mongo-cxx-driver-r3.7.0.tar.gz && \
    cd mongo-cxx-driver-r3.7.0/build && \
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DBSONCXX_POLY_USE_BOOST=0 && \
    cmake --build . --target install && \
    ldconfig

# Install Crow
RUN cd /tmp && \
    git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && \
    mkdir build && cd build && \
    cmake .. \
        -DCROW_BUILD_EXAMPLES=OFF \
        -DCROW_BUILD_TESTS=OFF \
        -DCROW_FEATURES="ssl;compression" && \
    cmake --build . --target install && \
    ldconfig

WORKDIR /app
COPY . .

# Build the application
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build .

EXPOSE 8080
CMD ["./build/todo_cpp"]