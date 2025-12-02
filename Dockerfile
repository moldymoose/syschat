# Multi-stage build for SysChat
FROM node:18-alpine AS base

# Install build dependencies
RUN apk add --no-cache gcc musl-dev make python3

WORKDIR /app

# Copy source code
COPY . .

# Install Node.js dependencies
RUN npm install ws

# Build C application
RUN make clean && make

# Production stage
FROM node:18-alpine

# Install runtime dependencies
RUN apk add --no-cache python3

WORKDIR /app

# Copy built application and dependencies
COPY --from=base /app/build ./build
COPY --from=base /app/node_modules ./node_modules
COPY --from=base /app/proxy-server.js .
COPY --from=base /app/chat.html .
COPY --from=base /app/package*.json ./

# Expose ports
EXPOSE 8000 3001 8080

# Create startup script
RUN echo '#!/bin/sh' > start.sh && \
    echo './build/server/syschat-server &' >> start.sh && \
    echo 'node proxy-server.js &' >> start.sh && \
    echo 'python3 -m http.server 8000 --bind 0.0.0.0' >> start.sh && \
    chmod +x start.sh

CMD ["./start.sh"]