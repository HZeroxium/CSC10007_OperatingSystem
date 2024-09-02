#include "Volume.h"

Volume::Volume(const std::wstring &drivePath)
{
	this->drivePath = drivePath;
	bytePerSector = 512;
	bytePerEntry = 1024;
}

std::vector<BYTE> Volume::ReadSector(uint64_t startSector, uint64_t count)
{
	HANDLE device = CreateFileW(drivePath.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (device == INVALID_HANDLE_VALUE)
	{
		std::cerr << "Error opening device: " << GetLastError() << std::endl;
		return std::vector<BYTE>();
	}
	DWORD bytesRead;
	BYTE *buffer = new BYTE[bytePerSector * count];
	std::vector<BYTE> resultSector(1, 0);
	uint64_t offset = startSector * bytePerSector;
	uint64_t sizeOfReadBytes = bytePerSector * count;

	LONG lDistanceToMove = static_cast<DWORD>(offset & 0xFFFFFFFF);
	LONG lDistanceToMoveHigh = static_cast<DWORD>(offset >> 32);
	DWORD dwNewPos = SetFilePointer(device, lDistanceToMove, &lDistanceToMoveHigh, FILE_BEGIN);
	if (dwNewPos == INVALID_SET_FILE_POINTER)
	{
		cerr << "Error setting file pointer: " << GetLastError() << endl;
	}
	if (ReadFile(device, buffer, sizeOfReadBytes, &bytesRead, NULL))
	{
		resultSector.resize(sizeOfReadBytes, 0);
		for (int i = 0; i < sizeOfReadBytes; i++)
		{
			resultSector[i] = buffer[i];
		}
	}
	else
	{
		cerr << "Error reading sector: " << GetLastError() << endl;
	}

	delete[] buffer;
	buffer = nullptr;
	CloseHandle(device);

	return resultSector;
}

void Volume::ReadBPB()
{
	std::vector<BYTE> bpb = ReadSector(0, 1);
	bytePerSector = static_cast<uint32_t>(Convert2LE(bpb.begin() + 0xB, 2));
	sectorPerCluster = static_cast<uint32_t>(Convert2LE(bpb.begin() + 0xD, 1));
	startClusterOfMFT = static_cast<uint32_t>(Convert2LE(bpb.begin() + 0x30, 8));

	std::cout << "Welcome to NTFS Explorer" << std::endl;
	std::cout << "* NTFS BPB information: " << std::endl;
	std::cout << "+ Bytes per sector: " << bytePerSector << std::endl;
	std::cout << "+ Sectors per cluster: " << sectorPerCluster << std::endl;
	std::cout << "+ Start cluster of MFT: " << startClusterOfMFT << std::endl;
	std::cout << "+ Bytes per entry: " << bytePerEntry << std::endl;
}

uint32_t Volume::Cluster2Sector(uint32_t cluster)
{
	return cluster * sectorPerCluster;
}

void Volume::ReadMFT()
{
	uint32_t startSector = Cluster2Sector(startClusterOfMFT);
	uint32_t currentSector = startSector;
	uint32_t endSector = currentSector + 500;
	for (uint32_t i = currentSector; i < endSector; i += 2)
	{
		vector<BYTE> entryBuffer = ReadSector(i, 2);
		uint32_t signature = Convert2LE(entryBuffer.begin(), 4);
		if (signature == 0)
		{
			continue;
		}
		Entry entry;
		entry.ReadEntry(entryBuffer);
		mftEntries.push_back(entry);
	}
	ExtractMFT();
}

void Volume::PrintMFT()
{
	std::cout << "----------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;

	constexpr int entryWidth = 5;
	constexpr int fileNameWidth = 80;
	constexpr int idWidth = 3;
	constexpr int parentIdWidth = 9;
	constexpr int flagsWidth = 10;

	std::cout << std::left << std::setw(entryWidth) << "|Entry"
			  << "|";
	std::cout << std::left << std::setw(fileNameWidth) << "File name"
			  << "|";
	std::cout << std::left << std::setw(idWidth) << "ID"
			  << "|";
	std::cout << std::left << std::setw(parentIdWidth) << "Parent ID"
			  << "|";
	std::cout << std::left << std::setw(flagsWidth) << "Flags"
			  << "|" << std::endl;
	for (int i = 0; i < mftEntries.size(); i++)
	{
		std::cout << "----------------------------------------------------------------------------------------------------------------------------------------------------" << std::endl;
		std::cout << "|";
		std::cout << std::left << std::setw(entryWidth) << i << "|";
		std::cout << std::left << std::setw(fileNameWidth) << mftEntries[i].GetFileName() << "|";
		std::cout << std::left << std::setw(idWidth) << mftEntries[i].id << "|";
		std::cout << std::left << std::setw(parentIdWidth) << mftEntries[i].GetParentDirectory() << "|";
		std::cout << std::left << std::setw(flagsWidth) << std::dec << mftEntries[i].GetStandardInformationFlags() << "|" << std::endl;
	}
}

void Volume::ExtractMFT()
{
	std::ofstream ofs("mft.csv");
	ofs << "Entry,File name,ID,Parent ID,Flags" << std::endl;
	for (int i = 0; i < mftEntries.size(); i++)
	{
		ofs << i << "," << mftEntries[i].GetFileName() << "," << mftEntries[i].id << "," << mftEntries[i].GetParentDirectory() << "," << ConvertStandardInformationFlagsToString(mftEntries[i].GetStandardInformationFlags()) << std::endl;
	}
}

void Volume::PrintMFTEntryData(const std::string &name)
{
	std::cout << mftEntries[33].GetTextFileData();
}

void Volume::ConstructEntryTree()
{
	EntryNode *root = new EntryNode(&mftEntries[5], nullptr);
	mftEntriesNode = ConstructEntryTreeHelper(root, mftEntries);
	for (int i = 0; i < mftEntriesNode.size(); i++)
	{
		root->children.push_back(mftEntriesNode[i]);
	}
	// PrintEntryTree(root, 0);
	currentRoot = root;
}

void Volume::FindEntryNode(const std::string &name)
{
	EntryNode *node = nullptr;
	for (int i = 0; i < currentRoot->children.size(); i++)
	{
		EntryNode *child = currentRoot->children[i];
		if (child->entry->GetFileName() == name)
		{
			node = child;
			break;
		}
	}
	if (node == nullptr)
	{
		std::cerr << "File or directory not found" << std::endl;
	}
	else if (node->IsDirectory())
	{
		std::cout << "... Change to directory " << node->entry->GetFileName() << std::endl;
		// PrintEntryTree(node, 0);
		currentRoot = node;
	}
	else if (node->IsFile())
	{
		std::cout << node->entry->GetTextFileData() << std::endl;
	}
	else
	{
		std::cerr << "Error" << std::endl;
	}
}

void Volume::PrintCurrentRoot()
{
	// Flag (20 space) | File name
	for (int i = 0; i < currentRoot->children.size(); i++)
	{
		std::cout << "      |" << std::setw(15) << std::left << ConvertStandardInformationFlagsToString(currentRoot->children[i]->entry->GetStandardInformationFlags()) << "| " << currentRoot->children[i]->entry->GetFileName() << std::endl;
	}
}

void Volume::Run()
{
	ReadBPB();
	ReadMFT();
	ConstructEntryTree();

	while (true)
	{
		std::cout << std::endl;
		std::cout << "==============================================================" << std::endl;
		std::cout << "==============================================================" << std::endl;
		std::cout << std::endl;
		std::cout << "*------------------------------------------------------------*" << std::endl;
		std::cout << "*--------------------- NTFS EXPLORER ------------------------*" << std::endl;
		std::cout << "*------------------------------------------------------------*" << std::endl;
		std::cout << "Choose an option: " << std::endl;
		std::cout << "1. Read file or directory" << std::endl;
		std::cout << "2. Back to parent directory" << std::endl;
		std::cout << "3. Print current root directory" << std::endl;
		std::cout << "0. Exit" << std::endl;

		int choice;
		std::cout << "Your choice: ";
		std::cin >> choice;
		std::cin.ignore();

		if (choice == 1)
		{
			std::string name;
			std::cout << "Enter file or directory name: ";
			std::getline(std::cin, name);
			FindEntryNode(name);
		}
		else if (choice == 2)
		{
			if (currentRoot->parent == nullptr)
			{
				std::cerr << "You are in the root directory" << std::endl;
			}
			else
			{
				currentRoot = currentRoot->parent;
				std::cout << "... Back to parent directory: " << currentRoot->entry->GetFileName() << std::endl;
				// PrintEntryTree(currentRoot, 0);
			}
		}
		else if (choice == 3)
		{
			std::cout << "... Printing current root directory: " << currentRoot->entry->GetFileName() << std::endl;
			PrintCurrentRoot();
		}
		else if (choice == 0)
		{
			break;
		}
	}
}
