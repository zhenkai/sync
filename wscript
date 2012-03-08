# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.0.1'
APPNAME='sync'

def options(opt):
    opt.add_option('--no-debug',action='store_true',default=False,dest='no_debug',help='''Make an optimized build of the library (remove debugging code)''')
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('boost')
    opt.load('doxygen')
    opt.load('ccnx tinyxml', tooldir=["waf-tools"])

def configure(conf):
    conf.load("compiler_cxx")
    conf.check_cfg(atleast_pkgconfig_version='0.20')
    conf.check_cfg(package='openssl', args=['--cflags', '--libs'], uselib_store='SSL')
    # conf.check_cfg(package='libxml-2.0', args=['--cflags', '--libs'], uselib_store='XML')

    conf.load('boost')
    conf.check_boost(lib='system iostreams test thread')
    
    conf.load('doxygen')
    conf.load('ccnx tinyxml')
    conf.check_ccnx (path=conf.options.ccnx_dir)
    conf.check_tinyxml (path=conf.options.ccnx_dir)

    conf.define ('STANDALONE', 1)
    if not conf.options.no_debug:
        conf.define ('_DEBUG', 1)
	
def build (bld):
    bld.shlib (target=APPNAME, 
               features=['cxx', 'cxxshlib'],
               source = bld.path.ant_glob(['model/sync-*.cc',
                                           'helper/sync-*.cc']),
               uselib = 'BOOST BOOST_IOSTREAMS BOOST_THREAD SSL TINYXML CCNX'
               )

    # Unit tests
    bld.program (target="unit-tests",
                 source = bld.path.ant_glob(['test/**/*.cc']),
                 features=['cxx', 'cxxprogram'],
                 use = 'BOOST_TEST sync')


# doxygen docs
from waflib.Build import BuildContext
class doxy (BuildContext):
    cmd = "doxygen"
    fun = "doxygen"

def doxygen (bld):
    bld (features="doxygen",
         doxyfile='doc/doxygen.conf',
         output_dir = 'doc')
