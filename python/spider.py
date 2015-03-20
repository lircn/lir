#!/usr/bin/python
# -*- coding: utf-8 -*-

import urllib2
import urllib
import re
import thread
import time
import socks
import socket
import os
import platform

from BeautifulSoup import BeautifulSoup

class Utils:
	def __init__(self):
		self.system = platform.system()

	def Clear(self):
		if self.system == 'Windows':
			os.system('cls')
		elif self.system == 'Linux':
			os.system('clear')


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

	def ShowPage(self, fence, page):
		for items in fence:
			break

	def Start(self):
		self.enable = True
		page = self.listPage

		print u'正在加载中...'

		self.LoadPage()
		#thread.start_new_thread(self.LoadPage, ())

#		while self.enable:
#			if self.pages:
#				fence = self.pages[0]
#				del self.pages[0]

class SpiderQb:
	def __init__(self):
		self.page = 1
		self.cache = []
		self.pages = []
		self.fence = 0
		self.anchor = 0
		self.utils = None
		self.textUrl = 'http://m.qiushibaike.com/text/page/'

	def SetUtils(self, utils):
		self.utils = utils

	def EnableProxy(self):
		socks.setdefaultproxy(socks.PROXY_TYPE_SOCKS4, "127.0.0.1", 28888)
		socket.socket = socks.socksocket

	def GetPage(self, page):
		url = self.textUrl + page
		userAgent = 'Mozilla/5.0 (Windows NT 6.1;)'
		headers = {'User-Agent':userAgent}
		req = urllib2.Request(url, headers = headers)
		try:
			resp = urllib2.urlopen(req)
		except urllib2.HTTPError, e:
			print u'''
			%d: %s
			''' % (e.code, e.reason)
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
		self.cache = self.GetPage(str(self.page))
		if len(self.cache) == 0:
			print u'''
			网太高，我上不去...
			请确保你能访问：%s
			''' % self.textUrl
		else:
			self.pages += self.cache
			self.anchor = len(self.cache) / 2
			self.page += 1

	def ShowPage(self):
		while True:
			self.utils.Clear()
			print u'''
			===============
			| h: 向上翻页 |
			| j: 向下翻页 |
			| q: 退出     |
			===============
			'''
			if self.fence < 0:
				self.fence = 0
			if self.fence >= len(self.pages) - self.anchor:
				print u'''
			正在自动为您加载更多...
				'''
				thread.start_new_thread(self.LoadPage, ())

			i = self.fence
			for i in range(self.fence, min(self.fence + 5, len(self.pages))):
				print str(i) + '. ' + self.pages[i]['time']
				print self.pages[i]['text'] + '\n'
			print '(' + str(i) + '/' + str(len(self.pages)) + ')'
			cmd = raw_input('h/j/q:')
			if cmd == 'h':
				self.fence -= 5
			elif cmd == 'j':
				self.fence += 5
				if self.fence >= len(self.pages) - 5:
					self.fence = len(self.pages) - 5
			elif cmd == 'q':
				print 'Bye-bye~'
				break

	def Start(self):
		self.LoadPage()
		self.ShowPage()


print 'Welcome!'
strIn = raw_input(
u'''选个站点吧～:
1. 糗事百科
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
	spider.SetUtils(Utils())
	spider.Start()
