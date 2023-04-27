#ifndef CFERSA5202UNPACKER_H
#define CFERSA5202UNPACKER_H

#include <vector>
#include <utility>
#include <cstdint>
#include <TranslatorPointer.h>

struct ParsedFERSA5202Event {
	double tstamp_us;
  uint16_t board_id;
  uint16_t nhits;
	uint64_t trigger_id;
	uint64_t chmask;
	uint16_t energyHG[64];
	uint16_t energyLG[64];
	uint32_t ToA_int[64];
  float    ToA_float[64];
	uint16_t ToT_int[64];
  float    ToT_float[64];
	uint32_t counts[64];
};

class CFERSA5202Unpacker
{
  public:
    using Iter=TranslatorPointer<std::uint16_t>;

  public:
    std::vector<ParsedFERSA5202Event>
      parseAll(const Iter& begin, const uint16_t acqmode);

  private:
    // Utility methods
    template<typename T>
    T getCombined(Iter &iter);
    
    ParsedFERSA5202Event parseCommonHeader(Iter& iter);
};

#endif
