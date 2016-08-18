import sys, os
import unittest
import time,datetime
here = os.path.dirname(__file__)
project_root = os.path.abspath(os.path.join(here, '../..') )
sys.path.append(os.path.abspath(project_root) )
from raweb.view_util import getSession
from raweb.load_raweb_json import getModelsClass
from raweb.models import RedAlert
from raweb.heart_beats import *
from HttpOperation import getHttpResponse, postHttpRequest
import subprocess
import time

tablePath = project_root + '/raweb/test/testData/current/sqlite'
versionPath = project_root + '/raweb/test/testData/tmp'

class heartBeatsUtilTest(unittest.TestCase):
    def setUp(self):
        self.trans=ServiceNode(3, tablePath, versionPath)
        self.session = getSession(tablePath)

    def clear(self):
        clear()

    def tearDown(self):
        if self.session:
            self.session.close()
    
    def test_addNode(self):
        self.trans.addNode('172.29.152.181:6', 1, 120)
        self.assertEqual('172.29.152.181:6', self.trans.peek()[0]['address'])
        inDB = map(lambda x:x.toJson(), self.session.query(getModelsClass('RedAlert')).all())
        self.clear()

    def test_addSomeNode(self):
        map(lambda x:self.trans.addNode('172.29.152.18%s:60'%str(x), 1, 120), range(0,10))
        inDB = map(lambda x:x.toJson(), self.session.query(getModelsClass('RedAlert')).all())
        print "++++", inDB
        self.assertEqual(10, len(self.trans.peek()))
        self.assertEqual(3, len(inDB))
        self.clear()
        
    def test_updateNode(self):
        self.trans.addNode('172.29.152.182:6', 3, 129)
        self.trans.updateNode("172.29.152.182:6", 2, 130)
        item = filter(lambda x:x['address']== '172.29.152.182:6', self.trans.peek())
        self.assertEqual(2, item[0]['interval_s'])
        self.assertEqual(-1, item[0]['version'])
        self.clear()

    def test_checkAlive(self):
        self.trans.addNode('172.29.152.183:6', 1, 120)
        time.sleep(4)
        self.trans.checkAlive()
        taget = filter(lambda x:x['address']=='172.29.152.183:6', self.trans.peek())
        self.assertEqual(0, taget[0]['status'])
        self.trans.addNode('172.29.152.184:6', 1, 120)
        self.trans.addNode('172.29.152.185:6', 1, 120)
        time.sleep(2.8)
        self.trans.checkAlive()
        self.assertEqual(2, len(filter(lambda x:x['status']!=0, self.trans.peek())))
        self.clear()

    def test_selectAlive(self):
        self.trans.addNode('152.23.36.3:5', 1, 120)
        time.sleep(3)
        self.trans.addNode('152.23.36.4:5', 1, 120)
        self.trans.addNode('152.23.36.5:5', 1, 120)
        self.trans.checkAlive()
        self.assertEqual(2, len(self.trans.selectAlive()))
        self.clear()

    def test_getSpecs(self):
        self.trans.addNode('152.23.36.3:56', 1, 120)
        time.sleep(3.2)
        self.trans.addNode('152.23.36.4:56', 1, 120)
        self.trans.addNode('152.23.35.5:56', 1, 120)
        self.trans.checkAlive()
        self.assertEqual(2, len(filter(lambda x:x['status']==1, self.trans.peek())))

def suite():
    suite = unittest.makeSuite(heartBeatsUtilTest, 'test')
    return suite

if __name__=='__main__':
    unittest.main()
