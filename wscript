# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.0.1'
APPNAME='sync'

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('boost')
    opt.load('doxygen')

def configure(conf):
    conf.load("compiler_cxx")
    conf.check_cfg(atleast_pkgconfig_version='0.20')
    conf.check_cfg(package='openssl', args=['--cflags', '--libs'], uselib_store='SSL')
    conf.define ('STANDALONE', 1)
    # conf.define ('DIGEST_BASE64', 1) # base64 is not working and probably will not work at all

    conf.load('boost')
    conf.check_boost(lib='system iostreams test')
    
    conf.load('doxygen')

def build (bld):
    bld.shlib (target=APPNAME, 
               features=['cxx', 'cxxshlib'],
               source = bld.path.ant_glob(['model/sync-*.cc',
                                           'helper/sync-*.cc']),
               uselib = 'BOOST BOOST_IOSTREAMS SSL'
               )

    bld.program (target="testapp",
                 source = "test/testapp.cc",
                 features=['cxx', 'cxxprogram'],
                 use = 'BOOST_TEST sync')

from waflib.Build import BuildContext
class doxy (BuildContext):
    cmd = "doxygen"
    fun = "doxygen"

def doxygen (bld):
    bld (features="doxygen",
         doxyfile='doc/doxygen.conf',
         output_dir = 'doc')
