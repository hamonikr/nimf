FROM ubuntu:24.04 AS builder

RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get -qq update \
 && apt-get -qq upgrade

RUN apt-get install --no-install-recommends -y \
        cmake \
        g++ \
        make \
        pkg-config \
        python3-dev \
        devscripts \
        equivs \
        sudo \
        xterm \
        apt-utils

COPY . /src
WORKDIR /build

# 자동으로 yes 응답을 전달
RUN yes | mk-build-deps --install --root-cmd sudo --remove /src/debian/control

# Build the entire project
RUN cd /src \
 && autoreconf --force --install --verbose \
 && ./configure --prefix=/usr/local \
 && make -j $(nproc) \
 && make install

ENTRYPOINT ["bash"]
