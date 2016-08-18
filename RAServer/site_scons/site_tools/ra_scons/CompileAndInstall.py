# -*- mode: python -*-
import sys, os, subprocess
import SCons.Script
import SCons.Environment
import SCons.Defaults
from SCons.Builder import Builder
def generate(env):
    env.AddMethod(compileAndInstallLibrary)
    env.AddMethod(compileAndInstallBinary)

def exists(env):
    return 1

def compileAndInstallLibrary(env, target, sources, package, libs):
    libObj = env.SharedLibrary(target = target,
                               source = sources,
                               package = package,
                               dirInPackage = 'lib64',
                               LIBS = libs)

    env.Default(libObj)
    env.Default(env.Install(env["RA_LIB_DIR"], libObj))
    env.installToPackage(package, libObj, "usr/local/lib64")

def compileAndInstallBinary(env, target, sources, package, libs):
    binTarget = env.Program(
        target = target,
        source = sources,
        LIBS = libs)

    env.Default(binTarget)
    env.Default(env.Install(env["RA_BIN_DIR"], binTarget))
    env.installToPackage(package, binTarget, "usr/local/bin")

