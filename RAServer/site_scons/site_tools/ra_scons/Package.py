# -*- mode: python -*-
import sys, os, subprocess
import SCons.Script
import SCons.Environment
import SCons.Defaults
from SCons.Builder import Builder
def generate(env):
    env.Append(BUILDERS = {"aTar":Builder(action= " tar -f ${TARGET} $TARFLAGS .")})
    env.Append(BUILDERS = {"aCopy":Builder(action= "if [ ! -f ${TARGET} ]; then  cp -r ${SOURCE} ${TARGET}; fi")})
    env.Append(BUILDERS = {"xLink":Builder(action= "cd ${LINK_DIR}; if [ -f ${TARGET_FILE} ]; then rm  ${TARGET_FILE};fi; ln -s  ${SOURCE_FILE} ${TARGET_FILE}; cd -")})
    env.Append(BUILDERS = {"installToPackage" : installToPackage})
    env.Append(BUILDERS = {"installDependLibrary" : installDependLibrary})
    env.Append(BUILDERS = {"package" : package})

def exists(env):
    return 1

def installToPackage(env, packageName, source, dirInPackage):
    if type(source) != type([]):
        source = [source]
    for p in packageName:
        vTarget = env.Install(env['RA_PACKAGES_STAGING_DIR'] + "/" + p + "/" +dirInPackage, source)
        env.Depends(packageName, vTarget)
    return packageName

def installDependLibrary(env, target, libs, dirInPackage):
    if type(libs) != type([]): 
        libs = [libs]
    libSearchDirs = []
    for x in [env["prefix"], ""]:
        for i in ["lib", "usr/lib", "usr/lib64", "usr/local/lib", "usr/local/lib64"]:
            libSearchDirs.append(os.path.abspath("%s/%s"%(x,i)))

    for p in target:
        destDir = env.Dir(env['RA_PACKAGES_STAGING_DIR']).abspath + "/" + p + "/" + dirInPackage
        for lib in libs:
            found = False
            for s in libSearchDirs:
                libName = lib
                if not libName.startswith("@"):
                    libName = "lib" + lib + ".so"
                else:
                    libName = libName.replace("@", "")
                sourcePath = s + "/" + libName
                
                if os.path.exists(sourcePath):
                    found = True 
                    cname = _getCanonicalizedName(sourcePath)
                    if len(cname) == 0:
                        print "Invalidate sourcePath %s" %sourcePath
                        raise
                    cNameBase = os.path.basename(cname)
                    targetFileName = destDir + "/" + cNameBase
                    
                    copyTarget = env.aCopy(targetFileName, cname)
                    if cname == sourcePath:
                        break

                    link1Target = env.xLink(target = targetFileName + "_fake1", source = targetFileName, LINK_DIR = destDir, SOURCE_FILE = cNameBase, TARGET_FILE = libName)
                    env.Depends(link1Target, copyTarget)
                    soName = _getSoName(cname)
                    if soName != cNameBase:
                        link2Target = env.xLink(target = targetFileName + "_fake2", source = targetFileName, LINK_DIR = destDir, SOURCE_FILE = cNameBase, TARGET_FILE = soName)
                        env.Depends(link2Target, copyTarget)
                    break

def package(env, target, source):
    for p in source:
        tarFileName = env['RA_PACKAGES_DIR'] + "/" + p + ".tar.gz"
        dirPath = env['RA_PACKAGES_STAGING_DIR'] + "/" + p
        env["TARFLAGS"] = "-cz -C %s " %(env.Dir(dirPath).abspath)
        package_staging = env.Dir(dirPath) 
        vTarget = env.aTar(tarFileName, package_staging)
        env.Alias('package', vTarget)
    
    return target

def _getSoName(pathName):
    cmd = "readelf -d %s | grep 'Library soname' | awk '{print $NF}' | sed 's/\[//' | sed 's/\]//'" %pathName
    return _runCommand(cmd).rstrip("\n")

def _getCanonicalizedName(pathName):
    return _runCommand("readlink -f %s" %pathName).rstrip("\n") 

def _installLink(linkDir, source):
    sourceBase = os.path.basename(source)
    cName = _getCanonicalizedName(source)
    cNameBase =  os.path.basename(cname)
    targetFileName = os.path.join(linkDir + cNameBase)
    copyTarget = env.aCopy(targetFileName, cname)
    link2Target = env.xLink(target = targetFileName + "_fake2", source = targetFileName, LINK_DIR = destDir, SOURCE_FILE = cNameBase, TARGET_FILE = sourceBase)
    env.Depends(link2Target, copyTarget)

def _searchApproximateSo(dirPath, lib):
    ret = []
    cmd = "ls  %s/lib%s\.so* 2>/dev/null | cat" %(dirPath, lib)
    fileList = _runCommand(cmd).rstrip("\n")
    if len(fileList) == 0:
        return ret

    for fileName in fileList.split("\n"):
        ret.append(fileName)
    return ret

def _isLink(path):
    cmd = "test -h %s; echo $?" %path
    return _runCommand(cmd) == "0"

def _MakeLink(source, target):
    cmd = "ln -s %s %s" %(source, target)
    _runCommand(cmd)

def ACopy(env, source, target):
    cmd = "cp -r %s %s" %(source, target)
    _runCommand(cmd)

def _runCommand(cmd):
    _proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    out = _proc.stdout.read()
    err = _proc.stderr.read()
    ret = _proc.wait()
    if ret:
        print "Fail to excute %s. err=%s, out=%s,code=%s" %(cmd, err, out, str(ret))
        raise 
    
    return out
