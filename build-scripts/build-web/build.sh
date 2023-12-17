#!/bin/bash

set -e

# Config

# To speed up builds with single-threaded full LTO linking,
# we run all builds in parallel each from their own folder.
export NUM_JOBS=5
export NUM_CORES=5
declare -a JOBS=(
  "target=editor use_closure_compiler=yes"
  "target=template_debug"
  "target=template_release"
  "target=template_debug dlink_enabled=yes"
  "target=template_release dlink_enabled=yes"
)

export SCONS="scons -j$(expr ${NUM_CORES} / ${NUM_JOBS}) verbose=yes warnings=no progress=no"
export OPTIONS="production=yes"
export TERM=xterm

source /root/emsdk/emsdk_env.sh

rm -rf godot
mkdir godot
cd godot
tar xf /root/godot.tar.gz --strip-components=1 --no-same-owner

# Classical

dnf -y install gettext

echo "Starting classical build for Web..."

python3 /root/build/build.py

cp -rvp /root/godot0/bin/*.editor*.zip /root/out-tools

for i in {1..4}; do
	cp -rvp /root/godot$i/bin/*.zip /root/out-templates
done

echo "Web build successful"
