#ifndef CFERSUNPACKER_H
#define CFERSUNPACKER_H

#include <config.h>
#include "CFERSA5202Unpacker.h"
#include <EventProcessor.h>
#include <TreeParameter.h>
#include <cstdint>
#include <cstddef>
#include <vector>

class CEvent;
class CAnalyzer;
class CBufferDecoder;

class CFERSUnpacker : public CEventProcessor
{
  private:
    CFERSA5202Unpacker     m_unpacker;
    int                    m_dataformat;
    CTreeParameterArray    m_lg;
    CTreeParameterArray    m_hg;
    CTreeParameterArray    m_counts;
    CTreeParameterArray    m_toa_int;
    CTreeParameterArray    m_tot_int;
    CTreeParameterArray    m_toa_float;
    CTreeParameterArray    m_tot_float;

    // Boards in the system, numbered 0 to numBoards-1 (Tcl: numBoards).
    // Events from boards >= numBoards are ignored.
    CTreeVariable          m_numBoards;

    // Janus writes one ring item per board. Board events sharing a
    // trigger_id are held here and histogrammed together as one event
    // once boards 0 to numBoards-1 have all arrived, or else when the
    // first item of the next trigger arrives.
    std::vector<ParsedFERSA5202Event> m_pending;
    uint32_t               m_pendingBoards;
    uint32_t               m_warnedBoards;

  public:
    CFERSUnpacker();
    virtual ~CFERSUnpacker();

    virtual Bool_t operator()(const Address_t pEvent,
                              CEvent& rEvent,
                              CAnalyzer& rAnalyzer,
                              CBufferDecoder& rDecoder);
    virtual Bool_t OnBegin(CAnalyzer& rAnalyzer,
                           CBufferDecoder& rDecoder);
  private:
    Bool_t unpack(TranslatorPointer<std::uint16_t> begin,
                  uint16_t acqmode);
    void fill(const ParsedFERSA5202Event& event);
    void flushPending();
};

#endif 
