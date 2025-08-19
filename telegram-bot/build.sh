#!/bin/bash

podman manifest rm telegram-bot:latest
podman rmi telegram-bot:latest
podman build --platform linux/amd64,linux/arm64 --manifest telegram-bot:latest .
podman manifest push telegram-bot:latest docker-registry.hraban.com/telegram-bot:latest

podman manifest rm telegram-bot:5.1
podman rmi telegram-bot:5.1
podman build --platform linux/amd64,linux/arm64 --manifest telegram-bot:5.1 .
podman manifest push telegram-bot:5.1 docker-registry.hraban.com/telegram-bot:5.1
