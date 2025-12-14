# SysChat
SysChat is an instant messaging program written in C for linux systems.  It accesses system resources to create a TCP socket for sending messages over a simple custom protocol.

It utilizes POSIX/Linux networking headers and is built with GNUmake

## Usage
The source code builds to a single program (SysChat).  By default, SysChat runs in client mode, but can run as a server by using the --server flag.  Flags can also be used to specify what IP address and port to connect to, otherwise the program will prompt the user for the connectiond details.

### Accepted Flags
#### --help
- Displays program usage
#### --server
- Deploys program in server mode
#### --address
- Allows user to specify server address to connect to
#### --port
- Allows user to specify to specify port for server connection
#### --verbose
- Enables VPRINTF macro

## Program Design
### SysChat Protocol
Data is exchanged over a TCP socket using an 8 byte header.
- The first byte is an enumerator representing the type of data being sent. It is followed by 3 bytes of padding.
- The last 4 bytes are a 32 bit integer representing the length of the payload.
- The subsequent payload can technically be any type of data, but both currently implemented protocol types cast it as a string.

## Event loops
- Both server/client event loops use Select() for IO multiplexing. allowing the program to accept data from multiple data streams.
- Upon recieving a payload with the "Message" protocol header, the server echos the payload to all connected clients along with the name of the sender.