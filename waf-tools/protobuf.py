#! /usr/bin/env python
# encoding: utf-8

'''

When using this tool, the wscript will look like:

	def options(opt):
	        opt.tool_options('protobuf', tooldir=["waf-tools"])

	def configure(conf):
		conf.load('compiler_cxx protobuf')

	def build(bld):
		bld(source='main.cpp', target='app', use='PROTOBUF')

Options are generated, in order to specify the location of protobuf includes/libraries.


'''
import sys
import re
from waflib import Utils,Logs,Errors
from waflib.Configure import conf
PROTOBUF_DIR=['/usr','/usr/local','/opt/local','/sw']
PROTOBUF_VERSION_FILE='google/protobuf/stubs/common.h'
PROTOBUF_VERSION_CODE='''
#include <iostream>
#include <google/protobuf/stubs/common.h>
int main() { std::cout << GOOGLE_PROTOBUF_VERSION ;}
'''

def options(opt):
	opt.add_option('--protobuf',type='string',default='',dest='protobuf_dir',help='''path to where protobuf is installed, e.g. /usr/local''')
@conf
def __protobuf_get_version_file(self,dir):
	try:
		return self.root.find_dir(dir).find_node('%s/%s' % ('include', PROTOBUF_VERSION_FILE))
	except:
		return None
@conf
def protobuf_get_version(self,dir):
	val=self.check_cxx(fragment=PROTOBUF_VERSION_CODE,includes=['%s/%s' % (dir, 'include')], execute=True, define_ret = True, mandatory=True)
	return val
@conf
def protobuf_get_root(self,*k,**kw):
	root=k and k[0]or kw.get('path',None)
	# Logs.pprint ('RED', '   %s' %root)
	if root and self.__protobuf_get_version_file(root):
		return root
	for dir in PROTOBUF_DIR:
		if self.__protobuf_get_version_file(dir):
			return dir
	if root:
		self.fatal('Protobuf not found in %s'%root)
	else:
		self.fatal('Protobuf not found, please provide a --protobuf argument (see help)')
@conf
def check_protobuf(self,*k,**kw):
	if not self.env['CXX']:
		self.fatal('load a c++ compiler first, conf.load("compiler_cxx")')

	var=kw.get('uselib_store','PROTOBUF')
	self.start_msg('Checking Protocol Buffer')
	root = self.protobuf_get_root(*k,**kw);
	self.env.PROTOBUF_VERSION=self.protobuf_get_version(root)

	self.env['INCLUDES_%s'%var]= '%s/%s' % (root, "include");
	self.env['LIB_%s'%var] = "protobuf"
	self.env['LIBPATH_%s'%var] = '%s/%s' % (root, "lib")

	self.end_msg(self.env.PROTOBUF_VERSION)
	if Logs.verbose:
		Logs.pprint('CYAN','	Protocol Buffer include : %s'%self.env['INCLUDES_%s'%var])
		Logs.pprint('CYAN','	Protocol Buffer lib     : %s'%self.env['LIB_%s'%var])
		Logs.pprint('CYAN','	Protocol Buffer libpath : %s'%self.env['LIBPATH_%s'%var])
