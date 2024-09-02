#include "Utility.h"

uint64_t Convert2LE(std::vector<BYTE>::iterator offset, uint32_t size) {
	uint64_t result = 0;
	for (int i = size - 1; i >= 0; i--)
	{
		result = (result << 8) + *(offset + i);
	}
	return result;
}

void PrintBytes(const std::vector<BYTE>& bytes, uint32_t offset, uint32_t size, uint32_t sectorOffset, std::ostream& outDevice) {
	uint32_t nRows = size / 16;
	uint32_t nRemainder = size % 16;
	// Print columns index 0 1 2 3 4 5 6 7 8 9 A B C D E F
	outDevice << "          ";
	for (int i = 0; i < 16; i++) {
		outDevice << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << i << " ";
	}
	outDevice << std::endl;
	for (int i = 0; i < nRows; i++) {
		outDevice << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << 0x4000 + sectorOffset * 512 + offset + i * 16 << "  ";
		for (int j = 0; j < 16; j++) {
			outDevice << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)bytes[offset + i * 16 + j] << " ";
		}
		outDevice << "  ";
		for (int j = 0; j < 16; j++) {
			if (isprint(bytes[offset + i * 16 + j])) {
				outDevice << bytes[offset + i * 16 + j];
			}
			else {
				outDevice << ".";
			}
		}
		outDevice << std::endl;
		// Turn off the hex output
		outDevice << std::dec;
	}
}

std::string ConvertAttributeTypeToString(AttributeType type) {
	switch (type) {
	case AttributeType::StandardInformation:
		return "$STANDARD_INFORMATION";
	case AttributeType::AttributeList:
		return "$ATTRIBUTE_LIST";
	case AttributeType::FileName:
		return "$FILE_NAME";
	case AttributeType::ObjectId:
		return "$OBJECT_ID";
	case AttributeType::SecurityDescriptor:
		return "$SECURITY_DESCRIPTOR";
	case AttributeType::VolumeName:
		return "$VOLUME_NAME";
	case AttributeType::VolumeInformation:
		return "$VOLUME_INFORMATION";
	case AttributeType::Data:
		return "$DATA";
	case AttributeType::IndexRoot:
		return "$INDEX_ROOT";
	case AttributeType::IndexAllocation:
		return "$INDEX_ALLOCATION";
	case AttributeType::Bitmap:
		return "$BITMAP";
	case AttributeType::ReparsePoint:
		return "$REPARSE_POINT";
	case AttributeType::EAInformation:
		return "$EA_INFORMATION";
	case AttributeType::EA:
		return "$EA";
	case AttributeType::PropertySet:
		return "$PROPERTY_SET";
	case AttributeType::LoggedUtilityStream:
		return "$LOGGED_UTILITY_STREAM";
	default:
		return "$UNKNOWN";
	}
}

std::string ConvertStandardInformationFlagsToString(uint16_t flags) {
	switch (flags) {
	case 0x0000:
		return "Directory";
	case 0x0001:
		return "Read Only";
	case 0x0002:
		return "Hidden";
	case 0x0003:
		return "Read Only, Hidden";
	case 0x0004:
		return "System";
	case 0x0005:
		return "Read Only, System";
	case 0x0006:
		return "Hidden, System";
	case 0x0007:
		return "Read Only, Hidden, System";
	case 0x0020:
		return "Archive";
	default:
		return "Unknown";
	}
}