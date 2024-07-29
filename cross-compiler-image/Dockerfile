
FROM ubuntu:latest

RUN apt-get update

COPY packages.txt .
RUN apt-get install -y --no-install-recommends --no-install-suggests $(cat packages.txt)

COPY gnu-keyring.gpg /gnu-keyring.gpg
RUN gpg --import /gnu-keyring.gpg

#ARG BINUTILS_MIRROR="https://ftp.gnu.org/gnu/binutils/"
ARG BINUTILS_MIRROR="https://gnuftp.uib.no/binutils/"
ARG BINUTILS_NAME="binutils-2.42"
ARG BINUTILS_ARCHIVE=${BINUTILS_NAME}".tar.gz"

#ARG GCC_MIRROR="https://ftp.gnu.org/gnu/gcc/gcc-14.1.0/"
ARG GCC_MIRROR="https://gnuftp.uib.no/gcc/gcc-14.1.0/"
ARG GCC_NAME="gcc-14.1.0"
ARG GCC_ARCHIVE=${GCC_NAME}".tar.gz"

ARG PREFIX=/opt/cross/
ARG TARGET=i686-elf
ENV PATH=${PREFIX}bin:${PATH}

WORKDIR build

# download and build gcc and binutils source
RUN set -x                                                      \
 && wget --no-verbose ${BINUTILS_MIRROR}${BINUTILS_ARCHIVE}     \
 && wget --no-verbose ${BINUTILS_MIRROR}${BINUTILS_ARCHIVE}.sig \
 && gpg  --verify "${BINUTILS_ARCHIVE}.sig"                     \
 && mkdir binutils                                              \
 && tar -xf ${BINUTILS_ARCHIVE} -C binutils                     \
 && mkdir binutils-build                                        \
 && cd binutils-build                                           \
 && ../binutils/${BINUTILS_NAME}/configure                      \
        --target=$TARGET                                        \
        --prefix=$PREFIX                                        \
        --with-sysroot                                          \
        --disable-nls                                           \
        --disable-werror                                        \
 && make -j 8                                                   \
 && make install                                                \
 && rm -rf binutils-build binutils ${BINUTLS_ARCHIVE}

RUN set -x                                             \
 && wget --no-verbose ${GCC_MIRROR}${GCC_ARCHIVE}      \
 && wget --no-verbose ${GCC_MIRROR}${GCC_ARCHIVE}.sig  \
 && gpg --verify "${GCC_ARCHIVE}.sig"                  \
 && mkdir gcc                                          \
 && tar -xf ${GCC_ARCHIVE} -C gcc                      \
 && mkdir gcc-build                                    \
 && cd gcc-build                                       \
 && ../gcc/${GCC_NAME}/configure                       \
        --target=$TARGET                               \
        --prefix=$PREFIX                               \
        --disable-nls                                  \
        --enable-languages=c                           \
        --without-headers                              \
 && make -j 8 all-gcc                                  \
 && make -j 8 all-target-libgcc                        \
 && make install-gcc                                   \
 && make install-target-libgcc                         \
 && rm -rf gcc-build gcc ${GCC_ARCHIVE}

WORKDIR /runtime
