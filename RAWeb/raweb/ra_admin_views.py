from __future__ import with_statement
import time
import logging
import sys
import time
import re
import os
import shutil
import commands
from werkzeug.wrappers import Request, Response
from werkzeug.routing import Map, Rule
import simplejson as json
from raweb.load_raweb_json import *
from raweb.view_util import *
from raweb.error_type import ErrorType
from raweb.parse import storagePath
import httplib
import urllib


class RaAdminClass():
    def __init__(self, jsonConfDir, tmpConfDir, tablePath, auxTablePath, serNode, storage):
        self.url_map = []
        self.url_map.append(Rule('/admin/deploy', endpoint=self.deploy))
        self.url_map.append(Rule('/admin/get_status', endpoint=self.get_status))
        self.url_map.append(Rule('/admin/reload', endpoint=self.reload))
        self.url_map.append(Rule('/admin/diff', endpoint=self.diff))
        self.url_map.append(Rule('/admin/recover', endpoint=self.recover))     
        self.url_map.append(Rule('/admin/reload_latest', endpoint=self.reload_latest))
        self.url_map.append(Rule('/admin/heartbeats', endpoint=self.heartbeats))
        self.jsonConfDir = jsonConfDir
        self.tmpConfDir = tmpConfDir
        self.tablePath = tablePath
        self.auxTablePath = auxTablePath
        self.serverNode = serNode
        self.storage = storage

    def init(self, author):
        self.author = author
        self.session = getSession(self.tablePath)
        self.auxSession = getSession(self.auxTablePath)

    def sendHttpRequest(self, ipList, path):
        httpClient = None
        logging.info("sent http reload path command ( %s ) to %s" % (path, str(ipList)))
        if not ipList:
            logging.info("NO Alive Bankend")
            return False
        for ip in ipList:
            try:
                params = json.dumps({'configPath': '%s' % (path)})
                headers = {"Content-type": "application/json", "Accept": "text/plain"}
                httpClient = httplib.HTTPConnection(ip.split(":")[0], ip.split(":")[1])
                httpClient.request("POST", "/api/load", params, headers)
                response = httpClient.getresponse()
                if response.status != 200:
                    return False
            except Exception, e:
                print e
            finally:
                if httpClient:
                    httpClient.close()
        return True

    def close(self):
        if self.auxSession:
            self.auxSession.close()
        if self.session:
            self.session.close()

    @commonReturn(readOnly=False)
    def deploy(self, request):
        logging.info("user[%s] %s end at: %0.3f %0.3f" % (self.author, request, time.time(), time.clock()))
        tables = ["Policy", "Shield", "RedAlert", "Pair", "DataSource"]
        ret = {}
        ret['ret'] = 0
        ret['msg'] = 'succ'
        ret["items"] = {}
        for table in tables:
            result, diffItems = deployTable(self.session, table, self.auxSession, "Aux" + table, self.author)
            ret["items"][table] = diffItems
        output, msg = self.commonDeploy(self.jsonConfDir)
        if output is False:
            ret["ret"] = -1
            ret["msg"] = msg
        ret["msg"] = msg
        return ret

    @commonReturn(readOnly=False)
    def diff(self, request):
        logging.info("user[%s] %s end at: %0.3f %0.3f" % (self.author, request, time.time(), time.clock()))
        ret = {}
        ret["items"] = {}
        tables = ["Policy", "Shield", "RedAlert", "Pair", "DataSource"]
        for table in tables:
            result, queryItems = diffTable(self.session, table, self.auxSession, "Aux" + table, self.author)
            ret["items"][table] = queryItems
        ret["ret"] = 0
        ret["msg"] = "succ"
        return ret

    def commonDeploy(self, jFileDir):
        ret, Latest = self.storage.deployNewVersion(jFileDir, self.tmpConfDir)
        if not ret:
            return False, Latest
        logging.info("deploy new local version... %s" % Latest)
        ipList = self.serverNode.getAllSpecs()
        re, Latest = self.storage.upload(Latest)
        if not re or not self.sendHttpRequest(ipList, Latest):
            ret = False
        return ret, Latest

    def copyConfFiles(self, tmpVersionDir):
        filelist = os.listdir(tmpVersionDir)
        for line in filelist:
            f = os.path.join(tmpVersionDir, line)
            if os.path.isfile(f):
                shutil.copy(f, self.jsonConfDir)

    @commonReturn(readOnly=True)
    def get_status(self, request):
        ret = {}
        ret['ret'] = 0
        ret['machines'] = []
        aliveNode = self.serverNode.selectAlive()
        for node in aliveNode:
            machineInfo = {}
            machineInfo['Machine'] = str(node['address'].decode('utf8'))
            machineInfo['MachineID'] = int(node['id'])
            machineInfo['ErrorMessage'] = ''
            machineInfo['verisonInfo'] = int(node['version'])
            machineInfo['Pid'] = ' RemoteService'
            ret['machines'].append(machineInfo)
        ret['versions'] = self.storage.listDir(self.tmpConfDir)
        if len(ret["versions"]) > 100:
            ret['versions'] = sorted(ret["versions"], reverse=True)[:100]
        else:
            ret['versions'].sort(reverse=True)
        msg = 'Available Machines: \n\t'
        for item in ret['machines']:
            msg = msg + "Machine " + str(item['Machine']) + ' MachineID ' + str(item['MachineID']) + ' MachineVersion ' + str(item['verisonInfo'])  + '\n\t'
        msg = msg + '\nAvailable versions %s\n\t'%(storagePath)
        for item in ret['versions']:
            msg = msg + str(item) + '\n\t'
        ret['msg'] = msg
        return ret

    @commonReturn(readOnly=False)
    def reload(self, request):
        ret = {}
        ret['ret'] = 0
        ret['msg'] = 'succ'
        logging.info("user[%s] %s end at: %0.3f %0.3f" % (self.author, request, time.time(), time.clock()))
        args = request.args
        if not args.has_key("version") or args["version"] == "":
            raise RaRequestError(ErrorType.RL_ERROR, "no version in url parameters")
        version = int(args["version"])
        if version < 0:
            raise RaRequestError(ErrorType.DP_ERROR, "version should greater than or equal 0")
        versionList = self.storage.listDir(self.tmpConfDir)
        if version not in versionList:
            raise RaRequestError(ErrorType.DP_ERROR, "no such version")
        self.storage.reloadFile(self.tmpConfDir + '/'+str(version) + '/sqlite', self.jsonConfDir)
        self.serverNode.dumpsRedAlert()
        val, Latest = self.storage.deployNewVersion(self.jsonConfDir, self.tmpConfDir)
        if val is False:
            ret['msg'] = Latest
        nodeList = self.serverNode.getAllSpecs()
        re, Latest = self.storage.upload(Latest)
        logging.info("upload %s return %s"%(Latest, re))
        if not re or not self.sendHttpRequest(nodeList, Latest):
            ret['ret'] = -1
            ret['msg'] = 'reload failed'
        return ret

    @commonReturn(readOnly=False)
    def recover(self, request):
        tables = ["Policy", "Shield", "RedAlert", "Pair", "DataSource"]
        for table in tables:
            auxClass = getModelsClass("Aux" + table)
            self.auxSession.query(auxClass).filter(auxClass.author == self.author).delete()
            self.auxSession.flush()
        self.auxSession.commit()
        ret = {}
        ret["ret"] = 0
        ret["msg"] = "recover succ!"
        return ret

    @commonReturn(readOnly=False)
    def reload_latest(self, request):
        ret = self.common_reload_latest(self.serverNode.getAllSpecs())
        return ret

    def common_reload_latest(self, ipList, Flag=False):
        logging.info("RELOAD LATEST VERSION")
        if Flag:
            self.storage.deployNewVersion(self.jsonConfDir, self.tmpConfDir)
        ret = {}
        ret['ret'] = 0
        ret['msg'] = 'succ'
        maxVersion = max(self.storage.listDir(self.tmpConfDir))
        retVersionDir = self.tmpConfDir + '/' + str(maxVersion)
        re, upLoadDir = self.storage.upload(retVersionDir)
        if not re or not self.sendHttpRequest(ipList, upLoadDir):
            ret['ret'] = -1
            ret['msg'] = 'Reload Failed'
        else:
            ret['msg'] = "Reload Version: %s success" % (str(maxVersion))
            logging.info( "RELOAD VERSION: %s SUCC AND INFORM %s" % (str(maxVersion), str(ipList)))
        return ret
        
    @commonReturn(readOnly=True)
    def heartbeats(self, request):
        ret = {}
        ret['ret'] = 0
        ret['msg'] = 'succ'
        data = json.loads(request.data.decode('utf8'))
        print "recv data:", data
        logging.info("RECV HEARTBEATS PACKAGE...CONTENT IS %s" % data)
        if request.method == 'POST':
            iplist = []
            if self.serverNode.aliveList:
                map(lambda x: iplist.append(x['address']), self.serverNode.aliveList)
                if data['address'] not in iplist:
                    self.serverNode.addNode(data['address'], data['interval'], data['version'])
                    print "A"
                    self.common_reload_latest(self.serverNode.getAllSpecs(), Flag=True)
                else:
                    Flag, Conflict = self.serverNode.updateNode\
                                     (data['address'], data['interval'], data['version'])
                    if Flag:
                        print "B"
                        self.common_reload_latest(self.serverNode.getAllSpecs(), Flag=True)
                    elif not Flag and Conflict:
                        self.common_reload_latest([str(data['address'].decode('utf8'))])
            else:
                self.serverNode.addNode(data['address'], data['interval'], data['version'])
                print "C"
                self.common_reload_latest(self.serverNode.getAllSpecs(), Flag=True)
            if self.serverNode.checkAlive():
                print "D"
                self.common_reload_latest(self.serverNode.getAllSpecs(), Flag=True)
            ret['result'] = self.serverNode.selectAlive()
            ret = ret['result']
        else:
            ret['ret'] = -1
            ret['msg'] = 'error '
            ret['result'] = ''

        return ret
        
if __name__ == "__main__":
    print "OK"
