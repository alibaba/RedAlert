import sys,os
import commands
import unittest
import httplib, urllib
import time
here = os.path.dirname(__file__)
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__),"../..")))

import simplejson as json

import fs_util_test
import storage_util_test
import views_util_test
import heart_util_test
import conn_util_test
import fetchMetric_util_test

if __name__ == "__main__":
    runner = unittest.TextTestRunner()
    allTests = unittest.TestSuite()
    allTests.addTest(fs_util_test.suite())
    allTests.addTest(storage_util_test.suite())
    time.sleep(2)
    allTests.addTest(views_util_test.suite())
    allTests.addTest(conn_util_test.suite())
    allTests.addTest(heart_util_test.suite())
    #allTests.addTest(fetchMetric_util_test.suite())

    runner.run(allTests)
