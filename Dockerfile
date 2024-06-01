# Use the official Ubuntu image
FROM ubuntu:24.04

# Set the maintainer label
LABEL maintainer="leixure@gmail.com"

ENV DEBIAN_FRONTEND noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip	\
    doxygen

# Install Conan
RUN pip install conan --break-system-packages

# Set environment variables
ENV CC=gcc
ENV CXX=g++

# Create a directory for the app
WORKDIR /app

# Copy the project files to the container
COPY . .

# Install Conan dependencies
RUN conan profile detect --force
RUN conan create . --build=missing -tf ""
RUN conan install --requires bb/1.0 --deployer direct_deploy

# Set the entrypoint to run the application
CMD ["./direct_deploy/bb/bin/bb", "./direct_deploy/bb/doc/html"]

