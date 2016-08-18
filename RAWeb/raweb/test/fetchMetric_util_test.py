import sys,os
import time
import threading
import httplib, urllib
import subprocess
import multiprocessing
import unittest
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__),"../..")))
import simplejson as json
from raweb.test.ra.raBackEnd import env_server_start

def getHttpResponse(ip, port, endpoint):
        conn = None
        try:
            conn = httplib.HTTPConnection(ip, port)
            conn.request("GET", endpoint)
            r1 = conn.getresponse()
            data1 = r1.read()
            return r1.status,r1.reason,data1
        except Exception, e:
            print e
            return -1, "Connection Failed", None
        finally:
            if conn:
                conn.close()


class fetchMetricUtilTest(unittest.TestCase):
    def setUp(self):
        self.p = multiprocessing.Process(target = env_server_start, args=(50009,))
        self.p.start()
        time.sleep(3)
        
    def test_fetchRoot(self):
        time.sleep(2)
        code, reason, ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics")
        self.assertEqual(code, 200)
        answerJs = json.loads(ret)
        retNum = int(answerJs['ret'])
        self.assertEqual(retNum, 0)

    def test_fetchRootWrong(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_wrong")
        self.assertEqual(code,404)

    def test_fetchService(self):
        code, reason, ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service=service1")
        self.assertEqual(code,200)
        answer=json.loads(ret)
        retNum=int(answer['ret'])
        self.assertEqual(retNum,0)
        
    def test_fetchServiceWrong(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service=wrong")
        self.assertEqual(code, 200)
        self.assertEqual(int(json.loads(ret)['ret']), 0)

    def test_fetchNode(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service=service1&nodepath=nodepath1")
        self.assertEqual(code, 200)
        self.assertEqual(int(json.loads(ret)['ret']), 0)

    def test_fetchNodeWrongService(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service=rr&nodepath=nodepath1")
        self.assertEqual(code, 200)
	self.assertEqual(json.loads(ret)['result'], [])
        self.assertEqual(int(json.loads(ret)['ret']), 0)

    def test_fetchNodeWrongNode(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service=service1&nodepath=yy")
        self.assertEqual(code, 200)
	self.assertEqual(json.loads(ret)['result'], [])
        self.assertEqual(int(json.loads(ret)['ret']), 0)

    def test_fetchNodeWrongAll(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?service='s1'&nodepath='y'")
        self.assertEqual(code, 200)
	self.assertEqual(json.loads(ret)['result'], [])
        self.assertEqual(int(json.loads(ret)['ret']), 0)
        
    def test_fetchNodeWithoutSev(self):
        code,reason,ret = getHttpResponse("0.0.0.0","5000","/api/fetch_metrics?nodepath=nodepath1")
	self.assertEqual(len(json.loads(ret)['result']), 2)
        self.assertEqual(code, 200)
        self.assertEqual(int(json.loads(ret)['ret']), 0)

    def tearDown(self):
        if self.p.is_alive():
            getHttpResponse("0.0.0.0", 50009, "/api/shutdown")
            self.p.terminate()
            time.sleep(3)

def suite():
    suite = unittest.makeSuite(fetchMetricUtilTest, 'test')
    return suite

if __name__=='__main__':
    unittest.main()
