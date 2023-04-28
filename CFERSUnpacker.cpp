#include "CFERSA5202Unpacker.h"
#include "CFERSUnpacker.h"
#include <BufferDecoder.h>
#include <TCLAnalyzer.h>
#include <iostream>
#include <stdexcept>
#include <climits>
#include <cfloat>

#include "FERS_fileheader.h"
#include "FERS_constant.h"

using namespace std;

CFERSUnpacker::CFERSUnpacker()
  : m_unpacker(),
    m_lg("LG", 10000, 0.0, (double) USHRT_MAX, "channels", 64, 0),
    m_hg("HG", 10000, 0.0, (double) USHRT_MAX, "channels", 64, 0),
    m_counts("counts", 10000, 0.0, (double) UINT_MAX, "counts", 64, 0),
    m_toa_int("ToA_int", 10000, 0.0, (double) UINT_MAX, "0.5 ns", 64, 0),
    m_tot_int("ToT_int", 10000, 0.0, (double) USHRT_MAX, "0.5 ns", 64, 0),
    m_toa_float("ToA_float", 10000, 0.0, FLT_MAX, "ns", 64, 0),
    m_tot_float("ToT_float", 10000, 0.0, FLT_MAX, "ns", 64, 0)
{
}

CFERSUnpacker::~CFERSUnpacker()
{
}

Bool_t
CFERSUnpacker::operator()(const Address_t pEvent,
                        CEvent& rEvent,
                        CAnalyzer& rAnalyzer,
                        CBufferDecoder& rDecoder)
{
  TranslatorPointer<uint16_t> p(*rDecoder.getBufferTranslator(), pEvent);

  int firstTwoBytes = *p++ & 0xffff;

  // File header event is always 21
  if (firstTwoBytes == sizeof(FileHeader_t)) {
    /*
    // Leave it just in case we need it
    TranslatorPointer<FileHeader_t> pp(p);
    FileHeader_t fileheader = *pp++;

    cout << dec << "dma: " << (int)fileheader.dataformat_major << endl;
    cout << dec << "dmi: " << (int)fileheader.dataformat_minor << endl;
    cout << dec << "sma: " << (int)fileheader.software_major << endl;
    cout << dec << "smi: " << (int)fileheader.software_minor << endl;
    cout << dec << "spa: " << (int)fileheader.software_patch << endl;
    cout << dec << "am: " << (int)fileheader.acqmode << endl;
    cout << dec << "tu: " << (int)fileheader.timeunit << endl;
    cout << dec << "ench: " << (int)fileheader.energyNCH << endl;
    cout << dec << "tc: " << (float)fileheader.timeconversion << endl;
    cout << dec << "sams: " << fileheader.startacq_ms << endl;
    */

    return kfTRUE;
  }

  return unpack(p, firstTwoBytes);
}

Bool_t
CFERSUnpacker::unpack(TranslatorPointer<uint16_t> begin, uint16_t metadata)
{
  try {
    vector<ParsedFERSA5202Event> events = m_unpacker.parseAll(begin, metadata);

    for (auto& event : events) {
      for (int iCh = 0; iCh < 64; iCh++) {
        if (event.hasEnergyLG[iCh])  m_lg[iCh] = event.energyLG[iCh];
        if (event.hasEnergyHG[iCh])  m_hg[iCh] = event.energyHG[iCh];
        if (event.hasCounts[iCh])    m_counts[iCh] = event.counts[iCh];
        if (event.hasToA_int[iCh])   m_toa_int[iCh] = event.ToA_int[iCh];
        if (event.hasToT_int[iCh])   m_tot_int[iCh] = event.ToT_int[iCh];
        if (event.hasToA_float[iCh]) m_toa_float[iCh] = event.ToA_float[iCh];
        if (event.hasToT_float[iCh]) m_tot_float[iCh] = event.ToT_float[iCh];
      }
    }
  } catch (exception& exc) {
    cout << "Parsing Failed! Reason=" << exc.what() << endl;
    return kfFALSE;
  }

  return kfTRUE;
}
