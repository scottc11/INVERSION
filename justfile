DOCKER_IMAGE := 'ok:mbed'
BUILD := 'BUILD'

default:
    @just --list

docker:
    docker build --tag {{DOCKER_IMAGE}} .

run IT='-it' +CMD='sh': docker
    clear
    docker run \
        {{IT}} \
        --mount type=bind,source="$(pwd)",target=/ok \
        {{DOCKER_IMAGE}} \
        {{CMD}}

do +CMD:
    @just run '' {{CMD}}
    
init:
    @just do mbed deploy

build PROFILE='develop':
    @just do mbed compile \
        --profile {{PROFILE}} \
        --build {{BUILD}} \
        ;

clean:
    @just do rm -rf {{BUILD}}
