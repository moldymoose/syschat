#!/bin/bash

echo "SysChat Deployment Script"

# check if docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker not found. Installing Docker..."
    curl -fsSL https://get.docker.com -o get-docker.sh
    sudo sh get-docker.sh
    sudo usermod -aG docker $USER
    echo "Docker installed. Please log out and back in, then run this script again."
    exit 1
fi

# check if docker compose is installed
if ! command -v docker-compose &> /dev/null; then
    echo "Docker Compose not found. Installing..."
    sudo curl -L "https://github.com/docker/compose/releases/download/v2.20.2/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
    sudo chmod +x /usr/local/bin/docker-compose
fi

# stop any existing containers
echo "Stopping existing containers..."
docker compose down 2>/dev/null || true

# build and start the application
echo "Building and starting SysChat..."
docker compose up --build -d

# wait for services to start
sleep 5

# check if containers are running
if docker compose ps | grep -q "Up"; then
    echo "SysChat is running"
    echo "Access your chat at: http://$(hostname -I | awk '{print $1}'):8000/chat.html"
    echo "View logs: docker-compose logs -f"
    echo "Stop: docker-compose down"
else
    echo "Failed to start. Check logs: docker-compose logs"
fi