#!/bin/bash

set -e
export basedir=$(pwd)

# Config

# For signing keystore and password.
source ./config.sh

godot_version="custom"
git_treeish="master"
build_classical=1

export webdir="../build/web/${templates_version}"
export reldir="../build/releases/${godot_version}"
export reldir_mono="../build/mono"
export tmpdir="../build/tmp"
export templatesdir="../build/templates"
export templatesdir_mono="../build/mono/templates"

export godot_basename="Godot_v${godot_version}"

# Cleanup and setup


rm -rf ${webdir}
rm -rf ${reldir}
rm -rf ${tmpdir}

mkdir -p ${webdir}
mkdir -p ${reldir}
mkdir -p ${reldir_mono}
mkdir -p ${templatesdir}
mkdir -p ${templatesdir_mono}



# Classical

if [ "${build_classical}" == "1" ]; then

  ## Web (Classical) ##

  # Editor
  unzip ../out/web/tools/godot.web.editor.wasm32.zip -d ${webdir}/
  brotli --keep --force --quality=11 ${webdir}/*
  binname="${godot_basename}_web_editor.zip"
  cp ../out/web/tools/godot.web.editor.wasm32.zip ${reldir}/${binname}

  # Templates
  cp ../out/web/templates/godot.web.template_release.wasm32.zip ${templatesdir}/web_release.zip
  cp ../out/web/templates/godot.web.template_debug.wasm32.zip ${templatesdir}/web_debug.zip

  cp ../out/web/templates/godot.web.template_release.wasm32.dlink.zip ${templatesdir}/web_dlink_release.zip
  cp ../out/web/templates/godot.web.template_debug.wasm32.dlink.zip ${templatesdir}/web_dlink_debug.zip

  ## SHA-512 sums (Classical) ##

  pushd ${reldir}
  sha512sum [Gg]* > SHA512-SUMS.txt
  mkdir -p ${basedir}/sha512sums/${godot_version}
  cp SHA512-SUMS.txt ${basedir}/sha512sums/${godot_version}/
  popd

fi