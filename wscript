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
        conf.define ('NS3_LOG_ENABLE', 1)
    else:
        conf.check_boost(lib='system iostreams test thread')
        conf.define ('STANDALONE', 1)

        conf.load ('ccnx')
        conf.check_ccnx (path=conf.options.ccnx_dir)

        if conf.options.log4cxx:
            conf.check_cfg(package='liblog4cxx', args=['--cflags', '--libs'], uselib_store='LOG4CXX', mandatory=True)

    if not conf.options.no_debug:
        conf.define ('_DEBUG', 1)
        conf.env.append_value('CXXFLAGS', ['-O0', '-g3'])
    else:
        conf.env.append_value('CXXFLAGS', ['-O3'])

    try:
        conf.load('doxygen')
    except:
        pass

    conf.load('tinyxml')
    conf.check_tinyxml ()
                   
def build (bld):
    if bld.get_define ("NS3_MODULE"):
        sync_ns3 = bld.shlib (
            target = "sync-ns3",
            features=['cxx', 'cxxshlib'],
            source =  [
                'ns3/sync-ccnx-wrapper.cc',
                'ns3/sync-ns3-name-info.cc',
                'ns3/sync-scheduler.cc',
                
                # 'model/sync-app-data-fetch.cc',
                # 'model/sync-app-data-publish.cc',
                # 'ns3/sync-app.cc',

                'model/sync-diff-leaf.cc',
                'model/sync-diff-state.cc',
                'model/sync-digest.cc',
                'model/sync-full-leaf.cc',
                'model/sync-full-state.cc',
                'model/sync-interest-table.cc',
                'model/sync-leaf.cc',
                'model/sync-logic.cc',
                'model/sync-name-info.cc',
                'model/sync-seq-no.cc',
                'model/sync-state.cc',
                'model/sync-std-name-info.cc',
                ],
            use = 'BOOST BOOST_IOSTREAMS SSL TINYXML CCNX ' + ' '.join (['ns3_'+dep for dep in ['core', 'network', 'internet', 'NDNabstraction']]).upper (),
            includes = ['model', 'ns3', 'helper'],
            )

        example = bld.program (
            target = "sync-example",
            features=['cxx', 'cxxprogram'],
            source = ['examples/sync-example.cc'],
            use = 'sync-ns3',
            includes = ['model', 'ccnx', 'helper'],
            )
        # from waflib import Utils,Logs,Errors
        # Logs.pprint ('CYAN', program.use)
        
    else:
        libsync = bld.shlib (
            target=APPNAME, 
            features=['cxx', 'cxxshlib'],
            source =  [
                'ccnx/sync-ccnx-wrapper.cc',
                'ccnx/sync-scheduler.cc',
                'ccnx/sync-log.cc',
                'ccnx/sync-app-data-fetch.cc',
                'ccnx/sync-app-data-publish.cc',
                'ccnx/sync-app-socket-c.cc',
                'ccnx/sync-app-socket.cc',
                
                'model/sync-diff-leaf.cc',
                'model/sync-diff-state.cc',
                'model/sync-digest.cc',
                'model/sync-full-leaf.cc',
                'model/sync-full-state.cc',
                'model/sync-interest-table.cc',
                'model/sync-leaf.cc',
                'model/sync-logic.cc',
                'model/sync-name-info.cc',
                'model/sync-seq-no.cc',
                'model/sync-state.cc',
                'model/sync-std-name-info.cc',
                ],
            use = 'BOOST BOOST_IOSTREAMS BOOST_THREAD SSL TINYXML CCNX',
            includes = ['model', 'ccnx', 'helper'],
            )
        
        # Unit tests
        unittests = bld.program (
            target="unit-tests",
            source = bld.path.ant_glob(['test/**/*.cc']),
            features=['cxx', 'cxxprogram'],
            use = 'BOOST_TEST sync',
            includes = ['model', 'ccnx', 'helper'],
            )

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
