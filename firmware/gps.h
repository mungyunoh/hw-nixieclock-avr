/*! \file gps.h \brief GPS position storage and processing library. */
//*****************************************************************************
//
// File Name	: 'gps.h'
// Title		: GPS position storage and processing function library
// Author		: Pascal Stang - Copyright (C) 2002
// Created		: 2002.08.29
// Revised		: 2002.08.29
// Version		: 0.1
// Target MCU	: Atmel AVR Series
// Editor Tabs	: 4
//
// NOTE: This code is currently below version 1.0, and therefore is considered
// to be lacking in some functionality or documentation, or may not be fully
// tested.  Nonetheless, you can expect most functions to work.
//
///	\ingroup driver_hw
/// \defgroup gps GPS Positioning and Navigation Function Library (gps.c)
/// \code #include "gps.h" \endcode
/// \par Overview
///		This library provides a generic way to store and process information
///	received from a GPS receiver.  Currently the library only stores the most
/// recent set of GPS data (position, velocity, time) from a GPS receiver.
/// Future revisions will include navigation functions like calculate
///	heading/distance to a waypoint.  The processing of incoming serial data
///	packets from GPS hardware is not done in this library.  The libraries
///	tsip.c and nmea.c do the packet processing for Trimble Standard Interface
/// Protocol and NMEA-0813 repectively, and store the results in this library.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#ifndef GPS_H
#define GPS_H

#include "global.h"

// constants/macros/typdefs
typedef union union_float_u32
{
	float f;
	uint32_t i;
	uint8_t b[4];
} float_u32;

typedef union union_double_u64
{
	double f;
	uint64_t i;
	uint8_t b[8];
} double_u64;


struct PositionLLA
{
	float_u32 lat;
	float_u32 lon;
	float_u32 alt;
	float_u32 TimeOfFix;
	uint16_t updates;
};

struct VelocityHS
{
	float_u32 heading;
	float_u32 speed;
	float_u32 TimeOfFix;
	uint16_t updates;
};

typedef struct struct_GpsInfo
{
	uint8_t numSVs;
	uint32_t UtDate;
	uint32_t validTimeReceivedMillis;

	struct PositionLLA PosLLA;
	struct VelocityHS VelHS;
} GpsInfoType;


// functions
void gpsInit(void);
GpsInfoType* gpsGetInfo(void);
void gpsProcess(void);
time_t gpsGetTime(void);

void gpsPowerEnable(void);
void gpsPowerDisable(void);

#endif
