FROM ubuntu:20.04

RUN apt-get update && \
    apt-get install -y \
        git \
        mercurial \
        python3 \
        python3-pip \
        wget \
    ;

RUN python3 -m pip install -U mbed-cli

RUN mkdir -p /tmp/m cd /tmp/m \
    && mbed new . \
    && pip install -r mbed-os/requirements.txt \
    && cd / && rm -rf /tmp/m

ARG GCC_ARM=gcc-arm-none-eabi-9-2020-q2-update
ARG HOST_TARGET=x86_64-linux
ARG GCC_ARM_PACKAGE=$GCC_ARM-$HOST_TARGET
RUN cd /opt \
    && wget https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/$GCC_ARM_PACKAGE.tar.bz2 \
    && bzip2 -d $GCC_ARM_PACKAGE.tar.bz2 \
    && tar xopf $GCC_ARM_PACKAGE.tar \
    && rm $GCC_ARM_PACKAGE.tar \
    ;
ENV PATH "$PATH:/opt/$GCC_ARM/bin/"

RUN mkdir ok
WORKDIR ok

CMD sh
