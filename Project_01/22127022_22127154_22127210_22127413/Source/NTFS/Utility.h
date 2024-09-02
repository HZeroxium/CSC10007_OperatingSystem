#pragma once

#include <iostream>
#include <vector>
#include <windows.h>
#include <iomanip>

uint64_t Convert2LE(std::vector<BYTE>::iterator offset, uint32_t size);

// Print bytes to the output device, 16 bytes per line with hex and ASCII representation
void PrintBytes(const std::vector<BYTE>& bytes, uint32_t offset, uint32_t size, uint32_t sectorOffset, std::ostream& outDevice);

enum AttributeType : uint32_t
{
	StandardInformation = 0x10,
	AttributeList = 0x20,
	FileName = 0x30,
	ObjectId = 0x40,
	SecurityDescriptor = 0x50,
	VolumeName = 0x60,
	VolumeInformation = 0x70,
	Data = 0x80,
	IndexRoot = 0x90,
	IndexAllocation = 0xA0,
	Bitmap = 0xB0,
	ReparsePoint = 0xC0,
	EAInformation = 0xD0,
	EA = 0xE0,
	PropertySet = 0xF0,
	LoggedUtilityStream = 0x100
};

enum StandardInformationFlags : uint16_t
{
	ReadOnly = 0x01,
	Hidden = 0x02,
	System = 0x04,
	Archive = 0x20,
	Device = 0x40,
	Normal = 0x80,
	Temporary = 0x100,
	SparseFile = 0x200,
	// ReparsePoint = 0x400,
	Compressed = 0x800,
	Offline = 0x1000,
	NotContentIndexed = 0x2000,
	Encrypted = 0x4000,
};

enum FilenameNamespace : uint8_t
{
	POSIX = 0x00,
	Win32 = 0x01,
	DOS = 0x02,
	Win32AndDOS = 0x03
};

std::string ConvertAttributeTypeToString(AttributeType type);

std::string ConvertStandardInformationFlagsToString(uint16_t flags);