# SysChat

A simple chat application with C backend and web frontend.

## Setup

### Install Dependencies
```bash
# Install build tools
apt update
apt install build-essential nodejs npm

# Install WebSocket library
npm install ws
```

### Build the Project
```bash
make
```

## Running the Chat

You need 3 terminals running simultaneously:

### Terminal 1: Start C Server
```bash
./build/server/syschat-server
```
*Should display: "Server listening on port 8080..."*

### Terminal 2: Start WebSocket Proxy
```bash
node proxy-server.js
```
*Should display: "WebSocket proxy running on port 3001"*

### Terminal 3: Start Web Server
```bash
python3 -m http.server 8000
```
*Should display: "Serving HTTP on 0.0.0.0 port 8000..."*

### Access the Chat
Open your browser and go to:
```
http://localhost:8000/chat.html
```

## Stopping the Servers
Press `Ctrl+C` in each terminal to stop the servers.

## Alternative: Command Line Client
Instead of the web interface, you can use the C client:
```bash
./build/client/syschat
```

## Architecture
```
Web Browser → WebSocket Proxy (port 3001) → C Server (port 8080)
```