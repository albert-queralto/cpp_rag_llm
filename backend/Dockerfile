FROM ubuntu:24.04

# Install system dependencies
RUN apt-get update && \
    apt-get install -y \
        clang \
        clang-format \
        clang-tidy \
        cmake \
        curl \
        g++ \
        git \
        libasio-dev \
        libboost-all-dev \
        libbrotli-dev \
        libcrypto++-dev \
        libcurl4-openssl-dev \
        libidn2-0-dev \
        libmimetic-dev \
        libnghttp2-dev \
        libpq-dev \
        libpqxx-dev \
        libpsl-dev \
        libsqlite3-dev \
        libssl-dev \
        libunistring-dev \
        libzstd-dev \
        meson \
        ninja-build \
        pkg-config \
        wget \
        zlib1g-dev && \
    rm -rf /var/lib/apt/lists/*

# Build and install CPR from source
RUN git clone --depth=1 https://github.com/libcpr/cpr.git /tmp/cpr && \
    cd /tmp/cpr && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCPR_USE_SYSTEM_CURL=ON . && \
    make -j$(nproc) && make install && \
    rm -rf /tmp/cpr

# Install nlohmann/json (header-only)
RUN mkdir -p /usr/local/include/nlohmann && \
    wget https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp -O /usr/local/include/nlohmann/json.hpp

# Install jwt-cpp (header-only)
RUN git clone --depth=1 https://github.com/Thalhammer/jwt-cpp.git /tmp/jwt-cpp && \
    cp -r /tmp/jwt-cpp/include/jwt-cpp /usr/local/include/ && \
    rm -rf /tmp/jwt-cpp

# Install picojson (header-only)
RUN mkdir -p /usr/local/include/picojson && \
    wget https://raw.githubusercontent.com/kazuho/picojson/master/picojson.h -O /usr/local/include/picojson/picojson.h

# Download and install crow_all.h
RUN wget https://github.com/CrowCpp/Crow/releases/download/v1.2.1.2/crow_all.h -O /usr/local/include/crow_all.h

# Install chromadb-cpp library
RUN git clone --recurse-submodules https://github.com/BlackyDrum/chromadb-cpp.git /tmp/chromadb-cpp && \
    cd /tmp/chromadb-cpp && \
    cmake -Bbuild -S. && \
    cmake --build build && \
    cmake --install build && \
    rm -rf /tmp/chromadb-cpp

WORKDIR /root/home/backend

# Copy the backend directory, including CMakeLists.txt
COPY backend/ .

# Build the project
RUN cmake -Bbuild -S. && cmake --build build

CMD ["./build/llm_backend"]