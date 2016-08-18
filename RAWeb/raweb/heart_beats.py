import sys
import os
import time
import logging
import threading
from raweb.models import RedAlert
from raweb.view_util import getSession
from raweb.load_raweb_json import getModelsClass
aliveList = []
nodeID = 1


class ServiceNode:
    def __init__(self, max_ra_server, tablePath, versionDir):
        global aliveList
        self.aliveList = aliveList
        self.MAX_RA_SERVER = max_ra_server
        self.lock = threading.Lock()
        self.tablePath = tablePath
        self.versionDir = versionDir

    def maxVersion(self):
        dirs = [f for f in os.listdir(self.versionDir) if not f.startswith('.')]
        if not dirs:
            return -1
        ver = map(lambda x: int(x), dirs)
        return max(ver)

    def peek(self):
        global aliveList
        self.aliveList = aliveList
        return self.aliveList

    def addNode(self, address, interval, version):
        logging.info("ADDING NODE (%s, %s, %s)" % (address, interval, version))
        global nodeID
        global aliveList
        node = {}
        node['address'] = address
        node['interval_s'] = interval
        node['version'] = version
        node['status'] = 1
        node['time'] = time.time()
        self.lock.acquire()
        node['id'] = nodeID
        aliveList.append(node)
        nodeID = nodeID + 1
        self.lock.release()
        self.dumpsRedAlert()

    def dumpsRedAlert(self):
        global aliveList
        self.session = getSession(self.tablePath)
        self.session.query(getModelsClass('RedAlert')).delete()
        dumpList = self.selectAlive()
        for item in dumpList:
            RedAlertItem = {}
            RedAlertItem['address'] = item['address']
            RedAlertItem['service'] = item['id']
            RedAlertItem['weight'] = 100
            RedAlertItem['author'] = 'admin'
            RedAlertItem['modifiedTime'] = ''
            self.session.add(RedAlert(RedAlertItem))
        self.session.flush()
        self.session.commit()
        return True

    def updateNode(self, address, interval, version):
        logging.info("UPDATE NODE (%s, %s, %s)" % (address, interval, version))
        Flag = False
        Conflict = False
        global aliveList
        self.lock.acquire()
        for node in aliveList:
            if node['address'] == address:
                node['time'] = time.time()
                node['interval_s'] = interval
                node['version'] = version
                if node['status'] == 0:
                    node['status'] = 1
                    Flag = True
                if node['status'] == 1 and node['version'] != self.maxVersion():
                    node['version'] = self.maxVersion()
                    Conflict = True
                break
        self.lock.release()
        if Flag:
            self.dumpsRedAlert()
        return Flag, Conflict

    def checkAlive(self):
        Flag = False
        global aliveList
        self.lock.acquire()
        for node in aliveList:
            if (float(time.time() - node['time'])) > (float(node['interval_s']))*3 and node['status'] == 1:
                node['status'] = 0
                Flag = True
        self.lock.release()
        if Flag:
            self.dumpsRedAlert()
        return Flag

    def selectAlive(self):
        global aliveList
        ret = []
        self.lock.acquire()
        map(lambda x: ret.append({'id': x['id'], 'address': x['address'], 'version': x['version']}), filter(lambda x: x['status'] == 1, aliveList))
        self.lock.release()
        ret.sort(key=lambda x: x['id'])
        if len(ret) > self.MAX_RA_SERVER:
            ret = ret[:self.MAX_RA_SERVER]
        return ret

    def selectAll(self):
        global aliveList
        self.lock.acquire()
        ret = filter(lambda x: x['status'] == 1, aliveList)
        self.lock.release()
        ret.sort(key=lambda x: x['id'])
        return ret

    def getSpecs(self):
        ret = self.selectAlive()
        addressList = []
        map(lambda x: addressList.append(str(x['address'].decode('utf8'))), ret)
        #logging.info("GET LIMITED ADDRESS LIST %s"%(str(addressList)))
        return addressList

    def getAllSpecs(self):
        ret = self.selectAll()
        addressList = []
        map(lambda x: addressList.append(str(x['address'].decode('utf8'))), ret)
        #logging.info("GET ALL ADDRESS %s"%(str(addressList)))
        return addressList


def clear():
    global aliveList
    aliveList = []
