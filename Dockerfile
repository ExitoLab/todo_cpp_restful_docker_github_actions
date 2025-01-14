FROM debian:bullseye-slim

# Install system dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        git \
        libcurl4-openssl-dev \
        libssl-dev \
        gnupg \
        wget \
        && rm -rf /var/lib/apt/lists/*

# Add MongoDB repository
RUN wget -qO - https://www.mongodb.org/static/pgp/server-6.0.asc
RUN apt-key add - < server-6.0.asc  # Replace server-6.0.asc with the downloaded filename (if different)

RUN echo "deb [ arch=amd64,arm64 ] https://repo.mongodb.org/apt/debian bullseye/mongodb-org/6.0 main" | tee /etc/apt/sources.list.d/mongodb-org-6.0.list

# Install MongoDB C++ Driver dependencies
RUN apt-get update && \
    apt-get install -y libboost-dev libbsoncxx-dev libmongocxx-dev && \
    rm -rf /var/lib/apt/lists/*

# Install crow
RUN git clone https://github.com/ipkn/crow.git /tmp/crow && \
    mkdir /tmp/crow/build && cd /tmp/crow/build && \
    cmake .. && make install && rm -rf /tmp/crow

# Set working directory
WORKDIR /app

# Copy CMakeLists.txt and source code
COPY CMakeLists.txt .
COPY main.cpp .

# Configure and build the application
RUN mkdir build && cd build && \
    cmake .. && make

# Expose the application port
EXPOSE 8080

# Command to run the application
CMD ["./build/todo_cpp"]