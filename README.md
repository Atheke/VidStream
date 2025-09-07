# C++ HLS Video Streaming Server

A high-performance asynchronous HTTP server built with C++ and Boost.Beast designed for streaming HLS (HTTP Live Streaming) video content. The server efficiently handles video chunk delivery with support for adaptive bitrate streaming and range requests.

## Features

- **Asynchronous I/O**: Non-blocking architecture using Boost.Asio for high concurrency  
- **HLS Protocol Support**: Serves `.m3u8` manifest files and `.ts` video segments  
- **HTTP Range Requests**: Supports `206 Partial Content` for efficient video seeking  
- **Adaptive Bitrate**: Ready for multi-quality streaming setups  
- **Cross-Platform**: Compatible with Linux, macOS, and Windows systems  


## Docker Deployment (Production)

```bash
# Build the Docker image
docker build -t video-server .

# Run the container with volume mounting
docker run -d -p 8080:8080 -v $(pwd)/www:/app/www --name video-server video-server

# Or use Docker Compose for easier management
docker compose up -d
```

## Development Build (Testing & Development)
```bash
# Configure and build with CMake
mkdir build
cd build
cmake ..
make 

# Run the server locally
./server

# Test the server response
curl http://localhost:8080
curl http://localhost:8080/media/hls_output/master.m3u8
```

## Usage

1. Place video content in `www/media/` directory:  
   - `master.m3u8` – HLS manifest file  
   - `master_*.ts` – Video chunk files  

2. Start the server using either Docker or CMake methods above.  

3. Access the player at [http://localhost:8080](http://localhost:8080).  

