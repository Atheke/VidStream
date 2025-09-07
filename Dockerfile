FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    libboost-all-dev \
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app
WORKDIR /app

# Build with CMake
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Expose port
EXPOSE 8080

# Run the server
CMD ["./build/server"]
