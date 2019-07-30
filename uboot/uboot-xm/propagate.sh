#!/bin/sh
TARGET=/svn/devbox-trunk/
FACTORY=/svn/art-trunk/factory_art
for f in $(find ${TARGET} -name ar7240-u-boot.rom 2>/dev/null); do 
    cp u-boot.bin "$f"
done;
cp u-boot.bin ${FACTORY}/common_parts/u-boot.tmp
cat ${FACTORY}/common_parts/fill >>${FACTORY}/common_parts/u-boot.tmp
dd if=${FACTORY}/common_parts/u-boot.tmp of=${FACTORY}/common_parts/u-boot bs=$((0x10000)) count=4
rm ${FACTORY}/common_parts/u-boot.tmp
ls -l ${FACTORY}/common_parts/u-boot
