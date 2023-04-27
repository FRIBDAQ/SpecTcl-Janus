/******************************************************************************
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2023.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
          Genie Jhang <changj@frib.msu.edu>

          Facility for Rare Isotope Beams
          Michigan State University
          East Lansing, MI 48824-1321
******************************************************************************/

#ifndef _FERS_CONSTANT_H_
#define _FERS_CONSTANT_H_

#define ACQMODE_SPECT      0x01
#define ACQMODE_TIMING     0x02
#define ACQMODE_SPECTIMING 0x03
#define ACQMODE_COUNTING   0x04

#define TIMEUNIT_LSB 0
#define TIMEUNIT_NS  1

#define DATATYPE_LG    0x01
#define DATATYPE_HG    0x02
#define DATATYPE_LGHG  0x03

#define DATATYPE_TOA     0x10
#define DATATYPE_TOT     0x20
#define DATATYPE_TOATOT  0x30

#endif