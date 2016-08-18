#!/bin/env python
import sys
import os
import shutil
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import simplejson as json
import logging
import time
from threading import Thread, Event
from werkzeug.wrappers import Request, Response
from werkzeug.contrib.wrappers import JSONRequestMixin
from werkzeug.routing import Map, Rule
from werkzeug.exceptions import HTTPException, NotFound
from werkzeug.wsgi import SharedDataMiddleware
from raweb.parse import *
from raweb.views import RaWebClass
from raweb.view_util import initSql
from raweb.conf_views import RaConfWebClass
from raweb.ra_admin_views import RaAdminClass
from raweb.error_type import ErrorType
from raweb.heart_beats import ServiceNode
from raweb.storage import storage_interface
from raweb.load_raweb_json import checkAndReload, loadJsonFile
here = os.path.split(os.path.realpath(__file__))[0]


def globalInit():
    logging.basicConfig(filename=os.path.join(here, 'log.txt'),
                        level=logging.INFO,
                        format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S')
    consoleHandler = logging.StreamHandler()
    logging.getLogger().addHandler(consoleHandler)


def initRaConf(root, raConfDir, vsDir):
    if not os.path.exists(root):
        try:
            os.makedirs(root)
        except Exception, e:
            print e
            return (False, 'No root or mkdir root failed')
    if not os.path.exists(raConfDir):
        try:
            os.mkdir(raConfDir)
        except Exception, e:
            print e
            return (False, "mkdir failed")
    if not os.path.exists(vsDir):
        try:
            os.mkdir(vsDir)
        except Exception, e:
            print e
            return (False, "mkdir failed")
    if not os.path.exists(os.path.join(root, 'raweb.aux.db')):
        try:
            shutil.copy(os.path.join(here, '../conf/raweb.aux.db'), root)
        except Exception, e:
            print e
            return (False, 'shutil copy failed')
    if not os.path.exists(os.path.join(root, 'raweb.json')):
        try:
            shutil.copy(os.path.join(here, "../conf/raweb.json"), root)
        except Exception, e:
            print e
            return (False, 'shutil copy failed')
    if not os.path.exists(raConfDir + '/sqlite'):
        try:
            shutil.copy(os.path.join(here, '../conf/sqlite'), raConfDir)
        except Exception, e:
            print e
            return (False, 'shutil copy failed')
        if not initSql(raConfDir + '/sqlite'):
            return (False, 'init sqlite failed')
    return (True, "OK")


serNode = ServiceNode(MAX_RA_SERVER, tablePath, tmpConfDir)


def application(environ, start_response):
    request = Request(environ)
    logging.info("%s start at: %0.3f %0.3f" % (request, time.time(), time.clock()))
    isInitOK, msg = initRaConf(confDir, raConfDir, tmpConfDir)
    if not isInitOK:
        ret = {}
        ret["ret"] = ErrorType.INIT_JSON_CONF_ERROR
        ret["msg"] = msg
        return Response(json.dumps(ret), mimetype='text/plain')
    isRaOK, rawebConfValue = loadJsonFile(raWebPath)
    if not isRaOK:
        logging.error("load %s error", raWebPath)
        ret = {}
        ret["ret"] = ErrorType.INIT_RA_ERROR
        ret["msg"] = "Error conf path %s" % raWebPath
        return Response(json.dumps(ret), mimetype='text/plain')
    storage = storage_interface()
    raAdmin = RaAdminClass(raConfDir, tmpConfDir, tablePath, auxTablePath, serNode, storage)
    raWeb = RaWebClass(rawebConfValue, tablePath, auxTablePath, serNode)
    raConfWeb = RaConfWebClass(rawebConfValue, tablePath, auxTablePath)

    url_map = Map(raWeb.url_map + raConfWeb.url_map + raAdmin.url_map)
    adapter = url_map.bind_to_environ(request.environ)
    try:
        endpoint, values = adapter.match()
        response = endpoint(request, **values)
    except HTTPException, e:
        response = e
        logging.info("%s end at: %0.3f %0.3f" % (response, time.time(), time.clock()))
    logging.info("%s end at: %0.3f %0.3f" % (request, time.time(), time.clock()))
    return response(environ, start_response)


def checkNode(jsonConfDir, tmpConfDir, serNode):
    while True:
        if serNode.checkAlive():
            print "E"
            checkAndReload(serNode.getAllSpecs(), jsonConfDir, tmpConfDir, Flag=True)
        time.sleep(2)
"""
@localStart
for local test
"""
def localStart():
    globalInit()
    from werkzeug.serving import run_simple
    static_dir = os.path.join(os.path.dirname(__file__), "..", "static")
    exports = {
        '/': static_dir,
        }
    app_entry = SharedDataMiddleware(application, exports)
    t = Thread(target=checkNode, args=(raConfDir, tmpConfDir, serNode))
    t.deamon = True
    t.start()
    run_simple('0.0.0.0', runPort, app_entry, use_debugger=True, use_reloader=True)
    t._Thread__stop()

"""
@make_app
Apache and uwsgi used
"""
def make_app():
    globalInit()
    t = Thread(target=checkNode, args=(raConfDir, tmpConfDir, serNode))
    t.deamon = True
    t.start()
    static_dir = os.path.join(os.path.dirname(__file__), "..", "static")
    exports = {
        '/': static_dir,
        }
    app_entry = SharedDataMiddleware(application, exports)
    return app_entry


if __name__ == '__main__':
    localStart()
