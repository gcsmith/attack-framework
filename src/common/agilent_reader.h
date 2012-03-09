// dpa framework - a collection of tools for differential power analysis
// Copyright (C) 2011  Garrett C. Smith
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef AGILENT_READER__H
#define AGILENT_READER__H

#include <stdint.h>
#include <stdio.h>

#define DATE_TIME_STRING_LENGTH 16
#define FRAME_STRING_LENGTH     24
#define SIGNAL_STRING_LENGTH    16
 
typedef enum
{
    PB_UNKNOWN,
    PB_NORMAL,
    PB_PEAK_DETECT,
    PB_AVERAGE,
    PB_HORZ_HISTOGRAM,
    PB_VERT_HISTOGRAM,
    PB_LOGIC
} ePBWaveformType;

typedef enum
{
    PB_DATA_UNKNOWN,
    PB_DATA_NORMAL,
    PB_DATA_MAX,
    PB_DATA_MIN,
    PB_DATA_TIME,
    PB_DATA_COUNTS,
    PB_DATA_LOGIC
} ePBDataType;

typedef struct
{
    uint8_t Cookie[2];
    uint8_t Version[2];
    int32_t FileSize;
    int32_t NumberOfWaveforms;
} tPBFileHeader;

typedef struct
{
    int32_t  HeaderSize;
    int32_t  WaveformType;
    int32_t  NWaveformBuffers;
    int32_t  Points;
    int32_t  Count;
    float    XDisplayRange;
    double   XDisplayOrigin;
    double   XIncrement;
    double   XOrigin;
    int32_t  XUnits;
    int32_t  YUnits;
    uint8_t  Date[DATE_TIME_STRING_LENGTH];
    uint8_t  Time[DATE_TIME_STRING_LENGTH];
    uint8_t  Frame[FRAME_STRING_LENGTH];
    uint8_t  WaveformLabel[SIGNAL_STRING_LENGTH];
    double   TimeTag;
    uint32_t SegmentIndex;
} tPBWaveformHeader;

typedef struct
{
    int32_t HeaderSize;
    int16_t BufferType;
    int16_t BytesPerPoint;
    int32_t BufferSize;
} tPBWaveformDataHeader;

int ReadAgilentHeader(FILE *fp, tPBFileHeader *hdr);

int ReadWaveformHeader(FILE *fp, tPBWaveformHeader *header);

int ReadWaveformDataHeader(FILE *fp, tPBWaveformDataHeader *header);

unsigned char *ReadLogicWaveform(FILE *fp, const tPBWaveformHeader *header);

float *ReadAnalogWaveform(FILE *fp, const tPBWaveformHeader *header, ePBDataType *type);

void DestroyAnalogWaveform(float *pWaveformData);

int *ReadHistogramWaveform(FILE *fp, const tPBWaveformHeader *header);

#endif // AGILENT_READER__H

