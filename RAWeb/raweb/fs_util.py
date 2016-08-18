#!/bin/env python
# -*- coding:utf-8 -*-
import subprocess
import string
import re

class Process:
    def run(self, cmd):
        """Fork a subprocess and run.
        @param cmd: Cmd.
        @return: Command status, stdout, stderr.
        @rtype: boolean, string, string.
        """
        _proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
        return  _proc.stdout.read(), _proc.stderr.read(), _proc.wait()

class FsUtilDelegate(object):
    def __init__(self, fsUtilExe, libPath):
        self.exe = fsUtilExe
        self.libPath = libPath
        self.process = Process()
        self.__initCmd()

    def __initCmd(self):
        if (self.libPath == None or self.libPath == "" ):
            envCmd = ""
        else:
            envCmd = "/bin/env LD_LIBRARY_PATH=%(libPath)s " % \
                {'libPath': self.libPath}
        self.cmd = envCmd + self.exe + ' '

    #Return true if path is directory, return false if path does not exist or path is file;
    #otherwise raise exception 
    def isDir(self, path):
        if not self.exists(path):
            return False
        cmd = "ls '" + path + "'"
        out, error, code = self.process.run(self.cmd + cmd)
        if int(code) == 11: #path is not directory
            return False
        elif code != 0:
            errorMsg = "ERROR: error message[" + error + "], error code[" + str(code) + "]"
            raise Exception, errorMsg
        return True
        
    #will not raise exception
    def copy(self, srcPath, destPath, overwrite = False):
        try:
            if self.exists(destPath) and overwrite:
                if not self.remove(destPath):
                    err = "ERROR: failed to copyDir %s to %s, error[cann't remove existing dir %s]" % (
                        srcPath, destPath, destPath)
                    print err
                    return False
        except Exception, exceptInfo:
            print exceptInfo
            return False

        cpCmd = "cp -r '" + srcPath + "' '" + destPath + "'"
        out, error, code = self.process.run(self.cmd + cpCmd)
        if code != 0:
            errorMsg = "copy from [" + srcPath + "] to [" + destPath  \
                +" ] failed! error message[" + error + "], error code[" + str(code) + "]"
            print errorMsg
            return False
        return True
    
    #return True if path exists, return False if path does not exists,
    #otherwise raise an exception
    def exists(self, path):
        cmd = "isexist '" + path + "'"
        out, error, code = self.process.run(self.cmd + cmd)
        if int(code) == 7: #path does not exist
            return False
        if int(code) == 16: #path does exist
            return True

        print "ERROR: failed to check exists, path:", path
        errorMsg = "ERROR: error message[" + error + "], error code[" + str(code) + "]"
        raise Exception, errorMsg

    #not raise exception
    def mkdir(self, path, recursive = True):
        if recursive:
            cmd = "mkdir -p '" + path + "'"
        else:
            cmd = "mkdir '" + path + "'"          

        out, error, code = self.process.run(self.cmd + cmd)
        if code != 0:
            return False
        return True

    def rename(self, srcPath, destPath):
        cmd = "rename '" + srcPath + "' '" + destPath + "'"
        out, error, code = self.process.run(self.cmd + cmd)
        if code != 0:
            return False 
        return True

    def listDir(self, path, sepStr = None):
        if not self.exists(path):
            return None
        cmd = "ls '" + path + "'"
        out, error, code = self.process.run(self.cmd + cmd)
        if code != 0:
            errorMsg = "ERROR: error message[" + error + "], error code[" + str(code) + "]"
            raise Exception, errorMsg
        return out.split(sepStr)
        
    #Return true if remove path success, return false if path does not exist;
    #otherwise raise exception 
    def remove(self, path):
        cmd = "rm '" + path + "'"
        out, error, code = self.process.run(self.cmd + cmd)
        if code == 10: #path does not exist
            return False
        elif code != 0:
            errorMsg = "ERROR: error message[" + error + "], error code[" + str(code) + "]"
            raise Exception, errorMsg
        return True

    #not raise exception
    def cat(self, filePath):
        catCmd = "cat '" + filePath + "'"
        (stdout, stderr, returncode) = self.process.run(self.cmd + catCmd)
        if returncode == 0:
            return stdout.rsplit('\n', 1)[0]
        else:
            # print "ERROR: cat file [%s] failed, %s %s" % (filePath, stdout, stderr)
            return None

    def normalizeDir(self, dirName):
        if dirName and not dirName.endswith('/'):
            return dirName + '/'
        return dirName

    def getMaxConfigVersion(self, path):
        versions = self.getConfigVersionList(path)
        if (len(versions) == 0):
            return -1

        return max(versions)
    
    def getConfigVersionList(self, path):
        versions = self.listDir(path, '\n')
        if versions == None:
            errorMsg = "ERROR: visit file system failed! path=%s"%(path)
            raise Exception, errorMsg

        ret = []
        for i in versions:
            if i.isdigit():
                ret.append(string.atoi(i))
        return ret
