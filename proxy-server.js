const WebSocket = require('ws');
const net = require('net');

const PROTO_HEADER_SIZE = 8;
const PROTO_MESSAGE = 2;
const PROTO_USERNAME = 3;

const wss = new WebSocket.Server({ port: 3001 });

wss.on('connection', (ws) => {
    console.log('Web client connected');
    
    let tcpClient = new net.Socket();
    let recvBuffer = Buffer.alloc(0);
    let tcpConnected = false;
    let pendingQueue = [];
    
    function connectTcp(host, port) {
        if (tcpConnected) {
            try { tcpClient.destroy(); } catch (_) {}
            tcpClient = new net.Socket();
            recvBuffer = Buffer.alloc(0);
            tcpConnected = false;
        }

        tcpClient.connect(port, host, () => {
            tcpConnected = true;
            console.log(`Connected to SysChat server ${host}:${port}`);
            try { ws.send(`Connected to SysChat server ${host}:${port}`); } catch (_) {}
            // flush any queued messages
            for (const buf of pendingQueue) {
                tcpClient.write(buf);
            }
            pendingQueue = [];
        });
        tcpClient.on('error', (err) => {
            console.error('TCP error', err);
            try { ws.send(`TCP error: ${err.message}`); } catch (_) {}
            tcpClient.destroy();
            tcpConnected = false;
        });
        tcpClient.on('close', () => { tcpConnected = false; try { ws.close(); } catch(_){} });
    }
    
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
                // Handle connection instruction from browser
                if (obj && obj.type === 'connect' && typeof obj.host === 'string' && (typeof obj.port === 'number' || typeof obj.port === 'string')) {
                    const port = typeof obj.port === 'string' ? parseInt(obj.port, 10) : obj.port;
                    if (!Number.isFinite(port)) {
                        try { ws.send('Invalid port'); } catch(_){}
                        return;
                    }
                    connectTcp(obj.host, port);
                    return;
                }
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
        const frame = Buffer.concat([header, payloadBuf]);
        if (tcpConnected) {
            tcpClient.write(frame);
        } else {
            // queue until connected
            pendingQueue.push(frame);
        }
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
    
    // ws close/error
    // tcp close/error handled in connectTcp
    // keep existing ws error handling
    ws.on('error', (err) => { console.error('WS error', err); ws.close(); });
});

console.log('WebSocket proxy running on port 3001');