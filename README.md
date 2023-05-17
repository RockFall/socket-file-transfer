# File Transfer via Sockets

This repository contains an implementation of a simple file transfer system using sockets in C. The system allows transferring plain text files between a server and a client over a local network without internet access.
## Features

The server can perform the following functions:

- Accept connection from a single client
- Receive files sent by the client and store them in the file system
- Confirm successful receipt of the file to the client
- Receive connection termination request

The client can execute the following actions:

- Connect to the server
- Select a file to send to the server
- Send the selected file
- Receive confirmation from the server that the file was successfully received
- Terminate the connection with the server

## Protocol

The communication protocol is simple, based on plain text messages. Each message has a header indicating the file name, followed by the file content. The client can send commands such as "select file [filename]" to select the file to be sent, "send file" to send the selected file, and "exit" to terminate the connection.

For implementation details, including message formatting and expected server responses, refer to the protocol section in the project documentation.
## Getting Started

To use the file transfer system, follow these steps:

1. Start the server by running the server program with the appropriate address type (v4 for IPv4 or v6 for IPv6) and a port number as command-line arguments.

    
    ```bash
    ./server v4 51511
    ```

2. Start the client by running the client program with the server's IP address and port number as command-line arguments.
    
    ```bash
    ./client 127.0.0.1 51511
    ```


## Usage

Once the client is connected to the server, use the following commands to interact with the system:

- **`select file [filename]`**: Selects the file to be sent to the server. Replace **`[filename]`** with the actual file name and extension.
- **`send file`**: Sends the selected file to the server.
- **`exit`**: Terminates the connection with the server.

## Notes

- Only plain text files with the extensions listed in the valid file types table can be transferred.
- The system supports messages up to 500 bytes in length, encoded in ASCII.
- Special characters and accented letters are not supported and should not be included in the file content.

## Valid File Types

    *.txt
    *.c
    *.cpp
    *.py
    *.tex
    *.java

 ## License

This project is licensed under the MIT License.
## Acknowledgements

This implementation is based on a practical assignment and can serve as a reference for implementing socket communication between a server and a client in C.
