#!/bin/bash

set -e

OPTIND=1

# Config

# For default registry and number of cores.
if [ ! -e config.sh ]; then
  echo "No config.sh, copying default values from config.sh.in."
  cp config.sh.in config.sh
fi
source ./config.sh

godot_version="custom"
git_treeish="master"

cd ..
sh misc/scripts/make_tarball.sh -v ${godot_version} -g ${git_treeish}
cp ../godot-${godot_version}.tar.gz ./godot-${godot_version}.tar.gz 
rm ../godot-${godot_version}.tar.gz

export basedir="$(pwd)"

mkdir -p ${basedir}/out
mkdir -p ${basedir}/out/logs
mkdir -p ${basedir}/out/web
mkdir -p ${basedir}/out/web/tools
mkdir -p ${basedir}/out/web/templates

export img_version=$IMAGE_VERSION

echo "build docker web"

winpty docker build \
	--build-arg img_version=${img_version} \
	-t godot-web:${img_version} \
	-f ./build-scripts/Dockerfile.web .

mkdir -p ${basedir}/out/web

echo "run docker"

export basepath="E:\\Repos\\WGoDot"

winpty docker run \
	-v ${basepath}/build-scripts/build-web:"/root/build" \
	-v ${basepath}/out/web/templates:"/root/out-templates" \
	-v ${basepath}/out/web/tools:"/root/out-tools" \
	-v ${basepath}/build-scripts/godot.tar.gz:"/root/godot.tar.gz" \
	godot-web:${img_version} bash build/build.sh