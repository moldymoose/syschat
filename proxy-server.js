const WebSocket = require('ws');
const net = require('net');

const wss = new WebSocket.Server({ port: 3001 });

wss.on('connection', (ws) => {
    console.log('Web client connected');
    
    const tcpClient = new net.Socket();
    
    tcpClient.connect(8080, '127.0.0.1', () => {
        console.log('Connected to C server');
    });
    
    ws.on('message', (message) => {
        tcpClient.write(message);
    });
    
    tcpClient.on('data', (data) => {
        ws.send(data.toString());
    });
    
    ws.on('close', () => {
        console.log('Web client disconnected');
        tcpClient.destroy();
    });
    
    tcpClient.on('close', () => ws.close());
});

console.log('WebSocket proxy running on port 3001');