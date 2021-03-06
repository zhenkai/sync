# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='0.0.1'
APPNAME='sync'

from waflib import Build, Logs

def options(opt):
    opt.add_option('--debug',action='store_true',default=False,dest='debug',help='''debugging mode''')
    opt.add_option('--log4cxx', action='store_true',default=False,dest='log4cxx',help='''Compile with log4cxx/native NS3 logging support''')
    opt.add_option('--ns3',     action='store_true',default=False,dest='ns3_enable',help='''Compile as NS-3 module''')
    opt.add_option('--ns3-debug', action='store_true',default=False,dest='ns3_debug',help='''Link against debug NS3 libraries. Optimized version will be used otherwise''')
    opt.add_option('--test', action='store_true',default=False,dest='_test',help='''build unit tests''')
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.load('boost')
    opt.load('doxygen')
    opt.load('gnu_dirs')
    opt.load('ccnx ns3 protobuf', tooldir=["waf-tools"])

def configure(conf):
    conf.load("compiler_cxx")
    conf.load('gnu_dirs')

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

        conf.load ('ccnx')
        conf.check_ccnx (path=conf.options.ccnx_dir)

        if conf.options.log4cxx:
            conf.check_cfg(package='liblog4cxx', args=['--cflags', '--libs'], uselib_store='LOG4CXX', mandatory=True)

    if conf.options.debug:
        conf.define ('_DEBUG', 1)
        conf.env.append_value('CXXFLAGS', ['-O0', '-g3'])
    else:
        conf.env.append_value('CXXFLAGS', ['-O3', '-g'])

    if conf.options._test:
      conf.define('_TEST', 1)

    try:
        conf.load('doxygen')
    except:
        pass

    conf.load('protobuf')

def build (bld):
    bld.post_mode = Build.POST_LAZY

    bld.add_group ("protobuf")

    x = bld (
        features = ["protobuf"],
        source = ["model/sync-state.proto"],
        target = ["model/sync-state.pb"],
        )

    bld.add_group ("code")

    if bld.get_define ("NS3_MODULE"):
        libsync = bld.shlib (
            target = "sync-ns3",
            features=['cxx', 'cxxshlib'],
            source =  [
                'ns3/sync-ccnx-wrapper.cc',
                'ns3/sync-ns3-name-info.cc',
                'ns3/sync-scheduler.cc',
                'ns3/sync-logic-helper.cc',
                
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
            dynamic_source = [
                'model/sync-state.pb.cc',
                ],
            use = 'BOOST BOOST_IOSTREAMS SSL PROTOBUF ' + ' '.join (['ns3_'+dep for dep in ['core', 'network', 'internet', 'NDNabstraction']]).upper (),
            includes = ['include', 'model', 'include/ns3', 'helper'],
            )

        example = bld.program (
            target = "sync-example",
            features=['cxx', 'cxxprogram'],
            source = ['examples/sync-example.cc'],
            use = 'libsync',
            includes = ['include', 'model', 'include/ns3', 'helper'],
            )

        sync_eval = bld.program (
            target = "sync-eval",
            features=['cxx', 'cxxprogram'],
            source = ['evaluation/sync-eval.cc',
                      'evaluation/standard-muc.cc',
                      'evaluation/sync-muc.cc',
                      ],
            use = 'libsync',
            includes = ['include', 'model', 'include/ns3', 'helper'],
            )
        # from waflib import Utils,Logs,Errors
        # Logs.pprint ('CYAN', program.use)
        
    else:
        libsync = bld (
            target=APPNAME,
            features=['cxx', 'cxxshlib'],
            source =  [
                'ccnx/sync-ccnx-wrapper.cc',
                'ccnx/sync-scheduler.cc',
                'ccnx/sync-log.cc',
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
            dynamic_source = [
                'model/sync-state.pb.cc',
                ],
            use = 'BOOST BOOST_IOSTREAMS BOOST_THREAD SSL PROTOBUF CCNX',
            includes = ['include', 'model', 'helper'],
            )
        
        # Unit tests
        if bld.get_define("_TEST"):
          unittests = bld.program (
              target="unit-tests",
              source = bld.path.ant_glob(['test/**/*.cc']),
              features=['cxx', 'cxxprogram'],
              use = 'BOOST_TEST sync',
              includes = ['include', 'model', 'helper'],
              )

        if bld.get_define ("HAVE_LOG4CXX"):
            libsync.use += ' LOG4CXX'
            if bld.get_define("_TEST"):
                unittests.use += ' LOG4CXX'

        headers = bld.path.ant_glob(['include/*.h'])
        headers.extend (bld.path.get_bld().ant_glob(['model/sync-state.pb.h']))
        bld.install_files ("%s/sync" % bld.env['INCLUDEDIR'], headers)

        pc = bld (
            features = "subst",
            source='libsync.pc.in',
            target='libsync.pc',
            install_path = '${LIBDIR}/pkgconfig',
            PREFIX       = bld.env['PREFIX'],
            INCLUDEDIR   = "%s/sync" % bld.env['INCLUDEDIR'],
            VERSION      = VERSION,
            )

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
