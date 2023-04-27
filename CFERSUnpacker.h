#ifndef CFERSUNPACKER_H
#define CFERSUNPACKER_H

#include <config.h>
#include "CFERSA5202Unpacker.h"
#include <EventProcessor.h>
#include <TreeParameter.h>
#include <cstdint>
#include <cstddef>

class CEvent;
class CAnalyzer;
class CBufferDecoder;

class CFERSUnpacker : public CEventProcessor
{
  private:
    CFERSA5202Unpacker     m_unpacker;
    CTreeParameterArray    m_lg;
    CTreeParameterArray    m_hg;
    CTreeParameterArray    m_counts;
    CTreeParameterArray    m_toa_int;
    CTreeParameterArray    m_tot_int;
    CTreeParameterArray    m_toa_float;
    CTreeParameterArray    m_tot_float;

  public:
    CFERSUnpacker();
    virtual ~CFERSUnpacker();

    virtual Bool_t operator()(const Address_t pEvent,
                              CEvent& rEvent,
                              CAnalyzer& rAnalyzer,
                              CBufferDecoder& rDecoder);
  private:
    Bool_t unpack(TranslatorPointer<std::uint16_t> begin,
                  uint16_t acqmode);
};

#endif 
