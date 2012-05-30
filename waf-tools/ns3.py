## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

import waflib
from waflib.Configure import conf
from waflib import Utils,Logs,Errors

@conf
def _print_optional_features(conf):
    # Write a summary of optional features status
    print "---- Summary of optional NS-3 features:"
    Logs.pprint ('RED', "---- Summary of optional NS-3 features:")
    # for (name, caption, was_enabled, reason_not_enabled) in conf.env['NS3_OPTIONAL_FEATURES']:
    #     if was_enabled:
    #         status = 'enabled'
    #     else:
    #         status = 'not enabled (%s)' % reason_not_enabled
    #     print "%-30s: %s" % (caption, status)

@conf
def _check_dependencies(conf, required, mandatory):
    # Logs.pprint ('CYAN', '  + %s' % required)
    found = []
    
    libversion = "optimized"
    if conf.options.ns3_debug:
        libversion = "debug"
    
    for module in required:
        retval = conf.check_cfg(package = 'libns3-dev-%s-%s' % (module, libversion),
                                args='--cflags --libs', mandatory=mandatory,
                                msg="Checking for ns3-%s" % module,
                                uselib_store='NS3_%s' % module.upper())
        # Logs.pprint ('CYAN', 'NS3_%s' % module.upper())
        if not retval is None:
            found.append(module)
    import copy
    if not 'NS3_MODULES_FOUND' in conf.env:
        conf.env['NS3_MODULES_FOUND'] = []
    conf.env['NS3_MODULES_FOUND'] = conf.env['NS3_MODULES_FOUND'] + copy.copy(found)

def modules_uselib(bld, names):
    return ['NS3_%s' % name.upper() for name in names] + \
        ['NS3_LIBRARY_%s' % name.upper() for name in names] + \
        ['NS3_HEADERS_%s' % name.upper() for name in names]

def modules_found(bld, needed):
    for module in needed:
        if not module in bld.env['NS3_MODULES_FOUND']:
            return False
    return True

@conf
def check_modules(conf, modules, mandatory = True):
    import os

    if not 'NS3_CHECK_MODULE_ONCE' in conf.env:
        conf.env['NS3_CHECK_MODULE_ONCE'] = ''

        conf.check_cfg(atleast_pkgconfig_version='0.0.0')

        if conf.options.log4cxx:
            conf.env.append_value('DEFINES', 'NS3_LOG_ENABLE')

    conf._check_dependencies(modules, mandatory)
    conf._print_optional_features

@conf
def print_ns3_feature_summary(conf):
    Logs.pprint ('CYAN', "---- Summary of optional NS-3 features:")
    conf._print_optional_features

