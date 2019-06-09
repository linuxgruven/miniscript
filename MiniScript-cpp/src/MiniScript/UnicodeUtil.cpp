// UnicodeUtil.cpp
//
//	Various utility functions for dealing with Unicode (especially UTF-8) text.

#include "UnicodeUtil.h"
#include "Dictionary.h"
#if _MSC_VER
	#include "wintype.h"
#endif
#include "QA.h"
#include "UnitTest.h"

namespace MiniScript {
	
	// Case Tables
	// Maps which convert a Unicode code point into the corresponding upper/lower case code point.
	static Dictionary<unsigned short, unsigned short, hashUShort> sUpperToLowerMap;
	static Dictionary<unsigned short, unsigned short, hashUShort> sLowerToUpperMap;
	static bool sMapsInitialized = false;

	// table of upper-case code points (each corresponds to the entry at the same
	// position in sLowerTable, and where an entry appears more than once, the
	// earlier (lower-valued) one is preferred):
	static unsigned short sUpperTable[] = {
	0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A,
	0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 0x0053, 0x0054,
	0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00B5, 0x00C0, 0x00C1, 0x00C2,
	0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC,
	0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x00D5, 0x00D6,
	0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x0100, 0x0102, 0x0104,
	0x0106, 0x0108, 0x010A, 0x010C, 0x010E, 0x0110, 0x0112, 0x0114, 0x0116, 0x0118,
	0x011A, 0x011C, 0x011E, 0x0120, 0x0122, 0x0124, 0x0126, 0x0128, 0x012A, 0x012C,
	0x012E, 0x0132, 0x0134, 0x0136, 0x0139, 0x013B, 0x013D, 0x013F, 0x0141, 0x0143,
	0x0145, 0x0147, 0x014A, 0x014C, 0x014E, 0x0150, 0x0152, 0x0154, 0x0156, 0x0158,
	0x015A, 0x015C, 0x015E, 0x0160, 0x0162, 0x0164, 0x0166, 0x0168, 0x016A, 0x016C,
	0x016E, 0x0170, 0x0172, 0x0174, 0x0176, 0x0178, 0x0179, 0x017B, 0x017D, 0x017F,
	0x0181, 0x0182, 0x0184, 0x0186, 0x0187, 0x0189, 0x018A, 0x018B, 0x018E, 0x018F,
	0x0190, 0x0191, 0x0193, 0x0194, 0x0196, 0x0197, 0x0198, 0x019C, 0x019D, 0x019F,
	0x01A0, 0x01A2, 0x01A4, 0x01A6, 0x01A7, 0x01A9, 0x01AC, 0x01AE, 0x01AF, 0x01B1,
	0x01B2, 0x01B3, 0x01B5, 0x01B7, 0x01B8, 0x01BC, 0x01C4, 0x01C5, 0x01C7, 0x01C8,
	0x01CA, 0x01CB, 0x01CD, 0x01CF, 0x01D1, 0x01D3, 0x01D5, 0x01D7, 0x01D9, 0x01DB,
	0x01DE, 0x01E0, 0x01E2, 0x01E4, 0x01E6, 0x01E8, 0x01EA, 0x01EC, 0x01EE, 0x01F1,
	0x01F2, 0x01F4, 0x01F6, 0x01F7, 0x01F8, 0x01FA, 0x01FC, 0x01FE, 0x0200, 0x0202,
	0x0204, 0x0206, 0x0208, 0x020A, 0x020C, 0x020E, 0x0210, 0x0212, 0x0214, 0x0216,
	0x0218, 0x021A, 0x021C, 0x021E, 0x0220, 0x0222, 0x0224, 0x0226, 0x0228, 0x022A,
	0x022C, 0x022E, 0x0230, 0x0232, 0x0345, 0x0386, 0x0388, 0x0389, 0x038A, 0x038C,
	0x038E, 0x038F, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398,
	0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F, 0x03A0, 0x03A1, 0x03A3,
	0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03C2, 0x03D0,
	0x03D1, 0x03D5, 0x03D6, 0x03D8, 0x03DA, 0x03DC, 0x03DE, 0x03E0, 0x03E2, 0x03E4,
	0x03E6, 0x03E8, 0x03EA, 0x03EC, 0x03EE, 0x03F0, 0x03F1, 0x03F2, 0x03F4, 0x03F5,
	0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409,
	0x040A, 0x040B, 0x040C, 0x040D, 0x040E, 0x040F, 0x0410, 0x0411, 0x0412, 0x0413,
	0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D,
	0x041E, 0x041F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F, 0x0460, 0x0462,
	0x0464, 0x0466, 0x0468, 0x046A, 0x046C, 0x046E, 0x0470, 0x0472, 0x0474, 0x0476,
	0x0478, 0x047A, 0x047C, 0x047E, 0x0480, 0x048A, 0x048C, 0x048E, 0x0490, 0x0492,
	0x0494, 0x0496, 0x0498, 0x049A, 0x049C, 0x049E, 0x04A0, 0x04A2, 0x04A4, 0x04A6,
	0x04A8, 0x04AA, 0x04AC, 0x04AE, 0x04B0, 0x04B2, 0x04B4, 0x04B6, 0x04B8, 0x04BA,
	0x04BC, 0x04BE, 0x04C1, 0x04C3, 0x04C5, 0x04C7, 0x04C9, 0x04CB, 0x04CD, 0x04D0,
	0x04D2, 0x04D4, 0x04D6, 0x04D8, 0x04DA, 0x04DC, 0x04DE, 0x04E0, 0x04E2, 0x04E4,
	0x04E6, 0x04E8, 0x04EA, 0x04EC, 0x04EE, 0x04F0, 0x04F2, 0x04F4, 0x04F8, 0x0500,
	0x0502, 0x0504, 0x0506, 0x0508, 0x050A, 0x050C, 0x050E, 0x0531, 0x0532, 0x0533,
	0x0534, 0x0535, 0x0536, 0x0537, 0x0538, 0x0539, 0x053A, 0x053B, 0x053C, 0x053D,
	0x053E, 0x053F, 0x0540, 0x0541, 0x0542, 0x0543, 0x0544, 0x0545, 0x0546, 0x0547,
	0x0548, 0x0549, 0x054A, 0x054B, 0x054C, 0x054D, 0x054E, 0x054F, 0x0550, 0x0551,
	0x0552, 0x0553, 0x0554, 0x0555, 0x0556, 0x1E00, 0x1E02, 0x1E04, 0x1E06, 0x1E08,
	0x1E0A, 0x1E0C, 0x1E0E, 0x1E10, 0x1E12, 0x1E14, 0x1E16, 0x1E18, 0x1E1A, 0x1E1C,
	0x1E1E, 0x1E20, 0x1E22, 0x1E24, 0x1E26, 0x1E28, 0x1E2A, 0x1E2C, 0x1E2E, 0x1E30,
	0x1E32, 0x1E34, 0x1E36, 0x1E38, 0x1E3A, 0x1E3C, 0x1E3E, 0x1E40, 0x1E42, 0x1E44,
	0x1E46, 0x1E48, 0x1E4A, 0x1E4C, 0x1E4E, 0x1E50, 0x1E52, 0x1E54, 0x1E56, 0x1E58,
	0x1E5A, 0x1E5C, 0x1E5E, 0x1E60, 0x1E62, 0x1E64, 0x1E66, 0x1E68, 0x1E6A, 0x1E6C,
	0x1E6E, 0x1E70, 0x1E72, 0x1E74, 0x1E76, 0x1E78, 0x1E7A, 0x1E7C, 0x1E7E, 0x1E80,
	0x1E82, 0x1E84, 0x1E86, 0x1E88, 0x1E8A, 0x1E8C, 0x1E8E, 0x1E90, 0x1E92, 0x1E94,
	0x1E9B, 0x1EA0, 0x1EA2, 0x1EA4, 0x1EA6, 0x1EA8, 0x1EAA, 0x1EAC, 0x1EAE, 0x1EB0,
	0x1EB2, 0x1EB4, 0x1EB6, 0x1EB8, 0x1EBA, 0x1EBC, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4,
	0x1EC6, 0x1EC8, 0x1ECA, 0x1ECC, 0x1ECE, 0x1ED0, 0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8,
	0x1EDA, 0x1EDC, 0x1EDE, 0x1EE0, 0x1EE2, 0x1EE4, 0x1EE6, 0x1EE8, 0x1EEA, 0x1EEC,
	0x1EEE, 0x1EF0, 0x1EF2, 0x1EF4, 0x1EF6, 0x1EF8, 0x1F08, 0x1F09, 0x1F0A, 0x1F0B,
	0x1F0C, 0x1F0D, 0x1F0E, 0x1F0F, 0x1F18, 0x1F19, 0x1F1A, 0x1F1B, 0x1F1C, 0x1F1D,
	0x1F28, 0x1F29, 0x1F2A, 0x1F2B, 0x1F2C, 0x1F2D, 0x1F2E, 0x1F2F, 0x1F38, 0x1F39,
	0x1F3A, 0x1F3B, 0x1F3C, 0x1F3D, 0x1F3E, 0x1F3F, 0x1F48, 0x1F49, 0x1F4A, 0x1F4B,
	0x1F4C, 0x1F4D, 0x1F59, 0x1F5B, 0x1F5D, 0x1F5F, 0x1F68, 0x1F69, 0x1F6A, 0x1F6B,
	0x1F6C, 0x1F6D, 0x1F6E, 0x1F6F, 0x1F88, 0x1F89, 0x1F8A, 0x1F8B, 0x1F8C, 0x1F8D,
	0x1F8E, 0x1F8F, 0x1F98, 0x1F99, 0x1F9A, 0x1F9B, 0x1F9C, 0x1F9D, 0x1F9E, 0x1F9F,
	0x1FA8, 0x1FA9, 0x1FAA, 0x1FAB, 0x1FAC, 0x1FAD, 0x1FAE, 0x1FAF, 0x1FB8, 0x1FB9,
	0x1FBA, 0x1FBB, 0x1FBC, 0x1FBE, 0x1FC8, 0x1FC9, 0x1FCA, 0x1FCB, 0x1FCC, 0x1FD8,
	0x1FD9, 0x1FDA, 0x1FDB, 0x1FE8, 0x1FE9, 0x1FEA, 0x1FEB, 0x1FEC, 0x1FF8, 0x1FF9,
	0x1FFA, 0x1FFB, 0x1FFC, 0x2126, 0x212A, 0x212B, 0x2160, 0x2161, 0x2162, 0x2163,
	0x2164, 0x2165, 0x2166, 0x2167, 0x2168, 0x2169, 0x216A, 0x216B, 0x216C, 0x216D,
	0x216E, 0x216F, 0x24B6, 0x24B7, 0x24B8, 0x24B9, 0x24BA, 0x24BB, 0x24BC, 0x24BD,
	0x24BE, 0x24BF, 0x24C0, 0x24C1, 0x24C2, 0x24C3, 0x24C4, 0x24C5, 0x24C6, 0x24C7,
	0x24C8, 0x24C9, 0x24CA, 0x24CB, 0x24CC, 0x24CD, 0x24CE, 0x24CF, 0xFF21, 0xFF22,
	0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C,
	0xFF2D, 0xFF2E, 0xFF2F, 0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36,
	0xFF37, 0xFF38, 0xFF39, 0xFF3A };

	// table of lower-case code points (each corresponds to the entry at the same
	// position in sUpperTable, and where an entry appears more than once, the
	// earlier (lower-valued) one is preferred):
	static unsigned short sLowerTable[] = {
	0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A,
	0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074,
	0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x03BC, 0x00E0, 0x00E1, 0x00E2,
	0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC,
	0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x00F5, 0x00F6,
	0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x0101, 0x0103, 0x0105,
	0x0107, 0x0109, 0x010B, 0x010D, 0x010F, 0x0111, 0x0113, 0x0115, 0x0117, 0x0119,
	0x011B, 0x011D, 0x011F, 0x0121, 0x0123, 0x0125, 0x0127, 0x0129, 0x012B, 0x012D,
	0x012F, 0x0133, 0x0135, 0x0137, 0x013A, 0x013C, 0x013E, 0x0140, 0x0142, 0x0144,
	0x0146, 0x0148, 0x014B, 0x014D, 0x014F, 0x0151, 0x0153, 0x0155, 0x0157, 0x0159,
	0x015B, 0x015D, 0x015F, 0x0161, 0x0163, 0x0165, 0x0167, 0x0169, 0x016B, 0x016D,
	0x016F, 0x0171, 0x0173, 0x0175, 0x0177, 0x00FF, 0x017A, 0x017C, 0x017E, 0x0073,
	0x0253, 0x0183, 0x0185, 0x0254, 0x0188, 0x0256, 0x0257, 0x018C, 0x01DD, 0x0259,
	0x025B, 0x0192, 0x0260, 0x0263, 0x0269, 0x0268, 0x0199, 0x026F, 0x0272, 0x0275,
	0x01A1, 0x01A3, 0x01A5, 0x0280, 0x01A8, 0x0283, 0x01AD, 0x0288, 0x01B0, 0x028A,
	0x028B, 0x01B4, 0x01B6, 0x0292, 0x01B9, 0x01BD, 0x01C6, 0x01C6, 0x01C9, 0x01C9,
	0x01CC, 0x01CC, 0x01CE, 0x01D0, 0x01D2, 0x01D4, 0x01D6, 0x01D8, 0x01DA, 0x01DC,
	0x01DF, 0x01E1, 0x01E3, 0x01E5, 0x01E7, 0x01E9, 0x01EB, 0x01ED, 0x01EF, 0x01F3,
	0x01F3, 0x01F5, 0x0195, 0x01BF, 0x01F9, 0x01FB, 0x01FD, 0x01FF, 0x0201, 0x0203,
	0x0205, 0x0207, 0x0209, 0x020B, 0x020D, 0x020F, 0x0211, 0x0213, 0x0215, 0x0217,
	0x0219, 0x021B, 0x021D, 0x021F, 0x019E, 0x0223, 0x0225, 0x0227, 0x0229, 0x022B,
	0x022D, 0x022F, 0x0231, 0x0233, 0x03B9, 0x03AC, 0x03AD, 0x03AE, 0x03AF, 0x03CC,
	0x03CD, 0x03CE, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8,
	0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF, 0x03C0, 0x03C1, 0x03C3,
	0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03C3, 0x03B2,
	0x03B8, 0x03C6, 0x03C0, 0x03D9, 0x03DB, 0x03DD, 0x03DF, 0x03E1, 0x03E3, 0x03E5,
	0x03E7, 0x03E9, 0x03EB, 0x03ED, 0x03EF, 0x03BA, 0x03C1, 0x03C3, 0x03B8, 0x03B5,
	0x0450, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 0x0459,
	0x045A, 0x045B, 0x045C, 0x045D, 0x045E, 0x045F, 0x0430, 0x0431, 0x0432, 0x0433,
	0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D,
	0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F, 0x0461, 0x0463,
	0x0465, 0x0467, 0x0469, 0x046B, 0x046D, 0x046F, 0x0471, 0x0473, 0x0475, 0x0477,
	0x0479, 0x047B, 0x047D, 0x047F, 0x0481, 0x048B, 0x048D, 0x048F, 0x0491, 0x0493,
	0x0495, 0x0497, 0x0499, 0x049B, 0x049D, 0x049F, 0x04A1, 0x04A3, 0x04A5, 0x04A7,
	0x04A9, 0x04AB, 0x04AD, 0x04AF, 0x04B1, 0x04B3, 0x04B5, 0x04B7, 0x04B9, 0x04BB,
	0x04BD, 0x04BF, 0x04C2, 0x04C4, 0x04C6, 0x04C8, 0x04CA, 0x04CC, 0x04CE, 0x04D1,
	0x04D3, 0x04D5, 0x04D7, 0x04D9, 0x04DB, 0x04DD, 0x04DF, 0x04E1, 0x04E3, 0x04E5,
	0x04E7, 0x04E9, 0x04EB, 0x04ED, 0x04EF, 0x04F1, 0x04F3, 0x04F5, 0x04F9, 0x0501,
	0x0503, 0x0505, 0x0507, 0x0509, 0x050B, 0x050D, 0x050F, 0x0561, 0x0562, 0x0563,
	0x0564, 0x0565, 0x0566, 0x0567, 0x0568, 0x0569, 0x056A, 0x056B, 0x056C, 0x056D,
	0x056E, 0x056F, 0x0570, 0x0571, 0x0572, 0x0573, 0x0574, 0x0575, 0x0576, 0x0577,
	0x0578, 0x0579, 0x057A, 0x057B, 0x057C, 0x057D, 0x057E, 0x057F, 0x0580, 0x0581,
	0x0582, 0x0583, 0x0584, 0x0585, 0x0586, 0x1E01, 0x1E03, 0x1E05, 0x1E07, 0x1E09,
	0x1E0B, 0x1E0D, 0x1E0F, 0x1E11, 0x1E13, 0x1E15, 0x1E17, 0x1E19, 0x1E1B, 0x1E1D,
	0x1E1F, 0x1E21, 0x1E23, 0x1E25, 0x1E27, 0x1E29, 0x1E2B, 0x1E2D, 0x1E2F, 0x1E31,
	0x1E33, 0x1E35, 0x1E37, 0x1E39, 0x1E3B, 0x1E3D, 0x1E3F, 0x1E41, 0x1E43, 0x1E45,
	0x1E47, 0x1E49, 0x1E4B, 0x1E4D, 0x1E4F, 0x1E51, 0x1E53, 0x1E55, 0x1E57, 0x1E59,
	0x1E5B, 0x1E5D, 0x1E5F, 0x1E61, 0x1E63, 0x1E65, 0x1E67, 0x1E69, 0x1E6B, 0x1E6D,
	0x1E6F, 0x1E71, 0x1E73, 0x1E75, 0x1E77, 0x1E79, 0x1E7B, 0x1E7D, 0x1E7F, 0x1E81,
	0x1E83, 0x1E85, 0x1E87, 0x1E89, 0x1E8B, 0x1E8D, 0x1E8F, 0x1E91, 0x1E93, 0x1E95,
	0x1E61, 0x1EA1, 0x1EA3, 0x1EA5, 0x1EA7, 0x1EA9, 0x1EAB, 0x1EAD, 0x1EAF, 0x1EB1,
	0x1EB3, 0x1EB5, 0x1EB7, 0x1EB9, 0x1EBB, 0x1EBD, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5,
	0x1EC7, 0x1EC9, 0x1ECB, 0x1ECD, 0x1ECF, 0x1ED1, 0x1ED3, 0x1ED5, 0x1ED7, 0x1ED9,
	0x1EDB, 0x1EDD, 0x1EDF, 0x1EE1, 0x1EE3, 0x1EE5, 0x1EE7, 0x1EE9, 0x1EEB, 0x1EED,
	0x1EEF, 0x1EF1, 0x1EF3, 0x1EF5, 0x1EF7, 0x1EF9, 0x1F00, 0x1F01, 0x1F02, 0x1F03,
	0x1F04, 0x1F05, 0x1F06, 0x1F07, 0x1F10, 0x1F11, 0x1F12, 0x1F13, 0x1F14, 0x1F15,
	0x1F20, 0x1F21, 0x1F22, 0x1F23, 0x1F24, 0x1F25, 0x1F26, 0x1F27, 0x1F30, 0x1F31,
	0x1F32, 0x1F33, 0x1F34, 0x1F35, 0x1F36, 0x1F37, 0x1F40, 0x1F41, 0x1F42, 0x1F43,
	0x1F44, 0x1F45, 0x1F51, 0x1F53, 0x1F55, 0x1F57, 0x1F60, 0x1F61, 0x1F62, 0x1F63,
	0x1F64, 0x1F65, 0x1F66, 0x1F67, 0x1F80, 0x1F81, 0x1F82, 0x1F83, 0x1F84, 0x1F85,
	0x1F86, 0x1F87, 0x1F90, 0x1F91, 0x1F92, 0x1F93, 0x1F94, 0x1F95, 0x1F96, 0x1F97,
	0x1FA0, 0x1FA1, 0x1FA2, 0x1FA3, 0x1FA4, 0x1FA5, 0x1FA6, 0x1FA7, 0x1FB0, 0x1FB1,
	0x1F70, 0x1F71, 0x1FB3, 0x03B9, 0x1F72, 0x1F73, 0x1F74, 0x1F75, 0x1FC3, 0x1FD0,
	0x1FD1, 0x1F76, 0x1F77, 0x1FE0, 0x1FE1, 0x1F7A, 0x1F7B, 0x1FE5, 0x1F78, 0x1F79,
	0x1F7C, 0x1F7D, 0x1FF3, 0x03C9, 0x006B, 0x00E5, 0x2170, 0x2171, 0x2172, 0x2173,
	0x2174, 0x2175, 0x2176, 0x2177, 0x2178, 0x2179, 0x217A, 0x217B, 0x217C, 0x217D,
	0x217E, 0x217F, 0x24D0, 0x24D1, 0x24D2, 0x24D3, 0x24D4, 0x24D5, 0x24D6, 0x24D7,
	0x24D8, 0x24D9, 0x24DA, 0x24DB, 0x24DC, 0x24DD, 0x24DE, 0x24DF, 0x24E0, 0x24E1,
	0x24E2, 0x24E3, 0x24E4, 0x24E5, 0x24E6, 0x24E7, 0x24E8, 0x24E9, 0xFF41, 0xFF42,
	0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C,
	0xFF4D, 0xFF4E, 0xFF4F, 0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56,
	0xFF57, 0xFF58, 0xFF59, 0xFF5A };

	// InitCaseMaps
	//
	//	Set up the maps that convert a Unicode code point into the corresponding
	//	index in our upper and lower case tables.  This is used for case folding.
	static void InitCaseMaps()
	{
		short qty = (short)(sizeof(sUpperTable) / sizeof(unsigned short));
		// Note: it's important to iterate backwards, because some entries appear more
		// than once, and the lower-numbered entry (i.e. earlier one in the table) is
		// the preferred one.
		for (short i=qty-1; i>=0; i--) {
			sUpperToLowerMap.SetValue( sUpperTable[i], sLowerTable[i] );
			sLowerToUpperMap.SetValue( sLowerTable[i], sUpperTable[i] );
		}
		
		sMapsInitialized = true;								// Mar 09 2003 -- JJS (1)
	}

	// MARK: -

	// AdvanceUTF8
	//
	//	Advance the character pointer within a UTF-8 buffer until it hits
	//	a limit defined by another character pointer, or has advanced the
	//	given number of characters.
	//
	// Author: JJS
	// Used in: StringDBCSLeft, etc.
	// Gets: c -- address of a pointer to advance
	//		 maxc -- pointer beyond which *c won't be moved
	//		 count -- how many characters to advance
	// Returns: <nothing>
	// Comment: Jun 03 2002 -- JJS (1)
	void AdvanceUTF8(unsigned char **c, const unsigned char *maxc, int count)
	{
		// UTF-8 characters always start with a byte whose high 2 bits
		// are not 10 (or put another way, subsequent bytes of a multi-byte
		// character sequence always have the high 2 bits set to 10).

		long charsFound = 0;
		while (charsFound < count && *c < maxc) {
			// advance to the end of the next character; that's at least one byte,
			// plus any intra-character bytes we might see
			(*c)++;
			while (IsUTF8IntraChar(**c) && *c < maxc) (*c)++;
			charsFound++;
		}
	}

	// BackupUTF8
	//
	//	Back up the character pointer within a UTF-8 buffer until it hits
	//	a limit defined by another character pointer, or has advanced the
	//	given number of characters.
	//
	// Author: JJS
	// Used in: StringDBCSLeft, etc.
	// Gets: c -- address of a pointer to advance
	//		 minc -- pointer beyond which *c won't be moved
	//		 count -- how many characters to advance
	// Returns: <nothing>
	// Comment: Jun 03 2002 -- JJS (1)
	void BackupUTF8(unsigned char **c, const unsigned char *minc, int count)
	{
		// UTF-8 characters always start with a byte whose high 2 bits
		// are not 10 (or put another way, subsequent bytes of a multi-byte
		// character sequence always have the high 2 bits set to 10).

		long charsFound = 0;
		while (charsFound < count and *c > minc) {
			// back up to the start of the previous character; that's at least one byte,
			// plus any intra-character bytes we might see
			(*c)--;
			while (IsUTF8IntraChar(**c) and *c > minc) (*c)--;
			charsFound++;
		}
	}

	// UTF8Encode
	//
	//	Encode the given Unicode code point in UTF-8 form, followed by a
	//	null terminator.  This requires up to 5 bytes.
	//
	// Author: JJS
	// Used in: various
	// Gets: uniChar -- Unicode code point (between 0 and 0x1FFFFF, inclusive)
	//		 outBuf -- pointer to buffer at least 5 bytes long
	// Returns: how many bytes were used (not counting the null), i.e., character length in bytes
	// Comment: Nov 22 2002 -- JJS (1)
	long UTF8Encode(unsigned long uniChar, unsigned char *outBuf)
	{
		unsigned char *c = outBuf;

		// There are four cases, depending on what range the code point is in.
		if (uniChar < 0x80) {
			*c = (unsigned char)uniChar;
			c++;
		}

		else if (uniChar < 0x800) {
			*c = ( 0xC0 | (unsigned char)(uniChar >> 6) );
			c++;
			*c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
			c++;
		}

		else if (uniChar < 0x10000) {
			*c = ( 0xE0 | (unsigned char)(uniChar >> 12) );
			c++;
			*c = ( 0x80 | ((unsigned char)(uniChar >> 6) & 0x3F) );
			c++;
			*c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
			c++;
		}
		
		else if (uniChar < 0x200000) {
			*c = ( 0xF0 | (unsigned char)(uniChar >> 18) );
			c++;
			*c = ( 0x80 | ((unsigned char)(uniChar >> 12) & 0x3F) );
			c++;
			*c = ( 0x80 | ((unsigned char)(uniChar >> 6) & 0x3F) );
			c++;
			*c = ( 0x80 | (unsigned char)(uniChar & 0x3F) );
			c++;
		}

		*c = 0;
		return c - outBuf;
	}

	// UTF8Decode
	//
	//	Decode the first character of the given UTF-8 String back into its
	//	Unicode code point.  This is the inverse of UTF8Encode.
	//
	// Author: JJS
	// Used in: various
	// Gets: inBuf -- pointer to buffer containing at least one UTF-8 character
	// Returns: Unicode code point (between 0 and 0x1FFFFF, inclusive)
	// Comment: Dec 02 2002 -- JJS (1)
	unsigned long UTF8Decode(unsigned char *inBuf)
	{
		unsigned char *c = inBuf;
		long out;
		
		// There are four cases, determined by the high bits of the first byte.
		if (0 == (*c & 0x80)) {
			out = *c;
		}
		else if (0xC0 == (*c & 0xE0)) {
			out = (c[0] & 0x1F);
			out = (out << 6) | (c[1] & 0x3F);
		}
		else if (0xE0 == (*c & 0xF0)) {
			out = (c[0] & 0x0F);
			out = (out << 6) | (c[1] & 0x3F);
			out = (out << 6) | (c[2] & 0x3F);
		}
		else {
			out = (c[0] & 0x07);
			out = (out << 6) | (c[1] & 0x3F);
			out = (out << 6) | (c[2] & 0x3F);
			out = (out << 6) | (c[3] & 0x3F);
		}
		
		return out;
	}

	// UTF8DecodeAndAdvance
	//
	//	Decode the first character of the given UTF-8 String back into its
	//	Unicode code point, and advance the given pointer to the next character.
	//	This is like calling UTF8Decode followed by UTF8Advance, but is more
	//	efficient.
	//
	// Author: JJS
	// Used in: various
	// Gets: inBuf -- address of pointer to buffer containing at least one UTF-8 character
	// Returns: Unicode code point (between 0 and 0x1FFFFF, inclusive)
	// Comment: Mar 04 2003 -- JJS (1)
	unsigned long UTF8DecodeAndAdvance(unsigned char **inBuf)
	{
		// We have to copy the UTF8Decode code here... fortunately
		// it's not very long.  We could combine them by having
		// UTF8Decode call this function, but then we would lose
		// some efficiency, and that is often important with UTF-8.

		unsigned char *c = *inBuf;
		long out;
		
		// There are four cases, determined by the high bits of the first byte.
		if (0 == (*c & 0x80)) {
			out = *c;
			*inBuf += 1;
		}
		else if (0xC0 == (*c & 0xE0)) {
			out = (c[0] & 0x1F);
			out = (out << 6) | (c[1] & 0x3F);
			*inBuf += 2;
		}
		else if (0xE0 == (*c & 0xF0)) {
			out = (c[0] & 0x0F);
			out = (out << 6) | (c[1] & 0x3F);
			out = (out << 6) | (c[2] & 0x3F);
			*inBuf += 3;
		}
		else {
			out = (c[0] & 0x07);
			out = (out << 6) | (c[1] & 0x3F);
			out = (out << 6) | (c[2] & 0x3F);
			out = (out << 6) | (c[3] & 0x3F);
			*inBuf += 4;
		}
		
		return out;
	}


	// UnicodeCharToUpper
	//
	//	Converts a unicode character into uppercase
	//
	// Author: AJB
	// Used in: various
	// Gets: low -- the lower case letter to find a match for
	// Returns: The uppercase version of the letter passed in, or the lowercase letter if there was an error
	// Comment: Dec 23 2002 -- AJB (1)
	unsigned long UnicodeCharToUpper( unsigned long lower )
	{
		if (lower > 0xFFFF) return lower;	// (our case folder only handles 16-bit code points)
		if (!sMapsInitialized) InitCaseMaps();
		unsigned short result = (unsigned short)lower;
		result = sLowerToUpperMap.Lookup(result, result);
		return result;
	}

	// UnicodeCharToLower
	//
	//	Converts a unicode character into lowercase
	//
	// Author: AJB
	// Used in: various
	// Gets: upper -- the upper case letter to find a match for
	// Returns: The lowercase version of the letter passed in, or the uppercase letter if there was an error
	// Comment: Dec 23 2002 -- AJB (1)
	unsigned long UnicodeCharToLower( unsigned long upper )
	{
		if (upper > 0xFFFF) return upper;	// (our case folder only handles 16-bit code points)
		if (!sMapsInitialized) InitCaseMaps();
		unsigned short result = (unsigned short)upper;
		result = sUpperToLowerMap.Lookup(result, result);
		return result;
	}

	// UTF8ToUpper
	//
	//	Converts a UTF8 String into uppercase
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the UTF8 String to convert
	//			byteCount	-- the number of bytes long the String is
	//			outBuf -- receives uppercase String in a newly created buffer (may be nil)
	//			outByteCount -- receives the byte count of the uppercase String (may be nil)
	// Returns: <nothing>
	// Comment: Dec 23 2002 -- AJB (1)
	void UTF8ToUpper( unsigned char *utf8String, unsigned long byteCount,
					  unsigned char **outBuf, unsigned long *outByteCount )
	{
		if (not outBuf and not outByteCount) return;
		
		// Note that the size of the buffer needed to hold the output text will never be larger
		// than the input buffer, though it may sometimes be smaller.  This is because in some
		// cases there are several upper-case equivalents for a lower-case letter, but when we
		// convert to upper-case, the "preferred" one is always the same size (in UTF-8) as
		// the lower-case character.  This was verified by inspecting our case tables.
		
		//	Allocate a buffer to return
		unsigned long retBufSize = byteCount + 5;							// Mar 03 2003 -- JJS (1)
		unsigned char *retBuffer = new unsigned char[retBufSize];
		//	Determine the end of the input buffer
		unsigned char *endOfBuffer = utf8String + byteCount;
		//	Keep track of our current position the return buffer
		unsigned char *curOutPos = retBuffer;
		//	Store the end of the output buffer (minus 5 to allow room for last char)
		unsigned char *endOfOutputBuffer = retBuffer + retBufSize - 5;
		
		//	While we're not at the end of the input buffer
		while (utf8String != endOfBuffer) {
			//	Decode a character from the input String, convert it to uppercase
			unsigned long uniChar = UnicodeCharToUpper( UTF8Decode( utf8String ) );
			
			//	Sanity Check: make sure we're not writing out beyond the end of our allocated output buffer
			if (curOutPos > endOfOutputBuffer) {
				break;
			}
			
			//	Encode the new letter, and advance out return string's buffer
			curOutPos += UTF8Encode( uniChar, curOutPos );
			
			//	Advance our input String
			AdvanceUTF8( &utf8String, endOfBuffer, 1 );
		}
		
		//	Return the beginning of our output String, and length of output buffer used
		if (outBuf) *outBuf = retBuffer;
		else delete[] retBuffer;
		if (outByteCount) *outByteCount = curOutPos - retBuffer;
	}

	// UTF8ToLower
	//
	//	Converts a UTF8 String into lowercase
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the UTF8 String to convert
	//			byteCount	-- the number of bytes long the String is
	//			outBuf -- receives lowercase String in a newly created buffer (may be nil)
	//			outByteCount -- receives the byte count of the lowercase String (may be nil)
	// Returns: <nothing>
	// Comment: Dec 23 2002 -- AJB (1)
	void UTF8ToLower( unsigned char *utf8String, unsigned long byteCount,
					  unsigned char **outBuf, unsigned long *outByteCount )
	{
		if (not outBuf and not outByteCount) return;

		// Note that the size of the buffer needed to hold the output text will never be larger
		// than the input buffer, though it may sometimes be smaller.  This is because in some
		// cases there are several upper-case equivalents for a lower-case letter, but when we
		// convert to upper-case, the "preferred" one is always the same size (in UTF-8) as
		// the lower-case character.  This was verified by inspecting our case tables.
		
		//	Allocate a buffer to return
		unsigned long retBufSize = byteCount + 5;						// Mar 03 2003 -- JJS (1)
		unsigned char *retBuffer = new unsigned char[retBufSize];
		//	Determine the end of the input buffer
		unsigned char *endOfBuffer = utf8String + byteCount;
		//	Keep track of our current position the return buffer
		unsigned char *curOutPos = retBuffer;
		//	Store the end of the output buffer (minus 5 to allow room for last char)
		unsigned char *endOfOutputBuffer = retBuffer + retBufSize - 5;
		
		//	While we're not at the end of the input buffer
		while (utf8String != endOfBuffer) {
			//	Decode a character from the input String, convert it to lowercase
			unsigned long uniChar = UnicodeCharToLower( UTF8Decode( utf8String ) );
			
			//	Sanity Check: make sure we're not writing out beyond the end of our allocated output buffer
			if (curOutPos > endOfOutputBuffer) {
				break;
			}
			
			//	Encode the new letter, and advance out return string's buffer
			curOutPos += UTF8Encode(uniChar, curOutPos);
			
			//	Advance our input String
			AdvanceUTF8(&utf8String, endOfBuffer, 1);
		}
		
		//	Return the beginning of our output String, and length of output buffer used
		if (outBuf) *outBuf = retBuffer;
		else delete[] retBuffer;
		if (outByteCount) *outByteCount = curOutPos - retBuffer;
	}

	// UTF8Capitalize
	//
	//	Capitalize each word of a UTF-8 String.
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the UTF8 String to convert
	//			byteCount	-- the number of bytes long the String is
	//			outBuf -- receives lowercase String in a newly created buffer (may be nil)
	//			outByteCount -- receives the byte count of the lowercase String (may be nil)
	// Returns: <nothing>
	// Comment: Mar 04 2003 -- JJS (1)
	void UTF8Capitalize( unsigned char *utf8String, unsigned long byteCount,
					  unsigned char **outBuf, unsigned long *outByteCount )
	{
		if (not outBuf and not outByteCount) return;

		// Note that the size of the buffer needed to hold the output text will never be larger
		// than the input buffer, though it may sometimes be smaller.  This is because in some
		// cases there are several upper-case equivalents for a lower-case letter, but when we
		// convert to upper-case, the "preferred" one is always the same size (in UTF-8) as
		// the lower-case character.  This was verified by inspecting our case tables.
		
		// Allocate a buffer to return
		unsigned long retBufSize = byteCount + 5;						// Mar 03 2003 -- JJS (1)
		unsigned char *retBuffer = new unsigned char[retBufSize];
		// Determine the end of the input buffer
		unsigned char *endOfBuffer = utf8String + byteCount;
		// Keep track of our current position the return buffer
		unsigned char *curOutPos = retBuffer;
		// Store the end of the output buffer (minus 5 to allow room for last char)
		unsigned char *endOfOutputBuffer = retBuffer + retBufSize - 5;
		// Keep track of when we're at the start of a word.
		bool atStartOfWord = true;
		
		//	While we're not at the end of the input buffer
		while (utf8String != endOfBuffer) {
			// Decode a character from the input String, convert case as needed
			unsigned long uniChar = UTF8Decode( utf8String );
			if (atStartOfWord) uniChar = UnicodeCharToUpper(uniChar);
			else uniChar = UnicodeCharToLower(uniChar);
			
			// Sanity Check: make sure we're not writing out beyond the end of our allocated output buffer
			if (curOutPos > endOfOutputBuffer) {
				break;
			}
			
			// Encode the new letter, and advance out return string's buffer
			curOutPos += UTF8Encode(uniChar, curOutPos);
			
			// Update our "at start of word" flag
			atStartOfWord = UnicodeCharIsWhitespace(uniChar);
			
			// Advance our input String
			AdvanceUTF8(&utf8String, endOfBuffer, 1);
		}
		
		//	Return the begining of our output String, and length of output buffer used
		if (outBuf) *outBuf = retBuffer;
		else delete[] retBuffer;
		if (outByteCount) *outByteCount = curOutPos - retBuffer;
	}

	// UTF8IsCaseless
	//
	//	Determine whether the given String contains only characters which have
	//	no upper/lowercase variants.
	//
	// Author: JJS
	// Used in: various
	// Gets: utf8String -- pointer to UTF-8 String
	//		 byteCount -- number of bytes in String
	// Returns: true if String is caseless, false if it contains characters which have case
	// Comment: Apr 02 2003 -- JJS (1)
	bool UTF8IsCaseless(unsigned char *utf8String, unsigned long byteCount)
	{
		unsigned char *ptr = utf8String;
		unsigned char *pend = utf8String + byteCount;
		
		while (ptr < pend) {
			unsigned long uchar = UTF8DecodeAndAdvance( &ptr );
			if (uchar <= 0xFFFF and 
				(sLowerToUpperMap.ContainsKey((unsigned short)uchar ) or sUpperToLowerMap.ContainsKey((unsigned short)uchar))) {
				// if it's in the upper or lower table, then it has case,
				// hence this String is not caseless
				return false;
			}
		}

		return true;
	}

	// UTF8StringCompare
	//
	//	Compare two UTF-8 strings, optionally ignoring case.
	//
	// Author: JJS
	// Used in: various
	// Gets: leftBuf -- pointer to left-hand UTF-8 String
	//		 leftByteCount -- number of bytes in left-hand String
	//		 rightBuf -- pointer to right-hand UTF-8 String
	//		 rightByteCount -- number of bytes in right-hand String
	//		 ignorCase -- if false, be case-sensitive; if true, be case-insensitive
	// Returns: -1 if leftBuf < rightBuf, 0 if they're equal, and 1 if leftBuf > rightBuf
	// Comment: Mar 04 2003 -- JJS (1)
	long UTF8StringCompare(unsigned char *leftBuf, unsigned long leftByteCount,
						   unsigned char *rightBuf, unsigned long rightByteCount,
						   bool ignoreCase)
	{
		unsigned char *lptr = leftBuf;
		unsigned char *rptr = rightBuf;
		unsigned char *lend = leftBuf + leftByteCount;
		unsigned char *rend = rightBuf + rightByteCount;
		
		while (lptr < lend and rptr < rend) {
			unsigned long lchar = UTF8DecodeAndAdvance(&lptr);
			unsigned long rchar = UTF8DecodeAndAdvance(&rptr);
			long comparison = UnicodeCharCompare(lchar, rchar, ignoreCase);
			if (comparison) return comparison;
		}
		// The strings match, as far as they go.  So, whichever one
		// is shorter comes first.
		if (leftByteCount < rightByteCount) return -1;
		if (leftByteCount > rightByteCount) return 1;
		return 0;
	}

	// UCS2StringCompare
	//
	//	Compare two UCS-2 strings, optionally ignoring case.
	//
	// Author: JJS
	// Used in: various
	// Gets: leftBuf -- pointer to left-hand UCS-2 String
	//		 leftByteCount -- number of bytes in left-hand String
	//		 rightBuf -- pointer to right-hand UCS-2 String
	//		 rightByteCount -- number of bytes in right-hand String
	//		 ignorCase -- if false, be case-sensitive; if true, be case-insensitive
	// Returns: -1 if leftBuf < rightBuf, 0 if they're equal, and 1 if leftBuf > rightBuf
	// Comment: Mar 04 2003 -- JJS (1)
	long UCS2StringCompare(unsigned short *leftBuf, unsigned long leftByteCount,
						   unsigned short *rightBuf, unsigned long rightByteCount,
						   bool ignoreCase)
	{
		long lchars = leftByteCount / 2;
		long rchars = rightByteCount / 2;
		long minChars = (lchars < rchars ? lchars : rchars);
		
		unsigned short *lptr = leftBuf;
		unsigned short *rptr = rightBuf;

		for (long i=0; i < minChars; i++) {
			long comparison = UnicodeCharCompare(*lptr, *rptr, ignoreCase);
			if (comparison) return comparison;
			lptr++;
			rptr++;
		}
		// The strings match, as far as they go.  So, whichever one
		// is shorter comes first.
		if (lchars < rchars) return -1;
		if (lchars > rchars) return 1;
		return 0;
	}

	// UCS2ToUpper
	//
	//	Converts a USC2 String into uppercase
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the USC2 String to convert
	//			byteCount	-- the number of bytes long the String is
	// Returns: The uppercase version of the String
	// Comment: Dec 23 2002 -- AJB (1)
	unsigned short *UCS2ToUpper(unsigned short *ucs2String, unsigned long byteCount)
	{
		//	Allocate a new buffer
		unsigned short *ret = new unsigned short[byteCount];
		//	Our loop counter is less than the byte count in the String
		unsigned long loopCount = byteCount / sizeof(unsigned short);
		
		//	Loop thru the number of characters we have
		for(unsigned long i = 0; i < loopCount; i++) {
			//	Make the character uppercase
			ret[i] = (unsigned short)UnicodeCharToUpper(ucs2String[i]);
		}
		
		//	Return our buffer
		return ret;
	}

	// UCS2ToLower
	//
	//	Converts a USC2 String into lowercase
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the USC2 String to convert
	//			byteCount	-- the number of bytes long the String is
	// Returns: The lowercase version of the String
	// Comment: Dec 23 2002 -- AJB (1)
	unsigned short *UCS2ToLower(unsigned short *ucs2String, unsigned long byteCount)
	{
		//	Allocate a new buffer
		unsigned short *ret = new unsigned short[byteCount];
		//	Our loop counter is less than the byte count in the String
		unsigned long loopCount = byteCount / sizeof(unsigned short);
		
		//	Loop thru the number of characters we have
		for(unsigned long i = 0; i < loopCount; i++) {
			//	Make the character lowercase
			ret[i] = (unsigned short)UnicodeCharToLower(ucs2String[i]);
		}
		
		//	Return our buffer
		return ret;
	}

	// UCS2Capitalize
	//
	//	Capitalize each word of a USC2 String.
	//
	// Author: AJB
	// Used in: various
	// Gets:	utf8String	-- the USC2 String to convert
	//			byteCount	-- the number of bytes long the String is
	// Returns: The lowercase version of the String (caller must dispose).
	// Comment: Mar 04 2003 -- JJS (1)
	unsigned short *UCS2Capitalize(unsigned short *ucs2String, unsigned long byteCount)
	{
		// Allocate a new buffer
		unsigned short *ret = new unsigned short[byteCount];
		// Our loop counter is less than the byte count in the String
		unsigned long loopCount = byteCount / sizeof(unsigned short);
		// Keep track of when we're at the start of a word.
		bool atStartOfWord = true;
		
		// Loop through the number of characters we have
		for(unsigned long i = 0; i < loopCount; i++) {
			// Adjust the case as necessary, and update our flag
			unsigned short uniChar = ucs2String[i];
			if (atStartOfWord) ret[i] = (unsigned short)UnicodeCharToUpper(uniChar);
			else ret[i] = (unsigned short)UnicodeCharToLower(uniChar);
			atStartOfWord = UnicodeCharIsWhitespace(uniChar);
		}
		
		//	Return our buffer
		return ret;
	}

	//--------------------------------------------------------------------------------
	// Unit Tests
	#if DEBUG && !STRuntime

	// class TestUnicodeUtil
	//
	//	Tests various methods in this module.
	//
	class TestUnicodeUtil : public UnitTest
	{
	  public:
		TestUnicodeUtil() : UnitTest("UnicodeUtil") {}
		virtual void Run();
	};

	void TestUnicodeUtil::Run()
	{
		// test UTF8Encode
		unsigned char buf[6];
		for (short i=0; i<6; i++) {
			buf[i] = 42;
		}
		
		long charSize = UTF8Encode(0xA9, buf);
		Assert(charSize == 2);
		Assert(buf[0] == 0xC2 and buf[1] == 0xA9 and buf[2] == 0);
		
		charSize = UTF8Encode(0x2260, buf);
		Assert(charSize == 3);
		Assert(buf[0] == 0xE2 and buf[1] == 0x89 and buf[2] == 0xA0 and buf[3] == 0);

		Assert(buf[5] == 42);
		
		// test UTF8Decode
		buf[0] = 0xC2;
		buf[1] = 0xA9;
		Assert(UTF8Decode(buf) == 0xA9);
		unsigned char *ptr = buf;
		Assert(UTF8DecodeAndAdvance(&ptr) == 0xA9);
		Assert(ptr == buf+2);

		UTF8Encode(0x123, buf);
		Assert(UTF8Decode(buf) == 0x123);

		UTF8Encode(0x1234, buf);
		Assert(UTF8Decode(buf) == 0x1234);

		UTF8Encode(0x12345, buf);
		Assert(UTF8Decode(buf) == 0x12345);
		
		// test various case folding
		Assert(UnicodeCharToUpper('a') == 'A');
		Assert(UnicodeCharToLower('A') == 'a');

		Assert(UnicodeCharToUpper(0xFF57) == 0xFF37);
		Assert(UnicodeCharToLower(0xFF37) == 0xFF57);

		Assert(UnicodeCharToLower(0x1FBE) == 0x03B9);
		Assert(UnicodeCharToUpper(0x03B9) == 0x0345);		// note asymmetry!
		
		Assert(UnicodeCharToUpper(0x1234) == 0x1234);
		Assert(UnicodeCharToLower(0x1234) == 0x1234);

		{
			unsigned char *buf = 0;
			unsigned long bufSize;
			UTF8ToUpper((unsigned char *)"aB9", 3, &buf, &bufSize);
			Assert(buf and 3==bufSize and 'A'==buf[0] and 'B'==buf[1] and '9'==buf[2]);
			delete[] buf;

			UTF8ToLower((unsigned char *)"aB9", 3, &buf, &bufSize);
			Assert(buf and 3==bufSize and 'a'==buf[0] and 'b'==buf[1] and '9'==buf[2]);
			delete[] buf;

			UTF8Capitalize((unsigned char *)"aB c", 4, &buf, &bufSize);
			Assert(buf and 4==bufSize and 'A'==buf[0] and 'b'==buf[1] and 'C'==buf[3]);
			delete[] buf;
		}

	}

	RegisterUnitTest(TestUnicodeUtil);

#endif  // DEBUG && !STRuntime

}


