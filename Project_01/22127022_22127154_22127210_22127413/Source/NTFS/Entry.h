#pragma once

#include "Utility.h"
#include <iomanip>
#include <string>
using std::string;

class Attribute
{
public:						  // Header of the attribute
	AttributeType type;		  // type of the attribute
	uint32_t sizeOfAttribute; // size of the attribute
	bool isNonResident;		  // if the attribute is non-resident
	uint8_t nameLength;		  // length of the name of the attribute
	uint16_t nameOffset;	  // offset to the name of the attribute (if it exists)
	uint16_t flags;			  // flags of the attribute: 0x01 = compressed, 0x02 = encrypted, 0x04 = sparse, 0x08 = reparse point
	uint16_t instance;
	uint32_t sizeOfContent;		   // size of the content of the file
	uint16_t startOffsetOfContent; // offset from the start of the file to the content

public:

	void ReadAttributeHeader(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute);

	uint32_t GetEndOffsetOfAttribute();

	uint32_t GetSizeOfAttribute();

	virtual void ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset) = 0;

	void PrintAttribute();
};

class StandardInformationAttribute : public Attribute
{
public:
	StandardInformationFlags flags; // flags of the file

public:
	void ReadStandardInformation(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute);

	void ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset) override;
};

class FileNameAttribute : public Attribute
{
public:
	uint64_t parentDirectory;		 // reference to the parent directory through base MFT entry
	FilenameNamespace namespaceType; // namespace of the file
	std::string filename;			 // name of the file

public:
	void ReadFileName(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute);

	void ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset) override;
};

class DataAttribute : public Attribute
{
public:
	std::vector<BYTE> data; // data of the file

public:
	void ReadData(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute);

	void ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset) override;

	string GetData();
};

class UnknownAttribute : public Attribute
{
public:
	void ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset) override
	{
	}
};

class Entry
{
public:
	uint16_t startOffsetOfAttribute;	 // offset from the start of the file to the attribute
	uint16_t flags;						 // 0x01 = used, 0x02 = directory, 0x04, 0x08 = unknown
	uint64_t baseEntryRef;				 // reference to the base entry
	uint32_t id;						 // id of the entry
	std::vector<Attribute *> attributes; // attributes of the entry
public:
	void ReadEntry(std::vector<BYTE> buffer);

	void PrintEntry();

	uint64_t GetParentDirectory() const;

	std::string GetFileName();

	std::string GetTextFileData();

	uint32_t GetStandardInformationFlags();
};

struct EntryNode
{
	std::string name;
	std::vector<EntryNode *> children;
	Entry *entry;
	EntryNode *parent;

	EntryNode(Entry *entry, EntryNode *parent)
	{
		this->entry = entry;
		this->name = entry->GetFileName();
		this->parent = parent;
	}
	uint32_t GetID() const
	{
		return entry->id;
	}
	uint32_t GetParentID() const
	{
		return parent->entry->id;
	}
	bool IsDirectory() const
	{
		return entry->GetStandardInformationFlags() == 0x0000;
	}
	bool IsFile() const
	{
		return entry->GetStandardInformationFlags() == 0x20;
	}
};

inline EntryNode *FindParentNode(uint32_t parentID, const std::vector<EntryNode *> &nodes)
{
	for (EntryNode *node : nodes)
	{
		if (node->GetID() == parentID)
		{
			return node;
		}
	}
	return nullptr;
}

// Return a list of nodes that are children of the root node
inline std::vector<EntryNode *> ConstructEntryTreeHelper(EntryNode *root, std::vector<Entry> &entries)
{
	std::vector<EntryNode *> children;
	for (Entry &entry : entries)
	{
		if (entry.GetParentDirectory() == root->GetID() && entry.GetParentDirectory() != entry.id)
		{
			EntryNode *node = new EntryNode(&entry, root);
			children.push_back(node);
			std::vector<EntryNode *> grandChildren = ConstructEntryTreeHelper(node, entries);
			node->children = grandChildren;
		}
	}
	return children;
}

inline void PrintEntryTree(const EntryNode *node, int depth = 0)
{
	if (node != nullptr)
	{
		for (int i = 0; i < depth; ++i)
		{
			std::cout << "    ";
		}
		std::cout << node->name << "(" << ConvertStandardInformationFlagsToString(node->entry->GetStandardInformationFlags()) << ")" << std::endl;

		for (const EntryNode *child : node->children)
		{
			PrintEntryTree(child, depth + 1);
		}
	}
}
