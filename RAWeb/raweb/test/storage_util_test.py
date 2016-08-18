import sys,os
import unittest
here=os.path.dirname(__file__)
sys.path.append(os.path.abspath(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../') )) )
from raweb.storage import *
import shutil
testPath = os.path.abspath(os.path.dirname(__file__))+'/testData/list'

fileFS = 'file://'+ os.path.abspath(os.path.dirname(__file__))+'/testData/LocalFS'
panguFS = 'pangu://' + os.path.abspath(os.path.dirname(__file__)) + '/testData/panguFS'

class storageUtilTest(unittest.TestCase):
    def setUp(self):
        os.mkdir(testPath)
        map(lambda x:open(testPath+'/'+str(x),'w'),[1,2,3,4])
        self.sto = storage_interface()

    def tearDown(self):
        shutil.rmtree(testPath)

    def test_listDir(self):
        ret = self.sto.listDir(testPath)
        ret=map(lambda x:int(x),ret)
        ret.sort()
        self.assertEqual([1,2,3,4], ret)

    def test_deployNewVersion(self):
        map(lambda x:os.mkdir(testPath + '/' + str(x)),[5,6,7,8])
        ret, ver = self.sto.deployNewVersion(os.path.abspath(os.path.dirname(__file__)) + '/testData', testPath)
        self.assertTrue(ret)
        self.assertEqual(9,int(ver.split('/')[-1]))

def suite():
    suite = unittest.makeSuite(storageUtilTest,'test')
    return suite

if __name__ == '__main__':
    unittest.main()
