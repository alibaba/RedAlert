import sys
import os
import time
import raweb.parse as conf
import shutil
import threading
import logging


class storage_interface(object):
    def __init__(self):
        self.lock = threading.Lock()

    def listDir(self, dstDir):
        versionList = None
        if os.path.isdir(dstDir):
            versionList = map(lambda x: int(x) if x.isdigit() else -1, os.listdir(dstDir))
        return versionList

    def reloadFile(self, src, dst):
        #logging.info("reloadFile from %s to %s" % (src, dst))
        if os.path.isfile(src) and os.path.isdir(dst):
            shutil.copy(src, dst)

    def upload(self, src):
        #logging.info("upload config from %s to %s" % (src, conf.storagePath))
        self.lock.acquire()
        schema, path = conf.storagePath.split(":/")
        if schema == 'file' and os.path.exists(path):
            try:
                newVersion = path + '/' + src.split('/')[-1]
                if os.path.exists(newVersion):
                    shutil.rmtree(newVersion)
                os.mkdir(newVersion)
                shutil.copy(src + '/sqlite', newVersion)
                return True, newVersion
            except Exception, e:
                print e
                return False, "UPLOAD_ERROR"
            finally:
                self.lock.release()
        elif schema == 'pangu':
            try:
                import raweb.fs_util
                self.fsUtil = raweb.fs_util.FsUtilDelegate(conf.fs_util, '')
                if self.fsUtil.isDir(conf.storagePath) and self.fsUtil.copy(src, conf.storagePath):
                    return True, conf.storagePath + '/' + os.path.basename(src)
            except Exception ,e:
                print e
                return False, "UPLOAD_ERROR"
            finally:
                self.lock.release()
        return False, "UNKNOWN_SCHEMA"

    def deployNewVersion(self, src, dst):
        #logging.info("[LOCAL]  DEPLOY NEW VERSION FROM %s TO %s" % (src, dst))
        sqliteFile = src + '/sqlite'
        if os.path.isfile(sqliteFile) and os.path.isdir(dst):
            self.lock.acquire()
            versionList = self.listDir(dst)
            if not versionList:
                Latest = dst + '/1'
            else:
                Latest = dst + '/' + str(max(versionList) + 1)
            os.mkdir(Latest)
            shutil.copy(sqliteFile, Latest)
            self.lock.release()
            return True, Latest
        else:
            return False, "DEPLOY FAILED"
