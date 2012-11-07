#
#    Tclshp, a Tcl API for shapelib
#    Copyright (C) 2006  Devin J. Eyre
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this library; if not, write to the Free
#    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
#
package provide Tclshp 0.2
load [file join [file dir [info script]] Tclshp.so]
namespace eval ::shp {
   rename ::shpadd ::shp::_add
   rename ::shpget ::shp::get
   rename ::shpinfo ::shp::info
   rename ::shpcreate ::shp::create
   proc add {shpfile coordlist} {
      if {[llength [lindex $coordlist 0]] > 1} {
         #It's a list of lists
         return [eval _add $shpfile [join $coordlist " + "]]
      } else {
         return [eval _add $shpfile $coordlist]
      }
   }
   proc Create {shpfile shptype field_defs} {
      create $shpfile $shptype
      set dbf_res [eval ::dbf::create $shpfile $field_defs]
      return $dbf_res
   }
   proc Add {shpfile shplist dbf_values} {
      add $shpfile $shplist
      set dbf_res [eval ::dbf::add $shpfile $dbf_values]
   }
}
namespace eval ::dbf {
   rename ::dbfcreate ::dbf::create
   rename ::dbfinfo ::dbf::info
   rename ::dbfadd ::dbf::add
   rename ::dbfget ::dbf::get
}
