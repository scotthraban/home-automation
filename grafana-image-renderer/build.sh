docker buildx build \
  --platform linux/arm64/v8 \
  --push \
  --tag docker-registry.hraban.com/grafana-image-renderer:latest \
  --tag docker-registry.hraban.com/grafana-image-renderer:3.7.1-arm64 \
  .
