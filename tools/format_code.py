#!/usr/bin/env python

# Copyright 2018 Google LLC
#
# This is part of the Google Cloud IoT Device SDK for Embedded C,
# it is licensed under the BSD 3-Clause license; you may not use this file
# except in compliance with the License.
#
# You may obtain a copy of the License at:
#  https://opensource.org/licenses/BSD-3-Clause
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import argparse
import uuid
import subprocess
import shlex

CLANG_FORMAT = "clang-format -style=Google"

def clangFormatFile( filename ):
    args = shlex.split( CLANG_FORMAT )
    args += [ "-i", filename ]

    print( args )

    p = subprocess.Popen( args )

def findFiles( startDir, fileExt, recLevel, currLevel = 0 ):
    contents    = os.listdir( startDir )

    with open(".clang-format-ignore") as f:
        files_to_ignore = [x.strip('\n') for x in f.readlines()]

    files   = [ x for x in contents if os.path.isfile( os.path.join( startDir, x ) ) and x.endswith( fileExt ) and x not in files_to_ignore ]
    dirs    = [ x for x in contents if os.path.isdir( os.path.join( startDir, x ) ) and x[ 0 ] != '.' and x not in files_to_ignore ]

    for f in files:
        filename = os.path.join( startDir, f )
        clangFormatFile( filename )

    if recLevel == 0 or ( recLevel > 0 and currLevel < recLevel ):
        for d in dirs:
            findFiles( os.path.join( startDir, d ), fileExt, recLevel,
                    currLevel + 1 )

if __name__ == '__main__':

    parser = argparse.ArgumentParser( description='Source code formatter' )
    parser.add_argument( '-r', dest='recursive', type=int, default=100,
                       help='recursive mode, default 1, set 0 if you want to enable unlimited recursion')

    args        = parser.parse_args()
    recursive   = args.recursive

    directories = [ '../src/', '../include/', '../examples/' ];

    for dir in directories:
        findFiles( dir, ".h", recursive )
        findFiles( dir, ".c", recursive )
