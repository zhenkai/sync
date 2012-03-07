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

#define TIXML_USE_STL

#include <string>
#include <tinyxml.h>
#include "sync-state.h"
#include "sync-std-name-info.h"

using namespace std;

namespace Sync
{
  string & operator >> (string &DataBuffer, State &state)
  {
    TiXmlDocument doc;

    doc.Parse(DataBuffer.c_str());
    for (TiXmlElement *iterator = doc.RootElement(); iterator != NULL; iterator = iterator->NextSiblingElement())
    {
      if (strcmp(iterator->Attribute("action"), "UPDATE") == 0)
      {
	TiXmlElement *name = iterator->FirstChildElement();
	TiXmlElement *session = name->NextSiblingElement();
	TiXmlElement *sequence = session->NextSiblingElement();
	NameInfoConstPtr info = StdNameInfo::FindOrCreate(name->GetText());
	SeqNo seqNo(atoi(session->GetText()), atoi(sequence->GetText()));

	state.update(info, seqNo);
      }
      else
      {
	TiXmlElement *name = iterator->FirstChildElement();
	NameInfoConstPtr info = StdNameInfo::FindOrCreate(name->GetText());
	state.remove(info);
      }
    }

    return DataBuffer;
  }
}