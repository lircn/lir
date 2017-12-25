#!/usr/bin/env python
# -*- coding: utf-8 -*-

import tornado.ioloop
import tornado.web
import os
import time
import json
import datetime

OK = 0
ERR = -1

srv_port = 10419

store_path = "/data/iot_store/"

class FileModule:
    def __init__(self):
        self.cgn_path = "cgn/"
        self.data = {}
        return

    def cgn_upload(self, files):
        name = datetime.datetime.now().strftime("%Y%m%d-%H%M")

        for meta in files:
            outpath = store_path + self.cgn_path + name
            with open(outpath, 'wb') as f:
                f.write(meta["body"])

        return OK

    def app_upload_img(self, usr, files):
        for meta in files:
            outpath = self.path + usr
            with open(outpath, 'wb') as f:
                f.write(meta["body"])

        return OK

    def dev_upload(self, usr, _type, files):
        self.data[usr] = {}
        self.data[usr]["type"] = _type
        self.data[usr]["data"] = files[0]["body"]
        return OK

    def app_download_info(self, usr):
        if self.data.has_key(usr):
            return OK, self.data[usr]
        else:
            return ERR

g_fm = FileModule()

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.write("IoT Hello world")

class CgnUploadHandler(tornado.web.RequestHandler):
    def post(self):
        code = g_fm.cgn_upload(self.request.files["file"])
        ret = {
            "code": code,
        }
        self.write(json.dumps(ret))

class DevPushHandler(tornado.web.RequestHandler):
    def post(self):
        usr = self.get_argument('usr')
        _type = self.get_argument('type')
        code = g_fm.dev_upload(usr, _type, self.request.files["file"])
        ret = {
            "code": code,
        }
        self.write(json.dumps(ret))

class AppGetInfoHandler(tornado.web.RequestHandler):
    def get(self):
        usr = self.get_argument('usr')
        code, data = g_fm.app_download_info(usr)
        ret = {
            "code": code,
            "data": data,
        }
        self.write(json.dumps(ret))

def app_init():
    handlers = [
        (r"/iot/", MainHandler),
        (r"/iot/cgn_upload", CgnUploadHandler),
    ]

    return tornado.web.Application(
        handlers=handlers,
        debug=True,
    )

if __name__ == "__main__":
    app = app_init()
    app.listen(srv_port)
    tornado.ioloop.IOLoop.current().start()
