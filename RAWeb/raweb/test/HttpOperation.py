import sys,os
import httplib
import urllib
import simplejson as json

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

def postHttpRequest(dic):
    httpClient = None
    try:
        params = json.dumps(dic)
        headers = {"Content-type": "application/json", "Accept": "text/plain"}
        httpClient = httplib.HTTPConnection("0.0.0.0", 5000)
        httpClient.request("POST", "/admin/heartbeats", params, headers)
        response = httpClient.getresponse()
        print response.status
        print response.reason
        print response.read()
        print response.getheaders()
        return response.status, response.reason, response.read()
    except Exception, e:
        print e
    finally:
        if httpClient:
            httpClient.close()

if __name__=='__main__':
	while True:
		postHttpRequest({"address":"1.2.3.4:56","interval":5,"version":1})
		time.sleep(3)
