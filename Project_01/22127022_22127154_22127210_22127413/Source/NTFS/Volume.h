#pragma once

#include "Utility.h"
#include <string>
#include "Entry.h"
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::vector;

class Volume
{
private:
	uint32_t bytePerSector;				///< Bytes per sector, usually 512
	uint32_t sectorPerCluster;			///< Sectors per cluster
	std::wstring drivePath;				///< Path to the drive (e.g. \\\\.\\C:)
	uint32_t startClusterOfMFT;			///< Start cluster of the Master File Table
	uint32_t bytePerEntry;				///< Bytes per entry in the MFT, usually 1024
	vector<Entry> mftEntries;			///< Entries in the MFT
	vector<EntryNode *> mftEntriesNode; ///< Entries in the MFT
	EntryNode *currentRoot;

public:
	Volume(const std::wstring &drivePath);

	std::vector<BYTE> ReadSector(uint64_t startSector, uint64_t count);

	void ReadBPB();

	uint32_t Cluster2Sector(uint32_t cluster);

	void ReadMFT();

	void PrintMFT();

	void ExtractMFT();

	void PrintMFTEntryData(const std::string &name);

	void ConstructEntryTree();

	void FindEntryNode(const std::string &name);

	void PrintCurrentRoot();

	void Run();
};
