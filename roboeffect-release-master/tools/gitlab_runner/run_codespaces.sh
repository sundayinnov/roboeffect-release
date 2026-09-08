
IMAGE="castle6610/andes_build_tools_v323:latest"

docker rmi ${IMAGE}
docker pull ${IMAGE}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

docker run -it --rm \
  -v "$REPO_ROOT":/workspace \
  -w /workspace \
  ${IMAGE} \
  /bin/bash -c "chmod +x build.sh; ./build.sh build; exit"