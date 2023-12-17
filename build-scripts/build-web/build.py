#! /usr/bin/env python

print("run python")

import subprocess
import os

NUM_JOBS = 5
NUM_CORES = 5

JOBS = [
  "target=editor use_closure_compiler=yes",
  "target=template_debug",
  "target=template_release",
  "target=template_debug dlink_enabled=yes",
  "target=template_release dlink_enabled=yes"
]

SCONS = f"scons -j{NUM_CORES // NUM_JOBS} verbose=yes warnings=no progress=no"

OPTIONS="production=yes"

processes = []


process = subprocess.Popen(["/root/build/sub_build.sh"], stdout=subprocess.PIPE, shell=True)
processes.append(process)

for p in processes:
    streamdata = p.communicate()[0]
    rc = p.returncode
    print(f"p.returncode = {rc}")
