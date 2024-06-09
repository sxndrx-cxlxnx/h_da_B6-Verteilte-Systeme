#Tutorial von: https://devblogs.microsoft.com/cppblog/c-development-with-docker-containers-in-visual-studio-code/
# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# install gRPC: https://grpc.io/docs/languages/cpp/quickstart/
RUN apt-get install -y automake bison flex g++ git libboost-all-dev libevent-dev libssl-dev libtool make pkg-config
RUN apt install curl
RUN apt install -y cmake
RUN apt-get -y install git
RUN apt install -y python3-pip
RUN apt install -y python3-setuptools
RUN curl https://dlcdn.apache.org/thrift/0.20.0/thrift-0.20.0.tar.gz > thrift-0.20.0.tar.gz
RUN tar -xvf thrift-0.20.0.tar.gz
RUN cd thrift-0.20.0 && ./bootstrap.sh && ./configure && make && make install
RUN apt install -y python3-thrift
RUN apt install libboost-all-dev

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/code

# Specify the working directory
WORKDIR /usr/src/code/socket

# Use Clang to compile the Test.cpp source file
RUN g++ RPCWettanbieter/gen-cpp/Wettanbieter_types.h RPCWettanbieter/gen-cpp/Wettanbieter.h RPCWettanbieter/gen-cpp/Wettanbieter.cpp httpSocket/socket.cpp -o httpSocket/http-server -lthrift

# Run the output program from the previous step
CMD ["./httpSocket/http-server"]
