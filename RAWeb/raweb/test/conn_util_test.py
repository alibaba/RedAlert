from __future__ import with_statement
import sys,os
import time
import httplib,urllib
import simplejson as json
import unittest
import subprocess

here = os.path.dirname(__file__)
project_root = os.path.abspath(os.path.join(here, '../..') )
sys.path.append(os.path.abspath(project_root) )
from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine
from raweb.models import *
from raweb.conf import tablePath, tmpConfDir


def postHttpRequest(port, interval):
    httpClient = None
    try:
        params = json.dumps({"address":"0.0.0.0:%s"%(port),"interval":interval,"version":1})
        headers = {"Content-type": "application/json", "Accept": "text/plain"}
        httpClient = httplib.HTTPConnection("0.0.0.0", 5000)
        httpClient.request("POST", "/admin/heartbeats", params, headers)
        response = httpClient.getresponse()
        print response.status
        print response.reason
        print response.read()
        print response.getheaders()
    except Exception, e:
        print e
    finally:
        if httpClient:
            httpClient.close()

class connUpDownUtilTest(unittest.TestCase):
    def setUp(self):
        self.db = self.session(tablePath)
        
    def tearDown(self):
        if self.db:
            self.db.close()

    def listAll(self, db, table):
            return map(lambda x:x.toJson(), db.query(table).all())

    def session(self, fileName):
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

    def checkDir(self):
        ConfWork = tmpConfDir
        re = map(lambda x:int(x), os.listdir(ConfWork))
        return max(re)

    def checkItem(self):
        pass
    
    def test_HeartBeatsV1(self):
        beforeTest = self.checkDir()
        postHttpRequest(50001, 5)
        afterTest = self.checkDir()
        self.assertEqual(1, int(afterTest)-int(beforeTest))
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(1, len(ret))
        
        time.sleep(5)
        self.assertEqual(afterTest, self.checkDir())
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(1, len(ret))

        time.sleep(13)
        self.assertEqual(afterTest + 1, self.checkDir())
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(0, len(ret))
        
    def test_HeartBeatsV2(self):
        before = self.checkDir()
        postHttpRequest(50002, 5)
        postHttpRequest(50003, 5)
        after = self.checkDir()
        self.assertEqual(2, after - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(2, len(ret))
        
        time.sleep(17)
        self.assertEqual(3, self.checkDir() - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(0, len(ret))

        postHttpRequest(50002, 1)
        self.assertEqual(4, self.checkDir() - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(1, len(ret))
        time.sleep(5)
        self.assertEqual(5, self.checkDir() - before)

    def test_HeartBeatsV3(self): 
        before = self.checkDir()
        postHttpRequest(50004, 5)
        time.sleep(4)
        postHttpRequest(50005, 5)
        after = self.checkDir()

        self.assertEqual(2, after - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(2, len(ret))

        time.sleep(13)
        self.assertEqual(3, self.checkDir() - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(1, len(ret))

        time.sleep(4)
        self.assertEqual(4, self.checkDir() - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(0, len(ret))

        postHttpRequest(50004, 1)
        self.assertEqual(5, self.checkDir() - before)
        ret = self.listAll(self.db, RedAlert)
        self.assertEqual(1, len(ret))
        time.sleep(5)
        
    def testTenNode(self):
        before = self.checkDir()
        map(lambda x:postHttpRequest(x, 1), [x for x in xrange(50005, 50015)])
        self.assertEqual(10, self.checkDir() - before)
        time.sleep(1)
        map(lambda x:postHttpRequest(x, 1), [x for x in xrange(50005, 50010)])
        time.sleep(3)
        self.assertEqual(11, self.checkDir() - before)
        time.sleep(5)
        self.assertEqual(12, self.checkDir() - before)

    def atestTenNodeV2(self):
        before = self.checkDir()
        x= 50015
        count=0
        while x < 50025:
            postHttpRequest(x, 5)
            x+=1
            time.sleep(1)
        while count < 15:
            print self.checkDir()
            count+=1
            time.sleep(1)
        

def suite():
    suite = unittest.makeSuite(connUpDownUtilTest, "test")
    return suite

if __name__=='__main__':
    unittest.main()
