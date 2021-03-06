#include "StdAfx.h"

//#if defined(VC_EXTRALEAN)
//#else
//#include "const.h"
//#endif

#include <assert.h>
#include <sstream>
#include <iomanip>
#include "ademco_func.h"

namespace ademco
{
unsigned short CalculateCRC_char(char c, unsigned short crc)
{
	static unsigned short crcTable[] = {
		/* DEFINE THE FIRST ORDER POLYINOMIAL TABLE */
		0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
		0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
		0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
		0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
		0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
		0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
		0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
		0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
		0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
		0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
		0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
		0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
		0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
		0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
		0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
		0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
		0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
		0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
		0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
		0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
		0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
		0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
		0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
		0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
		0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
		0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
		0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
		0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
		0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
		0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
		0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
		0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
	};

	unsigned short CRC = crc;
	CRC = (CRC >> 8) ^ (crcTable[(unsigned char)c ^ (CRC & 0xff)]);
	return CRC;
}

unsigned short CalculateCRC(const char* buff, int len, unsigned short crc)
{
	unsigned short CRC = crc;
	for (int i = 0; i < len; ++i) {
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

int HexCharArrayToDec(const char *hex, int len)
{
	int dec = 0;
	for (int i = 0; i < len; i++) {
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
		return d + '0';
	} else if (0x0A <= d && d <= 0x0F) {
		return d - 0x0A + 'A';
	} else {
		char log[128] = { 0 };
		sprintf_s(log, "Dec2Hex: not a 0-f value. (%c) (%d)", d, d);
		assert(0);
		JLOGA(log);
		return 0;
	}
}

int NumStr2Dec(const char* str, int str_len)
{
	int dec = 0;
	for (int i = 0; i < str_len; i++) {
		dec = dec * 10 + HexChar2Dec(str[i]);
	}
	return dec;
}

const char* HexCharArrayToStr(const char* hex, int len, unsigned char mask /* = (char)0x0f*/)
{
	static char ret[64];
	char tmp[8];
	memset(ret, 0, sizeof(ret));
	memset(tmp, 0, sizeof(tmp));
	unsigned char high = 0, low = 0;
	for (int i = 0; i < len; i++) {
		high = (hex[i] >> 4) & mask;
		if (high == mask)
			break;
		sprintf_s(tmp, "%d", high);
		strcat_s(ret, 64, tmp);

		low = hex[i] & mask;
		if (low == mask)
			break;
		sprintf_s(tmp, "%d", low);
		strcat_s(ret, 64, tmp);
	}
	return ret;
}

const char* HexCharArrayToStr(char* dst, const char* hex, int len,
							  unsigned char mask/* = (char)0x0f*/)
{
	memset(dst, 0, len * 2);
	char tmp[8] = { 0 };
	unsigned char high = 0, low = 0;

	for (int i = 0; i < len; i++) {
		high = (hex[i] >> 4) & mask;
		if (high == mask)
			break;
		sprintf_s(tmp, "%d", high);

		strcat_s(dst, len * 2 + 1, tmp);

		low = hex[i] & mask;
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
		strcpy_s(tmp, "0000");
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

void NumStr2HexCharArray_N(const char* str, char* hexarr, int max_hex_len/* = 9*/)
{
	if (str == nullptr) {
		JLOGA("NumStr2HexCharArray_N: memory access denied.");
		return;
	}
	int len = strlen(str);
	if (len > max_hex_len * 2) {
		JLOGA("NumStr2HexCharArray_N: length too long.");
		return;
	}
	int i = 0;
	for (i = 0; i < len; i++) {
		if (!isdigit(str[i])) {
			JLOGA("NumStr2HexCharArray_N: not all character is digit.");
			return;
		}
	}
	const unsigned int full_str_len = max_hex_len * 2;
	//char *full_str = new char[full_str_len + 1];
	char full_str[32] = { 0 };
	memset(full_str, 0, sizeof(full_str));
	strcpy_s(full_str, str);
	while (strlen(full_str) < full_str_len)
		strcat_s(full_str, 32, "f");
	for (i = 0; i < max_hex_len; i++) {
		char ch = HexChar2Dec(full_str[i * 2]) & 0x0f;
		ch <<= 4;
		ch |= HexChar2Dec(full_str[i * 2 + 1]) & 0x0f;
		hexarr[i] = ch;
	}
	//SAFEDELETEARR(full_str);
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
	//char *full_str = new char[full_str_len + 1];
	//char full_str[32] = { 0 };
	memset(dst, 0, dst_len);
	//strcpy(full_str, str);
	//while (strlen(full_str) < full_str_len)
	//	strcat(full_str, "f");
	for (size_t i = 0; i < dst_len; i++) {
		char ch = HexChar2Dec(src[i * 2]) & 0x0f;
		ch <<= 4;
		ch |= HexChar2Dec(src[i * 2 + 1]) & 0x0f;
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
	//_data.reserve(str.length());
	//_data.assign(str.begin(), str.end());
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	//sprintf(&_data[2], "%06X", ademco_id);
	_data.push_back('|');
	//_data[7] = '1';
	//_data[8] = '8';
	//_data[7] = ' ';
	//data[10] = IsCloseEvent(event) ? '3' : '1';
	ss.str(""); ss.clear();
	ss << std::setfill('0') << std::setw(4) << std::dec << ademco_event;
	str = ss.str();
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	//std::copy(std::istream_iterator<char>(ss), std::istream_iterator<char>(), std::back_inserter(_data));
	//sprintf(&_data[9], "%04d", ademco_event);
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
	//_data[12] = Dec2Hex((gg & 0xF0) >> 4);
	//_data[13] = Dec2Hex((gg & 0x0F));
	_data.push_back(' ');
	ss.str(""); ss.clear();
	ss << std::setfill('0') << std::setw(3) << std::dec << zone;
	str = ss.str();
	std::copy(str.begin(), str.end(), std::back_inserter(_data));
	//std::copy(std::begin(ss), std::end(ss), std::back_inserter(_data));
	//sprintf(&_data[17], "%03d", zone);
	_data.push_back(']');
	_len = 21;
	_valid = true;
}

bool AdemcoDataSegment::Parse(const char* pack, unsigned int pack_len)
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
			//_ademco_id = NumStr2Dec(p, 4);
			_ademco_id = HexCharArrayToDec(p, 4);
			_len += 4;
			p += 5;
		} else if (acct_len == 6) {
			if (*(p + 6) != '|')
				break;
			//_ademco_id = NumStr2Dec(p, 6);
			_ademco_id = HexCharArrayToDec(p, 6);
			_len += 6;
			p += 7;
		}

		//_mt = static_cast<unsigned char>(NumStr2Dec(p, 2));
		//p += 2;
		//if (*p++ != ' ')
		//	break;
		//data.q = NumStr2Dec(p++, 1);
		_ademco_event = static_cast<ADEMCO_EVENT>(NumStr2Dec(p, 4));
		p += 4;
		if (*p++ != ' ')
			break;
		//unsigned char gg_h = (HexChar2Dec(*p) & 0x0F);
		//unsigned char gg_l = (HexChar2Dec(*(p + 1)) & 0x0F);
		if (*p == 'E' && *(p + 1) == 'E') {
			_gg = 0xEE;
		} else if (*p == 'C' && *(p + 1) == 'C') {
			_gg = 0xCC;
		} else {
			_gg = (*p - '0') * 10 + (*(p + 1) - '0');
		}
		//_gg |= (HexChar2Dec(*(p + 1)) & 0x0F);
		p += 2;
		if (*p++ != ' ')
			break;
		_zone = NumStr2Dec(p, 3);
		_valid = true;
		_len = pack_len - 2;
		//_data._Construct(pack + 1, pack + 1 + _len);
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

bool AdemcoTimeStamp::Parse(const char* pack, unsigned int pack_len)
{
	assert(pack_len < sizeof(_data));
	//memset(this, 0, sizeof(AdemcoTimeStamp));
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
	//#ifdef _DEBUG
	//		wchar_t wtime[32] = { 0 };
	//		localtime_s(&tmtm, &_time);
	//		wcsftime(wtime, 32, L"%Y-%m-%d %H:%M:%S", &tmtm);
	//		LOGW(L"AdemcoTimeStamp::Parse result: %s\n", wtime);
	//#endif
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

	//size_t seg_len = 0; //  memcpy(pos, &SEG[0], seg_len); 
//#define COPYAdemcoPacket(SEG) \
//	seg_len = SEG.size(); \
//	std::copy(SEG.begin(), SEG.end(), pos); \ 
//	pos += seg_len;

#define COPY_FROM_ARRAY_TO_P(a, p) for (auto c : a) { *p++ = c; }
		//for (auto c : _id) { *pos++ = c; }
	COPY_FROM_ARRAY_TO_P(_id, pos);
	COPY_FROM_ARRAY_TO_P(_seq, pos);
	COPY_FROM_ARRAY_TO_P(_rrcvr, pos);
	COPY_FROM_ARRAY_TO_P(_lpref, pos);
	COPY_FROM_ARRAY_TO_P(_acct, pos);
	//COPY_FROM_ARRAY_TO_P(_ademco_data._data, pos);
	for (auto c : _ademco_data._data) {
		*pos++ = c;
	}
	//COPY_FROM_ARRAY_TO_P(_xdata, pos);
	if (_xdata) {
		for (auto c : *_xdata) {
			*pos++ = c;
		}
	}

	//std::copy(_id.begin(), _id.end(), pos); pos += _id.size();
	//std::copy(_seq.begin(), _seq.end(), pos); pos += _seq.size();
	//std::copy(_rrcvr.begin(), _rrcvr.end(), pos); pos += _rrcvr.size();
	//std::copy(_lpref.begin(), _lpref.end(), pos); pos += _lpref.size();
	//std::copy(_acct.begin(), _acct.end(), pos); pos += _acct.size();
	//auto data = _data._data;
	//std::copy(data.begin(), data.end(), pos); pos += data.size();
	//std::copy(_xdata.begin(), _xdata.end(), pos); pos += _xdata.size();

	memcpy(pos, _timestamp._data, _timestamp._len);
	pos += _timestamp._len;

	*pos++ = _CR;
	assert(pos - dst == (int)length);

	int ademco_len = length - 9 - 1;
	Dec2HexCharArray_4(CalculateCRC(id_pos, ademco_len), crc_pos, false);
	Dec2HexCharArray_4(ademco_len, len_pos);
}

size_t AdemcoPacket::Make(char* pack, size_t pack_len, const char* id,
						  int seq, char const* acct, int ademco_id,
						  ADEMCO_EVENT ademco_event, int gg, int zone,
						  const char_array_ptr& xdata)
{
	assert(pack); assert(id); //assert(acct);
	Clear();

	//_id._Construct(id, id + strlen(id));
	std::copy(id, id + strlen(id), std::back_inserter(_id));
	char tmp[5];
	sprintf_s(tmp, "%04d", seq);
	//_seq._Construct(tmp, tmp + 4);
	std::copy(tmp, tmp + 4, std::back_inserter(_seq));
	std::copy(RRCVR, RRCVR + strlen(RRCVR), std::back_inserter(_rrcvr));
	//_lpref._Construct(LPREF, LPREF + strlen(LPREF));
	std::copy(LPREF, LPREF + strlen(LPREF), std::back_inserter(_lpref));

	//if (_acct != acct) { // 2015-3-10 18:42:44 prevent to copy itself
	//	sprintf_s(_acct, "#%s", acct);
	//} else {
	//	char tmp[64] = { 0 };
	//	memcpy(tmp, _acct, sizeof(_acct));
	//	sprintf_s(_acct, "#%s", tmp);
	//}
	if (acct) {
		//_acct[0] = '#';
		//memcpy(_acct+1, acct, strlen(acct));
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
		_ademco_data.Make(); _xdata = nullptr;
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
		} else { _xdata = nullptr; }
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
			//_crc._Construct(pack + 1, pack + 1 + 4);
			std::copy(pack + 1, pack + 1 + 4, std::back_inserter(_crc));
			//_len._Construct(pack + 5, pack + 5 + 4);
			std::copy(pack + 5, pack + 5 + 4, std::back_inserter(_len));
			int ademco_crc = HexCharArrayToDec(pack + 1, 4);
			int ademco_len = HexCharArrayToDec(pack + 5, 4);

			// read till CR
			DWORD dwLenToParse = 9 + ademco_len + 1; // 1 for CR
			size_t seg_len = 0;
#define ASSERT_SEG_LENGTH(seg) seg_len = p - seg##_pos; \
	std::copy(seg##_pos, seg##_pos + seg_len, std::back_inserter(_##seg));
			//_##seg._Construct(seg##_pos, seg##_pos + seg_len);

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
			//seg_len = p - id_pos;
			ASSERT_SEG_LENGTH(id);
			//strncpy_s(_id, id_pos, seg_len); // copy id to _id

			// seq (and Rrcvr, it may not exists)
			const char* seq_pos = p;
			while (p < CR_pos && *p != 'R' && *p != 'L') { p++; }
			ASSERT_SEG_LENGTH(seq);
			if (*p == 'R') { // Rrcvr exists
				const char* rrcvr_pos = p;
				while (p < CR_pos && *p != 'L' && *p != '#') { p++; }
				ASSERT_SEG_LENGTH(rrcvr);
			} else if (*p == 'L') { // Rrcvr not exists, pass
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
				//while (p < CR_pos && *p != ']') { p++; }
				//if (*p != ']') { assert(0); break; } // ] of [xdata] not found.
				auto _xdata_len = xdata_len;
				//_xdata._Construct(xdata_pos, xdata_pos + _xdata_len);
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
			//int my_crc = CalculateCRC(cmd + 4, len / 2);
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
			//memcpy(pack + 2 + len / 2, pack + lenToParse - 4, 4);
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

	//memcpy(pos, _cmd._data, _cmd._size);
	//pos += _cmd._size;
	//std::copy(_cmd._data.begin(), _cmd._data.end(), pos); pos += _cmd._data.size();
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
	if (acct_machine)
		memcpy(_acct_machine, acct_machine, sizeof(_acct_machine));
	else
		memset(_acct_machine, 0xff, sizeof(_acct_machine));

	if (passwd_machine)
		memcpy(_passwd_machine, passwd_machine, sizeof(_passwd_machine));
	else
		memset(_passwd_machine, 0xff, sizeof(_passwd_machine));

	if (acct_csr) {
		//NumStr2HexCharArray_N(acct_csr, _acct);
		memcpy(_acct, acct_csr, sizeof(_acct));
	} else
		memset(_acct, 0xff, sizeof(_acct));

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

	//assert(_cmd.size() < 128);
	//char a[128];
	//std::copy(_cmd._data.begin(), _cmd._data.end(), a);
	//crc = CalculateCRC(a, _cmd._data.size(), crc);
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
			//_cmd.Assign(pos, cmd_len);
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

	sprintf_s(data_ + ndx, 5, "%04d", acct); // acct
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
	sprintf_s(data_ + ndx, 4, "%03d", evnt % 1000); // event
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
	sprintf_s(data_ + ndx, 4, "%03d", zone);
	ndx += 3;
	data_[ndx++] = 0x20;

	data_[ndx++] = 0x0D;

	return true;
}
};


