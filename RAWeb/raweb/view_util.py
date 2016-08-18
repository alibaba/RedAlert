from __future__ import with_statement
import logging
from werkzeug.wrappers import Request, Response
from werkzeug.routing import Map, Rule
from raweb.error_type import ErrorType
import sqlalchemy
from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine
from models import *
import simplejson as json
import sys
import os


class RaRequestError(Exception):
    def __init__(self, errorCode, value):
        self.errorCode = errorCode
        self.value = value

    def __str__(self):
        return repr("Error Code: %d, %s" % (self.errorCode, self.value))


def getSession(tablePath):
    dbFile = tablePath
    sqla_uri = 'sqlite:///%s' % dbFile
    sqla_params = {
        'echo': False,
        'encoding': 'utf-8'
        }
    engine = create_engine(sqla_uri, **sqla_params)
    Base.metadata.create_all(engine)
    DB_Session = sessionmaker(bind=engine, autoflush=False)
    return DB_Session()

def initSql(tablePath):
    sm = { "key": "smoothingFactor", "value": 0.45, "author": "admin", "modifiedTime": "" }
    tr = { "key": "trendFactor", "value": 0.45, "author": "admin", "modifiedTime": "" }
    try:
        handler = getSession(tablePath)
        handler.query(Pair).delete()
        pair1 = Pair(sm)
        pair2 = Pair(tr)
        handler.add(pair1)
        handler.add(pair2)
        handler.flush()
        handler.commit()
    except:
        return False
    return True

def checkUserName(request):
    username = request.cookies.get('username', "")
    isauth = request.cookies.get('isauth', "")
    if username == "" or isauth == "":
        username = request.args.get("username", "")
        if username == "":
            raise RaRequestError(ErrorType.USER_ID_ERROR, "input username is null, cannot access this interface")
        else:
            username = request.args.get("username", "")
    return username


def commonReturn(readOnly=False):
    def _wrapper(func):
        def _decorator(self, request, *args, **kwargs):
            ret = {}
            try:
                author = request.cookies.get('username', "")
                if not readOnly:
                    author = checkUserName(request)
                self.init(author)
                ret = func(self, request, *args)
            except KeyError, e:
                ret["ret"] = ErrorType.KEY_ERROR
                ret["msg"] = "KeyError:%s" % str(e)
                logging.error("key error: %s, input: %s" % (str(e), str(args)))
            except ValueError, vaError:
                ret["ret"] = ErrorType.VALUE_ERROR
                ret["msg"] = str(vaError)
                logging.error("value error: %s, input: %s" % (str(vaError), str(args)))
            except sqlalchemy.exc.OperationalError, dbError:
                ret["ret"] = ErrorType.DB_ERROR
                ret["msg"] = "db may be locked or not connect failed, plz retry"
                logging.error("db error: %s, input: %s" % (dbError, str(args)))
            except RaRequestError, raError:
                ret["ret"] = raError.errorCode
                ret["msg"] = raError.value
            except:
                _, err, _ = sys.exc_info()
                logging.error("unknown error: %s, input: %s" % (err, str(args)))
                logging.exception('Got exception on main handler')
                ret["ret"] = ErrorType.UNKNOWN_ERROR
                ret["msg"] = "unknown error, error:%s" % err
            finally:
                self.close()
            return Response(json.dumps(ret), mimetype='text/plain')
        return _decorator
    return _wrapper
