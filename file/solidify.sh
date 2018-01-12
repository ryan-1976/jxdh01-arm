#!/bin/sh

echo 'solidify.sh'

echo 'cp cfg_file.xml monitorObj.db'
cd /mnt/flash
mkdir files
cp /root/empprj_RUN/files/* /mnt/flash/files/
mkdir bak

echo 'cp app'
cp /root/empprj_RUN/app/* /mnt/flash/app/

cd /
#/bin/
