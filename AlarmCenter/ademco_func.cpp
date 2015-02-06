#include "StdAfx.h"
#include <assert.h>
#include "ademco_func.h"

namespace ademco
{
	unsigned short CalculateCRC(const char* buff, int len, unsigned short crc)
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
		for (int i = 0; i < len; ++i) {
			CRC = (CRC >> 8) ^ (crcTable[(unsigned char)buff[i] ^ (CRC & 0xff)]);
		}
		return CRC;
	}

	const char* GetAdemcoEventString(int ademco_event)
	{
		switch (ademco_event) {
			case EVENT_ARM:			return "ARM";		break;
			case EVENT_BURGLAR:		return "BURGLAR";	break;
			case EVENT_DISARM:		return "DISARM";	break;
			case EVENT_DURESS:		return "DURESS";	break;
			case EVENT_EMERGENCY:		return "EMERGENCY";	break;
			case EVENT_FIRE:			return "FIRE";		break;
			case EVENT_GAS:			return "GAS";		break;
			case EVENT_HALFARM:		return "HALFARM";	break;
			case EVENT_TEMPER:		return "TEMPER";	break;
			case EVENT_WATER:			return "WATER";		break;
			case EVENT_LOWBATTERY:	return "LOWBATTERY";	break;
			case EVENT_BADBATTERY:	return "BADBATTERY";	break;
			case EVENT_SOLARDISTURB:	return "SOLARDISTURB";	break;
			case EVENT_DISCONNECT:	return "DISCONNECT";		break;
			case EVENT_RECONNECT:	return "RECONNECT";		break;
			case EVENT_SERIAL485DIS:	return "485DIS";	break;
			case EVENT_SERIAL485CONN:	return "485CONN";	break;
			case EVENT_DOORRINGING:	return "DOORRINGING";	break;
			default:			return "null";		break;
		}
	}

	int HexCharArrayToDec(const char *hex, int len)
	{
		if (IsBadReadPtr(hex, len))
			throw _T("HexCharArrayToDec: memory access denied.");
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
			TCHAR log[128] = { 0 };
			_stprintf_s(log, _T("HexChar2Dec: not a hex char. (%c) (%d)"), hex, hex);
			ASSERT(0);
			throw log;
		}
	}

	int NumStr2Dec(const char* str, int str_len)
	{
		if (IsBadReadPtr(str, str_len))
			throw _T("NumStr2Dec: memory access denied.");
		int dec = 0;
		for (int i = 0; i < str_len; i++) {
			dec = dec * 10 + HexChar2Dec(str[i]);
		}
		return dec;
	}

	const char* HexCharArrayToStr(const char* hex, int len, unsigned char mask /* = (char)0x0f*/)
	{
		if (IsBadReadPtr(hex, len))
			throw _T("HexCharArrayToStr: memory access denied.");
		static char ret[64];
		char tmp[8];
		memset(ret, 0, sizeof(ret));
		memset(tmp, 0, sizeof(tmp));
		unsigned char high = 0, low = 0;
		for (int i = 0; i < len; i++) {
			high = (hex[i] >> 4) & mask;
			if (high == mask)
				break;
			_snprintf_s(tmp, 8, 1, "%d", high);
			strcat_s(ret, 64, tmp);

			low = hex[i] & mask;
			if (low == mask)
				break;
			_snprintf_s(tmp, 8, 1, "%d", low);
			strcat_s(ret, 64, tmp);
		}
		return ret;
	}

	const char* HexCharArrayToStr(char* dst, const char* hex, int len,
												unsigned char mask/* = (char)0x0f*/)
	{
		if (IsBadReadPtr(hex, len))
			throw _T("HexCharArrayToStr: memory access denied.");
		if (IsBadWritePtr(dst, len * 2))
			throw _T("HexCharArrayToStr: memory access denied.");
		memset(dst, 0, len * 2);
		char tmp[8] = { 0 };
		unsigned char high = 0, low = 0;
		//int value = 0;
		for (int i = 0; i < len; i++) {
			high = (hex[i] >> 4) & mask;
			if (high == mask)
				break;
			_snprintf_s(tmp, 8, 2, "%d", high);
			//value = value * 10 + high;

			strcat_s(dst, len * 2 + 1, tmp);

			low = hex[i] & mask;
			if (low == mask)
				break;
			_snprintf_s(tmp, 8, 2, "%d", low);
			strcat_s(dst, len * 2 + 1, tmp);
			//value = value * 10 + low;
		}
		//_snprintf_s(dst, len * 2, len * 2 - 1, "%d", value);
		return dst;
	}

	void Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF)
	{
		if (IsBadWritePtr(hex, 4))
			throw _T("Dec2HexCharArray_4: memory access denied.");
		if (dec < 0) {
			throw _T("0LLL can't be negative.");
		}
		if (dec == 0) {
			char tmp[8] = { 0 };
			//_snprintf_s(tmp, 5, 4, "0LLL");
			strcpy_s(tmp, 5, "0LLL");
			memcpy(hex, tmp, 4);
			//strcpy_s(hex, 5, "0LLL");
			return;
		}
		if (bMax0FFF && dec > 0x0fff) {
			throw _T("0LLL is bigger than 0x0fff.");
		}
		char tmp[8] = { 0 };
		_snprintf_s(tmp, 5, 4, "%04X", dec);
		memcpy(hex, tmp, 4);
	}

	void NumStr2HexCharArray_N(const char* str, char* hexarr, int max_hex_len/* = 9*/)
	{
		if (str == NULL || IsBadWritePtr(hexarr, max_hex_len))
			throw _T("NumStr2HexCharArray_N: memory access denied.");
		int len = strlen(str);
		if (len > max_hex_len * 2)
			throw _T("NumStr2HexCharArray_N: length too long.");
		int i = 0;
		for (i = 0; i < len; i++) {
			if (!isdigit(str[i]))
				throw _T("NumStr2HexCharArray_N: not all character is digit.");
		}
		const unsigned int full_str_len = max_hex_len * 2;
		//char *full_str = new char[full_str_len + 1];
		char full_str[32] = { 0 };
		memset(full_str, 0, sizeof(full_str));
		strcpy_s(full_str, 32, str);
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

	void AdemcoDataSegment::Make(int ademco_id, int ademco_event, int zone)
	{
		memset(_data, 0, sizeof(_data));
		_data[0] = '[';
		_data[1] = '#';
		_snprintf_s(&_data[2], 5, 4, "%04d", ademco_id);
		_data[6] = '|';
		_data[7] = '1';
		_data[8] = '8';
		_data[9] = ' ';
		//data[10] = IsCloseEvent(event) ? '3' : '1';
		_snprintf_s(&_data[10], 5, 4, "%04d", ademco_event);
		_data[14] = ' ';
		_data[15] = '0';
		_data[16] = '0';
		_data[17] = ' ';
		_snprintf_s(&_data[18], 4, 3, "%03d", zone);
		_data[21] = ']';
		_data[22] = 0;
		_len = 22;
	}

	bool AdemcoDataSegment::Parse(const char* pack, unsigned int pack_len)
	{
		if (IsBadReadPtr(pack, pack_len))
			throw _T("ParseAdmCid: memory access denied.");
		memset(this, 0, sizeof(AdemcoDataSegment));
		const char* p = pack;
		do {
			if (*p++ != '[')
				break;
			if (pack_len == 2 && *p == ']')
				return true;
			//                   [   #  acct |   mt  s   q event s   gg  s   zone ] 22
			else if (pack_len != 1 + 1 + 4 + 1 + 2 + 1 + 1 + 3 + 1 + 2 + 1 + 3 + 1)
				break;

			if (*p++ != '#')
				break;
			if (*(p + 4) != '|')
				break;
			_ademco_id = NumStr2Dec(p, 4);
			_len += 4;
			p += 5;
			_mt = static_cast<unsigned char>(NumStr2Dec(p, 2));
			p += 2;
			if (*p++ != ' ')
				break;
			//data.q = NumStr2Dec(p++, 1);
			_ademco_event = NumStr2Dec(p, 4);
			p += 4;
			if (*p++ != ' ')
				break;
			_gg = static_cast<unsigned char>(NumStr2Dec(p, 2));
			p += 2;
			if (*p++ != ' ')
				break;
			_zone = NumStr2Dec(p, 3);
			_valid = true;
			return true;
		} while (0);
		memset(this, 0, sizeof(AdemcoDataSegment));
		return false;
	}

	void AdemcoTimeStamp::Make()
	{
		_time = time(NULL);
		struct tm tmtm;
		localtime_s(&tmtm, &_time);
		_snprintf_s(_data, 21, "_%02d:%02d:%02d,%02d-%02d-%04d",
					tmtm.tm_hour, tmtm.tm_min, tmtm.tm_sec,
					tmtm.tm_mday, tmtm.tm_mon + 1, tmtm.tm_year + 1900);
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
							  &tmtm.tm_mday, &tmtm.tm_mon, &tmtm.tm_year);
		assert(ret == 6);
		_len = pack_len;
		tmtm.tm_year -= 1900;
		tmtm.tm_mon--;
		tmtm.tm_isdst = -1;
		_time = mktime(&tmtm);
		return true;
	}

	size_t AdemcoPacket::GetLength() const
	{ //      LF  CRC LEN               SEQ  
		return 1 + 4 + 4 + strlen(_id) + 4 + strlen(_rrcvr) + strlen(_lpref)
			+ strlen(_acct) + _data._len + _timestamp._len + 1; // CR
	}

	void AdemcoPacket::CopyData(char* dst, size_t length)
	{
		ASSERT(length == GetLength());

		char* pos = dst;
		*pos++ = _LF;

		char* crc_pos = pos; pos += 4;
		char* len_pos = pos; pos += 4;
		char* id_pos = pos;

		size_t seg_len = 0;
#define COPYAdemcoPacket(SEG) \
	seg_len = strlen(SEG); \
	memcpy(pos, SEG, seg_len); \
	pos += seg_len;

		COPYAdemcoPacket(_id); 
		COPYAdemcoPacket(_seq); 
		COPYAdemcoPacket(_rrcvr); 
		COPYAdemcoPacket(_lpref); 
		COPYAdemcoPacket(_acct);

		memcpy(pos, _data._data, _data._len);
		pos += _data._len;

		COPYAdemcoPacket(_xdata);

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
							  int ademco_event, int zone, const char* xdata)
	{
		assert(pack); assert(id); assert(acct);

		Clear();

		strcpy_s(_id, id);
		sprintf_s(_seq, "%04d", seq);
		strcpy_s(_rrcvr, RRCVR);
		strcpy_s(_lpref, LPREF);
		sprintf_s(_acct, "#%s", acct);

		if (is_null_data(id)) {
			_data.Make();
		} else {
			_data.Make(ademco_id, ademco_event, zone);
			if (xdata) { strcpy_s(_xdata, xdata); }
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
				if (pack[0] != _LF) { ASSERT(0); break; }

				// read crc & len
				strncpy_s(_crc, pack + 1, 4);
				strncpy_s(_len, pack + 5, 4);
				int ademco_crc = HexCharArrayToDec(_crc, 4);
				int ademco_len = HexCharArrayToDec(_len, 4);

				// read till CR
				DWORD dwLenToParse = 9 + ademco_len + 1; // 1 for CR

				// check if packet is enough to parse
				if (pack_len < dwLenToParse)
					return RESULT_NOT_ENOUGH;

				// check CR
				const char* id_pos = pack + 9;
				const char* CR_pos = id_pos + ademco_len;
				if (*CR_pos != _CR) { ASSERT(0); break; } 

				// check ademco CRC
				unsigned short crc_cal = CalculateCRC(id_pos, ademco_len);
				if (ademco_crc != crc_cal) { LOG(_T("crc failed!\n")); ASSERT(0); break; }

				// id
				if (*id_pos != '\"') { ASSERT(0); break; }	// find first " of "id".
				const char* p = id_pos + 1;					// find last  " of "id".
				while (p < CR_pos && *p != '\"') { p++; }
				if (*p != '\"') { ASSERT(0); break; }		// " not found.
				strncpy_s(_id, id_pos, ++p - id_pos); // copy id to _id

				// seq (and Rrcvr, it may not exists)
				const char* seq_pos = p;
				while (p < CR_pos && *p != 'R' && *p != 'L') { p++; }
				strncpy_s(_seq, seq_pos, p - seq_pos);
				if (*p == 'R') { // Rrcvr exists
					const char* rrcvr_pos = p;
					while (p < CR_pos && *p != 'L' && *p != '#') { p++; }
					strncpy_s(_rrcvr, rrcvr_pos, p - rrcvr_pos);
				} else if (*p == 'L') { // Rrcvr not exists, pass
				} else { ASSERT(0); break; }
	
				// Lpref
				if (*p != 'L') { ASSERT(0); break; } // L of Lpref not found.
				const char* lpref_pos = p;
				while (p < CR_pos && *p != '#') { p++; }
				strncpy_s(_lpref, lpref_pos, p - lpref_pos);

				// acct
				if (*p++ != '#') { ASSERT(0);break; } // # of #acct not found
				const char* acct_pos = p;
				while (p < CR_pos && *p != '[') { p++; }
				strncpy_s(_acct, acct_pos, p - acct_pos);

				// data
				if (*p != '[') { ASSERT(0); break; } // [ of [data] not found.
				const char* data_pos = p;
				while (p < CR_pos && *p != ']') { p++; }
				if (*p != ']') { ASSERT(0); break; } // ] of [data] not found.
				int ademco_cmd_len = ++p - data_pos;
				if (!is_null_data(_id) && !_data.Parse(data_pos, ademco_cmd_len)) {
					LOG(_T("parse data failed!\n")); ASSERT(0); break;
				}

				// [x...data...]
				if (*p == '[') { // xdata exists
					const char* xdata_pos = ++p;	// skip [
					while (p < CR_pos && *p != ']') { p++; }
					if (*p != ']') { ASSERT(0); break; } // ] of [xdata] not found.
					strncpy_s(_xdata, xdata_pos, p++ - xdata_pos); // skip ]
				}

				// timestamp, ademco format is _23:59:59,12-31-2000, so its len is 20.
				static const int TIMESTAMP_LEN = 20;
				if (*p != '_') { ASSERT(0); break; } // _ of _timestamp not found.
				if (!_timestamp.Parse(p, 1 + TIMESTAMP_LEN)) {
					LOG(_T("parse timestamp failed!\n")); ASSERT(0); break;
				}
				p += TIMESTAMP_LEN;

				// check CR
				if (p++ != CR_pos) { ASSERT(0); break; }

				cbCommited = p - pack;
				return RESULT_OK;
			} while (0);
		} catch (wchar_t* err) {
			LOGW(err);
			ASSERT(0);
		} catch (char* err) {
			LOGA(err);
			ASSERT(0);
		}
		return RESULT_DATA_ERROR;
	}

	size_t PrivatePacket::GetLength() const
	{
		//return (sizeof(PrivatePacket) - 2 - sizeof(PrivateCmd) + _cmd._size - 4);
		size_t len = 0;
		len += sizeof(_acct_machine);
		len += sizeof(_passwd_machine);
		len += sizeof(_acct);
		len += sizeof(_level);
		len += sizeof(_ip_csr);
		len += sizeof(_port_csr);
		len += sizeof(_big_type);
		len += sizeof(_lit_type);
		len += _cmd._size;
		return len;
	}

	void PrivatePacket::CopyData(char* dst, size_t length)
	{
		//ASSERT(length == GetLength());

		char* pos = dst;
		//char* acct_machine_pos = pos + 2;
		//char* crc_pos = pos + length - 4;

		int seg_len = 0;
#define COPY_PrivatePacket(seg) \
	seg_len = sizeof(seg); \
	memcpy(pos, seg, seg_len); \
	pos += seg_len;

		COPY_PrivatePacket(_len);
		COPY_PrivatePacket(_acct_machine);
		COPY_PrivatePacket(_passwd_machine);
		COPY_PrivatePacket(_acct);
		//COPY_PrivatePacket(&_level);
		*pos++ = _level;
		COPY_PrivatePacket(_ip_csr);
		COPY_PrivatePacket(_port_csr);
		//COPY_PrivatePacket(&_big_type);
		//COPY_PrivatePacket(&_lit_type);
		*pos++ = _big_type;
		*pos++ = _lit_type;

		memcpy(pos, _cmd._data, _cmd._size);
		pos += _cmd._size;
		memcpy(pos, _crc, sizeof(_crc));
		pos += sizeof(_crc);

		size_t writed_len = pos - dst;
		ASSERT(length == writed_len);
	}

	size_t PrivatePacket::Make(char* pack, size_t pack_len, char big_type, char lit_type,
							   const PrivateCmd& cmd)
	{
		
		memset(_acct_machine, 0xff, sizeof(_acct_machine));
		memset(_passwd_machine, 0xff, sizeof(_passwd_machine));
		memset(_acct, 0xff, sizeof(_acct));
		_level = 0;
		memset(_ip_csr, 0xff, sizeof(_ip_csr));
		memset(_port_csr, 0xff, sizeof(_port_csr));
		_big_type = big_type;
		_lit_type = lit_type;
		_cmd = cmd;
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
		crc = CalculateCRC(_cmd._data, _cmd._size, crc);
		Dec2HexCharArray_4(crc, _crc, false);

		CopyData(pack, 2 + length + 4);

		return 2 + length + 4;
	}

	ParseResult PrivatePacket::Parse(const char* pack, size_t pack_len, size_t& cbCommited)
	{
		do {
			const char* head_pos = pack;
			// read private cmd
			int len = MAKEWORD(*(char*)(head_pos + 1),
							   *(char*)(head_pos));
			size_t lenToParse = 2 + len + 4; // 4 for private CRC
			if (lenToParse > pack_len)
				return RESULT_NOT_ENOUGH;

			int crc = HexCharArrayToDec(pack + lenToParse - 4, 4);

			if (crc != CalculateCRC(head_pos + 2, len)) {
				CLog::WriteLog(_T("CalculateCRC PrivateProtocal Error\n"));
				ASSERT(0); break;
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
			//COPY_TO_PRIVATE_PACKET(&_level);
			_level = *pos++;
			COPY_TO_PRIVATE_PACKET(_ip_csr);
			COPY_TO_PRIVATE_PACKET(_port_csr);
			//COPY_TO_PRIVATE_PACKET(&_big_type);
			//COPY_TO_PRIVATE_PACKET(&_lit_type);
			_big_type = *pos++;
			_lit_type = *pos++;

			int cmd_len = pack + lenToParse - 4 - pos;
			_cmd.Assign(pos, cmd_len);
			pos += cmd_len;

			COPY_TO_PRIVATE_PACKET(_crc);
			ASSERT(pos - pack == len + 2 + 4);

			cbCommited = len + 2 + 4;
			return RESULT_OK;

		} while (0);
		return RESULT_DATA_ERROR;
	}

	//ParseResult ParsePacket(const char* pack, unsigned int pack_len,
	//								  AdemcoPrivateProtocal& app, DWORD *lpBytesCommited,
	//								  BOOL deal_private_cmd/* = FALSE*/)
	//{
	//	try {
	//		if (IsBadReadPtr(pack, pack_len))
	//			throw _T("ParsePacket: memory access denied.");
	//		unsigned int ademco_crc = 0;
	//		unsigned int ademco_len = 0;
	//		unsigned int timestamp_len = 0;
	//		unsigned int private_len = 0;
	//		unsigned int private_CRC = 0;
	//		const char* private_head_pos = NULL;
	//		do {
	//			bool hasPrivateData = false;
	//			if (pack_len < 9)	return RESULT_NOT_ENOUGH;
	//			if (pack[0] != 0x0a) {
	//				ASSERT(0);
	//				break;
	//			}					// check LF
	//			ademco_crc = HexCharArrayToDec(pack + 1, 4);
	//			ademco_len = HexCharArrayToDec(pack + 5, 4);
	//			// read till CR
	//			DWORD dwLenToParse = 9 + ademco_len + 1;		// 1 for CR, 2 for private len.

	//			if (pack_len < dwLenToParse)
	//				return RESULT_NOT_ENOUGH;
	//			else if (pack_len == dwLenToParse)
	//				hasPrivateData = false;
	//			else
	//				hasPrivateData = true;

	//			const char* CR_pos = pack + 9 + ademco_len;
	//			if (*CR_pos != 0x0d) {
	//				ASSERT(0);
	//				break;
	//			}		// check CR
	//			// check ademco CRC
	//			if (ademco_crc != CalculateCRC(pack + 9, ademco_len)) {
	//				OutputDebugString(_T("CalculateCRC ademco Error\n"));
	//				ASSERT(0);
	//				break;
	//			}

	//			if (hasPrivateData && deal_private_cmd) {
	//				private_head_pos = CR_pos + 1;
	//				// read private cmd
	//				private_len = MAKEWORD(*(char*)(private_head_pos + 1),
	//									   *(char*)(private_head_pos));
	//				dwLenToParse += 2 + private_len + 4;			// 4 for private CRC
	//				if (dwLenToParse > pack_len)
	//					return RESULT_NOT_ENOUGH;

	//				// check ademco and private protocal 's CRC
	//				private_CRC = HexCharArrayToDec(pack + dwLenToParse - 4, 4);

	//				if (private_CRC != CalculateCRC(private_head_pos + 2, private_len)) {
	//					CLog::WriteLog(_T("CalculateCRC PrivateProtocal Error\n"));
	//					break;
	//				}
	//			}

	//			// parse ademco
	//			// id
	//			if (pack[9] != '\"') {
	//				ASSERT(0);
	//				break;
	//			}			// find first " of "id".
	//			const char* p = pack + 10;			// find last  " of "id".
	//			while (p < CR_pos && *p != '\"') { p++; }
	//			if (*p != '\"') {
	//				ASSERT(0);
	//				break;
	//			}			// " not found.
	//			p++;								// skip "
	//			app.id = pack + 9;
	//			app.id_len = p - app.id;
	//			bool b_null_data = is_null_data(app.id, app.id_len);

	//			// seq (and Rrcvr, it may not exist)
	//			while (p < CR_pos && *p != 'R' && *p != 'L') { p++; }
	//			app.seq = app.id + app.id_len;
	//			app.seq_len = p - app.seq;
	//			if (*p == 'R') {
	//				app.Rrcvr = p;
	//				while (p < CR_pos && *p != 'L') { p++; }
	//				app.Rrcvr_len = p - app.Rrcvr;
	//			} else if (*p == 'L') {
	//				app.Rrcvr = NULL;
	//				app.Rrcvr_len = 0;
	//			} else {
	//				ASSERT(0);
	//				break;
	//			}
	//			// Lpref
	//			if (*p != 'L') {
	//				ASSERT(0);
	//				break;
	//			}			// L of Lpref not found.
	//			app.Lpref = p;
	//			while (p < CR_pos && *p != '#') { p++; }
	//			app.Lpref_len = p - app.Lpref;
	//			// acct
	//			if (*p++ != '#') {
	//				ASSERT(0);
	//				break;
	//			}			// # of #acct not found
	//			app.acct = p;
	//			while (p < CR_pos && *p != '[') { p++; }
	//			app.acct_len = p - app.acct;
	//			// data
	//			if (*p != '[') {
	//				ASSERT(0);
	//				break;
	//			}			// [ of [data] not found.
	//			const char* ademco_cmd = p;
	//			while (p < CR_pos && *p != ']') { p++; }
	//			p++;				// skip ]
	//			int ademco_cmd_len = p - ademco_cmd;
	//			if (!b_null_data && !app.data.Parse(ademco_cmd, ademco_cmd_len)) {
	//				ASSERT(0);
	//				break;
	//			}
	//			// [x...data...]
	//			if (*p == '[') {
	//				const char* xdata_pos = ++p;	// skip [
	//				app.xdata = xdata_pos;
	//				while (p < CR_pos && *p != ']') { p++; }
	//				int xdata_len = p - xdata_pos;
	//				app.xdata_len = xdata_len;
	//				p++;				// skip ]
	//			}

	//			// timestamp, standard format is _23:59:59,12-23-2000, so its len is 20.
	//			static const int TIMESTAMP_LEN = 20;
	//			if (*p != '_') {
	//				ASSERT(0);
	//				break;
	//			}			// _ of _timestamp not found.
	//			memcpy(app.timestamp, p, TIMESTAMP_LEN);
	//			app.timestamp[TIMESTAMP_LEN] = 0;
	//			timestamp_len = TIMESTAMP_LEN; // a little trick, use time_stamp_len as a counter.
	//			while (p < CR_pos && timestamp_len--) { p++; }
	//			if (timestamp_len != 0) {
	//				ASSERT(0);
	//				break;
	//			}
	//			if (p != CR_pos) {
	//				ASSERT(0);
	//				break;
	//			}


	//			if (hasPrivateData && deal_private_cmd) {
	//				// parse PrivateProtocal
	//				// acct of machine.
	//				p += 3;	// skip CR_pos and 2 bytes for private_len
	//				//strcpy_s(acct_machine, HexCharArrayToStr(p, 9));
	//				HexCharArrayToStr(app.acct_machine, p, 9);
	//				p += 9;
	//				// passwd of machine
	//				//strcpy_s(passwd_machine, HexCharArrayToStr(p, 4));
	//				HexCharArrayToStr(app.passwd_machine, p, 4);
	//				p += 4;
	//				// phone number
	//				//strcpy_s(phone, HexCharArrayToStr(p, 9));
	//				HexCharArrayToStr(app.phone, p, 9);
	//				p += 9;
	//				// alarm level
	//				app.level = (char)*p;
	//				p++;
	//				// ip of center station receiver
	//				_snprintf_s(app.ip_csr, 16, "%d.%d.%d.%d",
	//							*p, *(p + 1), *(p + 2), *(p + 3));
	//				p += 4;
	//				// port of csr
	//				app.port_csr = MAKEWORD(*p, *(p + 1));
	//				p += 2;
	//				// content and content_len
	//				app.private_cmd = p;
	//				app.private_cmd_len = private_len + 2 - (app.private_cmd - private_head_pos);
	//				//SAFEDELETEP(pDataPacket);
	//				if (app.private_cmd_len < 2) {
	//					ASSERT(0);
	//					break;
	//				}
	//				p += app.private_cmd_len + 4 - 1;
	//			}

	//			*lpBytesCommited = p - pack + 1;
	//			return RESULT_OK;
	//		} while (0);
	//	} catch (wchar_t* err) {
	//		OutputDebugStringW(err);
	//		ASSERT(0);
	//	} catch (char* err) {
	//		OutputDebugStringA(err);
	//		ASSERT(0);
	//	}
	//	return RESULT_DATA_ERROR;
	//}

	//int GenerateConnTestPacket(int conn_id, char* buff, int max_buff_len,
	//						   BOOL bResponce/* = TRUE*/, BOOL has_private_cmd/* = FALSE*/)
	//{
	//	if (IsBadReadPtr(buff, max_buff_len))
	//		throw _T("GenerateConnTestPacket: memory access denied.");
	//	char pack[1024];
	//	int pos = 0;
	//	memset(pack, 0, sizeof(pack));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = NULL;
	//	if (bResponce)
	//		id = AID_ACK;
	//	else
	//		id = AID_NULL;
	//	int id_len = strlen(id);
	//	memcpy(pack + pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy(pack + pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy(pack + pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy(pack + pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy(pack + pos++, "#", 1);
	//	memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
	//	pos += strlen(ACCOUNT);

	//	// [data]
	//	const char* data = "[]";
	//	int data_len = strlen(data);
	//	memcpy(pack + pos, data, data_len);
	//	pos += data_len;

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy(pack + pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	pack[pos] = 0x0D;//<CR>
	//	pos++;

	//	///////////////////////////////////////////////////////////////////
	//	//private public cmd 
	//	//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
	//	//             acct psw acct2 lv ip  port cmd 
	//	if (has_private_cmd) {
	//		const char* private_head_pos = pack + pos;
	//		int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
	//		pack[pos] = (private_len >> 8) & 0xff;
	//		pack[pos + 1] = private_len & 0xff;
	//		pos += 2;

	//		// alarm machine account
	//		//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
	//		//pos += 9;
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// password
	//		//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
	//		memcpy(pack + pos, "ffff", 4);
	//		pos += 4;

	//		// user mobile phone account
	//		//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// alarm level
	//		//pack[pos] = app.level;
	//		pack[pos] = 0;
	//		pos++;

	//		//alarm center ip address and port number // todo
	//		pos += 4;
	//		pos += 2;

	//		// big class and small class
	//		pack[pos++] = 0x06;
	//		pack[pos++] = 0x00;

	//		// conn 3 bytes
	//		pack[pos++] = LOBYTE(HIWORD(conn_id));
	//		pack[pos++] = HIBYTE(LOWORD(conn_id));
	//		pack[pos++] = LOBYTE(LOWORD(conn_id));

	//		//crc_private
	//		Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
	//						   pack + pos, false);
	//		pos += 4;
	//	}

	//	// Finally, copy memory.
	//	memcpy(buff, pack, pos);
	//	return pos;
	//}


	//int GenerateEventPacket(char* pack, int max_pack_len,
	//						int ademco_id, LPCSTR acct,
	//						int ademco_event, int zone,
	//						const char* psw			/* = NULL*/,
	//						BOOL has_private_cmd	/* = FALSE*/, 
	//						int conn_id				/* = 0*/)
	//{
	//	int pos = 0;
	//	memset(pack, 0, sizeof(max_pack_len));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = AID_HB;
	//	int id_len = strlen(id);
	//	memcpy_s(pack + pos, max_pack_len - pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy_s(pack + pos, max_pack_len - pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy_s(pack + pos, max_pack_len - pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy_s(pack + pos, max_pack_len - pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy_s(pack + pos, max_pack_len - pos, "#", 1);
	//	pos++;
	//	memcpy_s(pack + pos, max_pack_len - pos, acct, strlen(acct));
	//	pos += strlen(acct);

	//	// [data]
	//	AdemcoDataSegment data;
	//	data.Make(ademco_id, ademco_event, zone);
	//	//const char* data = "[]";
	//	memcpy_s(pack + pos, max_pack_len - pos, data._data, data._len);
	//	pos += data._len;

	//	// [xdata]
	//	if (psw) {
	//		int psw_len = strnlen_s(psw, 6);
	//		if (psw_len == 6) {
	//			char xdata[32] = { 0 };
	//			xdata[0] = '[';
	//			strcat_s(xdata, 32 - strlen(xdata), psw);
	//			strcat_s(xdata, 32 - strlen(xdata), "]");
	//			int xdata_len = strnlen_s(xdata, psw_len + 2);
	//			memcpy_s(pack + pos, max_pack_len - pos, xdata, xdata_len);
	//			pos += xdata_len;
	//		}
	//	}

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy_s(pack + pos, max_pack_len - pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	//pack[pos] = 0x0D;//<CR>
	//	memcpy_s(pack + pos, max_pack_len - pos, "\r", 1);
	//	pos++;

	//	if (has_private_cmd) {
	//		const char* private_head_pos = pack + pos;
	//		int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
	//		pack[pos] = (private_len >> 8) & 0xff;
	//		pack[pos + 1] = private_len & 0xff;
	//		pos += 2;

	//		// alarm machine account
	//		//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
	//		//pos += 9;
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// password
	//		//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
	//		memcpy(pack + pos, "ffff", 4);
	//		pos += 4;

	//		// user mobile phone account
	//		//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// alarm level
	//		//pack[pos] = app.level;
	//		pack[pos] = 0;
	//		pos++;

	//		//alarm center ip address and port number // todo
	//		pos += 4;
	//		pos += 2;

	//		// big class and small class
	//		pack[pos++] = 0x0c;
	//		pack[pos++] = 0x00;

	//		// conn 3 bytes
	//		pack[pos++] = LOBYTE(HIWORD(conn_id));
	//		pack[pos++] = HIBYTE(LOWORD(conn_id));
	//		pack[pos++] = LOBYTE(LOWORD(conn_id));

	//		//crc_private
	//		Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
	//						   pack + pos, false);
	//		pos += 4;
	//	}

	//	return pos;
	//}

	//int GenerateNullPacket(char* pack, int max_pack_len,
	//					   LPCSTR acct)
	//{
	//	int pos = 0;
	//	memset(pack, 0, sizeof(max_pack_len));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = AID_NULL;
	//	int id_len = strlen(id);
	//	memcpy_s(pack + pos, max_pack_len - pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy_s(pack + pos, max_pack_len - pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy_s(pack + pos, max_pack_len - pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy_s(pack + pos, max_pack_len - pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy_s(pack + pos, max_pack_len - pos, "#", 1);
	//	pos++;
	//	memcpy_s(pack + pos, max_pack_len - pos, acct, strlen(acct));
	//	pos += strlen(acct);

	//	// [data]
	//	const char* data = "[]";
	//	int data_len = strlen(data);
	//	memcpy_s(pack + pos, max_pack_len - pos, data, data_len);
	//	pos += data_len;

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy_s(pack + pos, max_pack_len - pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	//pack[pos] = 0x0D;//<CR>
	//	memcpy_s(pack + pos, max_pack_len - pos, "\r", 1);
	//	pos++;
	//	return pos;
	//}

	//int GenerateOnlinePackage(char* dst, size_t dst_len, int conn_id,
	//					  const char* csr_acct, size_t csr_acct_len)
	//{
	//	if (IsBadReadPtr(dst, dst_len))
	//		throw _T("GenerateOnlinePackage: memory access denied.");
	//	char pack[1024];
	//	int pos = 0;
	//	memset(pack, 0, sizeof(pack));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = AID_NULL;

	//	int id_len = strlen(id);
	//	memcpy(pack + pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy(pack + pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy(pack + pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy(pack + pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy(pack + pos++, "#", 1);
	//	memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
	//	pos += strlen(ACCOUNT);

	//	// [data]
	//	const char* data = "[]";
	//	int data_len = strlen(data);
	//	memcpy(pack + pos, data, data_len);
	//	pos += data_len;

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy(pack + pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	pack[pos] = 0x0D;//<CR>
	//	pos++;

	//	///////////////////////////////////////////////////////////////////
	//	//private public cmd 
	//	//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
	//	//             acct psw acct2 lv ip  port cmd 

	//	const char* private_head_pos = pack + pos;
	//	int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5 + csr_acct_len;
	//	pack[pos] = (private_len >> 8) & 0xff;
	//	pack[pos + 1] = private_len & 0xff;
	//	pos += 2;

	//	// alarm machine account
	//	//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
	//	//pos += 9;
	//	memcpy(pack + pos, "fffffffff", 9);
	//	pos += 9;

	//	// password
	//	//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
	//	memcpy(pack + pos, "ffff", 4);
	//	pos += 4;

	//	// user mobile phone account
	//	//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
	//	memcpy(pack + pos, "fffffffff", 9);
	//	pos += 9;

	//	// alarm level
	//	//pack[pos] = app.level;
	//	pack[pos] = 0;
	//	pos++;

	//	//alarm center ip address and port number		// todo
	//	pos += 4;
	//	pos += 2;

	//	// big class and small class
	//	pack[pos++] = 0x06;
	//	pack[pos++] = 0x01;

	//	// conn 3 bytes
	//	pack[pos++] = LOBYTE(HIWORD(conn_id));
	//	pack[pos++] = HIBYTE(LOWORD(conn_id));
	//	pack[pos++] = LOBYTE(LOWORD(conn_id));

	//	// csr_acct
	//	memcpy(pack + pos, csr_acct, csr_acct_len);
	//	pos += csr_acct_len;

	//	//crc_private
	//	Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
	//					   pack + pos, false);
	//	pos += 4;

	//	// Finally, copy memory.
	//	memcpy(dst, pack, pos);
	//	return pos;
	//}

	//DWORD GenerateAckOrNakEvent(BOOL bAck, int conn_id, char* buff, 
	//										 int max_buff_len, const char* acct, 
	//										 int acct_len, BOOL has_private_cmd)
	//{
	//	if (IsBadReadPtr(buff, max_buff_len))
	//		throw _T("GenerateConnTestPacket: memory access denied.");
	//	char pack[1024];
	//	int pos = 0;
	//	memset(pack, 0, sizeof(pack));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = bAck ? AID_ACK : AID_NAK;
	//	int id_len = strlen(id);
	//	memcpy(pack + pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy(pack + pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy(pack + pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy(pack + pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy(pack + pos++, "#", 1);
	//	memcpy(pack + pos, acct, acct_len);
	//	pos += acct_len;

	//	// [data]
	//	const char* data = "[]";
	//	int data_len = strlen(data);
	//	memcpy(pack + pos, data, data_len);
	//	pos += data_len;

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy(pack + pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	pack[pos] = 0x0D;//<CR>
	//	pos++;

	//	///////////////////////////////////////////////////////////////////
	//	//private public cmd 
	//	//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
	//	//             acct psw acct2 lv ip  port cmd 
	//	if (has_private_cmd) {
	//		const char* private_head_pos = pack + pos;
	//		int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
	//		pack[pos] = (private_len >> 8) & 0xff;
	//		pack[pos + 1] = private_len & 0xff;
	//		pos += 2;

	//		// alarm machine account
	//		//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
	//		//pos += 9;
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// password
	//		//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
	//		memcpy(pack + pos, "ffff", 4);
	//		pos += 4;

	//		// user mobile phone account
	//		//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
	//		memcpy(pack + pos, "fffffffff", 9);
	//		pos += 9;

	//		// alarm level
	//		//pack[pos] = app.level;
	//		pack[pos] = 0;
	//		pos++;

	//		//alarm center ip address and port number			// todo
	//		pos += 4;
	//		pos += 2;

	//		// big class and small class
	//		pack[pos++] = 0x0c;
	//		pack[pos++] = 0x00;

	//		// conn 3 bytes
	//		pack[pos++] = LOBYTE(HIWORD(conn_id));
	//		pack[pos++] = HIBYTE(LOWORD(conn_id));
	//		pack[pos++] = LOBYTE(LOWORD(conn_id));

	//		//crc_private
	//		Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
	//						   pack + pos, false);
	//		pos += 4;
	//	}

	//	// Finally, copy memory.
	//	memcpy(buff, pack, pos);
	//	return pos;
	//}

	//int GenerateRegRspPackage(char* dst, size_t dst_len, int ademco_id)
	//{
	//	if (IsBadReadPtr(dst, dst_len))
	//		throw _T("GenerateRegRspPackage: memory access denied.");
	//	char pack[1024];
	//	int pos = 0;
	//	memset(pack, 0, sizeof(pack));
	//	pack[0] = 0x0A;//<LF>	1
	//	// crc	4
	//	// 0LLL	4

	//	// “id”
	//	pos = 9;
	//	const char* id = AID_REG_RSP;
	//	int id_len = strlen(id);
	//	memcpy(pack + pos, id, id_len);
	//	pos += id_len;

	//	// seq
	//	memcpy(pack + pos, SEQ, strlen(SEQ));
	//	pos += strlen(SEQ);

	//	// Rrcvr
	//	memcpy(pack + pos, RRCVR, strlen(RRCVR));
	//	pos += strlen(RRCVR);

	//	// Lpref
	//	memcpy(pack + pos, LPREF, strlen(LPREF));
	//	pos += strlen(LPREF);

	//	// #acct
	//	memcpy(pack + pos++, "#", 1);
	//	memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
	//	pos += strlen(ACCOUNT);

	//	// [data]
	//	const char* data = "[]";
	//	int data_len = strlen(data);
	//	memcpy(pack + pos, data, data_len);
	//	pos += data_len;

	//	// [data]
	//	char xdata[16] = { 0 };
	//	sprintf_s(xdata, "[%04d]", ademco_id);
	//	assert(strlen(xdata) == 6);
	//	memcpy(pack + pos, xdata, 6);
	//	pos += 6;

	//	// _timestamp
	//	SYSTEMTIME st;
	//	GetLocalTime(&st);
	//	char time_stamp[] = "_02:15:36,04-03-2014";
	//	_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
	//				st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
	//	int time_len = strlen(time_stamp);
	//	memcpy(pack + pos, time_stamp, time_len);
	//	pos += time_len;

	//	// 0LLL
	//	int ademco_len = pos - 9;
	//	Dec2HexCharArray_4(ademco_len, pack + 5);

	//	// CRC
	//	Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

	//	// CR
	//	pack[pos] = 0x0D;//<CR>
	//	pos++;

	//	// Finally, copy memory.
	//	memcpy(dst, pack, pos);
	//	return pos;
	//}
};


