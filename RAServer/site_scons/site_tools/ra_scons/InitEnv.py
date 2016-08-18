import os
import SCons.Environment
import SCons.Defaults

def _parseRaBuildOption(env):
    if not ("ra_build_options" in os.environ):
        print "ra_build_options is not at os.environ"
        exit(1)

    items = os.environ['ra_build_options'].split("\n")
    for item in items:
        item = item.replace(" ", "")
        if len(item) == 0:
            continue
        parts = item.split("=")
        if len(parts) == 0:
            continue
        key = parts[0]
        if len(parts) == 1:
            env[key] = ""
            continue
        value = parts[1]
        if value.startswith("'"):
            env[key] = value.replace("'", "")
            continue
        env[key] = value.lower() in ("yes", "true")

def generate(env):
    _parseRaBuildOption(env)
    initBuildDirectory(env)
    initBuildOption(env)

def exists(env):
    return 1

def _parsePaths(pathStr):
    if len(pathStr) == 0:
        return []

    return pathStr.split()

def initBuildDirectory(env): 
    env['RA_BUILD_MODE'] = env['mode']
    env['RA_RELATIVE_BUILD_DIR'] = 'build/' + env['RA_BUILD_MODE'] 
    env['RA_BUILD_DIR'] = '#' + env['RA_RELATIVE_BUILD_DIR'] 
    env['RA_PACKAGES_DIR'] = env['RA_BUILD_DIR'] + '/packages' 
    env['RA_BIN_DIR'] = env['RA_BUILD_DIR'] +'/bin' 
    env['RA_LIB_DIR'] = env['RA_BUILD_DIR'] +'/lib' 
    env['RA_TOP_DIR'] = env.Dir('#').abspath 
    env['RA_ABS_BUILD_DIR'] = env['RA_TOP_DIR'] + '/' + env['RA_BUILD_DIR'][1:] 
    env['RA_PACKAGES_STAGING_DIR'] = env['RA_PACKAGES_DIR'] + '/staging' 
    env['RA_UNITTEST_DIR'] = env['RA_BUILD_DIR']+'/unittest'
    env['RA_DO_TEST'] = env['RA_TOP_DIR'] + "/ra/test/dotest.cpp"

def initBuildOption(env):
    env.AppendUnique(CXXFLAGS='-Woverloaded-virtual')
    env.AppendUnique(CCFLAGS ='-Wall')
    env.Append(CXXFLAGS = '-fpermissive -Wno-deprecated -Wno-strict-aliasing -fPIC')
    env.Append(CXXFLAGS = '-include unistd.h -include limits.h -include stdlib.h -include stdio.h -include string.h -include stdint.h')
    env.Append(CXXFLAGS = '-include limits -include algorithm -include vector -include map -include set')

    prefixPath = env['prefix']
    libpath = env['libpath']
    includeheader = env['includeheader']
    for headerPath in _parsePaths(includeheader):
        env.Append(CXXFLAGS = ("-I%s"%headerPath))
    
    if len(prefixPath) != 0:
        for p in ["include", "usr/include", "usr/local/include"]:
            env.Append(CXXFLAGS = ("-I%s/%s" %(prefixPath, p)))
            env.Append(CXXFLAGS = ("-I/%s" %(p)))
            
    env.Append(CXXFLAGS = ("-I%s" %env["RA_TOP_DIR"]))
    env.Append(CXXFLAGS = ("-I%s" %env.Dir(env["RA_BUILD_DIR"]).abspath))
   
    env.Append(CXXFLAGS = ("-L%s" %env.Dir(env['RA_LIB_DIR']).abspath))
    libPaths = _parsePaths(libpath)
    for libPath in libPaths:
        env.Append(CXXFLAGS = ("-L%s"%libPath))

    if len(prefixPath) != 0: 
        for p in ["lib", "usr/lib", "usr/lib64", "usr/local/lib", "usr/local/lib64"]:
            env.Append(LIBPATH = ["%s/%s" %(prefixPath, p)])
            env.Append(CXXFLAGS = ("-L%s/%s" %(prefixPath, p)))
            env.Append(LIBPATH = ["/%s" %(p)])
            env.Append(CXXFLAGS = ("-L/%s" %(p)))

    env.Append(LIBPATH = ["%s" %env.Dir(env['RA_LIB_DIR']).abspath])
    
    env.PrependENVPath("LD_LIBRARY_PATH", "%(prefixPath)s/lib:%(prefixPath)s/usr/lib/:%(prefixPath)s/usr/lib64/:%(prefixPath)s/usr/local/lib/:%(prefixPath)s/usr/local/lib64/:%(genLibDir)s:/usr/lib:usr/lib64:/usr/local/lib/:/usr/local/lib64" %{"prefixPath":prefixPath, "genLibDir":env.Dir(env['RA_LIB_DIR']).abspath})

    env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1
