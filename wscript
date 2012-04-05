# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.0.1'
APPNAME='sync'

def options(opt):
    opt.add_option('--no-debug',action='store_true',default=False,dest='no_debug',help='''Make an optimized build of the library (remove debugging code)''')
    opt.add_option('--log4cxx', action='store_true',default=False,dest='log4cxx',help='''Compile with log4cxx/native NS3 logging support''')
    opt.add_option('--ns3',     action='store_true',default=False,dest='ns3_enable',help='''Compile as NS-3 module''')
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('boost')
    opt.load('doxygen')
    opt.load('ccnx tinyxml ns3', tooldir=["waf-tools"])

def configure(conf):
    conf.load("compiler_cxx")
    conf.env.append_value('CXXFLAGS', ['-O0', '-g3'])

    if not conf.check_cfg(package='openssl', args=['--cflags', '--libs'], uselib_store='SSL', mandatory=False):
      libcrypto = conf.check_cc(lib='crypto',
                                header_name='openssl/crypto.h',
                                define_name='HAVE_SSL',
                                uselib_store='SSL')
    if not conf.get_define ("HAVE_SSL"):
        conf.fatal ("Cannot find SSL libraries")

    conf.load('boost')

    if conf.options.ns3_enable:
        conf.load('ns3')
        conf.define('NS3_MODULE', 1)
        conf.check_modules(['core', 'network', 'internet'], mandatory = True)
        conf.check_modules(['NDNabstraction'], mandatory = True)
        conf.check_modules(['point-to-point'], mandatory = False)
        conf.check_modules(['point-to-point-layout'], mandatory = False)

        conf.check_boost(lib='system iostreams thread')
    else:
        conf.check_boost(lib='system iostreams test thread')
        conf.define ('STANDALONE', 1)

    if not conf.options.no_debug:
        conf.define ('_DEBUG', 1)

    
    try:
        conf.load('doxygen')
    except:
        pass

    conf.load('ccnx tinyxml')
    conf.check_ccnx (path=conf.options.ccnx_dir)
    conf.check_tinyxml (path=conf.options.ccnx_dir)

    # else:
        # if 'CXXFLAGS' in conf.env:
        #     tmp = conf.env['CXXFLAGS']
        # else:
        #     tmp = []
        # conf.env['CXXFLAGS'] = tmp + ['-g']
        # if 'CFLAGS' in conf.env:
        #     tmp = conf.env['CFLAGS']
        # else:
        #     tmp = []
        # conf.env['CFLAGS'] = tmp + ['-g']
        # _report_optional_feature(conf, "debug", "Debug Symbols", True, '')

    if conf.options.log4cxx:
        conf.check_cfg(package='liblog4cxx', args=['--cflags', '--libs'], uselib_store='LOG4CXX', mandatory=True)
                   
def build (bld):
    if bld.get_define ("NS3_MODULE"):
        sync_ns3 = bld.shlib (
            target = "sync-ns3",
            features=['cxx', 'cxxshlib'],
            use = 'BOOST BOOST_IOSTREAMS SSL TINYXML CCNX ' + ' '.join (['ns3_'+dep for dep in ['core', 'network', 'internet', 'NDNabstraction']]).upper (),
            source = bld.path.ant_glob(['model/sync-*.cc',
                                        'helper/sync-*.cc']),
            )
        
        
        # from waflib import Utils,Logs,Errors
        # Logs.pprint ('CYAN', program.use)
        
    else:
        libsync = bld.shlib (target=APPNAME, 
                             features=['cxx', 'cxxshlib'],
                             source = bld.path.ant_glob(['model/sync-*.cc',
                                                         'helper/sync-*.cc']),
                             use = 'BOOST BOOST_IOSTREAMS BOOST_THREAD SSL TINYXML CCNX')

        # Unit tests
        unittests = bld.program (target="unit-tests",
                             source = bld.path.ant_glob(['test/**/*.cc']),
                             features=['cxx', 'cxxprogram'],
                             use = 'BOOST_TEST sync')

        if bld.get_define ("HAVE_LOG4CXX"):
            libsync.use += ' LOG4CXX'
            unittests.use += ' LOG4CXX'

# doxygen docs
from waflib.Build import BuildContext
class doxy (BuildContext):
    cmd = "doxygen"
    fun = "doxygen"

def doxygen (bld):
    if not bld.env.DOXYGEN:
        bld.fatal ("ERROR: cannot build documentation (`doxygen' is not found in $PATH)")
    bld (features="doxygen",
         doxyfile='doc/doxygen.conf',
         output_dir = 'doc')
