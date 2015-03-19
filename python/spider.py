# -*- coding: utf-8 -*-

import urllib2
import urllib
import re
import thread
import time
import socks
import socket

from BeautifulSoup import BeautifulSoup

class SpiderCnbeta:
	def __init__(self):
		self.listPage = 1
		self.pages = []
		self.enable = False

	def GetList(self, listPage):
		url = "http://m.cnbeta.com/list_latest_" + listPage + ".htm"
		userAgent = 'Mozilla/5.0 (Windows NT 6.1;)'
		headers = {'User-Agent':userAgent}
		req = urllib2.Request(url, headers = headers)
		resp = urllib2.urlopen(req)
		page = resp.read()
		unicodePage = page.decode("utf-8")

		soup = BeautifulSoup(page)
		print soup.title.text
		self.enable = False

	def LoadPage(self):
		while self.enable:
			if len(self.pages) < 2:
				try:
					page = self.GetList(str(self.listPage))
					self.listPage += 1
					self.pages.append(page)
				except:
					print 'can not connect'
			else:
				time.sleep(1)

	def ShowPage(self, nowPage, page):
		for items in nowPage:
			break

	def Start(self):
		self.enable = True
		page = self.listPage

		print u'正在加载中...'

		self.LoadPage()
		#thread.start_new_thread(self.LoadPage, ())

#		while self.enable:
#			if self.pages:
#				nowPage = self.pages[0]
#				del self.pages[0]

class SpiderQb:
	def __init__(self):
		self.page = 1
		self.cache = []
		self.pages = []
		self.enable = False

	def EnableProxy(self):
		socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS4, "127.0.0.1", 28888)
		socket.socket = socks.socksocket

	def GetPage(self, page):
		url = "http://m.qiushibaike.com/text/page/" + page
		userAgent = 'Mozilla/5.0 (Windows NT 6.1;)'
		headers = {'User-Agent':userAgent}
		req = urllib2.Request(url, headers = headers)
		resp = urllib2.urlopen(req)
		page = resp.read()

		soup = BeautifulSoup(page)
		articles = soup.findAll(attrs = {'class':'article block untagged mb15'})

		items = []
		for i in range(len(articles)):
			content = articles[i].find(attrs={'class':'content'})
			item = {'text':content.text, 'time':dict(content.attrs)['title']}
			items.append(item)
		return items

	def LoadPage(self):
		try:
			self.cache = self.GetPage(str(self.page))
			self.page += 1
		except:
			print 'can not connect'

	def ShowPage(self):
		for item in self.items:
			print item['time']
			print item['text'] + '\n'

	def Start(self):
		self.enable = True
		page = self.page

		print u'正在加载中...'

		self.LoadPage()
		thread.start_new_thread(self.LoadPage, ())

		self.ShowPage()

#		while self.enable:
#			if self.pages:
#				nowPage = self.pages[0]
#				del self.pages[0]

print 'Welcome!'
strIn = raw_input(
'''Choose a site:
1. QiuBai
2. CnBeta
''')

doFlag = False

if strIn == '1':
	spider = SpiderQb()
	strIn = raw_input('Use proxy(y/n)?')
	if strIn[0] == 'y' or strIn[0] == 'Y':
		spider.EnableProxy()
	doFlag = True

elif strIn == '2':
	print 'Do not support yet'

else:
	print 'Input error'

if doFlag:
	spider.Start()
