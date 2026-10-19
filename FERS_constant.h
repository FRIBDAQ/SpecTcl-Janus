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

// Janus writes one event per board; parameters are indexed as BCC,
// i.e. board_id*FERS_BOARD_STRIDE + channel (board 4 ch 3 -> LG.403).
// Indices B64 - B99 are never filled
#define FERS_NCH_PER_BOARD 64
#define FERS_MAX_BOARDS    8
#define FERS_BOARD_STRIDE  100
#define FERS_MAX_NCH       (FERS_BOARD_STRIDE*FERS_MAX_BOARDS)

#endif