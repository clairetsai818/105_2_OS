#! /usr/bin/env python3
# -*- coding: UTF-8 -*-
import os, uuid
import signal
import tornado.web
import tornado.ioloop
import tornado.httpserver
import subprocess as sp
import time 

from pprint import pprint

def sig_handler(signum, frame):
    print('Catch stop signal')
    tornado.ioloop.IOLoop.current().stop()
    print('Terminated')

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('index.html', showpicture=False, timestamp=time.time())

class UploadHandler(tornado.web.RequestHandler):
    def post(self):
        type = self.get_argument("type")
        print(type)
        fileinfo = self.request.files['filearg'][0]
        fname = fileinfo['filename']
        extn = os.path.splitext(fname)[1]
        cname = str(uuid.uuid4()) + extn
        fh = open(cname, 'wb')
        fh.write(fileinfo['body'])
        if (type == "color"):
            sp.call("mpiexec.mpich -n 3 -host 192.168.136.202,192.168.136.201,192.168.136.200 ./mpi_processor_color %s" % cname, shell = True)
        else:
            sp.call("mpiexec.mpich -n 3 -host 192.168.136.202,192.168.136.201,192.168.136.200 ./mpi_processor_gray %s" % cname, shell = True)
        self.render('index.html',showpicture=True, timestamp=time.time()) 

def make_app():
    return tornado.web.Application(
                handlers=[(r'/', MainHandler),
                        (r'/upload', UploadHandler),],
                static_path=os.path.join(os.path.dirname(__file__), './'),
                template_path=os.path.join(os.path.dirname(__file__), 'templates'),)

if __name__ == '__main__':
    signal.signal(signal.SIGINT, sig_handler)
    app = make_app()
    http_server = tornado.httpserver.HTTPServer(app)
    http_server.listen(8080)
    tornado.ioloop.IOLoop.current().start()
