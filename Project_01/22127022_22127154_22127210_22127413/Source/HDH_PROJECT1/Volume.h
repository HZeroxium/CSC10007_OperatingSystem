#ifndef VOLUMN_H
#define VOLUMN_H
#pragma once

#include <windows.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <array>
#include <utility>
#include <map>

namespace FAT32
{
    constexpr int32_t BOOTSECTOR_SIZE = 512;
    constexpr int32_t SECTOR_SIZE = 512; // bytes per bootsector
    typedef uint32_t cluster_t;
    typedef std::vector<BYTE> sector_t;
    enum Cluster : uint32_t {
        EMPTY = 0x00000000,
        ENDED = 0x0FFFFFFF,
        BROKE = 0xFFFFFFF7,
    };
    enum ByteAttribute {
        EMPTY_BYTE = 0x00,
        ENDED_BYTE = 0x0F,
        BROKE_BYTE = 0xFF,
    };
    enum FileAttribute : BYTE {
        ARCHIVE   = 0x20,
        DIRECTORY = 0x10,
        VOLLABEL  = 0x08,
        SYSTEM    = 0x04,
        HIDDEN    = 0x02,
        READONLY  = 0x01,
    };
    enum EntryAttribute {
        DELETED  = 0xe5,
        SPECIAL  = 0x0f,
        LONGNAME = 0x7e,
    };
    bool IsValidByte(BYTE byte);
};
using namespace FAT32;

class Volume
{
private: /// Required
    std::string sDrivePath;
private: /// Data
    sector_t FAT;
private: /// Bootsector info
    sector_t bootsector;
    uint32_t uBytePerSector;
    uint32_t uSectorPerCluster;//sc
    uint32_t uNumberOfSectorOfBootsector;//sb
    uint32_t uNumberOfFAT;//nf
    uint32_t uSectorPerFatTable;//sf
private: /// Entry info
    sector_t Entry;
    uint32_t uStartClusterRDET;
    uint32_t uNumberEntryOfRdet;
private: /// Directory
    std::vector <std::string> sURLs;
    std::map<std::string, uint32_t> Directory;
public: /// Constructors & Destructors
    Volume();
    Volume(const std::string& sDrivePath);
    ~Volume();
private: /// Checkers
    bool IsFolder(size_t Start) const;
    bool IsDeletedEntry(size_t Start) const;
    bool IsSpecialEntry(size_t Start) const;
    bool IsLongNameEntry(size_t Start) const;
private: /// Internalities
    bool Init();
    uint32_t ConvertClusterToSector(uint32_t Cluster) const;
    uint64_t ExtractSector(const sector_t& sector, int start, int end) const;
    uint32_t ExtractEntry();
    bool SetupBootsector();
private: /// Readers
    bool ReadSector(sector_t& buffer, const int nStartByte = 0, const int nReadSize = SECTOR_SIZE);
    bool ReadBootSector(const int offset, const int length, uint32_t&data);
    bool ReadFat();
    bool ReadEntry(uint32_t Cluster);
    bool ReadRdet();
    bool ReadShortName(uint32_t Start);
    bool ReadLongName(uint32_t Start);
    bool ReadContent(std::string ArchiveName);
private: /// Getters
    std::vector<cluster_t> GetRdetCluster(cluster_t uStartCluster) const;
    uint32_t GetStartCluster(uint32_t Anchor) const;
public: /// Getters
    std::string GetURL() const;
private: /// Setters
    void SetStartCluster(uint32_t Cluster);
private: /// Input-Output
    void PrintSector();
    void PrintSectorTable();
public: /// Commands
    bool Connect(const std::string& sPath);
    bool Dir();
    bool Open(const std::string &sFileFullName);
    bool Cd(const std::string& sPathToFile);
};

#endif // VOLUMN_H