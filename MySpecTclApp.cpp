/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2008, All rights reserved";

// Class: CMySpecTclApp

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "MySpecTclApp.h"
#include "EventProcessor.h"
#include "TCLAnalyzer.h"
#include <Event.h>
#include <TreeParameter.h>
#include "CFERSUnpacker.h"

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// Local Class definitions:

// Instantiate the unpackers we'll use.

static CFERSUnpacker       gFERSUnpacker;

//  Function:
//    void CreateAnalysisPipeline(CAnalyzer& rAnalyzer)
//  Operation Type:
//     Override
/*
Purpose:

  Sets up an analysis pipeline.  This function is required and must
be filled in by the SpecTcl user.  Pipeline elements are objects
which are members of classes derived from the CEventProcessor
class. They should be added to the Analyzer's event processing pipeline
by calling RegisterEventProcessor (non virtual base class member).
   The sample implementation in this
file produces a two step pipeline.  The first step decodes a fixed length
event into the CEvent array.  The first parameter is put into index 1 and so on.
The second step produces a compiled pseudo parameter by adding event array
elements 1 and 2 and putting the result into element 0.


*/

void
CMySpecTclApp::CreateAnalysisPipeline(CAnalyzer& rAnalyzer)
{
    RegisterEventProcessor(gFERSUnpacker, "FERS");
}

// Constructors, destructors and other replacements for compiler cannonicals:

CMySpecTclApp::CMySpecTclApp()
{
}

// Destructor:
CMySpecTclApp::~CMySpecTclApp ( )
{
}

// Functions for class CMySpecTclApp


//  Function:
//    void BindTCLVariables(CTCLInterpreter& rInterp)
//  Operation Type:
//     override
/*
Purpose:

 Add code to this function to bind any TCL variable to
 the SpecTcl interpreter.  Note that at this time,
 variables have not yet been necessarily created so you
 can do Set but not necessarily Get operations.



*/
void
CMySpecTclApp::BindTCLVariables(CTCLInterpreter& rInterp)
{
  CTclGrammerApp::BindTCLVariables(rInterp);
}

//  Function:
//    void SourceLimitScripts(CTCLInterpreter& rInterpreter)
//  Operation Type:
//     Override
/*
Purpose:

Add code here to source additional variable setting
scripts.  At this time the entire SpecTcl/Tk infrastructure
is not yet set up.  Scripts run at this stage can only run
basic Tcl/Tk commands, and not SpecTcl extensions.
Typically, this might be used to set a bunch of initial values
for variables which were bound in BindTCLVariables.

*/
void
CMySpecTclApp::SourceLimitScripts(CTCLInterpreter& rInterpreter)
{
  CTclGrammerApp::SourceLimitScripts(rInterpreter);
}

//  Function:
//    void SetLimits()
//  Operation Type:
//     overide
/*
Purpose:

Called after BindVariables and SourceLimitScripts.
This function can be used to fetch values of bound Tcl
variables which were modified/set by the limit scripts to
update program default values.

*/
void
CMySpecTclApp::SetLimits()
{
  CTclGrammerApp::SetLimits();
}

//  Function:
//    void CreateHistogrammer()
//  Operation Type:
//     Override
/*
Purpose:

Creates the histogramming data sink.  If you want to override
this in general you probably won't make use of the actual
base class function.  You might, however extend this by
defining a base set of parameters and histograms from within
the program.

*/
void
CMySpecTclApp::CreateHistogrammer()
{
  CTclGrammerApp::CreateHistogrammer();
}

/*! \brief Create custom displays
 *
 * If a user creates their own custom display that they want to use,
 * they can create it here by adding it to the DisplayInterface. You could
 * also use this to override certain parameters in the provided displays.
 */
void
CMySpecTclApp::CreateDisplays()
{
  CTclGrammerApp::CreateDisplays();
}

//  Function:
//    void SelectDisplayer()
//  Operation Type:
//     Override.
/*
Purpose:

Select a displayer object. The implementation in the base class
chooses the display and starts it. There is not a whole lot of
reason you would want to change this behavior.
*/
void
CMySpecTclApp::SelectDisplayer()
{
  CTclGrammerApp::SelectDisplayer();
}

/*! \brief Set up a displays
 *
 * This is typically used to add observer objects to the displays
 * that connect SpecTcl actions with the current display.
 */
void
CMySpecTclApp::SetUpDisplay()
{
  CTclGrammerApp::SetUpDisplay();
}

//  Function:
//    void SetupTestDataSource()
//  Operation Type:
//     Override
/*
Purpose:

 Allows you to set up a test data source.  At
present, SpecTcl must have a data source of some sort
connected to it... The default test data source produces a
fixed length event where all parameters are selected from
a gaussian distribution.  If you can figure out how to do it,
you can setup your own data source... as long as you don't
start analysis, the default one is harmless.

*/
void
CMySpecTclApp::SetupTestDataSource()
{
  CTclGrammerApp::SetupTestDataSource();
}

//  Function:
//    void CreateAnalyzer(CEventSink* pSink)
//  Operation Type:
//     Override
/*
Purpose:

Creates an analyzer.  The Analyzer is connected to the data
source which supplies buffers.  Connected to the analyzer is a
buffer decoder and an event unpacker.  The event unpacker is
the main experiment dependent chunk of code, not the analyzer.
The analyzer constructed by the base class is a CTclAnalyzer instance.
This is an analyzer which maintains statistics about itself in Tcl Variables.

*/
void
CMySpecTclApp::CreateAnalyzer(CEventSink* pSink)
{
  CTclGrammerApp::CreateAnalyzer(pSink);
}

//  Function:
//    void SelectDecoder(CAnalyzer& rAnalyzer)
//  Operation Type:
//     Override
/*
Purpose:

  Selects a decoder and attaches it to the analyzer.
A decoder is responsible for knowing the overall structure of
a buffer produced by a data analysis system.  The default code
constructs a CNSCLBufferDecoder object which knows the format
of NSCL buffers.

*/
void
CMySpecTclApp::SelectDecoder(CAnalyzer& rAnalyzer)
{
  CTclGrammerApp::SelectDecoder(rAnalyzer);
}



//  Function:
//    void AddCommands(CTCLInterpreter& rInterp)
//  Operation Type:
//     Override
/*
Purpose:

This function adds commands to extend Tcl/Tk/SpecTcl.
The base class function registers the standard SpecTcl command
packages.  Your commands can be registered at this point.
Do not remove the sample code or the SpecTcl commands will
not get registered.

*/
void
CMySpecTclApp::AddCommands(CTCLInterpreter& rInterp)
{
  CTclGrammerApp::AddCommands(rInterp);
}

//  Function:
//    void SetupRunControl()
//  Operation Type:
//     Override.
/*
Purpose:

  Sets up the Run control object.  The run control object
is responsible for interacting with the underlying operating system
and programming framework to route data from the data source to
the SpecTcl analyzer.  The base class object instantiates a
CTKRunControl object.  This object uses fd waiting within the
Tcl/TK event processing loop framework to dispatch buffers for
processing as they become available.

*/
void
CMySpecTclApp::SetupRunControl()
{
  CTclGrammerApp::SetupRunControl();
}

//  Function:
//    void SourceFunctionalScripts(CTCLInterpreter& rInterp)
//  Operation Type:
//     Override
/*
Purpose:

  This function allows the user to source scripts
which have access to the full Tcl/Tk/SpecTcl
command set along with whatever extensions have been
added by the user in AddCommands.

*/
void
CMySpecTclApp::SourceFunctionalScripts(CTCLInterpreter& rInterp)
{
  CTclGrammerApp::SourceFunctionalScripts(rInterp);
}

//  Function:
//    int operator()()
//  Operation Type:
//     Override.
/*
Purpose:

  Entered at Tcl/Tk initialization time (think of this
as the entry point of the SpecTcl program).  The base
class default  implementation calls the member functions
of this class in an appropriate order.  It's possible for the user
to extend this functionality by adding code to this function.

*/
int
CMySpecTclApp::operator()()
{
  return CTclGrammerApp::operator()();
}

CMySpecTclApp   myApp;



#ifdef SPECTCL_5_INIT
CTclGrammerApp* CTclGrammerApp::m_pInstance = &myApp;
CTCLApplication* gpTCLApplication;

#else
CTclGrammerApp& app(myApp);	// Create an instance of me.
CTCLApplication* gpTCLApplication=&app;  // Findable by the Tcl/tk framework.
#endif
