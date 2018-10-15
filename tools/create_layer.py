#!/usr/bin/env python

# Copyright 2018 Google LLC
#
# This is part of the Google Cloud IoT Edge Embedded C Client,
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

import argparse
import os.path

functions = [
            { "name" : "push", "gen" : "forward" }
          , { "name" : "pull", "gen" : "backward" }
          , { "name" : "init", "gen" : "forward" }
          , { "name" : "connect", "gen" : "backward" }
          , { "name" : "close", "gen" : "forward" }
          , { "name" : "close_externally", "gen" : "backward" } ]

def tabs_2_spaces( s ):
   return s.replace( "\t", "    " )
h_pro               = tabs_2_spaces( "// Copyright 2018 Google LLC\n// This is part of Google Cloud IoT Edge Embedded C Client." )
h_guard_beg         = tabs_2_spaces( "#ifndef __%(layer_name)s_H__\n#define __%(layer_name)s_H__" )
h_guard_end         = tabs_2_spaces( "#endif // __%s_H__" )
c_includes          = tabs_2_spaces( "#include \"%(layer_name)s.h\"\n#include \"%(layer_name)s_data.h\"\n#include \"iotc_macros.h\"\n#include \"iotc_layer_api.h\"" )
h_includes          = tabs_2_spaces( "#include \"iotc_layer.h\"" )
fun_decl            = tabs_2_spaces( "layer_state_t %s(\n\t  void* context\n\t, void* data\n\t, layer_state_t state )" )
layer_data_get_decl = tabs_2_spaces( "\t%(layer_name)s_layer_data_t* layer_data\n\t\t= ( %(layer_name)s_layer_data_t* ) CON_SELF( context )->user_data;" )
unused              = tabs_2_spaces( "\tIOTC_UNUSED( data );" )
if_no_layer_data    = tabs_2_spaces( "\tif( layer_data == 0 )\n\t{\n\t\t// cleaning of not finished requests\n\t\tgoto err_handling;\n\t}\n" )
call_layer          = tabs_2_spaces( "\treturn CALL_ON_%s_%s( context, data, state );" )
err_sec             = tabs_2_spaces( "\terr_handling:\n\t\treturn LAYER_STATE_ERROR;" )
cpp_beg             = tabs_2_spaces( "#ifdef __cplusplus\nextern \"C\" {\n#endif" )
cpp_end             = tabs_2_spaces( "#ifdef __cplusplus\n}\n#endif" )
data_t              = tabs_2_spaces( "typedef struct\n{\n\tint field;\n} %s_t;" );

def make_name( layer_name, fun_name ):
    return "%s_layer_%s" % ( layer_name, fun_name )

def create_fun_dec( name ):
    return ( fun_decl % name );

def create_data_decs( layer_name ):
    ret  = h_pro + "\n\n"
    ret += ( h_guard_beg % { "layer_name" : ( layer_name + "_layer_data" ).upper() } ) + "\n\n"
    ret += cpp_beg + "\n\n"
    ret += data_t % ( layer_name + "_layer_data" ) + "\n\n"
    ret += cpp_end + "\n\n"
    ret += ( h_guard_end % ( layer_name + "_layer_data" ).upper() ) + "\n"

    return ret

def create_funs_decs( layer_name ):
    ret  = h_pro + "\n\n"
    ret += ( h_guard_beg % { "layer_name" : ( layer_name + "_layer" ).upper() } ) + "\n\n"
    ret += h_includes + "\n\n"

    ret += cpp_beg + "\n\n"

    for f_data in functions:
        ret += create_fun_dec( make_name( layer_name, f_data[ "name" ] ) ) + ";\n\n"

    ret += cpp_end + "\n\n"

    ret += h_guard_end % ( layer_name + "_layer" ).upper() + "\n"

    return ret

def create_call( gen, fun_name ):
    if gen == "forward":
        return call_layer % ( "PREV", fun_name.upper() )
    elif gen == "backward":
        return call_layer % ( "NEXT", fun_name.upper() )

    return "unknown"

def create_fun_def( layer_name, fun_name, gen ):
    name = make_name( layer_name, fun_name )

    ret = create_fun_dec( name ) + "\n"
    ret += "{\n"
    ret += unused + "\n\n"
    ret += ( layer_data_get_decl % { "layer_name" : layer_name } ) + "\n\n"
    ret += if_no_layer_data + "\n\n"
    ret += create_call( gen, fun_name ) + "\n\n"
    ret += err_sec + "\n"
    ret += "}\n"

    return ret

def create_funs_defs( layer_name ):
    ret = c_includes % { "layer_name" : layer_name + "_layer" } + "\n\n"

    ret += cpp_beg + "\n\n"

    for f_data in functions:
        ret += create_fun_def( layer_name, f_data[ "name" ], f_data[ "gen" ] ) + "\n"

    ret += cpp_end + "\n\n"

    return ret

def write_to_file( file_name, s ):
    if os.path.isfile( file_name ):
        raise RuntimeError( "file already exists" )

    print "writing to: %s" % file_name

    with open( file_name, "wb" ) as f:
        f.write( s )

if __name__ == '__main__':

    parser = argparse.ArgumentParser( description="libiotc layer creator" )
    parser.add_argument( '--name', dest = 'name', type = str, required='True', help = 'name of the layer you want to create' )
    parser.add_argument( '--no-pretend', dest = 'no_pret', action = 'store_const', const = True, default = False, help = 'disable printing to the console and enables writing to files' )

    args = parser.parse_args()

    name = args.name

    path = os.path.join( '.', 'src', 'libiotc', name )

    h_layer_file_name   = os.path.join( path, name + "_layer.h" )
    c_layer_file_name   = os.path.join( path, name + "_layer.c" )
    h_data_file_name    = os.path.join( path, name + "_layer_data.h" )

    if args.no_pret:
        if os.path.isdir( path ):
            raise RuntimeError( "path already exists!" )
        else:
            os.makedirs( path )

        write_to_file( h_layer_file_name, create_funs_decs( name ) )
        write_to_file( c_layer_file_name, create_funs_defs( name ) )
        write_to_file( h_data_file_name, create_data_decs( name ) )
    else:
        print create_funs_defs( name )
        print create_funs_decs( name )
        print create_data_decs( name )



