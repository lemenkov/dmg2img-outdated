/*
 * DMG2IMG dmg2img.h
 * 
 * Copyright (c) 2004 vu1tur <to@vu1tur.eu.org> This program is free software; you
 * can redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <zlib.h>
#include <bzlib.h>
#include "adc.h"
#include <unistd.h>

#include <endian.h>

#define BT_ADC   0x80000004
#define BT_ZLIB  0x80000005
#define BT_BZLIB 0x80000006

#define BT_ZERO 0x00000000
#define BT_RAW 0x00000001
#define BT_IGNORE 0x00000002
#define BT_COMMENT 0x7ffffffe
#define BT_TERM 0xffffffff

z_stream z;
bz_stream bz;

const char plist_begin[] = "<plist version=\"1.0\">";
const char plist_end[] = "</plist>";
const char list_begin[] = "<array>";
const char list_end[] = "</array>";
const char chunk_begin[] = "<data>";
const char chunk_end[] = "</data>";
const char blkx_begin[] = "<key>blkx</key>";

uint32_t convert_char4(unsigned char *c)
{
	return (((uint32_t) c[0]) << 24) | (((uint32_t) c[1]) << 16) |
	(((uint32_t) c[2]) << 8) | ((uint32_t) c[3]);
}

uint64_t convert_char8(unsigned char *c)
{
	return ((uint64_t) convert_char4(c) << 32) | (convert_char4(c + 4));
}

struct _kolyblk {
	uint32_t Signature;
	uint32_t Version;
	uint32_t HeaderSize;
	uint32_t Flags;
	uint64_t RunningDataForkOffset;
	uint64_t DataForkOffset;
	uint64_t DataForkLength;
	uint64_t RsrcForkOffset;
	uint64_t RsrcForkLength;
	uint32_t SegmentNumber;
	uint32_t SegmentCount;
	uint32_t SegmentID1;
	uint32_t SegmentID2;
	uint32_t SegmentID3;
	uint32_t SegmentID4;
	uint32_t DataForkChecksumType;
	uint32_t Reserved1;
	uint32_t DataForkChecksum;
	uint32_t Reserved2;
	char Reserved3[120];
	uint64_t XMLOffset;
	uint64_t XMLLength;
	char Reserved4[120];
	uint32_t MasterChecksumType;
	uint32_t Reserved5;
	uint32_t MasterChecksum;
	uint32_t Reserved6;
	char Reserved7[120];
	uint32_t ImageVariant;
	uint64_t SectorCount;
	char Reserved8[12];
} __attribute__ ((__packed__));
struct _kolyblk kolyblk;


struct _mishblk {
	uint32_t BlocksSignature;
	uint32_t InfoVersion;
	uint64_t FirstSectorNumber;
	uint64_t SectorCount;
	uint64_t DataStart;
	uint32_t DecompressedBufferRequested;
	uint32_t BlocksDescriptor;
	char Reserved1[24];
	uint32_t ChecksumType;
	uint32_t Reserved2;
	uint32_t Checksum;
	uint32_t Reserved3;
	char Reserved4[120];
	uint32_t BlocksRunCount;
	char *Data;
} __attribute__ ((__packed__));


void read_kolyblk(FILE* F, struct _kolyblk* k)
{
	fread(k, 0x200, 1, F);
	k->Signature = be32toh(k->Signature);
	k->Version = be32toh(k->Version);
	k->HeaderSize = be32toh(k->HeaderSize);
	k->Flags = be32toh(k->Flags);
	k->RunningDataForkOffset = be64toh(k->RunningDataForkOffset);
	k->DataForkOffset = be64toh(k->DataForkOffset);
	k->DataForkLength = be64toh(k->DataForkLength);
	k->RsrcForkOffset = be64toh(k->RsrcForkOffset);
	k->RsrcForkLength = be64toh(k->RsrcForkLength);
	k->SegmentNumber = be32toh(k->SegmentNumber);
	k->SegmentCount = be32toh(k->SegmentCount);
	k->DataForkChecksumType = be32toh(k->DataForkChecksumType);
	k->DataForkChecksum = be32toh(k->DataForkChecksum);
	k->XMLOffset = be64toh(k->XMLOffset);
	k->XMLLength = be64toh(k->XMLLength);
	k->MasterChecksumType = be32toh(k->MasterChecksumType);
	k->MasterChecksum = be32toh(k->MasterChecksum);
	k->ImageVariant = be32toh(k->ImageVariant);
	k->SectorCount = be64toh(k->SectorCount);
}

void fill_mishblk(char* c, struct _mishblk* m)
{
	memset(m, 0, sizeof(struct _mishblk));
	memcpy(m, c, 0xCC);
	m->BlocksSignature = be32toh(m->BlocksSignature);
	m->InfoVersion = be32toh(m->InfoVersion);
	m->FirstSectorNumber = be64toh(m->FirstSectorNumber);
	m->SectorCount = be64toh(m->SectorCount);
	m->DataStart = be64toh(m->DataStart);
	m->DecompressedBufferRequested = be32toh(m->DecompressedBufferRequested);
	m->BlocksDescriptor = be32toh(m->BlocksDescriptor);
	m->ChecksumType = be32toh(m->ChecksumType);
	m->Checksum = be32toh(m->Checksum);
	m->BlocksRunCount = be32toh(m->BlocksRunCount);
}

