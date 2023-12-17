#!/bin/bash

winpty docker build \
	--build-arg img_version=${img_version} \
	-t godot-web:${img_version} \
	-f ./godot-build-scripts-main/Dockerfile.web .