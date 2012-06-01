/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_LOG_H
#define SYNC_LOG_H

#ifdef NS3_MODULE

#include <ns3/log.h>

#ifdef _DEBUG

#define INIT_LOGGER(name) NS_LOG_COMPONENT_DEFINE(name);

#define _LOG_INFO(x) NS_LOG_INFO(x)

#define _LOG_DEBUG(x) NS_LOG_DEBUG(x)

#define _LOG_TRACE(x) NS_LOG_LOGIC(x)

#define _LOG_FUNCTION(x) NS_LOG_FUNCTION(x)

#define _LOG_FUNCTION_NOARGS NS_LOG_FUNCTION_NOARGS

#else

#define INIT_LOGGER(name) 
#define _LOG_INFO(x) 
#define _LOG_DEBUG(x) 
#define _LOG_TRACE(x)
#define _LOG_FUNCTION(x)
#define _LOG_FUNCTION_NOARGS

#endif

#else

#ifdef HAVE_LOG4CXX

#include <log4cxx/logger.h>

#define INIT_LOGGER(name) \
  static log4cxx::LoggerPtr staticModuleLogger = log4cxx::Logger::getLogger (name);

#define _LOG_INFO(x) \
  LOG4CXX_INFO(staticModuleLogger, x);

#define _LOG_DEBUG(x)                           \
  LOG4CXX_DEBUG(staticModuleLogger, x);

#define _LOG_TRACE(x) \
  LOG4CXX_TRACE(staticModuleLogger, x);

#define _LOG_FUNCTION(x) \
  LOG4CXX_TRACE(staticModuleLogger, __FUNCTION__ << "(" << x << ")");

#define _LOG_FUNCTION_NOARGS \
  LOG4CXX_TRACE(staticModuleLogger, __FUNCTION__ << "()");

void
INIT_LOGGERS ();

#else

#define INIT_LOGGER(name)
#define _LOG_FUNCTION(x)
#define _LOG_FUNCTION_NOARGS
#define _LOG_TRACE(x)
#define _LOG_INFO(x)
#define INIT_LOGGERS(x)

#ifdef _DEBUG

#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>

#define _LOG_DEBUG(x) \
  std::clog << boost::get_system_time () << " " << boost::this_thread::get_id () << " " << x << endl;

#else
#define _LOG_DEBUG(x)
#endif

#endif // HAVE_LOG4CXX

#endif // NS3_MODULE

#endif // SYNC_LOG_H
