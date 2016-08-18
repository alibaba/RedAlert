import sys
import os
import ConfigParser

class loadConfig:
    def __init__(self):
        self.conf = ConfigParser.ConfigParser()
        confDir = os.path.abspath(os.path.join(os.path.dirname(__file__),"../conf"))
        try:
            self.conf.read(os.path.join(confDir,"red_alert_web.conf"))
        except:
            raise("CONFIG ERROR")
        self.sections = self.conf.sections()
        if "PathConfig" not in self.sections or "Parameter" not in self.sections:
            raise("CONFIG SESSIONS ERROR")

    def getRedAlertWebWorkRoot(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebWorkRoot".lower() not in options:
            raise("CONFIG RedAlertWebWorkRoot OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebWorkRoot")

    def getRedAlertWebConfDir(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebConfDir".lower() not in options:
            raise("CONFIG RedAlertWebWebConfDir OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebConfDir")

    def getRedAlertWebTablePath(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebTablePath".lower() not in options:
            raise("CONFIG RedAlertWebTablePath OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebTablePath")

    def getRedAlertWebAuxPath(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebAuxPath".lower() not in options:
            raise("CONFIG RedAlertWebAuxPath OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebAuxPath")

    def getRedAlertWebJsonPath(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebJsonPath".lower() not in options:
            raise("CONFIG RedAlertWebJsonPath OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebJsonPath")

    def getRedAlertWebVersionDir(self):
        options = self.conf.options("PathConfig")
        if "RedAlertWebVersionDir".lower() not in options:
            raise("CONFIG RedAlertWebVersionDir OPTION ERROR")
        return self.conf.get("PathConfig", "RedAlertWebVersionDir")

    def getrunPort(self):
        options = self.conf.options("Parameter")
        if "runPort".lower() not in options:
            raise("CONFIG runPort OPTION ERROR")
        return self.conf.get("Parameter", "runPort")

    def getMaxRaBackEnd(self):
        options = self.conf.options("Parameter")
        if "MaxRaBackEnd".lower() not in options:
            raise("CONFIG MaxRaBackEnd OPTION ERROR")
        return self.conf.get("Parameter", "MaxRaBackEnd")

    def getRedAlertStoragePath(self):
        options = self.conf.options("Parameter")
        if "RedAlertStoragePath".lower() not in options:
            raise("CONFIG RedAlertStoragePath OPTION ERROR")
        return self.conf.get("Parameter", "RedAlertStoragePath")

    def getfsUtil(self):
        if "fsLib" in self.sections:
            if "fsUtil".lower() in self.conf.options("fsLib"):
                return self.conf.get("fsLib", "fsUtil")
lc = loadConfig()

confDir = lc.getRedAlertWebWorkRoot()
raConfDir = lc.getRedAlertWebConfDir()
tablePath = lc.getRedAlertWebTablePath()
tmpConfDir = lc.getRedAlertWebVersionDir()
auxTablePath = lc.getRedAlertWebAuxPath()
raWebPath = lc.getRedAlertWebJsonPath()
runPort = int(lc.getrunPort())
MAX_RA_SERVER = lc.getMaxRaBackEnd()
storagePath = lc.getRedAlertStoragePath()
fs_util = lc.getfsUtil()

if __name__ == '__main__':
    print confDir
    print raConfDir
    print tablePath
    print tmpConfDir
    print auxTablePath
    print raWebPath
    print runPort
    print MAX_RA_SERVER
    print storagePath
    print fs_util
