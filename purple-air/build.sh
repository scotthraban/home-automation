docker buildx build \
  --platform linux/arm64/v8,linux/amd64 \
  --push \
  --tag docker-registry.hraban.com/purple-air:latest \
  --tag docker-registry.hraban.com/purple-air:0.9 \
  .
