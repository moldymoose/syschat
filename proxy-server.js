const WebSocket = require('ws');
const net = require('net');

const PROTO_HEADER_SIZE = 8;
const PROTO_MESSAGE = 2;
const PROTO_USERNAME = 3;

const wss = new WebSocket.Server({ port: 3001 });

wss.on('connection', (ws) => {
    console.log('Web client connected');
    
    const tcpClient = new net.Socket();
    let recvBuffer = Buffer.alloc(0);
    
    tcpClient.connect(8080, '127.0.0.1', () => {
        console.log('Connected to C server');
    });
    
    // Forward WebSocket messages to the C server using the message protocol
    ws.on('message', (message) => {
        // Default to regular message
        let protoType = PROTO_MESSAGE;
        let payloadBuf = null;

        // To robustly detect a JSON username payload, convert incoming
        // message to a UTF-8 string when possible and attempt JSON.parse.
        // This handles cases where the ws library gives us a Buffer.
        let msgStr = null;
        if (typeof message === 'string') {
            msgStr = message;
        } else if (Buffer.isBuffer(message)) {
            msgStr = message.toString('utf8');
        } else if (message && typeof message === 'object' && message instanceof ArrayBuffer) {
            msgStr = Buffer.from(message).toString('utf8');
        }

        if (msgStr) {
            try {
                const obj = JSON.parse(msgStr);
                if (obj && obj.type === 'username' && typeof obj.username === 'string') {
                    protoType = PROTO_USERNAME;
                    payloadBuf = Buffer.from(obj.username, 'utf8');
                }
            } catch (e) {
                // not JSON -- fallthrough to normal message
            }
        }

        if (payloadBuf === null) {
            payloadBuf = Buffer.isBuffer(message) ? message : Buffer.from(String(message), 'utf8');
        }

        const header = Buffer.alloc(PROTO_HEADER_SIZE);
        header.writeUInt8(protoType, 0);           // type
        // bytes 1-3 remain zero (padding)
        header.writeUInt32BE(payloadBuf.length, 4);       // length (big-endian)
        tcpClient.write(Buffer.concat([header, payloadBuf]));
    });
    
    // Parse framed protocol messages coming from the C server and forward payload to WS client
    tcpClient.on('data', (data) => {
        recvBuffer = Buffer.concat([recvBuffer, data]);
        while (recvBuffer.length >= PROTO_HEADER_SIZE) {
            const type = recvBuffer.readUInt8(0);
            const length = recvBuffer.readUInt32BE(4);
            if (recvBuffer.length < PROTO_HEADER_SIZE + length) break; // wait for full payload
            const payload = recvBuffer.slice(PROTO_HEADER_SIZE, PROTO_HEADER_SIZE + length);

            if (type === PROTO_MESSAGE) {
                // Convert payload to UTF-8 string and send as text
                const text = payload.toString('utf8');
                try { ws.send(text); } catch (err) { console.error('WS send error', err); }
            } else {
                // For other types, forward as binary
                try { ws.send(payload); } catch (err) { console.error('WS send error', err); }
            }
            recvBuffer = recvBuffer.slice(PROTO_HEADER_SIZE + length);
        }
    });
    
    ws.on('close', () => {
        console.log('Web client disconnected');
        tcpClient.destroy();
    });
    
    tcpClient.on('close', () => ws.close());
    tcpClient.on('error', (err) => { console.error('TCP error', err); tcpClient.destroy(); });
    ws.on('error', (err) => { console.error('WS error', err); ws.close(); });
});

console.log('WebSocket proxy running on port 3001');