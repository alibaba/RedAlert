import sys
import os
import shutil
import unittest
sys.path.append(os.path.abspath(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../'))))
from raweb.fs_util import FsUtilDelegate
here = os.path.abspath(os.path.dirname(__file__))
fsUtilExe = '../../fs_lib/bin/fs_util'
testPath = here + '/testData/fs_test_path'
nullPath = ''


class fsUtilTest(unittest.TestCase):
    def setUp(self):
        self.fs = FsUtilDelegate(fsUtilExe, '')
        os.mkdir(testPath)
        map(lambda x: os.mkdir(testPath + x), ['/test1', '/test2', '/test3', '/test3/test1', '/list'])
        map(lambda x: open(testPath + '/test1/' + x, 'w'), ['file1', 'file2', 'file3', 'file4'])
        map(lambda x: open(testPath + '/list/' + str(x), 'w'), [1, 2, 3, 4])

    def test_isDir(self):
        self.assertTrue(self.fs.isDir(testPath))
        self.assertFalse(self.fs.isDir(testPath + '/wrong'))

    def test_copy(self):
        self.assertTrue(self.fs.copy(testPath + '/test1', testPath + '/test2', overwrite=False))
        self.assertTrue(self.fs.copy(testPath + '/test1', testPath + '/test3', overwrite=False))
        self.assertTrue(self.fs.copy(testPath + '/test1', testPath + '/test3', overwrite=True))

    def test_exists(self):
        self.assertTrue(self.fs.exists(testPath + '/test1'))
        self.assertFalse(self.fs.exists(testPath + '/testt'))

    def test_mkdir(self):
        self.assertTrue(self.fs.mkdir(testPath + '/test4'))
        self.assertFalse(self.fs.mkdir(testPath + '/test3'))

    def test_rename(self):
        os.mkdir(testPath + '/testA')
        self.assertTrue(self.fs.rename(testPath + '/test1/file1', testPath + '/test1/file5'))

    def test_listDir(self):
        self.assertEqual(['file1', 'file2', 'file3', 'file4'], self.fs.listDir(testPath + '/test1'))

    def test_remove(self):
        os.mkdir(testPath + '/testB')
        self.assertTrue(self.fs.remove(testPath + '/testB'))
        self.assertFalse(self.fs.remove(testPath + '/test6'))

    def test_cat(self):
        try:
            f = open(testPath+'/test1/file1', 'w')
            f.write('abc')
        except Exception, e:
            raise('Open test failed')
        finally:
            f.close()
        print self.fs.cat(testPath + '/test1/file1')
        self.assertEqual('abc', self.fs.cat(testPath + '/test1/file1'))

    def test_normalizeDir(self):
        self.assertEqual(testPath + '/test1/', self.fs.normalizeDir(testPath + '/test1'))

    def test_getConfigVersionList(self):
        self.assertEqual([1, 2, 3, 4], map(lambda x: int(x), self.fs.getConfigVersionList(testPath + '/list')))

    def test_getMaxConfigVersion(self):
        self.assertEqual(4, self.fs.getMaxConfigVersion(testPath + '/list'))

    def tearDown(self):
        shutil.rmtree(testPath)

def suite():
    suite = unittest.makeSuite(fsUtilTest, 'test')
    return suite

if __name__ == "__main__":
    unittest.main()
