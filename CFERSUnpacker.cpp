#include "CFERSA5202Unpacker.h"
#include "CFERSUnpacker.h"
#include <BufferDecoder.h>
#include <TCLAnalyzer.h>
#include <iostream>
#include <stdexcept>
#include <climits>
#include <cfloat>

// Data file format for FileHeader_t layout selection; the actual
// format version is read from the header record at run time
// (3.3.0 = Janus 4.2.0, 3.4.0 = Janus 5.0.1 -- same header layout)
#define FM340 // 3.4.0 (Janus 5.0.1)
//#define FM330 // 3.3.0 (Janus 4.2.0)
//#define FM320 // 3.2.0
//#define FM310 // 3.1.0

#include "FERS_fileheader.h"
#include "FERS_constant.h"

using namespace std;

// NSCLDAQ ring item type of the fragments that carry Janus data
static const uint32_t kPhysicsEvent = 30;

// NSCLDAQ event builder (glom --build) fragment header:
// uint64 timestamp, uint32 source id, uint32 payload size, uint32 barrier
static const uint32_t kFragmentHeaderBytes = 20;

CFERSUnpacker::CFERSUnpacker()
  : m_unpacker(),
    m_dataformat(34),
    m_lg("LG", 10000, 0.0, (double) USHRT_MAX, "channels", FERS_MAX_NCH, 0),
    m_hg("HG", 10000, 0.0, (double) USHRT_MAX, "channels", FERS_MAX_NCH, 0),
    m_counts("counts", 10000, 0.0, (double) UINT_MAX, "counts", FERS_MAX_NCH, 0),
    m_toa_int("ToA_int", 10000, 0.0, (double) UINT_MAX, "0.5 ns", FERS_MAX_NCH, 0),
    m_tot_int("ToT_int", 10000, 0.0, (double) USHRT_MAX, "0.5 ns", FERS_MAX_NCH, 0),
    m_toa_float("ToA_float", 10000, 0.0, FLT_MAX, "ns", FERS_MAX_NCH, 0),
    m_tot_float("ToT_float", 10000, 0.0, FLT_MAX, "ns", FERS_MAX_NCH, 0),
    m_boardMask("boardMask", 0xfe, "mask"),
    m_pendingBoards(0),
    m_warnedBoards(0),
    m_builtBoards(0),
    m_warnedDuplicate(false)
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
  // A built event's body starts with its own byte count.  A Janus item
  // starts with uint16 inclusiveSize followed by a word that is never 0
  // (metadata, or the dataformat bytes of the header record), so the two
  // cannot be confused.
  TranslatorPointer<uint32_t> p32(*rDecoder.getBufferTranslator(), pEvent);
  uint32_t bodySize = rDecoder.getBodySize();
  if (*p32 == bodySize) {
    return unpackBuiltEvent(p32, bodySize);
  }

  // Plain Janus ring: one board per ring item
  TranslatorPointer<uint16_t> p(*rDecoder.getBufferTranslator(), pEvent);
  return unpackJanusItem(p, false);
}

Bool_t
CFERSUnpacker::OnBegin(CAnalyzer& rAnalyzer, CBufferDecoder& rDecoder)
{
  // Never merge boards across runs
  m_pending.clear();
  m_pendingBoards = 0;
  m_warnedBoards = 0;
  m_warnedDuplicate = false;

  return kfTRUE;
}

// Walk the fragments of a built event; each payload is a complete ring
// item (header, body header, body) as fersBoardFilter emitted it
Bool_t
CFERSUnpacker::unpackBuiltEvent(TranslatorPointer<uint32_t> p, uint32_t bodySize)
{
  Bool_t filled = kfFALSE;
  m_builtBoards = 0;

  uint32_t offset = sizeof(uint32_t);           // past the byte count
  p++;
  while (offset + kFragmentHeaderBytes <= bodySize) {
    p += 2;                                     // timestamp
    p++;                                        // source id (== board_id)
    uint32_t payloadSize = *p++;
    p++;                                        // barrier
    offset += kFragmentHeaderBytes;
    if (payloadSize < 3*sizeof(uint32_t) || offset + payloadSize > bodySize) {
      cout << "Built event: fragment of " << payloadSize
           << " bytes does not fit in the " << bodySize << " byte body" << endl;
      break;
    }

    // Ring item: uint32 size, uint32 type, then the body header whose
    // first word is its size (4 when there is none), then the body
    TranslatorPointer<uint32_t> item(p);
    item++;                                     // size
    uint32_t type = *item++;
    uint32_t bodyHeaderSize = *item;
    if (bodyHeaderSize < sizeof(uint32_t)) bodyHeaderSize = sizeof(uint32_t);
    TranslatorPointer<uint8_t> body(item);
    body += bodyHeaderSize;

    if (type == kPhysicsEvent) {
      TranslatorPointer<uint16_t> janus(body);
      if (unpackJanusItem(janus, true)) filled = kfTRUE;
    }

    TranslatorPointer<uint8_t> next(p);
    next += payloadSize;
    p = next;
    offset += payloadSize;
  }

  return filled;
}

Bool_t
CFERSUnpacker::unpackJanusItem(TranslatorPointer<uint16_t> p, bool built)
{
  int inclusiveSize = *p++ & 0xffff;

  // File header event is always 2(size) + FileHeader_t size + 4(ender)
  int headerSize = 2 + sizeof(FileHeader_t) + 4;
#if defined(FM330) || defined(FM340)
  // Since Janus 4.2.0 the item is padded with 0xff up to a multiple of 4
  // and the padding is counted in inclusiveSize
  headerSize = (headerSize + 3) & ~3;
#endif
  // A timing-mode event with one hit is as long as the header record; the
  // word after inclusiveSize tells them apart: dataformat major/minor
  // bytes (major 3) for the record, metadata for an event, whose low byte
  // is the time unit (0 or 1)
  if (inclusiveSize == headerSize && (*p & 0xff) > 1) {
    // First two bytes of the header record are the dataformat version
    TranslatorPointer<uint8_t> pVersion(p);
    int major = *pVersion++;
    int minor = *pVersion++;
    m_dataformat = major*10 + minor;

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

    // In a built event the record is one fragment among the boards'
    // events and histograms nothing by itself
    return built ? kfFALSE : kfTRUE;
  }

  // Reaching here are events

  int metadata = *p++ & 0xffff;

  return unpack(p, metadata, built);
}

Bool_t
CFERSUnpacker::unpack(TranslatorPointer<uint16_t> begin, uint16_t metadata, bool built)
{
  // Timing mode events carry no trigger_id, so each board stays its own event
  int acqmode = (metadata & 0x0F00) >> 8;
  bool mergeBoards = acqmode != ACQMODE_TIMING;

  // A set bit in boardMask masks that board out: 0xfc keeps boards 0 and 1
  uint32_t boardMask = (uint32_t)(int) m_boardMask & ((1u << FERS_MAX_BOARDS) - 1);
  uint32_t activeBoards = ~boardMask & ((1u << FERS_MAX_BOARDS) - 1);

  Bool_t emitted = kfFALSE;

  try {
    vector<ParsedFERSA5202Event> events = m_unpacker.parseAll(begin, metadata,
                                                              m_dataformat);

    for (auto& event : events) {
      if (event.board_id >= FERS_MAX_BOARDS) {
        string errmsg = "board_id " + to_string(event.board_id)
                      + " exceeds FERS_MAX_BOARDS ("
                      + to_string(FERS_MAX_BOARDS) + ")";
        throw runtime_error(errmsg);
      }

      // Built event: the event builder already grouped the boards
      if (built) {
        if ((m_builtBoards & (1u << event.board_id)) && !m_warnedDuplicate) {
          m_warnedDuplicate = true;
          cout << "Built event with two fragments of board " << event.board_id
               << " (glom coincidence window too wide?); later fragments "
               << "overwrite earlier ones. Reported once per run." << endl;
        }
        m_builtBoards |= 1u << event.board_id;
        fill(event);
        emitted = kfTRUE;
        continue;
      }

      // e.g. a masked board still sends one empty event at trigger 0
      if (!(activeBoards & (1u << event.board_id))) {
        if (!(m_warnedBoards & (1u << event.board_id))) {
          m_warnedBoards |= 1u << event.board_id;
          cout << "Ignoring board " << event.board_id << " (boardMask = 0x"
               << hex << boardMask << dec << ")" << endl;
        }
        continue;
      }

      if (!mergeBoards) {
        fill(event);
        emitted = kfTRUE;
        continue;
      }

      // A new trigger_id closes the held trigger even if boards are missing
      if (!m_pending.empty() && event.trigger_id != m_pending[0].trigger_id) {
        flushPending();
        emitted = kfTRUE;
      }

      m_pending.push_back(event);
      m_pendingBoards |= 1u << event.board_id;

      // Only one trigger can be histogrammed per item; a complete trigger
      // that could not go out now is released by the next trigger
      if (!emitted && (m_pendingBoards & activeBoards) == activeBoards) {
        flushPending();
        emitted = kfTRUE;
      }
    }
  } catch (exception& exc) {
    cout << "Parsing Failed! Reason=" << exc.what() << endl;
    return kfFALSE;
  }

  // kfFALSE while the trigger is still being collected keeps SpecTcl from
  // histogramming a partial event
  return emitted;
}

void
CFERSUnpacker::flushPending()
{
  for (auto& held : m_pending) fill(held);
  m_pending.clear();
  m_pendingBoards = 0;
}

void
CFERSUnpacker::fill(const ParsedFERSA5202Event& event)
{
  // Each board's channels 0-63 map to BCC = board_id*100 + channel
  int offset = event.board_id*FERS_BOARD_STRIDE;
  for (int iCh = 0; iCh < FERS_NCH_PER_BOARD; iCh++) {
    int idx = offset + iCh;
    if (event.hasEnergyLG[iCh])  m_lg[idx] = event.energyLG[iCh];
    if (event.hasEnergyHG[iCh])  m_hg[idx] = event.energyHG[iCh];
    if (event.hasCounts[iCh])    m_counts[idx] = event.counts[iCh];
    if (event.hasToA_int[iCh])   m_toa_int[idx] = event.ToA_int[iCh];
    if (event.hasToT_int[iCh])   m_tot_int[idx] = event.ToT_int[iCh];
    if (event.hasToA_float[iCh]) m_toa_float[idx] = event.ToA_float[iCh];
    if (event.hasToT_float[iCh]) m_tot_float[idx] = event.ToT_float[iCh];
  }
}
