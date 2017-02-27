#include "stdafx.h"

#include "ademco_func.h"
#include <cassert>
#include <sstream>
#include <iomanip>

namespace ademco
{

#ifdef __GNUG__
#define sprintf_s sprintf
#define strcat_s(dst, len, src) strcat(dst, src)
#define strcpy_s(dst, ct, src) strcpy(dst, src)
#define strnlen_s strnlen
#define sscanf_s sscanf
#endif // __GNUG__


unsigned short CalculateCRC_char(char c, unsigned short crc)
{
	static unsigned short crcTable[] = {
		/* DEFINE THE FIRST ORDER POLYINOMIAL TABLE */
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040,
	};

	unsigned short CRC = crc;
	auto tmp = static_cast<unsigned short>(CRC >> 8) ^ (crcTable[static_cast<unsigned char>(c) ^ static_cast<unsigned char>(CRC & 0xFF)]);
	CRC = static_cast<unsigned short>(tmp);
	return CRC;
}

unsigned short CalculateCRC(const char* buff, size_t len, unsigned short crc)
{
	unsigned short CRC = crc;
	for (size_t i = 0; i < len; ++i) {
		CRC = CalculateCRC_char(buff[i], CRC);
	}

	return CRC;
}

unsigned short CalculateCRC_Vector(const char_array& a, unsigned short crc)
{
	unsigned short CRC = crc;
	for (auto c : a) {
		CRC = CalculateCRC_char(c, CRC);
	}

	return CRC;
}

int HexCharArrayToDec(const char *hex, size_t len)
{
	int dec = 0;
	for (size_t i = 0; i < len; i++) {
		dec = dec * 0x10 + HexChar2Dec(hex[i]);
	}

	return dec;
}

int HexChar2Dec(char hex)
{
	if (hex >= '0' && hex <= '9')
		return hex - '0';
	else if (hex >= 'A' && hex <= 'F')
		return hex - 'A' + 10;
	else if (hex >= 'a' && hex <= 'f')
		return hex - 'a' + 10;
	else if (hex == 'l' || hex == 'L')
		return 0;
	else {
		char log[128] = { 0 };
		sprintf_s(log, "HexChar2Dec: not a hex char. (%c) (%d)", hex, hex);
		JLOGA(log);
		assert(0);
		return 0;
	}
}

char Dec2Hex(char d)
{
	if (0 <= d && d <= 9) {
		return (char)((unsigned char)d + (unsigned char)'0');
	} else if (0x0A <= d && d <= 0x0F) {
		return char(d - 0x0A + (char)'A');
	} else {
		char log[128] = { 0 };
		sprintf_s(log, "Dec2Hex: not a 0-f value. (%c) (%d)", d, d);
		assert(0);
		JLOGA(log);
		return 0;
	}
}

int NumStr2Dec(const char* str, size_t str_len)
{
	int dec = 0;
	for (size_t i = 0; i < str_len; i++) {
		dec = dec * 10 + HexChar2Dec(str[i]);
	}

	return dec;
}

const char* HexCharArrayToStr(const char* hex, size_t len, unsigned char mask /* = (char)0x0f*/)
{
	static char ret[64];
	char tmp[8];
	memset(ret, 0, sizeof(ret));
	memset(tmp, 0, sizeof(tmp));
	unsigned char high = 0, low = 0;
	for (size_t i = 0; i < len; i++) {
		high = (unsigned char)((hex[i] >> 4) & mask);
		if (high == mask)
			break;

		sprintf_s(tmp, "%d", high);
		strcat_s(ret, 64, tmp);

		low = (unsigned char)(hex[i] & mask);
		if (low == mask)
			break;

		sprintf_s(tmp, "%d", low);
		strcat_s(ret, 64, tmp);
	}
	return ret;
}

const char* HexCharArrayToStr(char* dst, const char* hex, size_t len,
							  unsigned char mask/* = (char)0x0f*/)
{
	memset(dst, 0, len * 2);
	char tmp[8] = { 0 };
	unsigned char high = 0, low = 0;

	for (size_t i = 0; i < len; i++) {
		high = (unsigned char)((hex[i] >> 4) & mask);
		if (high == mask)
			break;
		sprintf_s(tmp, "%d", high);

		strcat_s(dst, len * 2 + 1, tmp);

		low = (unsigned char)(hex[i] & mask);
		if (low == mask)
			break;
		sprintf_s(tmp, "%d", low);
		strcat_s(dst, len * 2 + 1, tmp);
	}
	return dst;
}

void Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF)
{
	if (dec < 0) {
		JLOGA("0LLL can't be negative.");
		return;
	}
	if (dec == 0) {
		char tmp[8] = { 0 };
		//sprintf_s(tmp, "0LLL");
		strcpy_s(tmp, 8, "0000");
		memcpy(hex, tmp, 4);
		return;
	}
	if (bMax0FFF && dec > 0x0fff) {
		JLOGA("0LLL is bigger than 0x0fff.");
		return;
	}
	char tmp[8] = { 0 };
	sprintf_s(tmp, "%04X", dec);
	memcpy(hex, tmp, 4);
}

void NumStr2HexCharArray_N(const char* str, char* hexarr, size_t max_hex_len/* = 9*/)
{
	if (str == nullptr) {
		JLOGA("NumStr2HexCharArray_N: memory access denied.");
		return;
	}

	auto len = strlen(str);
	if (len > max_hex_len * 2) {
		JLOGA("NumStr2HexCharArray_N: length too long.");
		return;
	}

	size_t i = 0;
	for (i = 0; i < len; i++) {
		if (!isdigit(str[i])) {
			JLOGA("NumStr2HexCharArray_N: not all character is digit.");
			return;
		}
	}

	const size_t full_str_len = max_hex_len * 2;
	char full_str[32] = { 0 };
	memset(full_str, 0, sizeof(full_str));
	strcpy_s(full_str, 32, str);
	while (strlen(full_str) < full_str_len) {
		strcat_s(full_str, 32, "f");
	}

	for (i = 0; i < max_hex_len; i++) {
		char ch = (char)HexChar2Dec(full_str[i * 2]) & 0x0F;
		ch = char(ch << 4);
		ch |= (char)HexChar2Dec(full_str[i * 2 + 1]) & 0x0F;
		hexarr[i] = ch;
	}
}

void ConvertHiLoAsciiToAscii(char* dst, const char* src, size_t len)
{
	for (size_t i = 0; i < len; i++) {
		if (!isdigit(src[i]) && !isalpha(src[i])) {
			JLOGA("NumStr2HexCharArray_N: not all character is digit or alpha.");
			return;
		}
	}

	const size_t dst_len = len / 2;
	memset(dst, 0, dst_len);

	for (size_t i = 0; i < dst_len; i++) {
		char ch = HexChar2Dec(src[i * 2]) & 0x0F;
		ch = char(ch << 4);
		ch |= (char)HexChar2Dec(src[i * 2 + 1]) & 0x0F;
		dst[i] = ch;
	}
}

void AdemcoDataSegment::Make(int ademco_id, int gg, ADEMCO_EVENT ademco_event, int zone)
{
	reset();
	std::stringstream ss;
	std::string str;
	_data.clear();
	_data.push_back('[');
	_data.push_back('#');
	ss << std::setfill('0') << std::setw(6) << std::uppercase << std::hex << ademco_id;
	str = ss.str();
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	_data.push_back('|');
	ss.str(""); ss.clear();
	ss << std::setfill('0') << std::setw(4) << std::dec << ademco_event;
	str = ss.str();
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	_data.push_back(' ');

	if (gg == 0xEE) {
		_data.push_back('E');
		_data.push_back('E');
	} else if (gg == 0xCC) {
		_data.push_back('C');
		_data.push_back('C');
	} else {
		_data.push_back(static_cast<char>(((gg / 10) & 0x0F) + '0'));
		_data.push_back(static_cast<char>(((gg % 10) & 0x0F) + '0'));
	}

	_data.push_back(' ');
	ss.str(""); ss.clear();
	ss << std::setfill('0') << std::setw(3) << std::dec << zone;
	str = ss.str();
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	_data.push_back(']');

	_ademco_id = ademco_id;
	_ademco_event = ademco_event;
	_zone = zone;
	_gg = gg;
	_len = 21;
	_valid = true;
}

bool AdemcoDataSegment::Parse(const char* pack, size_t pack_len)
{
	reset();
	const char* p = pack;
	do {
		if (*p++ != '[')
			break;

		int acct_len = 4;
		if (pack_len == 2 && *p == ']') {
			return true;
			//                     [   #  acct |   mt  s      q event  s   gg  s   zone ]   // 19
		} else if (pack_len == 1 + 1 + 4 + 1 + /*2 + 1 + */1 + 3 + 1 + 2 + 1 + 3 + 1) {
			acct_len = 4;
		} else if (pack_len == 1 + 1 + 6 + 1 + /*2 + 1 + */1 + 3 + 1 + 2 + 1 + 3 + 1) { // 21)
			acct_len = 6;
		} else
			break;

		if (*p++ != '#')
			break;
		if (acct_len == 4) {
			if (*(p + 4) != '|')
				break;
			_ademco_id = HexCharArrayToDec(p, 4);
			_len += 4;
			p += 5;
		} else if (acct_len == 6) {
			if (*(p + 6) != '|')
				break;
			_ademco_id = HexCharArrayToDec(p, 6);
			_len += 6;
			p += 7;
		}

		_ademco_event = static_cast<ADEMCO_EVENT>(NumStr2Dec(p, 4));
		p += 4;
		if (*p++ != ' ')
			break;

		if (*p == 'E' && *(p + 1) == 'E') {
			_gg = 0xEE;
		} else if (*p == 'C' && *(p + 1) == 'C') {
			_gg = 0xCC;
		} else {
			_gg = (unsigned char)((*p - '0') * 10 + (*(p + 1) - '0'));
		}

		p += 2;
		if (*p++ != ' ')
			break;

		_zone = NumStr2Dec(p, 3);
		_valid = true;
		_len = pack_len - 2;
		std::copy(pack + 1, pack + 1 + _len, std::back_inserter(_data));
		return true;
	} while (0);
	reset();
	return false;
}

void AdemcoTimeStamp::Make()
{
	_time = time(nullptr);
#ifdef VC_EXTRALEAN
	struct tm tmtm;
	localtime_s(&tmtm, &_time);
	sprintf_s(_data, "_%02d:%02d:%02d,%02d-%02d-%04d", // 20
			  tmtm.tm_hour, tmtm.tm_min, tmtm.tm_sec,
			  tmtm.tm_mon + 1, tmtm.tm_mday, tmtm.tm_year + 1900);
#else
	struct tm* tmtm = localtime(&_time);
	sprintf_s(_data, "_%02d:%02d:%02d,%02d-%02d-%04d",
			  tmtm->tm_hour, tmtm->tm_min, tmtm->tm_sec,
			  tmtm->tm_mon + 1, tmtm->tm_mday, tmtm->tm_year + 1900);
#endif
	_len = strnlen_s(_data, sizeof(_data));
}

bool AdemcoTimeStamp::Parse(const char* pack, size_t pack_len)
{
	assert(pack_len < sizeof(_data));
	memcpy(_data, pack, pack_len);

	struct tm tmtm;
	size_t ret = sscanf_s(pack, "_%02d:%02d:%02d,%02d-%02d-%04d",
						  &tmtm.tm_hour, &tmtm.tm_min, &tmtm.tm_sec,
						  &tmtm.tm_mon, &tmtm.tm_mday, &tmtm.tm_year);
	assert(ret == 6);
	_len = pack_len;
	if (tmtm.tm_year == 1900) {
		_time = time(nullptr);
#ifdef VC_EXTRALEAN
		localtime_s(&tmtm, &_time);
		sprintf_s(_data, "_%02d:%02d:%02d,%02d-%02d-%04d",
				  tmtm.tm_hour, tmtm.tm_min, tmtm.tm_sec,
				  tmtm.tm_mon + 1, tmtm.tm_mday, tmtm.tm_year + 1900);
		strftime(_data, sizeof(_data), "_%H:%M:%S,%m-%d-%Y", &tmtm);
#else
		struct tm* ptm = localtime(&_time);
		strftime(_data, sizeof(_data), "_%H:%M:%S,%m-%d-%Y", ptm);
#endif
		return true;
	}
	tmtm.tm_year -= 1900;
	tmtm.tm_mon--;
	tmtm.tm_isdst = -1;
	_time = mktime(&tmtm);
	if (_time < 0) {
		_time = time(nullptr);
		struct tm* ptm = localtime(&_time);
		strftime(_data, sizeof(_data), "_%H:%M:%S,%m-%d-%Y", ptm);
		return true;
	}
	return true;
}

size_t AdemcoPacket::GetLength() const
{ //      LF  CRC LEN               SEQ  
	return 1 + 4 + 4 + _id.size() + 4 + _rrcvr.size() + _lpref.size()
		+ _acct.size() + _ademco_data._len + /*strlen(_xdata)*/ (_xdata ? _xdata->size() : 0) +
		_timestamp._len + 1; // CR
}

void AdemcoPacket::CopyData(char* dst, size_t length)
{
	assert(length == GetLength());

	char* pos = dst;
	*pos++ = _LF;

	char* crc_pos = pos; pos += 4;
	char* len_pos = pos; pos += 4;
	char* id_pos = pos;

#define COPY_FROM_ARRAY_TO_P(a, p) for (auto c : a) { *p++ = c; }

	COPY_FROM_ARRAY_TO_P(_id, pos);
	COPY_FROM_ARRAY_TO_P(_seq, pos);
	COPY_FROM_ARRAY_TO_P(_rrcvr, pos);
	COPY_FROM_ARRAY_TO_P(_lpref, pos);
	COPY_FROM_ARRAY_TO_P(_acct, pos);

	for (auto c : _ademco_data._data) {
		*pos++ = c;
	}

	if (_xdata) {
		for (auto c : *_xdata) {
			*pos++ = c;
		}
	}

	memcpy(pos, _timestamp._data, _timestamp._len);
	pos += _timestamp._len;

	*pos++ = _CR;
	assert(pos - dst == (int)length);

	size_t ademco_len = length - 9 - 1;
	Dec2HexCharArray_4(CalculateCRC(id_pos, ademco_len), crc_pos, false);
	Dec2HexCharArray_4(ademco_len, len_pos);
}

size_t AdemcoPacket::Make(char* pack, size_t pack_len, const char* id,
						  int seq, char const* acct, int ademco_id,
						  ADEMCO_EVENT ademco_event, int gg, int zone,
						  const char_array_ptr& xdata)
{
	assert(pack); assert(id);
	Clear();

	std::copy(id, id + strlen(id), std::back_inserter(_id));
	char tmp[5];
	sprintf_s(tmp, "%04d", seq);
	std::copy(tmp, tmp + 4, std::back_inserter(_seq));
	std::copy(RRCVR, RRCVR + strlen(RRCVR), std::back_inserter(_rrcvr));
	std::copy(LPREF, LPREF + strlen(LPREF), std::back_inserter(_lpref));

	if (acct) {
		int len = strlen(acct);
		_acct.clear();
		_acct.push_back('#');

		for (auto i = 0; i < len; i++) {
			_acct.push_back(acct[i]);
		}
	} else {
		std::stringstream ss;
		ss << '#' << std::setw(6) << std::setfill('0') << std::uppercase << std::hex << ademco_id;
		_acct.clear();
		auto str = ss.str();
		std::copy(str.begin(), str.end(), std::back_inserter(_acct));
	}

	if (is_null_data(id)) {
		_ademco_data.Make(); 
		_xdata = nullptr;
	} else {
		_ademco_data.Make(ademco_id, gg, ademco_event, zone);

		if (xdata && xdata->size() > 0) {
			auto _xdata_len = xdata->size();

			if (_xdata == nullptr) {
				_xdata = std::make_shared<char_array>();
			}

			_xdata->clear();
			_xdata->push_back('[');
			_xdata->push_back(HIBYTE(LOWORD(_xdata_len)));
			_xdata->push_back(LOBYTE(LOWORD(_xdata_len)));
			std::copy(xdata->begin(), xdata->end(), std::back_inserter(*_xdata));
			_xdata->push_back(']');
		} else { 
			_xdata = nullptr;
		}
	}

	_timestamp.Make();

	size_t length = GetLength();
	assert(length < pack_len);

	CopyData(pack, length);

	return length;
}

ParseResult AdemcoPacket::Parse(const char* pack, size_t pack_len, size_t& cbCommited)
{
	try {
		do {
			if (pack_len < 9) return RESULT_NOT_ENOUGH;

			// check LF
			if (pack[0] != _LF) { JLOGA("pack[0] %c 0x%x is not _LF", pack[0], pack[0]); /*assert(0);*/ break; }

			Clear();

			// read crc & len
			std::copy(pack + 1, pack + 1 + 4, std::back_inserter(_crc));
			std::copy(pack + 5, pack + 5 + 4, std::back_inserter(_len));
			int ademco_crc = HexCharArrayToDec(pack + 1, 4);
			int ademco_len = HexCharArrayToDec(pack + 5, 4);

			// read till CR
			DWORD dwLenToParse = 9 + ademco_len + 1; // 1 for CR
			size_t seg_len = 0;
#define ASSERT_SEG_LENGTH(seg) seg_len = p - seg##_pos; \
	std::copy(seg##_pos, seg##_pos + seg_len, std::back_inserter(_##seg));

			// check if packet is enough to parse
			if (pack_len < dwLenToParse)
				return RESULT_NOT_ENOUGH;

			// check CR
			const char* id_pos = pack + 9;
			const char* CR_pos = id_pos + ademco_len;
			if (*CR_pos != _CR) { JLOGA("ademco_len err!\n"); JLOGB(pack, pack_len); assert(0); break; }

			// check ademco CRC
			unsigned short crc_cal = CalculateCRC(id_pos, ademco_len);
			if (ademco_crc != crc_cal) { JLOGA("crc failed!\n"); JLOGB(pack, pack_len); assert(0); break; }

			// id
			if (*id_pos != '\"') { JLOGA("find left \" of \"id\" faild!\n"); JLOGB(pack, pack_len); assert(0); break; }	// find first " of "id".
			const char* p = id_pos + 1;					// find last  " of "id".
			while (p < CR_pos && *p != '\"') { p++; }
			if (*p++ != '\"') { JLOGA("find right \" of \"id\" faild!\n"); JLOGB(pack, pack_len); assert(0); break; }		// " not found.
			ASSERT_SEG_LENGTH(id);

			// seq (and Rrcvr, it may not exists)
			const char* seq_pos = p;
			while (p < CR_pos && *p != 'R' && *p != 'L') { p++; }
			ASSERT_SEG_LENGTH(seq);
			if (*p == 'R') { // Rrcvr exists
				const char* rrcvr_pos = p;
				while (p < CR_pos && *p != 'L' && *p != '#') { p++; }
				ASSERT_SEG_LENGTH(rrcvr);
			} else if (*p == 'L') { // Rrcvr not exists, pass
				// pass
			} else { JLOGA("Lpref and Rrcvr not found!\n"); JLOGB(pack, pack_len); assert(0); break; }

			// Lpref
			if (*p != 'L') { JLOGA("Lpref not found!\n"); JLOGB(pack, pack_len); assert(0); break; } // L of Lpref not found.
			const char* lpref_pos = p;
			while (p < CR_pos && *p != '#') { p++; }
			ASSERT_SEG_LENGTH(lpref);

			// acct
			if (*p++ != '#') { assert(0); break; } // # of #acct not found
			const char* acct_pos = p;
			while (p < CR_pos && *p != '[') { p++; }
			ASSERT_SEG_LENGTH(acct);

			// data
			if (*p != '[') { assert(0); break; } // [ of [data] not found.
			const char* data_pos = p;
			while (p < CR_pos && *p != ']') { p++; }
			if (*p != ']') { assert(0); break; } // ] of [data] not found.
			int ademco_cmd_len = ++p - data_pos;
			if (!is_same_id(_id, AID_NULL) && !_ademco_data.Parse(data_pos, ademco_cmd_len)) {
				JLOGA("parse data failed!\n"); assert(0); break;
			}

			// [x...data...]
			if (*p == '[') { // xdata exists
				p++; // skip [ 
				int xdata_len = 0;
				bool b_advanced_gprs_machine = true;

				if (_ademco_data._ademco_event == EVENT_RETRIEVE_ZONE_OR_SUB_MACHINE) {
					for (size_t i = 0; i < 4; i++) {
						if (!isalpha(*(p + i)) && !isdigit(*(p + i))) {
							b_advanced_gprs_machine = false;
							break;
						}
					}
				}

				if (b_advanced_gprs_machine) {
					// special condition, use 4 char to represent length.
					xdata_len = HexCharArrayToDec(p, 4);
					//if (xdata_len <= 4 || 96 < xdata_len) {
					//	// max length of a 1704 serial packet is 48. 8 + 2 * 20
					//	assert(0); break;
					//}
					p += 4; // skip len
				} else {
					// normal condition, use 2 hex to represent length
					xdata_len = MAKEWORD(*(p + 1), *p);
					p += 2; // skip len
				}

				const char* xdata_pos = p;
				p += xdata_len;
				if (*p++ != ']' || p >= CR_pos) { assert(0); break; }// skip ]
				auto _xdata_len = xdata_len;
				if (_xdata == nullptr) {
					_xdata = std::make_shared<char_array>();
				}
				if (b_advanced_gprs_machine) {
					auto tmp = std::unique_ptr<char[]>(new char[_xdata_len]);
					ConvertHiLoAsciiToAscii(tmp.get(), xdata_pos, _xdata_len);
					std::copy(tmp.get(), tmp.get() + _xdata_len / 2, std::back_inserter(*_xdata));
				} else {
					std::copy(xdata_pos, xdata_pos + _xdata_len, std::back_inserter(*_xdata));
				}
			}

			// timestamp, ademco format is _23:59:59,12-31-2000, so its len is 20.
			static const int TIMESTAMP_LEN = 20;
			if (*p != '_') { assert(0); break; } // _ of _timestamp not found.
			if (!_timestamp.Parse(p, 1 + TIMESTAMP_LEN)) {
				JLOGA("parse timestamp failed!\n"); assert(0); break;
			}
			p += TIMESTAMP_LEN;

			// check CR
			if (p++ != CR_pos) { assert(0); break; }

			cbCommited = p - pack;
			return RESULT_OK;
		} while (0);
	} catch (const wchar_t* err) {
		JLOGW(err);
		assert(0);
	} catch (const char* err) {
		JLOGA(err);
		assert(0);
	} catch (...) {
		JLOGA("unhandled error on AdemcoPacket::Parse");
		assert(0);
	}
	return RESULT_DATA_ERROR;
}

ParseResult PrivatePacket::ParseAsc(char* pack, size_t pack_len, size_t& cbCommited, size_t& cbNewLength)
{
	try {
		do {
			if (pack_len < 4) {
				JLOGA("RESULT_NOT_ENOUGH, pack_len %zu\n", pack_len);
				return RESULT_NOT_ENOUGH;
			}

			const char* head_pos = pack;
			// read private cmd
			//LOGASC(head_pos, 4);
			//JLOGB(head_pos, 4);
			size_t len = HexCharArrayToDec(head_pos, 4);
			//int len = MAKEWORD(*(char*)(head_pos + 1),
			//				   *(char*)(head_pos));
			size_t lenToParse = 4 + len + 4; // first 4 for len, sencond 4 for private CRC
			if (lenToParse > pack_len) {
				JLOGA("RESULT_NOT_ENOUGH, lenToParse %zu, pack_len %zu\n", lenToParse, pack_len);
				return RESULT_NOT_ENOUGH;
			}

			char cmd[256] = { 0 };
			ConvertHiLoAsciiToAscii(cmd + 4, pack + 4, len);

			int crc = HexCharArrayToDec(pack + lenToParse - 4, 4);
			int my_crc = CalculateCRC(pack + 4, len);
			if (crc != my_crc) {
				JLOGA(("CalculateCRC PrivateProtocal Error, crc: %04X, my_crc: %04X\n"), crc, my_crc);
				JLOGASC(pack, pack_len);
				JLOGB(cmd + 4, len / 2);
				assert(0); break;
			}

			const char* pos = cmd + 4;
			int seg_len = 0;
#define COPY_TO_PRIVATE_PACKET_ASC(seg) \
	seg_len = sizeof(seg); \
	memcpy(seg, pos, seg_len); \
	pos += seg_len;

			COPY_TO_PRIVATE_PACKET_ASC(_acct_machine);
			COPY_TO_PRIVATE_PACKET_ASC(_passwd_machine);
			COPY_TO_PRIVATE_PACKET_ASC(_acct);
			_level = *pos++;
			COPY_TO_PRIVATE_PACKET_ASC(_ip_csr);
			COPY_TO_PRIVATE_PACKET_ASC(_port_csr);
			_big_type = *pos++;
			_lit_type = *pos++;

			int cmd_len = cmd + lenToParse - 4 - pos;
			_cmd.clear();
			std::copy(pos, pos + cmd_len, std::back_inserter(_cmd));
			pos += cmd_len;

			COPY_TO_PRIVATE_PACKET_ASC(_crc);
			cbCommited = len + 4 + 4;
			assert(size_t(pos - cmd) == cbCommited);

			cbNewLength = 2 + len / 2 + 4;
			pack[0] = ((len / 2) >> 8) & 0xFF;
			pack[1] = (len / 2) & 0xFF;
			memcpy(pack + 2, cmd + 4, len);
			unsigned short new_crc = CalculateCRC(pack + 2, len / 2);
			Dec2HexCharArray_4(new_crc, pack + 2 + len / 2, false);

			//JLOGB(pack, 2 + len + 4);
			//JLOG("PrivatePacket::Parse() ok\n");
			return RESULT_OK;

		} while (0);
	} catch (const char*e) {
		JLOGA("PrivatePacket::ParseAsc, caught an exception: %s\n", e);
	} catch (...) {
		JLOGA("something bad happened on PrivatePacket::ParseAsc");
	}
	return RESULT_DATA_ERROR;
}

size_t PrivatePacket::GetLength() const
{
	size_t len = 0;
	len += sizeof(_acct_machine);
	len += sizeof(_passwd_machine);
	len += sizeof(_acct);
	len += sizeof(_level);
	len += sizeof(_ip_csr);
	len += sizeof(_port_csr);
	len += sizeof(_big_type);
	len += sizeof(_lit_type);
	len += _cmd.size();
	return len;
}

void PrivatePacket::CopyData(char* dst, size_t length)
{
	char* pos = dst;

	int seg_len = 0;
#define COPY_PrivatePacket(seg) \
	seg_len = sizeof(seg); \
	memcpy(pos, seg, seg_len); \
	pos += seg_len;

	COPY_PrivatePacket(_len);
	COPY_PrivatePacket(_acct_machine);
	COPY_PrivatePacket(_passwd_machine);
	COPY_PrivatePacket(_acct);
	*pos++ = _level;
	COPY_PrivatePacket(_ip_csr);
	COPY_PrivatePacket(_port_csr);
	*pos++ = _big_type;
	*pos++ = _lit_type;

	for (auto c : _cmd) { *pos++ = c; }
	memcpy(pos, _crc, sizeof(_crc));
	pos += sizeof(_crc);

	size_t writed_len = pos - dst;
	assert(length == writed_len);
}

size_t PrivatePacket::Make(char* pack,
						   size_t pack_len,
						   char big_type,
						   char lit_type,
						   const char_array& cmd,
						   const char* acct_machine,
						   const char* passwd_machine,
						   const char* acct_csr,
						   char level
						   )
{
	if (acct_machine) memcpy(_acct_machine, acct_machine, sizeof(_acct_machine));
	else memset(_acct_machine, 0xff, sizeof(_acct_machine));

	if (passwd_machine) memcpy(_passwd_machine, passwd_machine, sizeof(_passwd_machine));
	else memset(_passwd_machine, 0xff, sizeof(_passwd_machine));

	if (acct_csr) memcpy(_acct, acct_csr, sizeof(_acct));
	else memset(_acct, 0xff, sizeof(_acct));

	_level = level;
	memset(_ip_csr, 0xff, sizeof(_ip_csr));
	memset(_port_csr, 0xff, sizeof(_port_csr));
	_big_type = big_type;
	_lit_type = lit_type;
	_cmd.clear();
	for (auto c : cmd) { _cmd.push_back(c); }
	size_t length = GetLength(); assert(length < pack_len);
	_len[0] = (length >> 8) & 0xff;
	_len[1] = length & 0xff;

	unsigned short crc = 0;
	crc = CalculateCRC(_acct_machine, sizeof(_acct_machine));
	crc = CalculateCRC(_passwd_machine, sizeof(_passwd_machine), crc);
	crc = CalculateCRC(_acct, sizeof(_acct), crc);
	crc = CalculateCRC(&_level, sizeof(_level), crc);
	crc = CalculateCRC(_ip_csr, sizeof(_ip_csr), crc);
	crc = CalculateCRC(_port_csr, sizeof(_port_csr), crc);
	crc = CalculateCRC(&_big_type, sizeof(_big_type), crc);
	crc = CalculateCRC(&_lit_type, sizeof(_lit_type), crc);

	for (auto c : cmd) { crc = CalculateCRC_char(c, crc); }
	Dec2HexCharArray_4(crc, _crc, false);

	assert(2 + length + 4 < pack_len);
	CopyData(pack, 2 + length + 4);

	return 2 + length + 4;
}

size_t PrivatePacket::MakeAsc(char* pack,
							  size_t pack_len,
							  char big_type,
							  char lit_type,
							  const char_array& cmd,
							  const char* acct_machine,
							  const char* passwd_machine,
							  const char* acct_csr,
							  char level)
{
	char buff[1024] = { 0 };
	size_t len = Make(buff, 1024, big_type, lit_type, cmd, acct_machine, passwd_machine, acct_csr, level);
	if (len * 2 > pack_len) { return 0; }

	for (size_t i = 2; i < len - 4; i++) {
		pack[i * 2 + 0] = Dec2Hex((buff[i] >> 4) & 0x0F);
		pack[i * 2 + 1] = Dec2Hex(buff[i] & 0x0F);
	}

	size_t new_len = (len - 6) * 2;
	Dec2HexCharArray_4(new_len, pack);
	Dec2HexCharArray_4(CalculateCRC(pack + 4, new_len), &pack[4 + new_len], false);

	return 4 + new_len + 4;
}

ParseResult PrivatePacket::Parse(const char* pack, size_t pack_len, size_t& cbCommited)
{
	try {
		do {
			const char* head_pos = pack;

			// read private cmd
			int len = MAKEWORD(*(char*)(head_pos + 1), *(char*)(head_pos));
			size_t lenToParse = 2 + len + 4; // 4 for private CRC
			if (lenToParse < 4 || pack_len < lenToParse)
				return RESULT_NOT_ENOUGH;

			int crc = HexCharArrayToDec(pack + lenToParse - 4, 4);

			if (crc != CalculateCRC(head_pos + 2, len)) {
				JLOGA("CalculateCRC PrivateProtocal Error\n");
				assert(0); break;
			}

			const char* pos = head_pos + 2;
			int seg_len = 0;
#define COPY_TO_PRIVATE_PACKET(seg) \
	seg_len = sizeof(seg); \
	memcpy(seg, pos, seg_len); \
	pos += seg_len;

			COPY_TO_PRIVATE_PACKET(_acct_machine);
			COPY_TO_PRIVATE_PACKET(_passwd_machine);
			COPY_TO_PRIVATE_PACKET(_acct);
			_level = *pos++;
			COPY_TO_PRIVATE_PACKET(_ip_csr);
			COPY_TO_PRIVATE_PACKET(_port_csr);
			_big_type = *pos++;
			_lit_type = *pos++;

			size_t cmd_len = pack + lenToParse - 4 - pos;

			if (cmd_len < 0 || cmd_len > lenToParse) {
				break;
			}

			_cmd.clear();
			std::copy(pos, pos + cmd_len, std::back_inserter(_cmd));
			pos += cmd_len;

			COPY_TO_PRIVATE_PACKET(_crc);
			assert(pos - pack == len + 2 + 4);

			cbCommited = len + 2 + 4;
			return RESULT_OK;

		} while (0);

	} catch (...) {
		JLOG(L"something bad happened on PrivatePacket::Parse");
		return RESULT_DATA_ERROR;
	}

	return RESULT_DATA_ERROR;
}

bool congwin_fe100_packet::from_ademco_data_segment(const AdemcoDataSegment * data)
{
	assert(data && data->_valid);
	if (!data || !data->_valid) {
		return false;
	}

	int acct = data->_ademco_id % 10000;
	auto evnt = data->_ademco_event;
	//int gg = data->_gg;
	int zone = data->_zone;

	int ndx = 0;
	data_[ndx++] = 0x0A; // LF
	data_[ndx++] = 0x20;

	data_[ndx++] = 0x30; // RG
	data_[ndx++] = 0x30;
	data_[ndx++] = 0x20;

	sprintf(data_ + ndx, "%04d", acct); // acct
	ndx += 4;
	data_[ndx++] = 0x20;

	data_[ndx++] = 0x31; // 18
	data_[ndx++] = 0x38;
	data_[ndx++] = 0x20;

	bool status_evnt = IsStatusEvent(evnt);
	if (status_evnt) {
		//data_[ndx++] = 'E';
	}

	if (evnt / 1000 == 1) {
		data_[ndx++] = 'E';
	} else {
		if (evnt == 3456) {
			data_[ndx++] = 'E';
		} else {
			data_[ndx++] = 'R';
		}
	}

	sprintf(data_ + ndx, "%03d", evnt % 1000); // event
	ndx += 3;
	data_[ndx++] = 0x20;

	data_[ndx++] = 0x30; // gg
	data_[ndx++] = 0x30;
	data_[ndx++] = 0x20;

	//data_[ndx++] = 0x43; // FCCC, F is always 'C' for zone, 'U' for user is never used.
	if (status_evnt) {
		data_[ndx++] = 0x55;
	} else {
		data_[ndx++] = 0x43;
	}

	sprintf(data_ + ndx, "%03d", zone);
	ndx += 3;

	data_[ndx++] = 0x20;
	data_[ndx++] = 0x0D;

	return true;
}
};


