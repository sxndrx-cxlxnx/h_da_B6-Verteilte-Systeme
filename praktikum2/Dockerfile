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
RUN curl https://dlcdn.apache.org/thrift/0.20.0/thrift-0.20.0.tar.gz > thrift-0.20.0.tar.gz
RUN tar -xvf thrift-0.20.0.tar.gz
#RUN cd /
#RUN cd thrift-0.20.0
#RUN ./bootstrap.sh
#RUN ./configure
#RUN make
##RUN make check
#RUN make install
#RUN apt install -y python3-thrift
#RUN apt install -y python3-pip
#RUN apt install libboost-all-dev

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/code

# Specify the working directory
WORKDIR /usr/src/code/socket

# Use Clang to compile the Test.cpp source file
RUN clang++ -o Socket httpSocket/socket.cpp

# Run the output program from the previous step
CMD ["./httpSocket/Socket"]
