# Tic-Tac-Toe Game with Command Execution

## Overview
==============================================================

1. **Part 1:** A Tic-Tac-Toe (TTT) game where you play against a basic AI.
2. **Part 2:** A command-line tool that checks for the presence of the `-e` flag. If the flag is found, it executes the command passed as an
   # mync - A Network Communication Utility

## Overview

`mync` is a versatile network communication tool that supports multiple protocols, including TCP, UDP, and UNIX Domain Sockets (UDS). It can function as both a client and a server, enabling a wide range of network-based operations, such as chatting or executing commands across different communication channels.

## Features

- Supports TCP and UDP communication.
- UNIX Domain Sockets (UDS) for both Datagram (DGRAM) and Stream (STREAM) modes.
- Bidirectional communication between client and server.
- Custom executable support.
- Timeout functionality for client/server operations.

## Usage

### Command-line Arguments

The command-line arguments for `mync` allow you to specify the protocol, mode of operation (input, output, or both), executable, and timeout.

- `-e`: Specify an executable to run (e.g., `./ttt`).
- `-i`: Input mode.
- `-o`: Output mode.
- `-b`: Both input and output mode.
- `-t`: Timeout (in seconds).

### Example Commands

#### TCP

- Start a TCP server on port 4055 with an executable:

  ```bash
  ./mync -e "./ttt 123456789" -i TCPS4055


### How to Run
# TCP Server
./mync -e "./ttt 123456789" -i TCPS4055

# TCP Client
./mync -e "./ttt 123456789" -o TCPClocalhost,4055

# UDP Server with Timeout
./mync -e "./ttt 123456789" -i UDPS4050 -t 10

# UDP Client with Timeout
./mync -e "./ttt 123456789" -o UDPClocalhost,4050 -t 10

# UDS Datagram Server with Timeout
./mync -e "./ttt 123456789" -i UDSSD/tmp/sock -t 10

# UDS Datagram Client with Timeout
./mync -e "./ttt 123456789" -o UDSCD/tmp/sock -t 10

# UDS Stream Server
./mync -e "./ttt 123456789" -i UDSSS/tmp/sock

# UDS Stream Client
./mync -e "./ttt 123456789" -o UDSCS/tmp/sock

To execute a command using the `-e` flag, use the following command:

```bash
./p2 -e "./ttt 123456789"
```

This example demonstrates how to execute the Tic-Tac-Toe game (`ttt`) through Part 2 (`p2`). The command provided after the `-e` flag (`"./ttt 123456789"`) will be executed using the `execvp` function.

## Notes

- Ensure that the `ttt` executable is in the same directory as `p2` or in a directory listed in your `PATH` environment variable.
- The Tic-Tac-Toe AI in this game is intentionally designed to be very simple, so you can easily win!

## Compilation

To compile the code, you can use the following commands:

```bash
make all
```


