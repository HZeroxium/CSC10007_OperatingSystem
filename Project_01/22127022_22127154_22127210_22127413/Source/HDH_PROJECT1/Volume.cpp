#pragma once
#include "Volume.h"
#include "Console.h"
#include <algorithm>

/// =============
/// EXTERNALITIES
/// =============
namespace FAT32
{
    bool IsValidByte(BYTE byte)
    {
        return (byte != ENDED_BYTE) && (byte != BROKE_BYTE);
    }
}

/// ==========================
/// CONSTRUCTORS & DESTRUCTORS
/// ==========================
Volume::Volume()
{
    Init();
}

Volume::Volume(const std::string& sDrivePath)
{
    Connect(sDrivePath);
}

Volume::~Volume()
{
}
/// ========
/// CHECKERS
/// ========
bool Volume::IsFolder(size_t uStart) const
{
    return Entry[uStart + 11] == FileAttribute::DIRECTORY;
}

bool Volume::IsDeletedEntry(size_t uStart) const
{
    return Entry[uStart] == EntryAttribute::DELETED;
}

bool Volume::IsSpecialEntry(size_t uStart) const
{
    return Entry[uStart + 11] == EntryAttribute::SPECIAL;
}

bool Volume::IsLongNameEntry(size_t uStart) const
{
    return Entry[uStart + 6] == EntryAttribute::LONGNAME;
}
/// =============
/// INTERNALITIES
/// =============
bool Volume::Init()
{
    /// Data
    FAT.clear();
    /// Bootsector info
    bootsector.clear();
    uBytePerSector = 0;
    uSectorPerCluster = 0;
    uNumberOfSectorOfBootsector = 0;
    uNumberOfFAT = 0;
    uSectorPerFatTable = 0;
    /// Entry info
    Entry.clear();
    uStartClusterRDET = 0;
    uNumberEntryOfRdet = 0;
    /// Directory
    sURLs.clear();
    Directory.clear();
    return true;
}

uint32_t Volume::ConvertClusterToSector(uint32_t Cluster) const
{
    return (uNumberOfSectorOfBootsector + uSectorPerFatTable * uNumberOfFAT + (Cluster - 2) * uSectorPerCluster);
}

uint64_t Volume::ExtractSector(const sector_t& sector, int start, int end) const
{
    uint64_t decimal = 0;
    for (int i = end; i >= start; i--) {
        decimal = decimal * 256 + sector[i];
    }
    return decimal;
}

uint32_t Volume::ExtractEntry()
{
    uint32_t uNameCount = 0;
    Directory.clear();
    for (size_t uStart = 0; uStart < Entry.size(); uStart += 32) {
        if (!IsDeletedEntry(uStart) && !IsSpecialEntry(uStart)) {
            std::string Temp = "";
            if (IsLongNameEntry(uStart)) {
                Temp = ReadLongName(uStart);
            }
            else {
                Temp = ReadShortName(uStart);
            }
            ++uNameCount;
        }
    }
    return uNameCount;
}

bool Volume::SetupBootsector()
{
    bootsector.clear();
    if (!ReadSector(bootsector)) {
        return false;
    }
    ReadBootSector(0x0b, 2, uBytePerSector);
    ReadBootSector(0x0d, 1, uSectorPerCluster);
    ReadBootSector(0x0e, 2, uNumberOfSectorOfBootsector);
    ReadBootSector(0x10, 1, uNumberOfFAT);
    ReadBootSector(0x11, 4, uNumberEntryOfRdet);
    ReadBootSector(0x24, 4, uSectorPerFatTable);
    ReadBootSector(0x2c, 4, uStartClusterRDET);
    return true;
}
/// ======
/// READER
/// ======
bool Volume::ReadSector(sector_t& buffer, const int nStartByte, const int nReadSize) {
    const std::wstring wsDrivePath = console::str_to_wstr(sDrivePath);
    LPCWSTR drive = wsDrivePath.c_str();
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;
    device = CreateFileW(
        drive,                              // Drive to open
        GENERIC_READ,                       // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE, // Share Mode
        NULL,                               // Security Descriptor
        OPEN_EXISTING,                      // How to create
        0,                                  // File attributes
        NULL                                // Handle to template
    );
    if (device == INVALID_HANDLE_VALUE) { // Open Error
        std::cerr << "Failed to open the disk: " << console::GetLastErrorAsString() << std::endl;
        return false;
    }
    SetFilePointer(device, nStartByte, NULL, FILE_BEGIN); // Set a Point to Read
    const int nSize = nReadSize + ((FAT32::SECTOR_SIZE - nReadSize) % FAT32::SECTOR_SIZE);
    buffer.resize(nSize);
    if (!ReadFile(device, buffer.data(), nSize, &bytesRead, NULL)) {
        console::Display({ STREAMTEXT("Failed to read the disk" << console::GetLastErrorAsString()), console::RED });
        CloseHandle(device); // Close the handle after usage
        return false;
    }
    else {
        CloseHandle(device); // Close the handle after usage
        return true;
    }
}
bool Volume::ReadBootSector(const int offset, const int length, uint32_t& data)
{
    data = ExtractSector(bootsector, offset, offset + length - 1);
    return true;
}
bool Volume::ReadFat()
{
    int startByte = uNumberOfSectorOfBootsector * FAT32::SECTOR_SIZE;
    int totalSize = uSectorPerFatTable * uNumberOfFAT * FAT32::SECTOR_SIZE;
    FAT.clear();
    return ReadSector(FAT, startByte, totalSize);
    //              ^Already read bootsector in [0..BOOTSECTOR_SIZE-1]
    //               the first bootsector starts from [BOOTSECTOR_SIZE..]
}

bool Volume::ReadEntry(uint32_t Cluster)
{
    Entry.clear();
    uint32_t StartPoint = ConvertClusterToSector(Cluster) * 512;
    const int Size = SECTOR_SIZE * uSectorPerCluster;
    sector_t buffer;
    ReadSector(buffer, StartPoint, Size);
    for (int i = 0; i < Size; i += 32) {
        if (buffer[i] == EMPTY_BYTE) {
            buffer.erase(buffer.begin() + i, buffer.end());
            Entry.insert(Entry.end(), buffer.begin(), buffer.end());
            break;
        }
    }
    return true;
}

bool Volume::ReadRdet()
{
    std::vector<cluster_t> Clus = GetRdetCluster(uStartClusterRDET);
    for (int i = 0; i < Clus.size(); i++) {
        uint32_t Cur = Clus[i];
        if (!ReadEntry(Cur)) {
            return false;
        }
    }
    return true;
}

bool Volume::ReadShortName(uint32_t Start)
{
    uint32_t Anchor = Start;
    std::string Name;
    for (int i = Start; i < Start + 8; i++) {
        if (Entry[i] != ' ') {
            Name += (char)Entry[i];
        }
    }
    if (!IsFolder(Start)) {
        Name += '.';
        for (int i = Start + 8; i <= Start + 10; i++) {
            Name += (char)Entry[i];
        }
        /*Archive[Name] = Anchor;
        return 0;*/
    }
    Directory[Name] = Anchor;
    return 0;
}
// neu duoi tep qua dai thi' co the co bug
bool Volume::ReadLongName(uint32_t Start)
{
    uint32_t Anchor = Start;
    char Num = ExtractSector(Entry, Start + 7, Start + 7);
    int NumberSubEntry = Num - '0';
    std::string Name;
    for (int i = 0; i <= NumberSubEntry; i++) {
        Start -= 32;
        for (int j = Start + 1; (j < Start + 10) && IsValidByte(Entry[j]); j++) {
            if (Entry[j] != EMPTY_BYTE) {
                Name += (char)Entry[j];
            }
        }
        for (int j = Start + 14; (j < Start + 15 + 12) && IsValidByte(Entry[j]); j++) {
            if (Entry[j] != EMPTY_BYTE) {
                Name += (char)Entry[j];
            }
        }
        for (int j = Start + 28; (j < Start + 32) && IsValidByte(Entry[j]); j++) {
            if (Entry[j] != EMPTY_BYTE) {
                Name += (char)Entry[j];
            }
        }
    }

    Name.erase(std::remove(Name.begin(), Name.end(), -1), Name.end());
    Directory[Name] = Anchor;
    return 0;
}

bool Volume::ReadContent(std::string ArchiveName) {

    uint32_t StartCluster = GetStartCluster(Directory[ArchiveName]);
    uint32_t StartPoint = ConvertClusterToSector(StartCluster) * 512;
    uint32_t Size = ExtractSector(Entry, Directory[ArchiveName] + 28, Directory[ArchiveName] + 31);
    uint32_t ReadSize = Size + (512 - Size % 512);
    sector_t buffer;
    ReadSector(buffer, StartPoint, 512);
    for (int i = 0; i < Size; i++) {
        std::cout << static_cast<char>(buffer[i]);
    }
    return 0;
}
/// =======
/// GETTERS
/// =======
std::vector<cluster_t> Volume::GetRdetCluster(cluster_t uStartCluster) const
{
    std::vector<cluster_t> Cluster;
    for (cluster_t uCurrentCluster = uStartCluster; uCurrentCluster != Cluster::ENDED; ) {
        Cluster.push_back(uCurrentCluster);
        uCurrentCluster = ExtractSector(FAT, uCurrentCluster * 4, uCurrentCluster * 4 + 4);
    }
    return Cluster;
}

uint32_t Volume::GetStartCluster(uint32_t Anchor) const
{
    uint32_t High = (uint32_t)ExtractSector(Entry, Anchor + 20, Anchor + 21);
    uint32_t Low = (uint32_t)ExtractSector(Entry, Anchor + 26, Anchor + 27);
    return High * 16 * 16 * 16 * 16 + Low;
}

std::string Volume::GetURL() const
{
    std::string sPath = ".";
    for (const std::string& sFolder : sURLs) {
        sPath += "/" + sFolder;
    }
    return sPath;
}
/// =======
/// SETTERS
/// =======
void Volume::SetStartCluster(uint32_t Cluster)
{
    uStartClusterRDET = Cluster;
}
/// ============
/// INPUT-OUTPUT
/// ============
void Volume::PrintSector()
{
    for (int i = 0; i < 512; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bootsector[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec;
}

void Volume::PrintSectorTable()
{
    for (int i = 0; i < bootsector.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bootsector[i]) << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec; // Reset stream to decimal output
}
/// ========
/// COMMANDS
/// ========
bool Volume::Connect(const std::string& sPath)
{
    Init();
    sDrivePath = sPath;
    sURLs = { console::get_drive_name(sDrivePath) };
    SetupBootsector();
    ReadFat();
    return true;
}

bool Volume::Dir()
{
    ReadRdet();
    ExtractEntry();
    for (const auto& Fol : Directory) {
        std::string name = Fol.first;
        uint32_t Idx = Fol.second;
        std::string sInfo = " RHSVDA " + name;
        for (int bit = 1; bit <= 6; ++bit) {
            if (!(Entry[Idx + 11] >> (bit - 1) & 1)) {
                sInfo[bit] = '-';
            }
        }
        std::cout << sInfo << std::endl;
    }
    return true;
}

bool Volume::Open(const std::string& sFileFullName)
{
    std::string Ext = console::get_file_extension(sFileFullName);
    if (Directory.count(sFileFullName) == 1 && console::case_sensitive_eq(Ext, "TXT")) {
        ReadContent(sFileFullName);
        return true;
    }
    return false;
}

bool Volume::Cd(const std::string& sPathToFile)
{
    std::vector<std::string> sFolders = console::split_str(sPathToFile, "/\\");
    for (const std::string& sFolder : sFolders) {
        ReadRdet();
        ExtractEntry();
        if (!Directory.count(sFolder) || sFolder.empty()) {
            return false;
        }
        uint32_t StartCluster = GetStartCluster(Directory[sFolder]);
        if (sFolder != "..") {
            sURLs.push_back(sFolder);
        }
        else if (sURLs.size()) {
            sURLs.pop_back();
        }
        if (StartCluster != 0) {
            SetStartCluster(StartCluster);
        }
        else {
            SetStartCluster(StartCluster + 2);
        }
    }
    return true;
}