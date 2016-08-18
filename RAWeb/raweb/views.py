from __future__ import with_statement
import simplejson as json
import time
import logging
import sqlite3
from werkzeug.wrappers import Request, Response
from werkzeug.routing import Map, Rule
from raweb.load_raweb_json import *
from raweb.view_util import *
import sqlalchemy
import datetime
from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine
from models import *
import httplib
import urllib

class MetricsFetchAPI:
    def __init__(self, specs):
        self.specs = specs
        self.specs.sort()

    def gethttpResponse(self, endpoint):
        conn = None
        ip, port = self.specs[0].split(':')
        try:
            conn = httplib.HTTPConnection(ip, port)
            conn.request("GET", endpoint)
            r1 = conn.getresponse()
            data1 = r1.read()
            return r1.status, r1.reason, data1
        except Exception, e:
            print e
            return -1, 'Connection failed', e
        finally:
            if conn:
                conn.close()

    def mergeMetricMap(self, dst, key, src):
        if not dst.has_key(key):
            dst[key] = {}
            dst[key].update(src)
            return dst

    def getMetrices(self, p, sTime, eTime):
        try:
            if p == '/':
                path = "/api/list?start=%s&end=%s" % (sTime, eTime)
                code, re, resp = self.gethttpResponse(path)
            else:
                p = urllib.quote_plus(p)
                path = "/api/list?path=%s&start=%s&end=%s" % (p, sTime, eTime)
                code, re, resp = self.gethttpResponse(path)
            return code, re, resp
        except Exception, e:
            return -1, "Get HttpResponse failed", e


class RaWebClass():
    def __init__(self, rawebConfValue, tablePath, auxTablePath, serNode):
        self.url_map = []
        self.url_map.append(Rule('/api/add_policy', endpoint=self.add_policy))
        self.url_map.append(Rule('/api/get_policies', endpoint=self.get_policies))
        self.url_map.append(Rule('/api/get_policy', endpoint=self.get_policy))
        self.url_map.append(Rule('/api/cancel_policy', endpoint=self.cancel_policy))        
        self.url_map.append(Rule('/api/group_move', endpoint=self.group_move))        
        self.url_map.append(Rule('/api/get_groups', endpoint=self.get_groups))
        self.url_map.append(Rule('/api/delete_policies', endpoint=self.delete_policies))
        self.url_map.append(Rule('/api/update_policy', endpoint=self.update_policy))
        self.url_map.append(Rule('/api/invalid_policy', endpoint=self.invalid_policy))
        self.url_map.append(Rule('/api/valid_policy', endpoint=self.valid_policy))
        self.url_map.append(Rule('/api/search_metric', endpoint=self.search_metric))
        self.url_map.append(Rule('/api/fetch_metrics', endpoint=self.fetch_metrics))

        self.rawebConfValue = rawebConfValue
        self.tablePath = tablePath
        self.auxTablePath = auxTablePath
        self.serviceNode = serNode
        self.session = None
        self.auxSession = None
        self.author = ""

    def init(self, author):
        self.session = getSession(os.path.join(self.tablePath))
        self.auxSession = getSession(os.path.join(self.auxTablePath))
        self.author = author

    def close(self):
        if self.auxSession:
            self.auxSession.close()
        if self.session:
            self.session.close()

    @commonReturn(readOnly=False)
    def update_policy(self, request):
        ret = {}
        args = request.args.to_dict()
        if not args.has_key("id") or args["id"] == "":
            raise RaRequestError(ErrorType.NO_SIDS_ERROR, "id is error")
        pid = int(args["id"])
        allItems = self.getAllItems()
        if pid not in allItems:
            raise RaRequestError(ErrorType.IDS_ERROR, "Error id input")
        oldItem = allItems[pid] 
        if "opType" in oldItem and oldItem["opType"] == "Delete":
            raise RaRequestError(ErrorType.IDS_ERROR, "id has been deleted")

        if args.has_key("effectiveTime"):
            if len(args["effectiveTime"]) == 0:
                args.pop("effectiveTime")
            else:
                isOK, effectiveValue = convertEffectiveTime(args["effectiveTime"])
                if not isOK:
                    raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "Error effectiveTime input")
                args["effectiveTime"] = effectiveValue

        if args.has_key("policyFilter"):
            if len(args["policyFilter"]) == 0:
                args.pop("policyFilter")
            else:
                isOK, policyFilterValue = convertPolicyShield(args["policyFilter"])
                if not isOK:
                    raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "Error policyFilter input")
                args["policyFilter"] = policyFilterValue

        isCheckOK, msg, item = checkItem(self.rawebConfValue, args)
        if not isCheckOK:
            raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, msg)

        ret["ret"] = 0
        ret["msg"] = "succ"
        item["author"] = self.author
        item["modifiedTime"] = oldItem["modifiedTime"]
        item["opType"] = "Update"
        if "opType" in oldItem:
            if oldItem["opType"] == "Add":
                item["opType"] = "Add"
            self.auxSession.query(AuxPolicy).\
                filter(AuxPolicy.id == pid).\
                filter(AuxPolicy.author == self.author).\
                update(AuxPolicy(item).toNormalJson())
        else:
            self.auxSession.add(AuxPolicy(item))
        
        self.auxSession.flush()
        self.auxSession.commit()
        return ret

    def getMaxId(self):
        defaultId = 1
        qry = self.session.query(sqlalchemy.sql.func.max(Policy.id).label("max_id"))
        if qry.count() >= 1:
            defaultId = max(defaultId, qry.one().max_id)
        qry = self.auxSession.query(sqlalchemy.sql.func.max(AuxPolicy.id).label("max_id"))
        if qry.count() >= 1:
            defaultId = max(defaultId, qry.one().max_id)
        return defaultId

    @commonReturn(readOnly=False)
    def add_policy(self, request):
        ret = {}
        self.init(self.author)
        args = request.args.to_dict()
        logging.info("user[%s] %s end at: %0.3f %0.3f" % \
                     (self.author, request, time.time(), time.clock()))
        if args.has_key("effectiveTime"):
            if len(args["effectiveTime"]) == 0:
                args.pop("effectiveTime")
            else:
                isOK, effectiveValue = convertEffectiveTime(args["effectiveTime"])
                if not isOK:
                    raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "Error effectiveTime input")
                args["effectiveTime"] = effectiveValue
        if args.has_key("policyFilter"):
            if len(args["policyFilter"]) == 0:
                args.pop("policyFilter")
            else:
                isOK, policyFilterValue = convertPolicyShield(args["policyFilter"])
                if not isOK:
                    raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "Error policyFilter input")
                args["policyFilter"] = policyFilterValue

        args["id"] = self.getMaxId() + 1
        isCheckOK, msg, item = checkItem(self.rawebConfValue, args)
        if not isCheckOK:
            raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, msg)
        ret["ret"] = 0
        ret["msg"] = "succ"
        args["author"] = self.author
        args["opType"] = "Add"
        args["modifiedTime"] = ""
        self.auxSession.add(AuxPolicy(args))
        self.auxSession.flush()
        self.auxSession.commit()
        items = map(lambda x: x.toJson(), self.auxSession.query(AuxPolicy).all())
        ret["total_num"] = self.session.query(Policy).count() + \
                           self.auxSession.query(AuxPolicy).filter(AuxPolicy.author == self.author).count()
        logging.info("user[%s] %s end at: %0.3f %0.3f" % (self.author, request, time.time(), time.clock()))
        return ret

    def getAllItems(self):
        items = map(lambda x: x.toJson(), self.session.query(Policy).all())
        auxItems = []
        if self.author != "":
            auxItems = map(lambda x: x.toJson(),self.auxSession.query(AuxPolicy).\
                           filter(AuxPolicy.author == self.author).all())
        allItems = {}
        for item in items:
            allItems.update(item)
        for item in auxItems:
            allItems.update(item)
        return allItems
    
    @commonReturn(readOnly=True)
    def search_metric(self, request):
        allItems = self.getAllItems()
        args = request.args
        ret = {}
        group = args.get("group", "")
        query = args.get("query", "")
        items = filter(lambda x: x["metric"].find(query) >= 0, allItems.values())        
        tmpItems = filter(lambda x: x["group"] == group or len(group) == 0, items)
        ret["policies"] = tmpItems
        ret["total_num"] = len(tmpItems)
        ret["group"] = group
        ret["ret"] = 0
        ret["msg"] = "succ"
        return ret

    @commonReturn(readOnly=True)
    def get_groups(self, request):
        allItems = self.getAllItems()
        ret = {}
        groups = set()
        for item in allItems.values():
            groups.add(item["group"])
        ret["ret"] = 0
        ret["msg"] = "succ"
        ret["groups"] = list(groups)
        ret["groups_num"] = len(groups)
        return ret

    @commonReturn(readOnly=False)
    def cancel_policy(self, request):
        allItems = self.getAllItems()
        args = request.args
        ret = {}
        if not args.has_key("id") or args["id"] == "":
            raise RaRequestError(ErrorType.NO_SIDS_ERROR, "id is error")
        pid = int(args["id"])
        items = filter(lambda x: x["id"] == pid, allItems.values())
        if len(items) < 1:
            raise RaRequestError(ErrorType.NO_IDS_ERROR, "id is not found")
        ret["ret"] = 0
        ret["msg"] = "succ"
        if "opType" not in items[0] or items[0]["author"] != self.author:
            raise RaRequestError(ErrorType.NO_IDS_ERROR, "id is not error, can not cancel")
        cancelItem(self.auxSession, "AuxPolicy", self.author, pid)
        return ret

    @commonReturn(readOnly=True)
    def get_policy(self, request):
        allItems = self.getAllItems()
        args = request.args
        ret = {}
        if not args.has_key("id") or args["id"] == "":
            raise RaRequestError(ErrorType.NO_SIDS_ERROR, "id is error")
        pid = int(args["id"])
        items = filter(lambda x: x["id"] == pid, allItems.values())
        if len(items) < 1:
            raise RaRequestError(ErrorType.NO_IDS_ERROR, "id is not found")
        ret["ret"] = 0
        ret["msg"] = "succ"
        ret["policy"] = items[0]
        return ret

    @commonReturn(readOnly=True)
    def get_policies(self, request):
        allItems = self.getAllItems()
        args = request.args
        ret = {}
        page_no = int(args.get("page_no", 1))
        page_size = int(args.get("page_size", 20))
        group = args.get("group", "")
        if page_no < 1:
            raise RaRequestError(ErrorType.PAGE_NO_ERROR, "page_no is invalidatd")
        if page_size < 1:
            raise RaRequestError(ErrorType.PAGE_SIZE_ERROR, "page_size is invalidatd")
        if len(group) == 0:
            tmpItems = allItems.values()
        else:
            tmpItems = filter(lambda x: x["group"] == group, allItems.values())
        ret["total_num"] = len(tmpItems)
        tmpItems = tmpItems[(page_no-1)*page_size: page_no*page_size]
        ret["policies"] = tmpItems
        ret["page_no"] = page_no
        ret["page_size"] = page_size
        ret["group"] = group
        ret["ret"] = 0
        ret["msg"] = "succ"
        return ret

    @commonReturn(readOnly=False)
    def delete_policies(self, request):
        args = request.args
        ret = {}
        if not args.has_key("ids") or args["ids"] == "":
            raise RaRequestError(ErrorType.NO_SIDS_ERROR, "sids is error")
        pids = args["ids"]
        if len(pids) == 0:
            raise RaRequestError(ErrorType.NO_IDS_ERROR, "input ids is error")
        allItems = self.getAllItems()
        pidlist = pids.split('_')
        for pid in pidlist:
            intPid = int(pid)
            if intPid in allItems:
                oldItem = allItems[intPid]
                if "opType" in oldItem:
                    policItem = self.auxSession.query(AuxPolicy).\
                                filter(AuxPolicy.id == intPid).\
                                filter(AuxPolicy.author == self.author)
                    if oldItem["opType"] == "Add":
                        policItem.delete()
                    else:
                        policItem.update({"opType": "Delete"})
                else:
                    oldItem["author"] = self.author
                    oldItem["opType"] = "Delete"
                    self.auxSession.add(AuxPolicy(oldItem))
                self.auxSession.flush()
                self.auxSession.commit()

        ret["ret"] = 0
        ret["msg"] = "succ"
        allItems = self.getAllItems()
        ret["total_num"] = len(allItems)
        groups = set()
        for item in allItems.values():
            groups.add(item["group"])
        ret["groups"] = list(groups)
        ret["groups_num"] = len(groups)
        return ret

    @commonReturn(readOnly=False)
    def group_move(self, request):
        args = request.args
        ret = {}
        if not args.has_key("newGroup") or args["newGroup"] == "":
            raise RaRequestError(ErrorType.GROUP_NAME_ERROR, "new group name is error")
        allItems = self.getAllItems()
        tmpItems = []
        if not args.has_key("ids") or args["ids"] == "" or len(args["ids"]) == 0:
            if not args.has_key("oldGroup") or args["oldGroup"] == "":
                raise RaRequestError(ErrorType.GROUP_NAME_ERROR, "old group name is error")
            tmpItems = filter(lambda x: x["group"] == args["oldGroup"], allItems.values())
        else:
            pidlist = args["ids"].split('_')
            for pid in pidlist:
                if int(pid) in allItems:
                    tmpItems.append(allItems[int(pid)])
        for item in tmpItems:
            if "opType" in item:
                self.auxSession.query(AuxPolicy).\
                    filter(AuxPolicy.id == item["id"]).\
                    filter(AuxPolicy.author == self.author).\
                    update({"groupName": args["newGroup"]})
            else:
                item["group"] = args["newGroup"]
                item["author"] = self.author
                item["opType"] = "Update"
                self.auxSession.add(AuxPolicy(item))
                self.auxSession.flush()
        self.auxSession.commit()

        ret["ret"] = 0
        ret["msg"] = "succ"
        allItems = self.getAllItems()
        ret["total_num"] = len(allItems)
        groups = set()
        for item in allItems.values():
            groups.add(item["group"])
        ret["groups"] = list(groups)
        ret["groups_num"] = len(groups)
        return ret

    @commonReturn(readOnly=False)
    def invalid_policy(self, request):
        return self.valid_invalid_policy(request, False)

    @commonReturn(readOnly=False)
    def valid_policy(self, request):
        return self.valid_invalid_policy(request, True)

    def valid_invalid_policy(self, request, isValid):
        ret = {}
        args = request.args.to_dict()
        if not args.has_key("id") or args["id"] == "":
            raise RaRequestError(ErrorType.NO_SIDS_ERROR, "id is error")
        pid = int(args["id"])
        allItems = self.getAllItems()
        if pid not in allItems:
            raise RaRequestError(ErrorType.IDS_ERROR, "Error id input")
        item = allItems[pid] 
        if isValid:
            item["validTime"] = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        else:
            item["validTime"] = datetime.date(2037, 1, 1).strftime('%Y-%m-%d %H:%M:%S')
        ret["ret"] = 0
        ret["msg"] = "succ"
        item["author"] = self.author
        if "opType" in item:
            self.auxSession.query(AuxPolicy).filter(AuxPolicy.id == pid).\
                filter(AuxPolicy.author == self.author).\
                update(AuxPolicy(item).toNormalJson())
        else:
            item["opType"] = "Update"
            self.auxSession.add(AuxPolicy(item))
        self.auxSession.flush()
        self.auxSession.commit()
        return ret

    @commonReturn(readOnly=True)
    def fetch_metrics(self, request):
        ret = {}
        ret["ret"] = 0
        ret["msg"] = "succ"
        ret["result"] = []

        endTime = int(time.time() * 1000000)
        startTime = endTime - 300*1000000
        Specs = self.serviceNode.getSpecs()
        api = MetricsFetchAPI(Specs)
        args = request.args.to_dict()
        logging.info("FETCH METRICS...")
        if not args.has_key("service"):
            val, reason, string = api.getMetrices('/', startTime, endTime)
            if val != 200:
                ret['ret'] = val
                ret['msg'] = 'fail'
                ret['result'] = ''
                return ret
            answer = json.loads(string)
            for item in answer:
                ret["result"].append(item)
            return ret

        if args.has_key("service") and args.has_key("nodepath"):
            endpoint = args['service'] + ':' + args['nodepath']
            val, reason, answer = api.getMetrices(endpoint, startTime, endTime)
            if val != 200:
                ret['ret'] = val
                ret['msg'] = 'fail'
                ret['result'] = ''
                return ret
            NextPath = {}
            answer = json.loads(answer)
            for item in answer:
                ret["result"].append(item)
            return ret

        if args.has_key('service') and not args.has_key('nodepath'):
            endpoint = args['service']
            val, reason, answer = api.getMetrices(endpoint, startTime, endTime)
            if val != 200:
                ret['ret'] = val
                ret['msg'] = 'fail'
                ret['result'] = ''
                return ret
            Nodepath = {}
            answer = json.loads(answer)
            for item in answer:
                ret["result"].append(item)
            return ret
