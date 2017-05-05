#! /usr/bin/env python3
# -*- coding: UTF-8 -*-
import os
import signal
import tornado.web
import tornado.ioloop
import tornado.httpserver
from pprint import pprint

def sig_handler(signum, frame):
    print('Catch stop signal')
    tornado.ioloop.IOLoop.current().stop()
    print('Terminated')

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render('index.html')

curr_load = {}
all_load = {}
chart_info = []

class InfoHandler(tornado.web.RequestHandler):
    def get(self):
        self.finish({'results': chart_info})
    def post(self):
        self.finish()
        hostname = self.get_argument('hostname', None)
        cpu_load = self.get_argument('cpu_load', None)
        if hostname and cpu_load:
            curr_load[hostname] = float(cpu_load)

def make_app():
    return tornado.web.Application(
                handlers=[(r'/', MainHandler),
                          (r'/info', InfoHandler),],
                static_path=os.path.join(os.path.dirname(__file__), 'static'),
                template_path=os.path.join(os.path.dirname(__file__), 'templates'),)

def add_info():
    global all_load, curr_load, chart_info
    for hostname, cpu_load in curr_load.items():
        try:
            all_load[hostname].append(cpu_load)
        except KeyError:
            all_load[hostname] = [None] * 9 + [cpu_load]
        if len(all_load[hostname]) > 10:
            all_load[hostname].pop(0)
    data = sorted(all_load.items(), key=lambda key: key[0])
    col = ['Time']
    row = [[i] for i in range(-10, 0)]
    for hostname, cpu_loads in data:
        if len(cpu_loads) != 10:
            continue
        col.append(hostname)
        for i in range(10):
            row[i].append(cpu_loads[i])
    chart_info = [col] + row

if __name__ == '__main__':
    signal.signal(signal.SIGINT, sig_handler)
    app = make_app()
    http_server = tornado.httpserver.HTTPServer(app)
    http_server.listen(8888)
    tornado.ioloop.PeriodicCallback(add_info, 1000).start()
    tornado.ioloop.IOLoop.current().start()
