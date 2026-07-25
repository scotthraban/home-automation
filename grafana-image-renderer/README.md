git@github.com:grafana/grafana-image-renderer.gitNotes on how to build an arm64 docker image of Grafana Image Renderer.

Replace the version number that is desired.

```
git clone git@github.com:grafana/grafana-image-renderer.git
cd grafana-image-renderer
git fetch --all --tags
git checkout tags/v3.7.1 -b v3.7.1-arm64
```

Modify the build.sh file to contain the appropriate version.

```
./build.sh
```
