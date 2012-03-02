# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build (bld):
    deps = ['core', 'network', 'NDNabstraction']
    if bld.env['ENABLE_PYTHON_BINDINGS']:
        deps.append ('visualizer')

    module = bld.create_ns3_module ('sync', deps)
    module.uselib = 'BOOST BOOST_IOSTREAMS'

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

