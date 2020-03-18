FROM ubuntu:18.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y --no-install-recommends \
  ca-certificates \
  libhdf5-dev \
  g++ \
  make \
  pkg-config \
  python-setuptools \
  wget

RUN wget -O jobTree.tar.gz https://github.com/benedictpaten/jobTree/archive/072be69f6214e06bd8588b5e73a60b758c191663.tar.gz \
 && tar -xzf jobTree.tar.gz \
 && mv jobTree-* /usr/local/lib/python2.7/dist-packages/jobTree \
 && rm -f jobTree.tar.gz

COPY ./ /hal

RUN cd /hal/sonLib \
  && python setup.py build \
  && python setup.py install \
  && make

RUN cd /hal \
  && make sonLibRootDir=${PWD}/sonLib

FROM ubuntu:18.04
 
ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y --no-install-recommends \
  libsz2 \
  python-biopython \
  && rm -rf /var/lib/apt/lists/*
 
COPY --from=builder /usr/local/lib/python2.7/dist-packages/ /usr/local/lib/python2.7/dist-packages/
COPY --from=builder /hal/bin/ /usr/local/bin/
COPY ./ /usr/local/lib/python2.7/dist-packages/hal
