#Tutorial von: https://devblogs.microsoft.com/cppblog/c-development-with-docker-containers-in-visual-studio-code/
# Get the base Ubuntu image from Docker Hub
FROM ubuntu:latest

# Update apps on the base image
RUN apt-get -y update && apt-get install -y

# Install the Clang compiler
RUN apt-get -y install clang

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY . /usr/src/code

# Specify the working directory
WORKDIR /usr/src/code/socket

# Use Clang to compile the Test.cpp source file
RUN clang++ -o Test socket.cpp

# Run the output program from the previous step
CMD ["./Test"]
