from __future__ import with_statement
import simplejson as json
import logging
from decimal import Decimal
from werkzeug.wrappers import Request, Response
from werkzeug.routing import Map, Rule
from raweb.load_raweb_json import *
from raweb.error_type import ErrorType
from raweb.view_util import *


class RaConfWebClass():
    def __init__(self, rawebConfValue, tablePath, auxTablePath):
        self.url_map = []
        self.url_map.append(Rule('/sapi/get_trendFactor', endpoint=self.get_trendFactor))
        self.url_map.append(Rule('/sapi/set_trendFactor', endpoint=self.set_trendFactor))
        self.url_map.append(Rule('/sapi/get_items', endpoint=self.get_items))
        self.url_map.append(Rule('/sapi/add_item', endpoint=lambda x: self.process_item(x, "add")))
        self.url_map.append(Rule('/sapi/update_item', endpoint=lambda x: self.process_item(x, "update")))
        self.url_map.append(Rule('/sapi/del_item', endpoint=lambda x: self.process_item(x, "del")))
        self.url_map.append(Rule('/sapi/cancel_item', endpoint=lambda x: self.process_item(x, "cancel")))
        self.rawebConfValue = rawebConfValue
        self.tablePath = tablePath
        self.auxTablePath = auxTablePath
        self.session = None
        self.auxSession = None
        self.author = ""

    def init(self, author):
        self.session = getSession(os.path.join(self.tablePath))
        self.auxSession = getSession(os.path.join(self.auxTablePath))
        self.author = author
        self.res = readCommonShieldToItems(self.session, self.auxSession, author)

    def close(self):
        if self.auxSession:
            self.auxSession.close()
        if self.session:
            self.session.close()

    @commonReturn(readOnly=True)
    def get_trendFactor(self, request):
        ret = {}
        ret['ret'] = 0
        ret['msg'] = 'succ'
        trendNames = ["smoothingFactor", "trendFactor"]
        for name in trendNames:
            ret[name] = self.res[name]
        return ret

    @commonReturn(readOnly=False)
    def set_trendFactor(self, request):
        pairItems = getMixResults(self.session, self.auxSession, "Pair", "AuxPair", self.author)
        ret = {}
        trendNames = ["smoothingFactor", "trendFactor"]
        for name in trendNames:
            ret["ret"] = 0
            ret["msg"] = "succ"
            if name not in self.res.keys() or name not in request.args.keys():
                raise RaRequestError(ErrorType.KEY_ERROR, "no %s input" % (','.join(trendNames)))
            oldItem = filter(lambda x: x["key"] == name, pairItems.values())
            if len(oldItem) != 1:
                raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "load %s failed" % name)
            value = Decimal(str(float(request.args[name])))
            newItem = oldItem[0].copy()
            newItem["key"] = name
            newItem["value"] = str(value)
            updateItem(self.auxSession, "AuxPair", self.author, oldItem[0], newItem)
        return ret

    def getTypeName(self, request):
        args = request.args
        if "type" not in args.keys():
            raise RaRequestError(ErrorType.TYPE_ERROR, "no type in url parameters")
        itemType = args["type"]
        items = []
        for key in self.res["items"].keys():
            if key.lower() == itemType.lower():
                keyType = key
                break
        else:
            raise RaRequestError(ErrorType.TYPE_ERROR, "type[%s] is not found" % itemType)
        return keyType

    @commonReturn(readOnly=True)
    def get_items(self, request):
        keyType = self.getTypeName(request)
        items = self.res["items"][keyType].values()
        args = request.args
        ret = {}
        page_no = int(args.get("page_no", 1))
        page_size = int(args.get("page_size", 20))
        if page_no < 1:
            raise RaRequestError(ErrorType.PAGE_NO_ERROR, "page_no is invalidatd")
        if page_size < 1:
            raise RaRequestError(ErrorType.PAGE_SIZE_ERROR, "page_size is invalidatd")
        items.sort(key=lambda x: x["id"])
        tmpItems = items[(page_no-1)*page_size: page_no*page_size]
        ret["ret"] = 0
        ret["msg"] = "succ"
        ret["total_num"] = len(items)
        ret["items"] = tmpItems
        ret["page_no"] = page_no
        ret["page_size"] = page_size
        return ret

    @commonReturn(readOnly=False)
    def process_item(self, request, oper):
        args = request.args
        keyType = self.getTypeName(request)
        ret = {}
        if oper == "add":
            item = args.to_dict()
            isCheckOK, msg, item = checkItem(self.rawebConfValue, item)
            if not isCheckOK:
                raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, msg)
            addItem(self.session, self.auxSession, keyType, "Aux" + keyType, self.author, item)
        elif oper == "del":
            if not args.has_key("ids") or args["ids"] == "":
                raise RaRequestError(ErrorType.NO_IDS_ERROR, "ids is error")
            sidlist = args["ids"].split("_")
            hasUpdate = False
            for k, v in self.res["items"][keyType].items():
                if str(v["id"]) in sidlist:
                    deleteItem(self.auxSession, "Aux" + keyType, self.author, v)
        elif oper == "update":
            if not args.has_key("id") or args["id"] == "":
                raise RaRequestError(ErrorType.NO_IDS_ERROR, "id is error")
            sid = int(args["id"])
            if not self.res["items"].has_key(sid):
                raise RaRequestError(ErrorType.NO_IDS_ERROR, "Error id input")
            item = args.to_dict()
            isCheckOK, msg, item = checkItem(self.rawebConfValue, item)
            if not isCheckOK:
                raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, msg)
            updateItem(self.auxSession, "Aux" + keyType, self.author, self.res["items"][keyType][sid], args)
        elif oper == "cancel":
            if not args.has_key("id") or args["id"] == "":
                raise RaRequestError(ErrorType.NO_IDS_ERROR, "id is error")
            sid = int(args["id"])
            if not self.res["items"][keyType].has_key(sid):
                raise RaRequestError(ErrorType.NO_IDS_ERROR, "Error id input")
            cancelItem(self.auxSession, "Aux" + keyType, self.author, sid)
        ret["ret"] = 0
        ret["msg"] = "succ"
        return ret
