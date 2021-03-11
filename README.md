# pfs-modulus

## Clone

```sh
$ mkdir pfs-modulus
$ cd pfs-modulus
$ mkdir builds
$ git clone --recurse-submodules https://github.com/semenovf/pfs-modulus.git src

# Optional if submodule 'pfs-common' was not cloned
$ git submodule add https://github.com/semenovf/pfs-common.git src/pfs-common
```

## Build

```sh
$ cd builds
$ mkdir modulus.cxx11
$ cd modulus.cxx11

$ cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_STANDARD=11 \
    -Dpfs-modulus_BUILD_TESTS=ON \
    -Dpfs-modulus_BUILD_DEMO=ON \
    ../../src
```
