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

for i in {0..4}; do
	cp -r /root/godot /root/godot$i
	cd /root/godot$i
	echo "$SCONS platform=web ${OPTIONS} ${JOBS[$i]}"
	$SCONS platform=web ${OPTIONS} ${JOBS[$i]} | tee /root/build/log-$i.log
done
