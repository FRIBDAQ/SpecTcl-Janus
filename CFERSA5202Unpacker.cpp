#include "CFERSA5202Unpacker.h"
#include <string>
#include <stdexcept>
#include <iostream>

#include "FERS_constant.h"

using namespace std;

static const uint32_t TYPE_MASK (0x07000000);
static const uint32_t TYPE_HDR  (0x02000000);
static const uint32_t TYPE_DATA (0x00000000);
static const uint32_t TYPE_TRAIL(0x04000000);

static const unsigned GEO_SHIFT(27);
static const uint32_t GEO_MASK (0xf8000000);

static const unsigned HDR_COUNT_SHIFT(8);      
static const uint32_t HDR_COUNT_MASK (0x00003f00);
static const unsigned HDR_CRATE_SHIFT(16);
static const uint32_t HDR_CRATE_MASK (0x00ff0000);

static const unsigned DATA_CHANSHIFT(16);
static const uint32_t DATA_CHANMASK (0x001f0000);
static const uint32_t DATA_CONVMASK (0x00003fff);

static const uint32_t TRAIL_COUNT_MASK(0x00ffffff);
static const uint32_t BERR(0xffffffff);

template <typename T>
T CFERSA5202Unpacker::getCombined(Iter &iter) {
  size_t outputBitWidth = sizeof(T)*8;
  size_t inputBitWidth = sizeof(uint16_t)*8;
  int totalORs = outputBitWidth/inputBitWidth;

  T value = 0;
  for (int i = 0; i < totalORs; i++) {
    value |= (T) *iter++ << (outputBitWidth - (i + 1)*inputBitWidth);
  }

  return value;
}

void
CFERSA5202Unpacker::parseCommonHeader(Iter& iter, ParsedFERSA5202Event &anEvent) {
  int eventSize = *iter++;

  TranslatorPointer<uint8_t> oneByteIter(iter);
  anEvent.board_id = *oneByteIter++;
  iter = oneByteIter;
  anEvent.tstamp_us = (double)getCombined<uint64_t>(iter);
  anEvent.trigger_id = getCombined<uint64_t>(iter);
  anEvent.chmask = getCombined<uint64_t>(iter);
}

void
CFERSA5202Unpacker::initialize(ParsedFERSA5202Event &anEvent) {
  anEvent.tstamp_us = 0;
  anEvent.board_id = 0;
  anEvent.nhits = 0;
  anEvent.trigger_id = 0;
  anEvent.chmask = 0;
  for (int iCh = 0; iCh < 64; iCh++) {
    anEvent.hasEnergyHG[iCh] = 0;
    anEvent.energyHG[iCh] = 0;
    anEvent.hasEnergyLG[iCh] = 0;
    anEvent.energyLG[iCh] = 0;
    anEvent.hasToA_int[iCh] = 0;
    anEvent.ToA_int[iCh] = 0;
    anEvent.hasToA_float[iCh] = 0;
    anEvent.ToA_float[iCh] = 0.f;
    anEvent.hasToT_int[iCh] = 0;
    anEvent.ToT_int[iCh] = 0;
    anEvent.hasToT_float[iCh] = 0;
    anEvent.ToT_float[iCh] = 0.f;
    anEvent.hasCounts[iCh] = 0;
    anEvent.counts[iCh] = 0;
  }
}

vector<ParsedFERSA5202Event>
CFERSA5202Unpacker::parseAll(const Iter& begin,
                             const uint16_t metadata)
{
  vector<ParsedFERSA5202Event> parsedData;

  int acqmode = (metadata & 0xFF00) >> 8;
  int timeunit = metadata & 0XFF;

  auto iter = begin;

  ParsedFERSA5202Event anEvent;
  initialize(anEvent);

  switch (acqmode) {
    case ACQMODE_SPECT:
    {
      parseCommonHeader(iter, anEvent);
      int numChannels = __builtin_popcountll(anEvent.chmask);

      for (int iCh = 0; iCh < numChannels; iCh++) {
        TranslatorPointer<uint8_t> oneByteIter(iter);
        int chIdx = *oneByteIter++;
        int dataType = *oneByteIter++;
        iter = oneByteIter;
        if (dataType & DATATYPE_LG) {
          anEvent.hasEnergyLG[chIdx] = true;
          anEvent.energyLG[chIdx] = *iter++;
        }
        if (dataType & DATATYPE_HG) {
          anEvent.hasEnergyHG[chIdx] = true;
          anEvent.energyHG[chIdx] = *iter++;
        }
      }

      parsedData.push_back(anEvent);

      break;
    };

    case ACQMODE_SPECTIMING:
    {
      parseCommonHeader(iter, anEvent);
      int numChannels = __builtin_popcountll(anEvent.chmask);

      switch (timeunit) {
        case TIMEUNIT_LSB:
        {
          for (int iCh = 0; iCh < numChannels; iCh++) {
            TranslatorPointer<uint8_t> oneByteIter(iter);
            int chIdx = *oneByteIter++;
            int dataType = *oneByteIter++;
            iter = oneByteIter;
            if (dataType & DATATYPE_LG) {
              anEvent.hasEnergyLG[chIdx] = true;
              anEvent.energyLG[chIdx] = *iter++;
            }
            if (dataType & DATATYPE_HG) {
              anEvent.hasEnergyHG[chIdx] = true;
              anEvent.energyHG[chIdx] = *iter++;
            }
            if (dataType & DATATYPE_TOA) {
              anEvent.hasToA_int[chIdx] = true;
              anEvent.ToA_int[chIdx] = getCombined<uint32_t>(iter);
            }
            if (dataType & DATATYPE_TOT) {
              anEvent.hasToT_int[chIdx] = true;
              anEvent.ToT_int[chIdx] = *iter++;
            }
          }

          break;
        }

        case TIMEUNIT_NS:
        {
          for (int iCh = 0; iCh < numChannels; iCh++) {
            TranslatorPointer<uint8_t> oneByteIter(iter);
            int chIdx = *oneByteIter++;
            int dataType = *oneByteIter++;
            iter = oneByteIter;
            if (dataType & DATATYPE_LG) {
              anEvent.hasEnergyLG[chIdx] = true;
              anEvent.energyLG[chIdx] = *iter++;
            }
            if (dataType & DATATYPE_HG) {
              anEvent.hasEnergyHG[chIdx] = true;
              anEvent.energyHG[chIdx] = *iter++;
            }
            if (dataType & DATATYPE_TOA) {
              anEvent.hasToA_float[chIdx] = true;
              anEvent.ToA_float[chIdx] = (float)getCombined<uint32_t>(iter);
            }
            if (dataType & DATATYPE_TOT) {
              anEvent.hasToT_float[chIdx] = true;
              anEvent.ToT_float[chIdx] = (float)getCombined<uint32_t>(iter);
            }
          }

          break;
        }

        default:
        {
          string errmsg = "Invalid TimeUnit! (" + to_string(timeunit) + ")";
          throw runtime_error(errmsg);
        }
      }

      parsedData.push_back(anEvent);

      break;
    }
    case ACQMODE_COUNTING: 
    {
      parseCommonHeader(iter, anEvent);
      int numChannels = __builtin_popcountll(anEvent.chmask);

      for (int iCh = 0; iCh < numChannels; iCh++) {
        TranslatorPointer<uint8_t> oneByteIter(iter);
        int chIdx = *oneByteIter++;
        iter = oneByteIter;
        anEvent.hasCounts[chIdx] = true;
        anEvent.counts[chIdx] = getCombined<uint32_t>(iter);
      }

      parsedData.push_back(anEvent);

      break;
    }

    case ACQMODE_TIMING:
    {
      int eventSize = *iter++;
      TranslatorPointer<uint8_t> oneByteIter(iter);
      anEvent.board_id = *oneByteIter++;
      iter = oneByteIter;
      anEvent.tstamp_us = (double) getCombined<uint64_t>(iter);
      anEvent.nhits = *iter++;

      switch (timeunit) {
        case TIMEUNIT_LSB:
        {
          for (int iHit = 0; iHit < anEvent.nhits; iHit++) {
            TranslatorPointer<uint8_t> oneByteIter(iter);
            int chIdx = *oneByteIter++;
            int dataType = *oneByteIter++;
            iter = oneByteIter;
            if (dataType & DATATYPE_TOA) {
              anEvent.hasToA_int[chIdx] = true;
              anEvent.ToA_int[chIdx] = getCombined<uint32_t>(iter);
            }
            if (dataType & DATATYPE_TOT) {
              anEvent.hasToT_int[chIdx] = true;
              anEvent.ToT_int[chIdx] = *iter++;
            }
          }

          break;
        }

        case TIMEUNIT_NS:
        {
          for (int iHit = 0; iHit < anEvent.nhits; iHit++) {
            TranslatorPointer<uint8_t> oneByteIter(iter);
            int chIdx = *oneByteIter++;
            int dataType = *oneByteIter++;
            iter = oneByteIter;
            if (dataType & DATATYPE_TOA) {
              anEvent.hasToA_float[chIdx] = true;
              anEvent.ToA_float[chIdx] = (float)getCombined<uint32_t>(iter);
            }
            if (dataType & DATATYPE_TOT) {
              anEvent.hasToT_float[chIdx] = true;
              anEvent.ToT_float[chIdx] = (float)getCombined<uint32_t>(iter);
            }
          }

          break;
        }

        default:
        {
          string errmsg = "Invalid TimeUnit! (" + to_string(timeunit) + ")";
          throw runtime_error(errmsg);
        }

      }

      parsedData.push_back(anEvent);

      break;
    }

    default:
    {
      string errmsg = "Invalid AcqMode! (" + to_string(acqmode) + ")";
      throw runtime_error(errmsg);
    }
  }

  return parsedData;
}