# SysChat

A simple chat application with C backend and web frontend.

## Deployment Options

### Option 1: Docker Container (Recommended)
```bash
# Quick start
./deploy.sh

# Access at: http://localhost:8000/chat.html or from link in terminal
# Stop with: stop.sh, or docker compose down
```

### Option 2: Manual Terminal Setup

#### Install Dependencies
```bash
# Install build tools
apt update
apt install build-essential nodejs npm

# Install WebSocket library
npm install ws
```

#### Build the Project
```bash
make
```

#### Running the Chat (3 terminals needed)

**Terminal 1: Start C Server**
```bash
./build/syschat -s -p 8080
```
*Should display: "Server listening on port 8080..."*

**Terminal 2: Start WebSocket Proxy**
```bash
node proxy-server.js
```
*Should display: "WebSocket proxy running on port 3001"*

**Terminal 3: Start Web Server**
```bash
python3 -m http.server 8000
```
*Should display: "Serving HTTP on 0.0.0.0 port 8000..."*

#### Access the Chat
Open your browser and go to:
```
http://localhost:8000/chat.html
```

#### Stopping the Servers
Press `Ctrl+C` in each terminal to stop the servers.

## Alternative: Command Line Client
Instead of the web interface, you can use the C client:
```bash
./build/syschat -p 8080 -a 0.0.0.0
```

## Architecture
```
Web Browser → WebSocket Proxy (port 3001) → C Server (port 8080)
```