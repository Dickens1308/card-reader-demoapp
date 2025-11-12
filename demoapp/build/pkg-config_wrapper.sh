#!/bin/sh
PKG_CONFIG_SYSROOT_DIR=/home/dart/aep/sdk-4.18/toolchain-cortexa9hf-2.4/sysroots/cortexa9hf-neon-poky-linux-gnueabi
export PKG_CONFIG_SYSROOT_DIR
PKG_CONFIG_LIBDIR=/home/dart/aep/sdk-4.18/toolchain-cortexa9hf-2.4/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/lib/pkgconfig:/home/dart/aep/sdk-4.18/toolchain-cortexa9hf-2.4/sysroots/cortexa9hf-neon-poky-linux-gnueabi/usr/share/pkgconfig:/home/dart/aep/sdk-4.18/toolchain-cortexa9hf-2.4/sysroots/cortexa9hf-neon-poky-linux-gnueabi/aep-dev/lib/pkgconfig
export PKG_CONFIG_LIBDIR
exec pkg-config "$@"
