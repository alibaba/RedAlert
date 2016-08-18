# -*- coding: utf-8 -*-
import sys,os
here = os.path.dirname(__file__)
project_root = os.path.abspath(os.path.join(here, '../..') )
sys.path.append(os.path.abspath(project_root) )
from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine
from raweb.models import *

def listAll(db, table):
    try:
        result = db.query(table).all()
        for p in result:
            print p.toJson()
    except Exception, e:
        print 'query failed: ' + str(e)


def session(fileName):
        global here
        here = os.path.abspath(here)
        dbFile = os.path.join(here, fileName)
        sqla_uri = 'sqlite:///%s' %dbFile
        sqla_params = {
            'echo': False,
            'encoding': 'utf-8'
            }
        engine = create_engine(sqla_uri, **sqla_params)
        Base.metadata.create_all(engine)
        DB_Session = sessionmaker(bind=engine, autoflush = False)
        return DB_Session()


def initsql():
    handler = session('/var/www/html/red_alert_web/conf/sqlite')
    handler.query(Pair).delete()
    handler.flush()
    handler.commit()
    smoo = {"key" : "smoothingFactor",
            "value" : 0.45,
            "author" : "admin",
            "modifiedTime" : ""
            }
    trend = {"key" : "trendFactor", 
             "value" : 0.45,
             "author" : "admin",
             "modifiedTime" : ""
             }
    pair1 = Pair(smoo)
    pair2 = Pair(trend)
    handler.add(pair1)
    handler.add(pair2)
    handler.flush()
    handler.commit()


if __name__=='__main__':
    initsql()
#    auxdb = session('/home/etc/red_alert_web/raweb.aux.db')
    #db = session('/home/etc/red_alert_web/current/sqlite')
    #listAll(db, RedAlert)

