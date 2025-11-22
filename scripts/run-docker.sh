#!/bin/bash
cd "$(dirname "$0")"/..
SCRIPT_FILE=$0

if [ "$1" = "--build" ]; then
    docker rmi boiler-controller-env
    docker build -f scripts/Dockerfile -t boiler-controller-env --build-arg UN=$(id -un) --build-arg UI=$(id -u) --build-arg GN=$(id -gn) --build-arg GI=$(id -g) .
    shift
fi

if ! docker image inspect boiler-controller-env >/dev/null 2>&1; then
    echo "Error: Docker image 'boiler-controller-env' does not exist." >&2
    echo "Hint:  Run '$SCRIPT_FILE --build' first." >&2
    echo "Hint:  You can also later run '$SCRIPT_FILE --build' to rebuild the image." >&2
    exit 1
fi

if docker ps --format '{{.Names}}' | grep -q '^boiler-controller-docker$'; then
    echo "This is your secondary shell inside the Docker container. It will be available as long as the main shell is running."
    if [ $# -eq 0 ]; then
        docker exec -it boiler-controller-docker bash
    else
        docker exec -it boiler-controller-docker "$@"
    fi
else
    echo "This is your main shell inside the Docker container. If you exit, the container will be removed."
    if [ $# -eq 0 ]; then
        docker run --rm -it --name boiler-controller-docker -p 8080:8080 -v "$(pwd)":/home/$(id -un)/app boiler-controller-env bash
    else
        docker run --rm -it --name boiler-controller-docker -p 8080:8080 -v "$(pwd)":/home/$(id -un)/app boiler-controller-env "$@"
    fi
fi

