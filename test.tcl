#!/usr/bin/env tclsh

lappend auto_path .
package require Tclshp 0.3

# Based on shapelib-1.3.0/makeshape.sh 

set test "testshp"

::shp::create $test 5
::dbf::create $test -s Description 30 -n TestInt 6 0 -n TestDouble 16 5

::shp::add $test 0 0 0 100 100 100 100 0 0 0 + 20 20 20 30 5 30 20 20
::dbf::add $test "Square with triangle missing" 1.5 2.5

::shp::add $test 150 150 160 150 180 170 150 150
::dbf::add $test "Smaller triangle" 100 1000.25

::shp::add $test 150 150 160 150 180 170 150 150
::dbf::add $test "" "" ""

puts [::shp::info $test]
puts [join [::shp::get $test] "\n"]

puts [::dbf::info $test]
puts [join [::dbf::get $test] "\n"]
