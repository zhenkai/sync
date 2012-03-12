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

#include "sync-state.h"
#include "sync-diff-leaf.h"
#include "sync-std-name-info.h"

#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/throw_exception.hpp>
#include <boost/lexical_cast.hpp>

#define TIXML_USE_STL
#include <tinyxml.h>

using namespace std;
using namespace boost;

typedef error_info<struct tag_errmsg, string> info_str; 

using namespace Sync::Error;

namespace Sync {

#ifdef _DEBUG
#define DEBUG_ENDL os << "\n";
#else
#define DEBUG_ENDL
#endif

std::ostream &
operator << (std::ostream &os, const State &state)
{
  os << "<state>"; DEBUG_ENDL;
  
  BOOST_FOREACH (shared_ptr<const Leaf> leaf, state.getLeaves ().get<ordered> ())
    {
      shared_ptr<const DiffLeaf> diffLeaf = dynamic_pointer_cast<const DiffLeaf> (leaf);
      if (diffLeaf != 0)
        {
          os << "<item action=\"" << diffLeaf->getOperation () << "\">"; DEBUG_ENDL;
        }
      else
        {
          os << "<item>"; DEBUG_ENDL;
        }
      os << "<name>" << *leaf->getInfo () << "</name>"; DEBUG_ENDL;
      if (diffLeaf == 0 || (diffLeaf != 0 && diffLeaf->getOperation () == UPDATE))
        {
          os << "<seq>" << leaf->getSeq () << "</seq>"; DEBUG_ENDL;
        }
      os << "</item>"; DEBUG_ENDL;
    }
  os << "</state>";
}

std::istream &
operator >> (std::istream &in, State &state)
{
  TiXmlDocument doc;
  in >> doc;

  if (doc.RootElement() == 0)
        BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("Empty XML"));
  
  for (TiXmlElement *iterator = doc.RootElement()->FirstChildElement ("item");
       iterator != 0;
       iterator = iterator->NextSiblingElement("item"))
    {
      TiXmlElement *name = iterator->FirstChildElement ("name");
      if (name == 0 || name->GetText() == 0)
        BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<name> element is missing"));
        
      NameInfoConstPtr info = StdNameInfo::FindOrCreate (name->GetText());
      
      if (iterator->Attribute("action") == 0 || strcmp(iterator->Attribute("action"), "update") == 0)
        {
          TiXmlElement *seq = iterator->FirstChildElement ("seq");
          if (seq == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<seq> element is missing"));
          
          TiXmlElement *session = seq->FirstChildElement ("session");
          TiXmlElement *seqno = seq->FirstChildElement ("seqno");

          if (session == 0 || session->GetText() == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<session> element is missing"));
          if (seqno == 0 || seqno->GetText() == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<seqno> element is missing"));

          state.update (info, SeqNo (
                                     lexical_cast<uint32_t> (session->GetText()),
                                     lexical_cast<uint32_t> (seqno->GetText())
                                     ));
        }
      else
        {
          state.remove (info);
        }
    }

  return in;
}

}
