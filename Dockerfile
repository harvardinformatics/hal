FROM ubuntu:20.04 AS builder

ARG DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y --no-install-recommends \
  f2c \
  libatlas-base-dev \
  libbz2-dev \
  libhdf5-dev \
  libmariadb-dev-compat \
  libpng-dev \
  libssl-dev \
  g++ \
  make \
  pkg-config \
  python3-setuptools \
  uuid-dev

ADD https://hgdownload.cse.ucsc.edu/admin/exe/userApps.archive/userApps.v389.src.tgz .
RUN tar -xvf userApps.v389.src.tgz \
  && export BINDIR=/usr/local/bin \
  && for dir in lib htslib hg/lib \
                hg/bedSort \
                utils/bedToBigBed \
                utils/bedSort \
                utils/bigBedToBed \
                utils/wigToBigWig \
                utils/hgGcPercent \
                utils/faToTwoBit \
                utils/twoBitInfo; \
     do cd /userApps/kent/src/${dir} && make -j; done \
  && cd /userApps/kent/src/hg/makeDb/hgGcPercent && make MYSQLLIBS='/usr/lib/x86_64-linux-gnu/libmysqlclient.a -ldl -lstdc++ -lrt -lgnutls'

WORKDIR /hal

ADD https://github.com/ComparativeGenomicsToolkit/sonLib/archive/50e46d9d19d683b35938864bf3d9337713ba3098.tar.gz .

RUN tar -xzf *.tar.gz \
  && mv sonLib-* sonLib \
  && rm *.tar.gz

RUN cd sonLib \
  && python3 setup.py install \
  && make -j

# debian phast package lacks headers & libphast (needed for phyloP)
ADD http://compgen.cshl.edu/phast/downloads/phast.v1_5.tgz /
RUN tar -C / -xzf /phast.v1_5.tgz
RUN cd /phast/src/ \
  && make CLAPACKPATH=''

COPY ./ .

RUN make -j sonLibRootDir=${PWD}/sonLib ENABLE_PHYLOP=1

FROM ubuntu:20.04
 
RUN apt update && apt install -y --no-install-recommends \
  libsz2 \
  phast \
  python3-biopython \
  toil \
  && rm -rf /var/lib/apt/lists/*
 
COPY --from=builder /usr/local/lib/python3.8/dist-packages/ /usr/local/lib/python3.8/dist-packages/
COPY --from=builder /hal/bin/ /usr/local/bin/ /usr/local/bin/
COPY ./ /usr/local/lib/python3.8/dist-packages/hal
