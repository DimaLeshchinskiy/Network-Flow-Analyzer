FROM gcc:latest
LABEL Name=sniffer

# Copy the current folder which contains C++ source code to the Docker image under /usr/src
COPY ./src /usr/src

# Specify the working directory
WORKDIR /usr/src

# Use GCC to compile the Test.cpp source file
RUN g++ -o main main.cpp

# Run the program output from the previous step
CMD ["./main"]
