import sys,os
import unittest
here = os.path.dirname(__file__)
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__),"../..")))

import simplejson as json
from werkzeug.test import Client, EnvironBuilder, create_environ, run_wsgi_app
from werkzeug.wrappers import Request
from werkzeug.testapp import test_app
from werkzeug.wrappers import BaseResponse
from sqlalchemy.orm import sessionmaker
from sqlalchemy import create_engine
from raweb.models import *
from raweb.main import application
from raweb.conf import *


def getHttpResponse(ip, port, endpoint):
        conn=None
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


class rawebApiUnitTest(unittest.TestCase):    
    def session(self, fileName):
        global here
        here = os.path.abspath(here)
        dbFile = os.path.join(here, fileName)
        sqla_uri = 'sqlite:///%s' %dbFile
        sqla_params = {
            'echo':False,
            'encoding':'utf-8'
            }
        engine = create_engine(sqla_uri, **sqla_params)
        Base.metadata.create_all(engine)
        DB_Session = sessionmaker(bind=engine, autoflush = False)
        return DB_Session()
    
    def loadJsonFile(self, jsonFile):
        try:
            file_content = open(jsonFile).read()
            jsonVal = json.loads(file_content)
        except:
            _, err, _ = sys.exc_info()
            return (False, None)
        
        return (True, jsonVal)
    
    def setUp(self):
        self.db = self.session(tablePath)
        self.auxDb = self.session(auxTablePath)
      	
    def tearDown(self):
        if self.db:
            self.db.close()
        if self.auxDb:
            self.auxDb.close()

    def envSetup(self, Path, Method = "GET", Data = None):
        builder = EnvironBuilder(method = Method,
                                 base_url = "http://0.0.0.0:50001",
                                 data = Data,
                                 headers = {"Content-type": "application/json", "Accept": "text/plain"},
                                 path = Path,
                                 charset='utf-8',
                                 )
        env = builder.get_environ()
        return env

    def test_sapi_getTrendFactor(self):
        env = self.envSetup("sapi/get_trendFactor")
        app_iter, status, headers = run_wsgi_app(application, env)
        self.assertEqual(200, int(status.split(' ')[0]))

    def test_sapi_setTrendFactor(self):
        test_api = ["sapi/set_trendFactor?username=admin&smoothingFactor=0.99&trendFactor=0.98",
		    "sapi/set_trendFactor?username=admin&smoothingFactor=0.99",
		    "sapi/set_trendFactor?username=admin&smoothingFactor=abc&trendFactor=def"]
	envs = map(lambda x:self.envSetup(x), test_api)
	ans = map(lambda x:run_wsgi_app(application, x), envs)
	self.assertEqual([0, 101, 102], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_api_addPolicy(self):
        test_api = ['/api/add_policy?username=admin&group=test1&metric=service1.nodepath1.nodepath4.metrics3&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1',
		    '/api/add_policy?group=test1&metric=service1.nodepath1.nodepath4.metrics3&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1',
		    '/api/add_policy?username=admin&group=test1&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1']
        envs = map(lambda x:self.envSetup(x), test_api)
        ans = map(lambda x:run_wsgi_app(application, x), envs)
        self.assertEqual([0, 108, 110], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_api_cancelPolicy(self):
        env = self.envSetup('/api/add_policy?username=admin&group=test1&metric=service1.nodepath1.nodepath4.metrics3&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1&_=1468392539780')
	run_wsgi_app(application, env)
        ver = sorted(map(lambda x:x.toJson(), self.auxDb.query(AuxPolicy).all()))[0].keys()
        if ver:
            env = self.envSetup('/api/cancel_policy?id=%s&username=admin'%(ver[0]))
            app_iter, status, headers = run_wsgi_app(application, env)
            self.assertEqual(200, int(status.split(' ')[0]))
            item = json.loads(''.join(app_iter).splitlines()[0])
            self.assertEqual(0, int(item['ret']))
            
    def test_api_getPolicy(self):
        policy_id = map(lambda x:x.toJson(), self.db.query(Policy).all())[0].keys()[0]
	test_api = ['/api/get_policy?username=admin&id=%s'%(policy_id),
		    '/api/get_policy']
	envs = map(lambda x:self.envSetup(x), test_api)
        ans = map(lambda x:run_wsgi_app(application, x), envs)
        self.assertEqual([0, 301], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_api_getPolicies(self):
      	env = self.envSetup('/api/get_policies')
	app_iter, status, headers = run_wsgi_app(application, env)
	item = json.loads(''.join(app_iter).splitlines()[0])
	self.assertEqual(0, int(item['ret']))

    def test_api_getGroups(self):
        env = self.envSetup('/api/get_policies')
	app_iter, status, headers = run_wsgi_app(application, env)
	item = json.loads(''.join(app_iter).splitlines()[0])
	self.assertEqual(0, int(item['ret']))
        
    def test_api_deletePolicies(self):
        aForm = ['/api/add_policy?username=admin&group=test1&metric=service1.nodepath1.nodepath4.metrics3&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1&_=1468392539780',
		 '/api/add_policy?username=admin&group=test1&metric=service1.nodepath1.nodepath4.metrics3&checkType=single&validTime=2016-07-13+14%3A49%3A00&type=threshold&alarmLevel=curlmail&alarmGroup=fafafa&fetchInterval_s=15&minAlarmInterval_s=15&effectiveTime=&policyFilter=&upBound=211&downBound=1&_=1468392539780']
	map(lambda x:run_wsgi_app(application, x), map(lambda x:self.envSetup(x), aForm))
        keys = [item.keys()[0] for item in sorted(map(lambda x:x.toJson(), self.auxDb.query(AuxPolicy).all()))]
        if len(keys) > 1:
            env = self.envSetup('/api/delete_policies?username=admin&ids=%s_%s'%(keys[0], keys[1]))
            app_iter, status, headers = run_wsgi_app(application, env)
            self.assertEqual(200, int(status.split(' ')[0]))
            item = json.loads(''.join(app_iter).splitlines()[0])
            self.assertEqual(0, int(item['ret']))
        
    def test_api_invalidPolicy(self):
        ids = [item.keys()[0] for item in sorted(map(lambda x:x.toJson(), self.db.query(Policy).all()))]
	if not ids:
            print "No online Policies"
	test_api = ['/api/invalid_policy?username=admin&id=%s'%(ids[0]),
		    '/api/invalid_policy?username=admin',
		    '/api/invalid_policy?username=admin&id=abc',
		    '/api/invalid_policy?username=admin&id=100000']
	envs = map(lambda x:self.envSetup(x), test_api)
	ans = map(lambda x:run_wsgi_app(application, x), envs)
	self.assertEqual([0, 102, 205, 301], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_api_validPolicy(self):
        ids = [item.keys()[0] for item in sorted(map(lambda x:x.toJson(), self.db.query(Policy).all()))]
	apis = ['/api/invalid_policy?username=admin&id=%s'%(ids[0]),
		'/api/invalid_policy?username=admin&id=100000',
		'/api/invalid_policy?username=admin&id=abc',
		'/api/invalid_policy?username=adminc',
		'/api/invalid_policy?id=%s'%(ids[0]),]
	envs = map(lambda x:self.envSetup(x), apis)
	results = map(lambda x:run_wsgi_app(application, x), envs)
	ret = sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], results))
	self.assertEqual([0, 102, 110, 205, 301], ret)

    def test_api_AdminDeploy(self):
        test_api = ['/admin/deploy',
		    '/admin/deploy?username=admin']
        envs = map(lambda x:self.envSetup(x), test_api)
	ans = map(lambda x:run_wsgi_app(application, x), envs)
	self.assertEqual([0, 110], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_api_AdminDiff(self):
        test_api = ['/admin/diff',
		    '/admin/diff?username=admin']
	envs = map(lambda x:self.envSetup(x), test_api)
	ans = map(lambda x:run_wsgi_app(application, x), envs)
	self.assertEqual([0, 110], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))
	
    def test_api_AdminGetStatus(self):
        env = self.envSetup('/admin/get_status')
	app_iter, status, headers = run_wsgi_app(application, env)
	self.assertEqual(200, int(status.split(' ')[0]))
	item = json.loads(''.join(app_iter).splitlines()[0])
	self.assertEqual(0, int(item['ret']))
	
    def test_api_AdminRecover(self):
        test_api = ['/admin/recover',
		    '/admin/recover?username=admin']
	envs = map(lambda x:self.envSetup(x), test_api)
	ans = map(lambda x:run_wsgi_app(application, x), envs)
	self.assertEqual([0, 110], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

    def test_searchMetrics(self):
        test_api = ['/api/search_metric?group=&query=service1.nodepath1.nodepath4.metrics3',
		    '/api/search_metric?group=&query=abc',
		    '/api/search_metric?query=abc',
		    '/api/search_metric']
	envs = map(lambda x:self.envSetup(x), test_api)
        ans = map(lambda x:run_wsgi_app(application, x), envs)
        self.assertEqual([0, 0, 0, 0], sorted(map(lambda x:json.loads(''.join(x[0]).splitlines()[0])['ret'], ans)))

def suite():
    suite = unittest.makeSuite(rawebApiUnitTest, "test")
    return suite

if __name__=="__main__":
    unittest.main()

