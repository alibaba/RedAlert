import os
import sys
curPath = os.path.split(os.path.realpath(__file__))[0]
sys.path.insert(0, curPath)
import InitEnv
import Package
import Test
import GenerateFile
import AInstall
import CompileAndInstall

def generate(env):
    cwd = os.getcwd()
    if cwd[-13:] in ['build/release', 'build/debug64']:
        os.chdir(cwd[:-13])

    print "Loading plug-in: ra_scons"
    print "#####################################################"
    InitEnv.generate(env)
    Package.generate(env)
    Test.generate(env)
    GenerateFile.generate(env)
    AInstall.generate(env)
    CompileAndInstall.generate(env) 

def exists(env):
    return 1
