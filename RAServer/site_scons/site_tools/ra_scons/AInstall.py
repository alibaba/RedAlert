# -*- mode: python -*-
import sys, os
import SCons.Script
import SCons.Environment
import SCons.Defaults
from SCons.Builder import Builder
def generate(env):
    env.Append(BUILDERS = {"aInstall" : aInstall})

def exists(env):
    return 1

def aInstall(env, packageName, source, dirInPackage):
    if type(source) != type([]):
        source = [source]
    for p in packageName:
        vTarget = env.Install(env['installroot'] + "/" +dirInPackage, source)
        env.Depends(packageName, vTarget)
        env.Alias("install", vTarget)
    return packageName
