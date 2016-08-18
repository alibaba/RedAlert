# -*- mode: python -*-
import sys, os, re
import SCons.Script
import SCons.Environment
import SCons.Defaults
from SCons.Builder import Builder
def generate(env):
    addEnvProperty(env)
    env.AddMethod(generateFile)  

def exists(env):
    return 1

def addEnvProperty(env):
    env.Append(ACSUBST = {'BUILDDIR': env['RA_RELATIVE_BUILD_DIR']})
    env.Append(ACSUBST = {'TOP_BUILDDIR': env['RA_RELATIVE_BUILD_DIR']})
    env.Append(ACSUBST = {'TOP_SRCDIR': env['RA_TOP_DIR']})
    env.Append(ACSUBST = {'abs_top_srcdir': env['RA_TOP_DIR']})
    env.Append(ACSUBST = {'VERSION': env['version']})
    env.Append(ACSUBST = {'PY_PKG_DIR': os.path.join(env['RA_LIB_DIR'], 'lib/python/site-packages/')})
    env.Append(ACSUBST = {'DOTEST_LOGGER_CONF': env['RA_TOP_DIR'] + '/ra/test/ra_log.conf'})

def generateFile(env, target, source): 
    srcfile = open(source, 'r') 
    destfile = open(target, 'w')
    acsustdict = {} 
    if 'ACSUBST' in env: 
        if type(env['ACSUBST'] ) == type({}): 
            acsustdict = env['ACSUBST']   
    
    for line in srcfile.readlines(): 
        for key, value in acsustdict.items(): 
            line = re.sub( '@' + key + '@', value, line) 
        destfile.write(line) 
    srcfile.close() 
    destfile.close() 
        
