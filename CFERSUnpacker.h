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

// Unpacks FERS (Janus) data in either of two forms:
//
//  * built events from the NSCLDAQ event builder fed by fersBoardFilter:
//    one fragment per board (plus, once per run, the Janus file-header
//    record).  Every fragment of a built event is histogrammed as one
//    SpecTcl event; boardMask is not used.
//
//  * the plain Janus ring (or event files recorded from it): one ring item
//    per board and trigger.  Board items sharing a trigger_id are held and
//    histogrammed together once every unmasked board has arrived, or when
//    the first item of the next trigger arrives.
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

    // One bit per board, set = masked out (Tcl: boardMask, default 0xfe
    // keeps board 0 only; 0xfc keeps boards 0 and 1). Only used for the
    // plain Janus ring: events from masked boards are ignored there.
    CTreeVariable          m_boardMask;

    // Plain Janus ring: board events of the trigger being collected
    std::vector<ParsedFERSA5202Event> m_pending;
    uint32_t               m_pendingBoards;
    uint32_t               m_warnedBoards;

    // Built events: boards seen in the current built event, to notice a
    // board twice in one event (glom window too wide)
    uint32_t               m_builtBoards;
    bool                   m_warnedDuplicate;

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
    Bool_t unpackBuiltEvent(TranslatorPointer<std::uint32_t> p,
                            std::uint32_t bodySize);
    Bool_t unpackJanusItem(TranslatorPointer<std::uint16_t> p, bool built);
    Bool_t unpack(TranslatorPointer<std::uint16_t> begin,
                  uint16_t metadata, bool built);
    void fill(const ParsedFERSA5202Event& event);
    void flushPending();
};

#endif
