#!/bin/bash

winpty docker run\
	-v ${basepath}\\godot-build-scripts-main\\build-web:"/root/build" \
	-v ${basepath}\\out\\web\\templates:"/root/out-templates" \
	-v ${basepath}\\out\\web\\tools:"/root/out-tools" \
	-v ${basepath}\\godot-custom.tar.gz:"/root/godot.tar.gz" \
	-v ${basepath}\\godot-build-scripts-main\\files:"/root/files":z \
	godot-web:${img_version} bash build/build.sh