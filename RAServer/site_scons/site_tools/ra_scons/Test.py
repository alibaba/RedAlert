import os, copy
import SCons.Script
from SCons.Builder import Builder
def generate(env):
    runUnitBuilder = Builder(action = runUnitTest) 
    env.Append(BUILDERS = {"runUnitTest" : runUnitBuilder})  
    UnitTestBuilder = Builder(action = UnitTest)
    #env.Append(BUILDERS = {"UnitTest" : UnitTest})
    env.AddMethod(UnitTest)
def exists(env):
    return 1

def runUnitTest(env, target, source): 
    cmd = 'env' 
    for item in env['ENV'].items(): 
        cmd += (' %s=%s' % item)

    if env['heapchecktype'] == 'tcmalloc': 
        lib_path = os.path.join(str(env['prefix']),  'usr/local/lib64')
        cmd += ' PPROF_PATH=/usr/bin/pprof' 
        cmd += ' LD_PRELOAD=' + lib_path + '/libtcmalloc.so' 
        cmd += ' HEAPCHECK=normal' 
    elif env['heapchecktype'] == 'valgrind': 
        cmd += ' valgrind --error-exitcode=11' 
        if 'valgrindleakcheck' in env: 
            cmd += ' --leak-check=%s' % env['valgrindleakcheck'] 
 
    cmd += ' %s' 
    for p in source:
        dirname = os.path.dirname(p.abspath) 
        print "ATest: Entering directory `%s'" % dirname 
        curCmd = copy.deepcopy(cmd) % p.abspath 
        print curCmd 
        ret = os.system(curCmd) 
        print "ATest: Leaving directory `%s'" % dirname 
        if ret != 0: 
            return ret 
    return 0    

def UnitTest(env, target, source, LIBS): 
    if type(target) != type([]):
        target = [target]
    for p in target:
        programList = env.Program(target = p, source = _renameDoTest(env, target, source), LIBS = LIBS)
        vTarget = env.runUnitTest(target = str(p) + '_dummy', source = programList) 
        env.Alias('test', vTarget)
        
    return target

def _renameDoTest(env, target, sources):
    ret = []
    for source in sources:
        if source == env['RA_DO_TEST']:
            tDoTest = "RA_DO_TEST" + "_" + target[0]
            ret.append(env.Object(tDoTest, source))
        else:
            ret.append(source)
    return ret
