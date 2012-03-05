# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def options(opt):
    opt.load('compiler_c')
    opt.load('compiler_cxx')
    opt.tool_options('boost', tooldir=["waf-tools"])

def configure(conf):
    conf.load("compiler_cxx")
    conf.check_cfg(atleast_pkgconfig_version='0.20')
    conf.check_cfg(package='openssl', args=['--cflags', '--libs'], uselib_store='SSL')

    conf.check_tool('boost')
    conf.check_boost(lib='signals filesystem iostreams regex')
    if not conf.env.LIB_BOOST:
        conf.check_boost(lib='signals filesystem iostreams regex', libpath="/usr/lib64")

def build (bld):
    module = bld.new_task_gen(features=['cxx', 'cxxshlib'])
    module.source = bld.path.ant_glob(['model/*.cc',
                                       'helper/*.cc'])
    module.uselib = 'BOOST BOOST_IOSTREAMS SSL'

def build_ns3 (bld):
    deps = ['core', 'network', 'NDNabstraction']
    if bld.env['ENABLE_PYTHON_BINDINGS']:
        deps.append ('visualizer')

    module = bld.create_ns3_module ('sync', deps)
    module.uselib = 'BOOST BOOST_IOSTREAMS SSL'

    # tests = bld.create_ns3_module_test_library('sync')
    # tests.source = [
    #     'test/sync-test-suite.cc',
    #     ]

    headers = bld.new_task_gen(features=['ns3header'])
    headers.module = 'sync'
    headers.source = [
        'model/sync-app.h',
        ]

    # if not bld.env['ENABLE_NDN_ABSTRACT']:
    #     bld.env['MODULES_NOT_BUILT'].append('NDNabstraction')
    #     return
   
    module.source = bld.path.ant_glob(['model/*.cc',
                                       'helper/*.cc'])

    # if bld.env.ENABLE_EXAMPLES:
    #     bld.add_subdirs('examples')

    # bld.ns3_python_bindings()

