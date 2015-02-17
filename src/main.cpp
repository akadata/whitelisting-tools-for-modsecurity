/*
    This file is part of auditlog2db.

    auditlog2db is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    auditlog2db is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with auditlog2db.  If not, see <http://www.gnu.org/licenses/>.
*/

// inlcude 3rd party libraries
#include <iostream>
#include <string>
#include <boost/regex.hpp>
#include <sqlite3.h>
#include <fstream>
#include <vector>
#include <getopt.h> // option parsing

// include other bits of this program
#include "headerlines.h"
#include "prompt_inputs.h"
#include "logchop.h"
#include "help.h"


using namespace std;
using std::vector;





//===========================================================================
// 1. Parse commandline arguments
// 2. Check if database name and logfile name were specified with command
//	line option, if not prompt user
// 3. Run headerlines function to get a list of header locations within the
//	log file
// 4. Run logchop function to read log file and split at header locations,
//	insert into database
//===========================================================================



// main function has two arguments: first (argc) is the number of commandline
// arguments second (argv) is an array of pointers to the options.
// NB: argv[0] is the program name


int main (int argc , char **argv) {

  // ========================================================================
  // 1. Parse command line arguments
  // ========================================================================

  // store command line arguments as variables
  string progname = argv[0]; // program name is always argv[0]
  string database;
  string logfile;

  
  // use getopt long to parse the commandline arguments
  // https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
  
  // define flags, must be ints not bools for getopt
  static int debug;
  static int force;
  static int quiet;
  
  
  int c;
  while (1) {
    static struct option long_options[] = {
	// these options set a flag
	{"debug", no_argument, &debug, 1},
	{"force", no_argument, &force, 1},
	{"quiet", no_argument, &quiet, 1},
	// these options don't set a flag, instead they set a case for switch
	{"help", no_argument, 0, 'h'},
	{"input", required_argument, 0, 'i'},
	{"output", required_argument, 0, 'o'},
	{"version", no_argument, 0, 'v'},
	// array must be terminated with an element containing all zeros
	{0,0,0,0}
      };
    
    //getopt_long stores the option index here
    int option_index = 0;
    
    /* --- getopt_long ---
     * returns 0 if it has set a flag
     * returns -1 if there are no more options to handle
     * for any long option, returns the index in the array of longopts of the options
     *   definition, storing it into option_index
     * list of letters is possible options, letter then colon means required argument
     *   letter then two colons means optional argument
     *   still need to provide cases for the short options that set flags above
    */
    c = getopt_long (argc, argv, "dfqhi:o:v", long_options, &option_index);
    
    // if getopt_long returned -1, there are no more options, end the loop
    if (c == -1) { 
      break;
    } 
    
    switch (c) {
      case 0:
	// if this option set a flag, do nothing else now because the result
	//	of the flag will be printed later
	if (long_options[option_index].flag !=0) {
	  break;
	}
	// else the option has set a flag to zero and it won't be picked up
	// 	by the if statement below because if (debug) won't return
	//	true - print now
	cout << "Option " << long_options[option_index].name;
	if (optarg) { cout << "with arg " << optarg; }
	cout << endl;
	break;
      case 'd':
	// set debugging to true and break
	debug = 1;
	break;
      case 'f':
	// set force to true and break
	force = 1;
	break;
      case 'h':
	// print help and exit.
	help ();
	return 0;
      case 'i':
	if (!optarg) {
	  cout << "no optional argument" << endl;
	}
	cout << "option -i with value " << optarg << endl;
	logfile = optarg;
	break;
      case 'o':
	cout << "option -o with value " << optarg << endl;
	database = optarg;
	break;
      case 'q':
	quiet = 1;
	break;
      case 'v':
	// print version and exit. PACKAGE_VERSION is set by configure.ac
	cout << PACKAGE_VERSION << endl;
	return 0;
      case '?':
	// getopt_long already printed an error message
	break;
      default:
	abort();
    }
  }
  
  // instead of reporting '--debug' and '--force', we can report the flag set
  //	by them
  if (debug) { cout << "Debugging is on" << endl;}
  if (force) { cout << "Force option was specified" << endl;}
  if (quiet && debug) { cout << "Quiet option was specified" << endl;}
  
  // print any remaining command line arguments (not options)
  if (optind < argc) {
    cerr << "Non-option ARGV-elements: ";
      while (optind < argc) { cerr << argv[optind++] << ", ";}
      cerr << endl;
  }
  
  
  
  
  // ========================================================================
  // 2. Check if database name and logfile name were specified with command
  //	line option, if not prompt user
  // ========================================================================
  
  // the declarations and definitions for setdblocation and setlogfile are in prompt_inputs.cpp and prompt_inputs.h
  if (database == "") {
    cout << "Database location has not been specified" << endl;
    database = setdblocation(debug);
  }
  
  if (logfile == "") {
    cout << "Logfile location has not been specified" << endl;
    logfile = setlogfile(debug);
  }
  
  
  
  // ========================================================================
  // 3. Run headerlines function to get a list of header locations within the
  //	log file
  // ========================================================================
  
  // the declaration and definition for headerlines_vector_pair is in
  //	headerlines.cpp and headerlines.h
  vector<pair<int,string>> results = headerlines(logfile, debug);

  
  // ========================================================================
  // 4. Run logchop function to read log file and split at header locations,
  //	insert into database
  // ========================================================================
  
  
  int logchop_status = logchop(database, logfile, results, debug, force);
  
  
  
  return 0;
}
