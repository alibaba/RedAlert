import re
import os
import sys
import time
import logging
import types
import datetime
import httplib
import urllib
from decimal import Decimal
here = os.path.dirname(__file__)
project_root = os.path.abspath(os.path.join(here, '..'))
sys.path.append(os.path.abspath(project_root))
sys.path.insert(0, os.path.abspath(os.path.dirname(__file__)))
from models import DataSource
import sqlalchemy
from raweb.view_util import *
from raweb.storage import storage_interface


def uploadPath(ipList, path):
        httpClient = None
        print path
	if not ipList:
            return False
        for ip in ipList:
            try:
                params = json.dumps({'configPath': '%s'%(path)})
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


def checkAndReload(ipList, jsonConfDir, tmpConfDir, Flag=False):
    store = storage_interface()
    if Flag:
        val, Latest = store.deployNewVersion(jsonConfDir, tmpConfDir)
    maxVersion = max(store.listDir(tmpConfDir))
    retVersionDir = tmpConfDir + '/' + str(maxVersion)
    re, upLoadDir = store.upload(retVersionDir)
    uploadPath(ipList, upLoadDir)
    return True


def loadJsonFile(jsonFile):
    import simplejson as json
    try:
        file_content = open(jsonFile).read()
        jsonVal = json.loads(file_content, use_decimal=True)
    except:
        _, err, _ = sys.exc_info()
        return (False, None)
    return (True, jsonVal)


def validate_policyFilter(value):
    if type(value) is not types.ListType or len(value) == 0:
        return (False, "policyFilter should be list", None)
    return (True, "Everything is OK", value)


def validate_effectiveTime(value):
    if type(value) is not types.ListType or len(value) == 0:
        return (False, "effectiveTime should be list", None)
    for e in value:
        if type(e) is not types.DictType or not e.has_key("begin") or not e.has_key("end"):
            return (False, "effectiveTime key error", None)
        reTime = "^([0-1]\d|2[0-3]):([0-5]\d)"
        if not re.match(reTime, e["begin"]) or not re.match(reTime, e["end"]):
            return (False, "effectiveTime key error", None)
        if time.strptime(e["begin"], "%H:%M") > time.strptime(e["end"], "%H:%M"):
            return (False, "EndTime should larger than BeginTime", None)
    return (True, "Everything is OK", value)


def validate_minAlarmInterval_s(value, fetchInterval_s):
    if Decimal(value) >= Decimal(fetchInterval_s):
        return (True, "Everything is OK", Decimal(value))
    else:
        return (False, "Error minAlarmInterval", Decimal(value))


def convertEffectiveTime(effectiveTimeStr):
    if len(effectiveTimeStr) == 0:
        return (False, None)
    beginEndTime = effectiveTimeStr.split('|')
    effectiveTime = []
    for s in beginEndTime:
        be = s.split("_")
        if len(be) != 2:
            return (False, None)
        bedict = {}
        bedict["begin"] = be[0]
        bedict["end"] = be[1]
        effectiveTime.append(bedict)
    return (True, effectiveTime)


def convertPolicyShield(policyFilterStr):
    if len(policyFilterStr) == 0:
        return (False, None)
    policys = policyFilterStr.split('|')
    policyFilter = []
    for s in policys:
        be = s.split("@")
        if len(be) != 2:
            return (False, None)
        bedict = {}
        bedict["metric"] = be[0]
        bedict["host"] = be[1]
        policyFilter.append(bedict)
    return (True, policyFilter)


def convertSingleItem(sterm, value):
    if sterm["type"] == "string" or sterm["type"] == "regex" or sterm["type"] == "datetime":
        value = str(value)
        if len(value) == 0:
            return (False, None)
    elif sterm["type"] == "int" or sterm["type"] == "enum":
        try:
            value = int(value)
            if value < 0:
                return (False, None)
        except ValueError, e:
            return (False, None)
    elif sterm["type"] == "Decimal":
        try:
            value = Decimal(str(float(value)))
        except ValueError, e:
            return (False, None)
    if sterm["type"] == "enum":
        if value not in sterm["valuelist"]:
            return (False, None)
    elif sterm["type"] == "regex":
        if not re.match(sterm["val"], value):
            return (False, None)
    elif sterm["type"] == "datetime":
        try:
            time.strptime(value, sterm["val"])
        except:
            return (False, None)
    return (True, value)


def checkItem(rawebConfValue, item):
    if not item or not item.has_key("type"):
        return (False, "item is Error", None)
    raVal = rawebConfValue[item["type"]]
    for k in item.keys():
        if k not in raVal.keys() and k != "type":
            item.pop(k)
            
    for k, v in raVal.items():
        if item.has_key(k) and v.has_key("compatibleKeys"):
            for replaceKey in v["compatibleKeys"]:
                item[replaceKey] = item[k]
            item.pop(k, None)
            
    for k, v in raVal.items():
        if item.has_key(k) or not v.has_key("noNeed"):
            if not item.has_key(k):
                return (False, "item does not has key %s" % k, None)
            isSingleItemOK, tmpItem = convertSingleItem(v, item[k])
            if not isSingleItemOK:
                return (False, "key %s is error" % k, None)
            item[k] = tmpItem

    for k, v in raVal.items():
        if v["type"] == "func" and item.has_key(k):
            if v.has_key("kvmap"):
                kvmap = {}
                for k1, v1 in v["kvmap"].items():
                    if v1.startswith("@"):
                        if not item.has_key(v1[1:]):
                            return (False, "item does not has key %s" % v1[1:], None)
                        else:
                            kvmap[str(k1)] = item[v1[1:]]
                isOK, msg, item[k] = globals()[v["funcname"]](item[k], **kvmap)
            else:
                isOK, msg, item[k] = globals()[v["funcname"]](item[k])
            if not isOK:
                return (False, msg, None)
    return (True, "Everything is OK", item)


def getModelsClass(tableName):
    return getattr(__import__("models"), tableName)


def getMixResults(session, auxSession, tableName, auxTableName, author):
    mainClass = getModelsClass(tableName)
    auxClass = getModelsClass(auxTableName)
    items = map(lambda x: x.toJson(), session.query(mainClass).all())
    auxItems = []
    if author != "":
        auxItems = map(lambda x: x.toJson(), auxSession.query(auxClass).filter(auxClass.author == author).all())
    allItems = {}
    for item in items:
        allItems.update(item)
    for item in auxItems:
        allItems.update(item)
    return allItems


def getMaxId(session, auxSession, mainClass, auxClass):
    defaultId = 1
    qry = session.query(sqlalchemy.sql.func.max(mainClass.id).label("max_id"))
    if qry.count() >= 1:
        defaultId = max(defaultId, qry.one().max_id)
        qry = auxSession.query(sqlalchemy.sql.func.max(auxClass.id).label("max_id"))
    if qry.count() >= 1:
        defaultId = max(defaultId, qry.one().max_id)
    return defaultId


def addItem(session, auxSession, tableName, auxTableName, author, item):
    mainClass = getModelsClass(tableName)
    auxClass = getModelsClass(auxTableName)
    sId = getMaxId(session, auxSession, mainClass, auxClass) + 1
    item["id"] = sId
    item["author"] = author
    item["opType"] = "Add"
    item["modifiedTime"] = ""
    auxSession.add(auxClass(item))
    auxSession.flush()
    auxSession.commit()
    return True


def updateItem(auxSession, auxTableName, author, oldItem, item):
    auxClass = getModelsClass(auxTableName)
    item["author"] = author
    item["modifiedTime"] = oldItem["modifiedTime"]
    item["opType"] = "Update"
    if "opType" in oldItem:
        if oldItem["opType"] == "Add":
            item["opType"] = "Add"
        auxSession.query(auxClass).filter(auxClass.id == item["id"]).filter(auxClass.author == author).delete()
        auxSession.add(auxClass(item))
    else:
        auxSession.add(auxClass(item))
    auxSession.flush()
    auxSession.commit()
    return True


def cancelItem(auxSession, auxTableName, author, sid):
    auxClass = getModelsClass(auxTableName)
    auxSession.query(auxClass).filter(auxClass.id == sid).filter(auxClass.author == author).delete()
    auxSession.flush()
    auxSession.commit()
    return True


def deleteItem(auxSession, auxTableName, author, oldItem):
    auxClass = getModelsClass(auxTableName)
    if "opType" in oldItem:
        queryItem = auxSession.query(auxClass).filter(auxClass.id == oldItem["id"]).filter(auxClass.author == author)
        if oldItem["opType"] == "Add":
            queryItem.delete()
        else:
            queryItem.update({"opType": "Delete"})
    else:
        oldItem["author"] = author
        oldItem["opType"] = "Delete"
        auxSession.add(auxClass(oldItem))
    auxSession.flush()
    auxSession.commit()
    return True


def diffTable(session, tableName, auxSession, auxTableName, author):
    mainClass = getModelsClass(tableName)
    auxClass = getModelsClass(auxTableName)
    queryItems = auxSession.query(auxClass).filter(auxClass.author == author).all()
    for queryItem in queryItems:
        if queryItem.opType == "Delete" or queryItem.opType == "Update":
            mainItems = session.query(mainClass).filter(mainClass.id == queryItem.id).all()
            if len(mainItems) == 0 or mainItems[0].modifiedTime != queryItem.modifiedTime:
                queryItem.opType = queryItem.opType + "_Conflicted"
    retItems = {}
    auxItems = map(lambda x: x.toJson(), auxSession.query(auxClass).filter(auxClass.author == author).all())
    for item in auxItems:
        retItems.update(item)
    return True, retItems


def deployTable(session, tableName, auxSession, auxTableName, author):
    mainClass = getModelsClass(tableName)
    auxClass = getModelsClass(auxTableName)
    modifiedTime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    diffItems = []
    queryItems = auxSession.query(auxClass).filter(auxClass.author == author).all()
    for queryItem in queryItems:
        mainItems = session.query(mainClass).filter(mainClass.id == queryItem.id)
        if queryItem.opType != "Add" \
           and (len(mainItems.all()) == 0 or mainItems.all()[0].modifiedTime != queryItem.modifiedTime):
            queryItem.opType = "Canceled"
            diffItems.append(queryItem.toJson().values()[0])
        elif queryItem.opType == "Delete":
            mainItems.delete()
        elif queryItem.opType == "Update":
            item = queryItem.toJson().values()[0]
            item.pop("opType")
            item["modifiedTime"] = modifiedTime
            mainItems.update(mainClass(item).toNormalJson())
        elif queryItem.opType == "Add":
            item = queryItem.toJson().values()[0]
            item.pop("opType")
            item["modifiedTime"] = modifiedTime
            session.add(mainClass(item))
        session.flush()
        auxSession.query(auxClass).filter(auxClass.author == author).filter(auxClass.id == queryItem.id).delete()
        auxSession.flush()
    auxSession.commit()
    session.commit()
    return True, diffItems


def readCommonShieldToItems(session, auxSession, author):
    pairItems = getMixResults(session, auxSession, "Pair", "AuxPair", author)
    smoothItems = filter(lambda x: x["key"] == "smoothingFactor", pairItems.values())
    trendItems = filter(lambda x: x["key"] == "trendFactor", pairItems.values())
    if len(smoothItems) != 1 or len(trendItems) != 1:
        raise RaRequestError(ErrorType.CHECK_ITEM_ERROR, "load smoothingFactor trendFactor failed")
    ret = {}
    ret["smoothingFactor"] = smoothItems[0]["value"]
    ret["trendFactor"] = trendItems[0]["value"]
    ret["items"] = {}
    tableList = ["RedAlert", "Shield", "DataSource"]
    for tableName in tableList:
        tableItems = getMixResults(session, auxSession, tableName, "Aux" + tableName, author)
        ret["items"][tableName] = tableItems
    return ret

if __name__ == "__main__":
    print "OK"
