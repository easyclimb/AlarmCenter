#ifndef ___ADEMCO_FUNC_IMPLEMENTATION_H____
#include "StdAfx.h"
#include <assert.h>
#include "ademco_func.h"

namespace Ademco
{
	unsigned int CAdemcoFunc::CalculateCRC(const char* buff, int len)
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
		unsigned int CRC = 0;
		for (int i = 0; i < len; ++i) {
			CRC = (CRC >> 8) ^ (crcTable[(unsigned char)buff[i] ^ (CRC & 0xff)]);
		}
		return CRC;
	}

	const char* CAdemcoFunc::GetAdemcoEventString(int event)
	{
		switch (event) {
			case EVENT_ARM:			return "EVENT_ARM";		break;
			case EVENT_BURGLAR:		return "BURGLAR";	break;
			case EVENT_DISARM:		return "EVENT_DISARM";	break;
			case EVENT_DURESS:		return "DURESS";	break;
			case EVENT_EMERGENCY:		return "EMERGENCY";	break;
			case EVENT_FIRE:			return "FIRE";		break;
			case EVENT_GAS:			return "GAS";		break;
			case EVENT_HALFARM:		return "EVENT_HALFARM";	break;
			case EVENT_TEMPER:		return "TEMPER";	break;
			case EVENT_WATER:			return "WATER";		break;
			case EVENT_LOWBATTERY:	return "LOWBATTERY";	break;
			case EVENT_SOLARDISTURB:	return "SOLARDISTURB";	break;
			case EVENT_DISCONNECT:	return "DISCONNECT";		break;
			case EVENT_SERIAL485DIS:	return "SERIAL485DIS";	break;
			case EVENT_SERIAL485CONN:	return "SERIAL485CONN";	break;
			case EVENT_DOORRINGING:	return "DOORRINGING";	break;
			default:			return "null";		break;
		}
	}

	int CAdemcoFunc::HexCharArrayToDec(const char *hex, int len)
	{
		if (IsBadReadPtr(hex, len))
			throw _T("HexCharArrayToDec: memory access denied.");
		int dec = 0;
		for (int i = 0; i < len; i++) {
			dec = dec * 0x10 + HexChar2Dec(hex[i]);
		}
		return dec;
	}

	int CAdemcoFunc::HexChar2Dec(char hex)
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

	int CAdemcoFunc::NumStr2Dec(const char* str, int str_len)
	{
		if (IsBadReadPtr(str, str_len))
			throw _T("NumStr2Dec: memory access denied.");
		int dec = 0;
		for (int i = 0; i < str_len; i++) {
			dec = dec * 10 + HexChar2Dec(str[i]);
		}
		return dec;
	}

	const char* CAdemcoFunc::HexCharArrayToStr(const char* hex, int len, unsigned char mask /* = (char)0x0f*/)
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

	const char* CAdemcoFunc::HexCharArrayToStr(char* dst, const char* hex, int len,
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

	void CAdemcoFunc::Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF)
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

	void CAdemcoFunc::NumStr2HexCharArray_N(const char* str, char* hexarr, int max_hex_len/* = 9*/)
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

	bool CAdemcoFunc::ParseAdmCid(const char* pack, unsigned int pack_len, ADMCID& data)
	{
		if (IsBadReadPtr(pack, pack_len))
			throw _T("ParseAdmCid: memory access denied.");
		memset(&data, 0, sizeof(data));
		const char* p = pack;
		do {
			if (*p++ != '[')
				break;
			if (pack_len == 2 && *p == ']')
				return true;
			//                  [   #  acct |  mt   s   q event s   gg  s  zone ] 22
			else if (pack_len != 1 + 1 + 4 + 1 + 2 + 1 + 1 + 3 + 1 + 2 + 1 + 3 + 1)
				break;

			if (*p++ != '#')
				break;
			if (*(p + 4) != '|')
				break;
			data.acct = NumStr2Dec(p, 4);
			data.len += 4;
			p += 5;
			data.mt = static_cast<unsigned char>(NumStr2Dec(p, 2));
			p += 2;
			if (*p++ != ' ')
				break;
			//data.q = NumStr2Dec(p++, 1);
			data.event = NumStr2Dec(p, 4);
			p += 4;
			if (*p++ != ' ')
				break;
			data.gg = static_cast<unsigned char>(NumStr2Dec(p, 2));
			p += 2;
			if (*p++ != ' ')
				break;
			data.zone = NumStr2Dec(p, 3);
			return true;
		} while (0);
		return false;
	}

	AttachmentReturnValue CAdemcoFunc::ParsePacket(const char* pack, unsigned int pack_len,
									  AdemcoPrivateProtocal& app, DWORD *lpBytesCommited,
									  BOOL deal_private_cmd/* = FALSE*/)
	{
		try {
			if (IsBadReadPtr(pack, pack_len))
				throw _T("ParsePacket: memory access denied.");
			unsigned int ademco_crc = 0;
			unsigned int ademco_len = 0;
			unsigned int timestamp_len = 0;
			unsigned int private_len = 0;
			unsigned int private_CRC = 0;
			const char* private_head_pos = NULL;
			do {
				bool hasPrivateData = false;
				if (pack_len < 9)	return ARV_PACK_NOT_ENOUGH;
				if (pack[0] != 0x0a) {
					ASSERT(0);
					break;
				}					// check LF
				ademco_crc = HexCharArrayToDec(pack + 1, 4);
				ademco_len = HexCharArrayToDec(pack + 5, 4);
				// read till CR
				DWORD dwLenToParse = 9 + ademco_len + 1;		// 1 for CR, 2 for private len.

				if (pack_len < dwLenToParse)
					return ARV_PACK_NOT_ENOUGH;
				else if (pack_len == dwLenToParse)
					hasPrivateData = false;
				else
					hasPrivateData = true;

				const char* CR_pos = pack + 9 + ademco_len;
				if (*CR_pos != 0x0d) {
					ASSERT(0);
					break;
				}		// check CR
				// check ademco CRC
				if (ademco_crc != CalculateCRC(pack + 9, ademco_len)) {
					OutputDebugString(_T("CalculateCRC Ademco Error\n"));
					ASSERT(0);
					break;
				}

				if (hasPrivateData && deal_private_cmd) {
					private_head_pos = CR_pos + 1;
					// read private cmd
					private_len = MAKEWORD(*(char*)(private_head_pos + 1),
										   *(char*)(private_head_pos));
					dwLenToParse += 2 + private_len + 4;			// 4 for private CRC
					if (dwLenToParse > pack_len)
						return ARV_PACK_NOT_ENOUGH;

					// check ademco and private protocal 's CRC
					private_CRC = HexCharArrayToDec(pack + dwLenToParse - 4, 4);

					if (private_CRC != CalculateCRC(private_head_pos + 2, private_len)) {
						CLog::WriteLog(_T("CalculateCRC PrivateProtocal Error\n"));
						break;
					}
				}

				// parse Ademco
				// id
				if (pack[9] != '\"') {
					ASSERT(0);
					break;
				}			// find first " of "id".
				const char* p = pack + 10;			// find last  " of "id".
				while (p < CR_pos && *p != '\"') { p++; }
				if (*p != '\"') {
					ASSERT(0);
					break;
				}			// " not found.
				p++;								// skip "
				app.id = pack + 9;
				app.id_len = p - app.id;
				bool b_null_data = is_null_data(app.id, app.id_len);

				// seq (and Rrcvr, it may not exist)
				while (p < CR_pos && *p != 'R' && *p != 'L') { p++; }
				app.seq = app.id + app.id_len;
				app.seq_len = p - app.seq;
				if (*p == 'R') {
					app.Rrcvr = p;
					while (p < CR_pos && *p != 'L') { p++; }
					app.Rrcvr_len = p - app.Rrcvr;
				} else if (*p == 'L') {
					app.Rrcvr = NULL;
					app.Rrcvr_len = 0;
				} else {
					ASSERT(0);
					break;
				}
				// Lpref
				if (*p != 'L') {
					ASSERT(0);
					break;
				}			// L of Lpref not found.
				app.Lpref = p;
				while (p < CR_pos && *p != '#') { p++; }
				app.Lpref_len = p - app.Lpref;
				// acct
				if (*p++ != '#') {
					ASSERT(0);
					break;
				}			// # of #acct not found
				app.acct = p;
				while (p < CR_pos && *p != '[') { p++; }
				app.acct_len = p - app.acct;
				// data
				if (*p != '[') {
					ASSERT(0);
					break;
				}			// [ of [data] not found.
				const char* ademco_cmd = p;
				while (p < CR_pos && *p != ']') { p++; }
				p++;				// skip ]
				int ademco_cmd_len = p - ademco_cmd;
				if (!b_null_data) {
					if (!ParseAdmCid(ademco_cmd, ademco_cmd_len, app.admcid)) {
						ASSERT(0);
						break;
					}
				}
				// [x...data...]
				if (*p == '[') {
					const char* xdata_pos = ++p;	// skip [
					app.xdata = xdata_pos;
					while (p < CR_pos && *p != ']') { p++; }
					int xdata_len = p - xdata_pos;
					app.xdata_len = xdata_len;
					p++;				// skip ]
				}

				// timestamp, standard format is _23:59:59,12-23-2000, so its len is 20.
				static const int TIMESTAMP_LEN = 20;
				if (*p != '_') {
					ASSERT(0);
					break;
				}			// _ of _timestamp not found.
				memcpy(app.timestamp, p, TIMESTAMP_LEN);
				app.timestamp[TIMESTAMP_LEN] = 0;
				timestamp_len = TIMESTAMP_LEN; // a little trick, use time_stamp_len as a counter.
				while (p < CR_pos && timestamp_len--) { p++; }
				if (timestamp_len != 0) {
					ASSERT(0);
					break;
				}
				if (p != CR_pos) {
					ASSERT(0);
					break;
				}


				if (hasPrivateData && deal_private_cmd) {
					// parse PrivateProtocal
					// acct of machine.
					p += 3;	// skip CR_pos and 2 bytes for private_len
					//strcpy_s(acct_machine, HexCharArrayToStr(p, 9));
					HexCharArrayToStr(app.acct_machine, p, 9);
					p += 9;
					// passwd of machine
					//strcpy_s(passwd_machine, HexCharArrayToStr(p, 4));
					HexCharArrayToStr(app.passwd_machine, p, 4);
					p += 4;
					// phone number
					//strcpy_s(phone, HexCharArrayToStr(p, 9));
					HexCharArrayToStr(app.phone, p, 9);
					p += 9;
					// alarm level
					app.level = (char)*p;
					p++;
					// ip of center station receiver
					_snprintf_s(app.ip_csr, 16, "%d.%d.%d.%d",
								*p, *(p + 1), *(p + 2), *(p + 3));
					p += 4;
					// port of csr
					app.port_csr = MAKEWORD(*p, *(p + 1));
					p += 2;
					// content and content_len
					app.private_cmd = p;
					app.private_cmd_len = private_len + 2 - (app.private_cmd - private_head_pos);
					//SAFEDELETEP(pDataPacket);
					if (app.private_cmd_len < 2) {
						ASSERT(0);
						break;
					}
					p += app.private_cmd_len + 4 - 1;
				}

				*lpBytesCommited = p - pack + 1;
				return ARV_OK;
			} while (0);
		} catch (wchar_t* err) {
			OutputDebugStringW(err);
			ASSERT(0);
		} catch (char* err) {
			OutputDebugStringA(err);
			ASSERT(0);
		}
		return ARV_PACK_DATA_ERROR;
	}

	int CAdemcoFunc::GenerateConnTestPacket(int conn_id, char* buff, int max_buff_len,
							   BOOL bResponce/* = TRUE*/, BOOL has_private_cmd/* = FALSE*/)
	{
		if (IsBadReadPtr(buff, max_buff_len))
			throw _T("GenerateConnTestPacket: memory access denied.");
		char pack[1024];
		int pos = 0;
		memset(pack, 0, sizeof(pack));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = NULL;
		if (bResponce)
			id = AID_ACK;
		else
			id = AID_NULL;
		int id_len = strlen(id);
		memcpy(pack + pos, id, id_len);
		pos += id_len;

		// seq
		memcpy(pack + pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy(pack + pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy(pack + pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy(pack + pos++, "#", 1);
		memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
		pos += strlen(ACCOUNT);

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy(pack + pos, data, data_len);
		pos += data_len;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy(pack + pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		pack[pos] = 0x0D;//<CR>
		pos++;

		///////////////////////////////////////////////////////////////////
		//private public cmd 
		//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
		//             acct psw acct2 lv ip  port cmd 
		if (has_private_cmd) {
			const char* private_head_pos = pack + pos;
			int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
			pack[pos] = (private_len >> 8) & 0xff;
			pack[pos + 1] = private_len & 0xff;
			pos += 2;

			// alarm machine account
			//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
			//pos += 9;
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// password
			//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
			memcpy(pack + pos, "ffff", 4);
			pos += 4;

			// user mobile phone account
			//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// alarm level
			//pack[pos] = app.level;
			pack[pos] = 0;
			pos++;

			//alarm center ip address and port number // todo
			pos += 4;
			pos += 2;

			// big class and small class
			pack[pos++] = 0x06;
			pack[pos++] = 0x00;

			// conn 3 bytes
			pack[pos++] = LOBYTE(HIWORD(conn_id));
			pack[pos++] = HIBYTE(LOWORD(conn_id));
			pack[pos++] = LOBYTE(LOWORD(conn_id));

			//crc_private
			Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
							   pack + pos, false);
			pos += 4;
		}

		// Finally, copy memory.
		memcpy(buff, pack, pos);
		return pos;
	}

	int CAdemcoFunc::GenerateConnIDPacket(const AdemcoPrivateProtocal& app,
							 char* buff, int max_buff_len)
	{
		if (IsBadReadPtr(buff, max_buff_len))
			throw _T("GenerateConnIDPacket: memory access denied.");
		char pack[1024];
		int pos = 0;
		memset(pack, 0, sizeof(pack));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		int id_len = strlen(AID_ACK);
		memcpy(pack + pos, AID_ACK, id_len);
		pos += id_len;

		// seq
		memcpy(pack + pos, app.seq, app.seq_len);
		pos += app.seq_len;

		// Rrcvr
		memcpy(pack + pos, app.Rrcvr, app.Rrcvr_len);
		pos += app.Rrcvr_len;

		// Lpref
		memcpy(pack + pos, app.Lpref, app.Lpref_len);
		pos += app.Lpref_len;

		// #acct
		memcpy(pack + pos, app.acct, app.acct_len);
		pos += app.acct_len;

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy(pack + pos, data, data_len);
		pos += data_len;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy(pack + pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		pack[pos] = 0x0D;//<CR>
		pos++;
		/*
		///////////////////////////////////////////////////////////////////
		//private public cmd
		//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
		//             acct psw acct2 lv ip  port cmd
		const char* private_head_pos = pack + pos;
		int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
		pack[pos]	= (private_len >> 8) & 0xff;
		pack[pos+1] = private_len & 0xff;
		pos += 2;

		// alarm machine account
		NumStr2HexCharArray_N(app.acct_machine, pack + pos, 9);
		pos += 9;

		// password
		NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
		pos += 4;

		// user mobile phone account
		NumStr2HexCharArray_N(app.phone, pack + pos, 9);
		pos += 9;

		// alarm level
		pack[pos] = app.level;
		pos ++;

		//alarm center ip address and port number // todo
		
		pos += 4;
		pos += 2;

		// big class and small class
		pack[pos++] = 0x03;
		pack[pos++] = 0x01;

		// conn 3 bytes
		//srand(time(NULL));
		pack[pos++] = (g_conn_id >> 16) & 0xff;
		pack[pos++] = (g_conn_id >> 8) & 0xff;
		pack[pos++] = g_conn_id & 0xff;
		g_conn_id++;
		//crc_private
		Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
		pack + pos, false);
		pos += 4;
		*/
		// Finally, copy memory.
		memcpy(buff, pack, pos);
		return pos;
	}

	int CAdemcoFunc::GenerateEventPacket(char* pack, int max_pack_len,
							int ademco_id, LPCSTR acct,
							int event, int zone, 
							const char* psw			/* = NULL*/,
							BOOL has_private_cmd	/* = FALSE*/, 
							int conn_id				/* = 0*/)
	{
		int pos = 0;
		memset(pack, 0, sizeof(max_pack_len));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = AID_HB;
		int id_len = strlen(id);
		memcpy_s(pack + pos, max_pack_len - pos, id, id_len);
		pos += id_len;

		// seq
		memcpy_s(pack + pos, max_pack_len - pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy_s(pack + pos, max_pack_len - pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy_s(pack + pos, max_pack_len - pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy_s(pack + pos, max_pack_len - pos, "#", 1);
		pos++;
		memcpy_s(pack + pos, max_pack_len - pos, acct, strlen(acct));
		pos += strlen(acct);

		// [data]
		AdemcoData data(ademco_id, event, zone);
		//const char* data = "[]";
		int data_len = data.GetLength();
		memcpy_s(pack + pos, max_pack_len - pos, data.GetBuffer(), data_len);
		pos += data_len;

		// [xdata]
		if (psw) {
			int psw_len = strnlen_s(psw, 6);
			if (psw_len == 6) {
				char xdata[32] = { 0 };
				xdata[0] = '[';
				strcat_s(xdata, 32 - strlen(xdata), psw);
				strcat_s(xdata, 32 - strlen(xdata), "]");
				int xdata_len = strnlen_s(xdata, psw_len + 2);
				memcpy_s(pack + pos, max_pack_len - pos, xdata, xdata_len);
				pos += xdata_len;
			}
		}

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy_s(pack + pos, max_pack_len - pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		//pack[pos] = 0x0D;//<CR>
		memcpy_s(pack + pos, max_pack_len - pos, "\r", 1);
		pos++;

		if (has_private_cmd) {
			const char* private_head_pos = pack + pos;
			int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
			pack[pos] = (private_len >> 8) & 0xff;
			pack[pos + 1] = private_len & 0xff;
			pos += 2;

			// alarm machine account
			//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
			//pos += 9;
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// password
			//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
			memcpy(pack + pos, "ffff", 4);
			pos += 4;

			// user mobile phone account
			//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// alarm level
			//pack[pos] = app.level;
			pack[pos] = 0;
			pos++;

			//alarm center ip address and port number // todo
			pos += 4;
			pos += 2;

			// big class and small class
			pack[pos++] = 0x0c;
			pack[pos++] = 0x00;

			// conn 3 bytes
			pack[pos++] = LOBYTE(HIWORD(conn_id));
			pack[pos++] = HIBYTE(LOWORD(conn_id));
			pack[pos++] = LOBYTE(LOWORD(conn_id));

			//crc_private
			Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
							   pack + pos, false);
			pos += 4;
		}

		return pos;
	}

	int CAdemcoFunc::GenerateNullPacket(char* pack, int max_pack_len,
						   LPCSTR acct)
	{
		int pos = 0;
		memset(pack, 0, sizeof(max_pack_len));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = AID_NULL;
		int id_len = strlen(id);
		memcpy_s(pack + pos, max_pack_len - pos, id, id_len);
		pos += id_len;

		// seq
		memcpy_s(pack + pos, max_pack_len - pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy_s(pack + pos, max_pack_len - pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy_s(pack + pos, max_pack_len - pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy_s(pack + pos, max_pack_len - pos, "#", 1);
		pos++;
		memcpy_s(pack + pos, max_pack_len - pos, acct, strlen(acct));
		pos += strlen(acct);

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy_s(pack + pos, max_pack_len - pos, data, data_len);
		pos += data_len;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy_s(pack + pos, max_pack_len - pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		//pack[pos] = 0x0D;//<CR>
		memcpy_s(pack + pos, max_pack_len - pos, "\r", 1);
		pos++;
		return pos;
	}

	int CAdemcoFunc::GenerateOnlinePackage(char* dst, size_t dst_len, int conn_id,
						  const char* csr_acct, size_t csr_acct_len)
	{
		if (IsBadReadPtr(dst, dst_len))
			throw _T("GenerateOnlinePackage: memory access denied.");
		char pack[1024];
		int pos = 0;
		memset(pack, 0, sizeof(pack));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = AID_NULL;

		int id_len = strlen(id);
		memcpy(pack + pos, id, id_len);
		pos += id_len;

		// seq
		memcpy(pack + pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy(pack + pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy(pack + pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy(pack + pos++, "#", 1);
		memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
		pos += strlen(ACCOUNT);

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy(pack + pos, data, data_len);
		pos += data_len;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy(pack + pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		pack[pos] = 0x0D;//<CR>
		pos++;

		///////////////////////////////////////////////////////////////////
		//private public cmd 
		//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
		//             acct psw acct2 lv ip  port cmd 

		const char* private_head_pos = pack + pos;
		int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5 + csr_acct_len;
		pack[pos] = (private_len >> 8) & 0xff;
		pack[pos + 1] = private_len & 0xff;
		pos += 2;

		// alarm machine account
		//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
		//pos += 9;
		memcpy(pack + pos, "fffffffff", 9);
		pos += 9;

		// password
		//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
		memcpy(pack + pos, "ffff", 4);
		pos += 4;

		// user mobile phone account
		//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
		memcpy(pack + pos, "fffffffff", 9);
		pos += 9;

		// alarm level
		//pack[pos] = app.level;
		pack[pos] = 0;
		pos++;

		//alarm center ip address and port number		// todo
		pos += 4;
		pos += 2;

		// big class and small class
		pack[pos++] = 0x06;
		pack[pos++] = 0x01;

		// conn 3 bytes
		pack[pos++] = LOBYTE(HIWORD(conn_id));
		pack[pos++] = HIBYTE(LOWORD(conn_id));
		pack[pos++] = LOBYTE(LOWORD(conn_id));

		// csr_acct
		memcpy(pack + pos, csr_acct, csr_acct_len);
		pos += csr_acct_len;

		//crc_private
		Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
						   pack + pos, false);
		pos += 4;

		// Finally, copy memory.
		memcpy(dst, pack, pos);
		return pos;
	}

	DWORD CAdemcoFunc::GenerateAckOrNakEvent(BOOL bAck, int conn_id, char* buff, 
											 int max_buff_len, const char* acct, 
											 int acct_len, BOOL has_private_cmd)
	{
		if (IsBadReadPtr(buff, max_buff_len))
			throw _T("GenerateConnTestPacket: memory access denied.");
		char pack[1024];
		int pos = 0;
		memset(pack, 0, sizeof(pack));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = bAck ? AID_ACK : AID_NAK;
		int id_len = strlen(id);
		memcpy(pack + pos, id, id_len);
		pos += id_len;

		// seq
		memcpy(pack + pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy(pack + pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy(pack + pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy(pack + pos++, "#", 1);
		memcpy(pack + pos, acct, acct_len);
		pos += acct_len;

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy(pack + pos, data, data_len);
		pos += data_len;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy(pack + pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		pack[pos] = 0x0D;//<CR>
		pos++;

		///////////////////////////////////////////////////////////////////
		//private public cmd 
		//private cmd length 8：表示大类、子类、和2个连接号；4：表示校验和。
		//             acct psw acct2 lv ip  port cmd 
		if (has_private_cmd) {
			const char* private_head_pos = pack + pos;
			int private_len = 9 + 4 + 9 + 1 + 4 + 2 + 5;
			pack[pos] = (private_len >> 8) & 0xff;
			pack[pos + 1] = private_len & 0xff;
			pos += 2;

			// alarm machine account
			//NumStr2HexCharArray_N(ACCT_M, pack + pos, 9);
			//pos += 9;
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// password
			//NumStr2HexCharArray_N(app.passwd_machine, pack + pos, 4);
			memcpy(pack + pos, "ffff", 4);
			pos += 4;

			// user mobile phone account
			//NumStr2HexCharArray_N(app.phone, pack + pos, 9);
			memcpy(pack + pos, "fffffffff", 9);
			pos += 9;

			// alarm level
			//pack[pos] = app.level;
			pack[pos] = 0;
			pos++;

			//alarm center ip address and port number			// todo
			pos += 4;
			pos += 2;

			// big class and small class
			pack[pos++] = 0x0c;
			pack[pos++] = 0x00;

			// conn 3 bytes
			pack[pos++] = LOBYTE(HIWORD(conn_id));
			pack[pos++] = HIBYTE(LOWORD(conn_id));
			pack[pos++] = LOBYTE(LOWORD(conn_id));

			//crc_private
			Dec2HexCharArray_4(CalculateCRC(private_head_pos + 2, private_len),
							   pack + pos, false);
			pos += 4;
		}

		// Finally, copy memory.
		memcpy(buff, pack, pos);
		return pos;
	}

	int CAdemcoFunc::GenerateRegRspPackage(char* dst, size_t dst_len, int ademco_id)
	{
		if (IsBadReadPtr(dst, dst_len))
			throw _T("GenerateRegRspPackage: memory access denied.");
		char pack[1024];
		int pos = 0;
		memset(pack, 0, sizeof(pack));
		pack[0] = 0x0A;//<LF>	1
		// crc	4
		// 0LLL	4

		// “id”
		pos = 9;
		const char* id = AID_REG_RSP;
		int id_len = strlen(id);
		memcpy(pack + pos, id, id_len);
		pos += id_len;

		// seq
		memcpy(pack + pos, SEQ, strlen(SEQ));
		pos += strlen(SEQ);

		// Rrcvr
		memcpy(pack + pos, RRCVR, strlen(RRCVR));
		pos += strlen(RRCVR);

		// Lpref
		memcpy(pack + pos, LPREF, strlen(LPREF));
		pos += strlen(LPREF);

		// #acct
		memcpy(pack + pos++, "#", 1);
		memcpy(pack + pos, ACCOUNT, strlen(ACCOUNT));
		pos += strlen(ACCOUNT);

		// [data]
		const char* data = "[]";
		int data_len = strlen(data);
		memcpy(pack + pos, data, data_len);
		pos += data_len;

		// [data]
		char xdata[16] = { 0 };
		sprintf_s(xdata, "[%04d]", ademco_id);
		assert(strlen(xdata) == 6);
		memcpy(pack + pos, xdata, 6);
		pos += 6;

		// _timestamp
		SYSTEMTIME st;
		GetLocalTime(&st);
		char time_stamp[] = "_02:15:36,04-03-2014";
		_snprintf_s(time_stamp, _countof(time_stamp), 20, "_%02d:%02d:%02d,%02d-%02d-%04d",
					st.wHour, st.wMinute, st.wSecond, st.wMonth, st.wDay, st.wYear);
		int time_len = strlen(time_stamp);
		memcpy(pack + pos, time_stamp, time_len);
		pos += time_len;

		// 0LLL
		int ademco_len = pos - 9;
		Dec2HexCharArray_4(ademco_len, pack + 5);

		// CRC
		Dec2HexCharArray_4(CalculateCRC(pack + 9, ademco_len), pack + 1, false);

		// CR
		pack[pos] = 0x0D;//<CR>
		pos++;

		// Finally, copy memory.
		memcpy(dst, pack, pos);
		return pos;
	}
};


