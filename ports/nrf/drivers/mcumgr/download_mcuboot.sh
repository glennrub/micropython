#!/bin/bash

SCRIPT_DIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo ""
echo "####################################"
echo "### Downloading mcuboot v1.6.0   ###"
echo "####################################"
echo ""

echo ${SCRIPT_DIR}

cd ${SCRIPT_DIR}
wget https://github.com/JuulLabs-OSS/mcuboot/archive/v1.6.0.zip
unzip -u v1.6.0.zip
rm v1.6.0.zip
mv mcuboot-1.6.0 mcuboot
cd -
