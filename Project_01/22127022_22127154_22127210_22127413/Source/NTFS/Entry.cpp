#include "Entry.h"

//********************************************************************************
//****************************** ATTRIBUTE CLASS *********************************
//********************************************************************************

void Attribute::ReadAttributeHeader(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute)
{
    type = AttributeType(Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x00, 4));
    sizeOfAttribute = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x04, 4);
    isNonResident = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x08, 1);
    nameLength = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x09, 1);
    nameOffset = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x0A, 2);
    flags = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x0C, 2);
    instance = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x0E, 2);
    sizeOfContent = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x10, 4);
    startOffsetOfContent = Convert2LE(buffer.begin() + startOffsetOfAttribute + 0x14, 2);
}
uint32_t Attribute::GetEndOffsetOfAttribute()
{
    return startOffsetOfContent + sizeOfContent;
}
uint32_t Attribute::GetSizeOfAttribute()
{
    return sizeOfAttribute;
}

void Attribute::PrintAttribute()
{
    // Indent 4 spaces when printing the attribute from the left
    std::cout << std::setw(4) << "+ Type: " << std::hex << type << std::dec << ": " << ConvertAttributeTypeToString(type) << std::endl;
    std::cout << std::setw(4) << "+ Size of attribute: " << sizeOfAttribute << std::endl;
    std::cout << std::setw(4) << "+ Is non-resident: " << isNonResident << std::endl;
    std::cout << std::setw(4) << "+ Name length: " << nameLength << std::endl;
    std::cout << std::setw(4) << "+ Name offset: " << nameOffset << std::endl;
    std::cout << std::setw(4) << "+ Flags: " << flags << std::endl;
    std::cout << std::setw(4) << "+ Instance: " << instance << std::endl;
    std::cout << std::setw(4) << "+ Size of content: " << sizeOfContent << std::endl;
    std::cout << std::setw(4) << "+ Start offset of content: " << startOffsetOfContent << std::endl;
}

//********************************************************************************
//****************** STANDARD INFORMATION ATTRIBUTE CLASS ************************
//********************************************************************************

void StandardInformationAttribute::ReadStandardInformation(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute)
{
    flags = StandardInformationFlags(Convert2LE(buffer.begin() + startOffsetOfAttribute + startOffsetOfContent + 0x20, 4));
}

void StandardInformationAttribute::ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset)
{
    ReadStandardInformation(buffer, offset);
}

//********************************************************************************
//********************** FILE NAME ATTRIBUTE CLASS *******************************
//********************************************************************************

void FileNameAttribute::ReadFileName(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute)
{
    parentDirectory = Convert2LE(buffer.begin() + startOffsetOfAttribute + startOffsetOfContent + 0x00, 6);
    uint32_t nameLength = Convert2LE(buffer.begin() + startOffsetOfAttribute + startOffsetOfContent + 0x40, 1);
    namespaceType = FilenameNamespace(Convert2LE(buffer.begin() + startOffsetOfAttribute + startOffsetOfContent + 65, 1));

    uint32_t offset = startOffsetOfAttribute + startOffsetOfContent + 66;
    for (uint32_t i = 0; i < nameLength; i++)
    {
        uint32_t n = Convert2LE(buffer.begin() + offset, 2);
        // If the character is not a valid character, replace with '.'
        if (n < 0x20 || n > 0x7E)
        {
            filename += '.';
        }
        else
        {
            filename += (char)n;
        }
        offset += 2;
    }
}

void FileNameAttribute::ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset)
{
    ReadFileName(buffer, offset);
}

//********************************************************************************
//************************* DATA ATTRIBUTE CLASS ********************************
//********************************************************************************

void DataAttribute::ReadData(std::vector<BYTE> buffer, uint32_t startOffsetOfAttribute)
{
    data = std::vector<BYTE>(buffer.begin() + startOffsetOfAttribute + startOffsetOfContent, buffer.begin() + startOffsetOfAttribute + startOffsetOfContent + sizeOfContent);
}

void DataAttribute::ReadAttributeContent(std::vector<BYTE> buffer, uint32_t offset)
{
    ReadData(buffer, offset);
}
string DataAttribute::GetData()
{
    return std::string(data.begin(), data.end());
}

//********************************************************************************
//****************************** ENTRY CLASS *************************************

void Entry::ReadEntry(std::vector<BYTE> buffer)
{
    startOffsetOfAttribute = Convert2LE(buffer.begin() + 0x14, 2);
    flags = Convert2LE(buffer.begin() + 0x16, 2);
    baseEntryRef = Convert2LE(buffer.begin() + 0x20, 8);
    id = Convert2LE(buffer.begin() + 0x2C, 4);
    std::vector<AttributeType> attributeTypes;
    std::vector<uint32_t> attributeOffsets;
    // Find the end of the entry (0xFFFFFFFF)
    uint32_t endOffset = startOffsetOfAttribute;
    while (Convert2LE(buffer.begin() + endOffset, 4) != 0xFFFFFFFF)
    {
        Attribute *attribute = new UnknownAttribute();
        attribute->ReadAttributeHeader(buffer, endOffset);
        attributeTypes.push_back(attribute->type);
        attributeOffsets.push_back(endOffset);
        endOffset += attribute->GetSizeOfAttribute();
        delete attribute;
    }
    for (uint32_t i = 0; i < attributeTypes.size(); i++)
    {
        Attribute *attribute;
        switch (attributeTypes[i])
        {
        case AttributeType::StandardInformation:
            attribute = new StandardInformationAttribute();
            break;
        case AttributeType::FileName:
            attribute = new FileNameAttribute();
            break;
        case AttributeType::Data:
            attribute = new DataAttribute();
            break;
        default:
            attribute = new UnknownAttribute();
            break;
        }
        attribute->ReadAttributeHeader(buffer, attributeOffsets[i]);
        attribute->ReadAttributeContent(buffer, attributeOffsets[i]);
        attributes.push_back(attribute);
    }
}

void Entry::PrintEntry()
{
    std::cout << "- Start offset of attribute: " << startOffsetOfAttribute << std::endl;
    std::cout << "- Flags: " << flags << std::endl;
    std::cout << "- Base entry reference: " << baseEntryRef << std::endl;
    int i = 1;
    for (auto attribute : attributes)
    {
        std::cout << " - Attribute " << i++ << std::endl;
        attribute->PrintAttribute();
    }
}

uint64_t Entry::GetParentDirectory() const
{
    for (auto attribute : attributes)
    {
        if (attribute->type == AttributeType::FileName)
        {
            return ((FileNameAttribute *)attribute)->parentDirectory;
        }
    }
    return 0;
}

std::string Entry::GetFileName()
{
    for (auto attribute : attributes)
    {
        if (attribute->type == AttributeType::FileName)
        {
            return ((FileNameAttribute *)attribute)->filename;
        }
    }
    return "Unknown";
}

std::string Entry::GetTextFileData()
{
    for (auto attribute : attributes)
    {
        if (attribute->type == AttributeType::FileName)
        {

            std::string txtExtension = ".txt";
            if (((FileNameAttribute *)attribute)->filename.find(txtExtension) == std::string::npos)
            {
                FileNameAttribute file = *(FileNameAttribute *)attribute;
                std::cout << file.filename.size() << std::endl;
                return "This file is not a text file. Pls use another method to read the content of the file.";
            }
        }
    }
    for (auto attribute : attributes)
    {
        if (attribute->type == AttributeType::Data)
        {
            return ((DataAttribute *)attribute)->GetData();
        }
    }
    return "Unknown";
}

uint32_t Entry::GetStandardInformationFlags()
{
    for (auto attribute : attributes)
    {
        if (attribute->type == AttributeType::StandardInformation)
        {
            return ((StandardInformationAttribute *)attribute)->flags;
        }
    }
    return 0;
}
