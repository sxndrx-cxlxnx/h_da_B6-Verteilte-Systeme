#Tutorial von: https://devblogs.microsoft.com/cppblog/c-development-with-docker-containers-in-visual-studio-code/
# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# install everything else we need :D
RUN apt-get install -y automake bison flex g++ git libboost-all-dev libevent-dev libssl-dev libtool make pkg-config
RUN apt install curl
RUN apt install -y cmake
RUN apt install -y python3-pip
RUN apt-get -y install systemd
RUN apt install -y mosquitto
CMD ["systemctl start mosquitto"]

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/code
