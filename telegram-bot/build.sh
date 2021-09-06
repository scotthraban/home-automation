docker buildx build \
  --platform linux/arm64/v8,linux/amd64 \
  --push \
  --tag docker-registry.hraban.com/telegram-bot:latest \
  --tag docker-registry.hraban.com/telegram-bot:4.1 \
  .