#ifndef CFERSA5202UNPACKER_H
#define CFERSA5202UNPACKER_H

#include <vector>
#include <utility>
#include <cstdint>
#include <TranslatorPointer.h>

struct ParsedFERSA5202Event {
	double tstamp_us;
	double rel_tstamp_us;
	double DeltaTref;
  uint16_t board_id;
  uint16_t nhits;
	uint64_t trigger_id;
	uint64_t chmask;
  bool     hasEnergyHG[64];
	uint16_t energyHG[64];
  bool     hasEnergyLG[64];
	uint16_t energyLG[64];
  bool     hasToA_int[64];
	uint32_t ToA_int[64];
  bool     hasToA_float[64];
  float    ToA_float[64];
  bool     hasToT_int[64];
	uint16_t ToT_int[64];
  bool     hasToT_float[64];
  float    ToT_float[64];
  bool     hasCounts[64];
	uint64_t counts[64];
};

class CFERSA5202Unpacker
{
  public:
    using Iter=TranslatorPointer<std::uint16_t>;

  public:
    // dataformat: major*10 + minor from the file header record
    // (33 = Janus 4.2.0, 34 = Janus 5.0.1)
    std::vector<ParsedFERSA5202Event>
      parseAll(const Iter& begin, const uint16_t acqmode,
               const int dataformat = 34);

  private:
    // Utility methods
    template<typename T>
    T getCombined(Iter &iter);

    double getDouble(Iter &iter);
    void parseCommonHeader(Iter& iter, ParsedFERSA5202Event &anEvent,
                           bool has2ndTstamp, bool hasDeltaTref,
                           bool hasNumHits);
    void initialize(ParsedFERSA5202Event &anEvent);
};

#endif
