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
 *         卞超轶 Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-app-data-fetch.h"
#include "sync-log.h"

using namespace std;
using namespace boost;

namespace Sync
{

INIT_LOGGER ("AppDataFetch");

void
AppDataFetch::onUpdate (const std::string &prefix, const SeqNo &newSeq, const SeqNo &oldSeq)
{
  _LOG_FUNCTION (this << ", " << prefix << ", " << newSeq << ", " << oldSeq);
  
  // sequence number logic here
  uint32_t start = 0;
  if (oldSeq.isValid ())
    {
      start = oldSeq.getSeq () + 1;
    }
  uint32_t end = newSeq.getSeq ();

  //
  // add logic for wrap around
  //

  for (uint32_t i = start; i <= end; i++)
    {
      ostringstream interestName;
      interestName << prefix << "/" << newSeq.getSession () << "/" << i;
      m_ccnxHandle->sendInterest (interestName.str (), m_dataCallback);
    }
}

void
AppDataFetch::onRemove (const std::string &prefix)
{
  _LOG_FUNCTION (this << ", " << prefix);
  
  // I guess this should be somewhere in app
}

}
