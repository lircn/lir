#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import socket
import struct
import copy
import datetime
import time
import sys
import subprocess

TAB_TIME_FMT = "%Y%m%d"
arr = [
        [1,2,3],
        [3,20,2],
        [4,2,5]]

d = {
    "b":2,
    "a":1,
    "c":3}

c = {
    "b":4,
    "a":3,
    "c":1}

def cal_avg(arr, idx):
    return long(cal_sum(arr, idx) / len(arr))

def cal_sum(arr, idx):
    total = 0
    for item in arr:
        total += item[idx]
    return total

def int2ip(ip):
    ip = int(ip)
    return socket.inet_ntoa(struct.pack('I', socket.htonl(ip)))

def ip2int(ip):
    ip = str(ip)
    return socket.ntohl(struct.unpack('I', socket.inet_aton(str(ip)))[0])

def cal_avg_dict(arr):
    ret = {}
    length = len(arr)
    for key in arr[0].keys():
        total = 0
        for item in arr:
            total += item[key]
        ret[key] = total / length

    return ret

def getStartOfDay(_time):
    t = time.localtime(_time)
    ret = time.mktime(time.strptime(time.strftime('%Y-%m-%d 00:00:00', t),'%Y-%m-%d %H:%M:%S'))
    return long(ret)

def getEndOfDay(_time):
    t = time.localtime(_time)
    ret = time.mktime(time.strptime(time.strftime('%Y-%m-%d 23:59:59', t),'%Y-%m-%d %H:%M:%S'))
    return long(ret)

def getts():
    ret = time.mktime(time.strptime('2018-01-12 06:18:00', '%Y-%m-%d %H:%M:%S'))
    return long(ret)

def get2WeeksAgo():
    now = datetime.datetime.now()
    ret = now - datetime.timedelta(days=14)
    ret = time.mktime(ret.timetuple())
    return long(ret)

def sec2str(sec):
    return datetime.datetime.fromtimestamp(long(sec))

def auto_reload():
    mod = "config"
    module = sys.modules[mod]

    filename = module.__file__
    if filename.endswith(".pyc"):
        filename = filename.replace(".pyc", ".py")
    print filename
    mod_time = os.path.getmtime(filename)
    print mod_time
    if not "loadtime" in module.__dict__:
        module.loadtime = 0

    if mod_time > module.loadtime:
        reload(module)

    module.loadtime = mod_time


def test_list():
    syn_keys = [
        "syn_current",
        "syn_accept",
        "syn_state",
        "syn_cookie",
        "syn_illegal_drop",
        "syn_table_drop",
        "filter_hit",
        "filter_miss",
    ]
    syn_delta_keys = [
        "filter_hit",
        "filter_miss",
    ]

    for key in syn_keys:
        if key in syn_delta_keys:
            print "in: " + key
        else:
            print "out: " + key


def fn_timer(func):
    #@warps(func)
    def func_timer(*args, **kwargs):
        t0 = long(time.time())
        ret = func(*args, **kwargs)
        t1 = long(time.time())
        print (args[0].__class__.__name__)
        print("Time running %s %s: %s" % (type(func).__name__, func.func_name, str(t1-t0)))
        return ret
    return func_timer

class SS():
    @fn_timer
    def get(self, x):
        return x * x


@fn_timer
def square(x):
    """Calculate the square of the given number."""
    return x * x

import urllib
import urllib2
import json
#
#ip = '10.56.162.136'
#url = "http://10.49.88.207/cgi-bin/svr_mgt/cgi-bin/api/get_apd_server.cgi"
#val = { 'ip': ip }
#try:
#    data = urllib.urlencode(val)
#    req = urllib2.Request(url, data)
#    #req.add_header('Content-Type', 'application/json')
#    recv = urllib2.urlopen(req).read()
#    print recv
#    ret = json.loads(recv, encoding='utf-8')
#except Exception,e:
#    ret = str(e)
#
#print ret
#print ret["errno"]
#print ret["result"][0]["IDC"]


def _trans_file(ip):
    url = 'http://10.223.30.66:32780'
    data_str = 'cmd_type=create2&json_job={"run_mode":"continue","delete_mode":"auto","now_run":"immediate","client_module":399,"client_passwd":"Qsa7Ee7l","signal":0,"author":"zaynli","query_key":"","job_type":"","job_desc":"zaynli","data":"","step_list":[{"type":1,"desc":"","info":"","err_deal":0,"out_max_con_retry":1,"src_ip":"10.219.146.220","src_path":"/data/www/html/zaynli/lbr_only.tar","dest_ip":"%s","dest_path":"/tmp/"}]}' % (ip)
    req = urllib2.Request(url)
    ret = urllib2.urlopen(req, data_str, timeout=5)
    ret = ret.read()
    return ret

def do_sh(data):
    try:
        cmd = "./../t.sh " + "'"+data+"'"
        ret = subprocess.call(cmd, shell=True)
        if ret != 0:
            print("upload apd error: %d" % (ret))
    except Exception,e:
        print("upload error: %s" % (str(e)))
    return


def do_sql(sql):
    try:
        cmd = "java -jar ~/work/java/sql_parser.jar " + "\""+sql+"\""
        r = os.popen(cmd)
        info = r.readlines()
        ret = ""
        for line in info:
            ret += line
        print ret
    except Exception,e:
        print(str(e))



def fab(_max):
    n, a, b = 0, 0, 1
    while n < _max:
        yield b
        a, b = b, a + b
        n += 1

class Fab(object):
    def __init__(self, _max):
        self.max = _max
        self.n = 0
        self.a = 0
        self.b = 1

    def __iter__(self):
        return self

    def next(self):
        if self.n < self.max:
            r = self.b
            self.a, self.b = self.b, self.a + self.b
            self.n += 1
            return r
        raise StopIteration()



def save_atk_cc():
    if True:
        a = 1
    else:
        a = 2
    return a

s = "ss\r\n"
print s+"|"
print s.strip()+"|"
