/*
* ademco_func.h
*/
#ifndef ___ADEMCO_FUNC_H____
#define ___ADEMCO_FUNC_H____

#pragma once

#include <cassert>
#include <cstring>
#include <list>
#include <math.h>
#include "ademco_event.h"

namespace ademco
{

#ifdef __GNUG__

typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned int	    DWORD;
typedef DWORD				DWORD_PTR;

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD_PTR)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD_PTR)(b)) & 0xff))) << 8))
#define MAKEDWORD(a, b)     ((DWORD)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)           ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)           ((BYTE)(((DWORD_PTR)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((DWORD_PTR)(w)) >> 8) & 0xff))

#else // __GNUG__
#define MAKEDWORD MAKELONG
#endif // __GNUG__


typedef enum ParseResult
{
	RESULT_OK,
	RESULT_NOT_ENOUGH,
	RESULT_DATA_ERROR,
}ParseResult;

static const int AID_NUM = 9;
static const char* AID_NULL = "\"NULL\"";
static const char* AID_ACK = "\"ACK\"";
static const char* AID_NAK = "\"NAK\"";
static const char* AID_DUH = "\"DUH\"";
static const char* AID_HB = "\"HENG-BO\"";
static const char* AID_ADM_CID = "\"ADM-CID\"";
static const char* AID_MODULE_REG = "\"MODULE-REG\"";
static const char* AID_REG_RSP = "\"REG-RSP\"";
static const char* AID_PWW = "\"PWW\"";

//static const char* SEQ = "0000";
static const char* RRCVR = "R1234";
static const char* LPREF = "L6789";
//static const char* ACCOUNT = "18240888101";
//const char* ACCT_M	= "18240888102";

static const char* g_aid[AID_NUM] = {
	AID_NULL,
	AID_ACK,
	AID_NAK,
	AID_DUH,
	AID_HB,
	AID_ADM_CID,
	AID_MODULE_REG,
	AID_REG_RSP,
	AID_PWW,
};

static const char* g_valid_ademco_protocals[] = {
	AID_HB,
	AID_ADM_CID,
};

enum ademco_protocal {
	heng_bo,
	adm_cid,
};

inline bool is_same_id(const char_array& a, const char* id) {
	auto len = strlen(id);
	if (len != a.size()) return false;
	for (size_t i = 0; i < a.size(); i++) {
		if (a[i] != id[i])
			return false;
	}
	return true;
}

inline bool is_valid_ademco_protocal(const char_array& protocal, ademco_protocal& ap) {
	if (is_same_id(protocal, AID_HB)) {
		ap = heng_bo;
		return true;
	} else if (is_same_id(protocal, AID_ADM_CID)) {
		ap = adm_cid;
		return true;
	} else {
		return false;
	}
}


class AdemcoDataSegment
{
public:
	bool			_valid;
	char_array		_data;
	unsigned int	_len;
	unsigned int	_ademco_id;
	//unsigned char	_mt;
	ademco::ADEMCO_EVENT	_ademco_event;
	unsigned char	_gg;
	unsigned int	_zone;

	AdemcoDataSegment() { reset(); }

	AdemcoDataSegment(const AdemcoDataSegment& rhs) {
		reset();
		_valid = rhs._valid;
		_data = rhs._data;
		_len = rhs._len;
		_ademco_id = rhs._ademco_id;
		_ademco_event = rhs._ademco_event;
		_gg = rhs._gg;
		_zone = rhs._zone;
	}

	void reset() {
		_valid = false;
		_data.clear();
		_len = 0;
		_ademco_id = 0;
		_ademco_event = EVENT_INVALID_EVENT;
		_gg = 0;
		_zone = 0;
	}

	// maker
	void Make(int ademco_id, int gg, ADEMCO_EVENT ademco_event, int zone);

	void Make() {
		reset();
		_valid = true;
		_data.push_back('[');
		_data.push_back(']');
		_len = 2;
		_ademco_id = 0;
		_ademco_event = EVENT_INVALID_EVENT;
		_gg = 0;
		_zone = 0;
	}

	// parser
	bool Parse(const char* pack, size_t pack_len);

	bool operator==(const AdemcoDataSegment& rhs) const {
		return _valid == rhs._valid
			&& _len == rhs._len
			&& _ademco_id == rhs._ademco_id
			&& _ademco_event == rhs._ademco_event
			&& _gg == rhs._gg
			&& _zone == rhs._zone;
	}
};

// 2016-11-4 16:47:51 for compatibility of congwin fe100 protocal
class congwin_fe100_packet {
public:
	char data_[32] = { 0 };
	const int len_ = 27;

	bool from_ademco_data_segment(const AdemcoDataSegment* data);
};

class AdemcoTimeStamp
{
public:
	char _data[32];
	size_t _len;
	time_t _time;

	AdemcoTimeStamp() { memset(this, 0, sizeof(AdemcoTimeStamp)); }

	void Make();

	bool Parse(const char* pack, size_t pack_len);
};

class AdemcoPacket
{
public:
	static const char _LF = 0x0A;
	char_array _crc;
	char_array _len;
	char_array _id;
	char_array _seq;
	char_array _rrcvr;
	char_array _lpref;
	char_array _acct;
	AdemcoDataSegment _ademco_data;
	char_array_ptr _xdata;
	AdemcoTimeStamp _timestamp;
	static const char _CR = 0x0D;

	AdemcoPacket() : _xdata() { Clear(); }
	
	~AdemcoPacket() {
		Clear();
	}

	void Clear() {
		_crc.clear();
		_len.clear();
		_id.clear();
		_seq.clear();
		_rrcvr.clear();
		_lpref.clear();
		_acct.clear();
		_ademco_data.reset();
		_xdata = nullptr;
	}

	size_t GetLength() const;

	void CopyData(char* dst, size_t length);

	// maker
	size_t Make(char* pack, size_t pack_len, const char* id,
				int seq, char const* acct, int ademco_id,
				ADEMCO_EVENT ademco_event, int gg, int zone,
				const char_array_ptr& xdata = nullptr);

	// parser
	ParseResult Parse(const char* pack, size_t pack_len, size_t& cbCommited);
};

class ConnID
{
public:
	unsigned char _1;
	unsigned char _2;
	unsigned char _3;

	ConnID() : _1(0xFF), _2(0xFF), _3(0xFF) {}

	ConnID(int conn_id) {
		FromInt(conn_id);
	}

	void FromInt(int conn_id) {
		_1 = LOBYTE(HIWORD(conn_id));
		_2 = HIBYTE(LOWORD(conn_id));
		_3 = LOBYTE(LOWORD(conn_id));
	}

	void FromCharArray(char arr[3])
	{
		_1 = arr[0];
		_2 = arr[1];
		_3 = arr[2];
	}

	bool valid() const
	{
		return !(_1 == 0xFF && _2 == 0xFF && _3 == 0xFF);
	}

	int ToInt()
	{
		return valid() ? MAKEDWORD(MAKEWORD(_3, _2), MAKEWORD(_1, 0)) : -1;
	}

	operator int()
	{
		return ToInt();
	}
};

inline void AppendConnIdToCharArray(char_array& a, const ConnID& id) {
	a.push_back(id._1);
	a.push_back(id._2);
	a.push_back(id._3);
}

inline ConnID GetConnIdFromCharArray(const char_array& a) {
	assert(a.size() >= 3);
	char c[3] = {};
	for (int i = 0; i < 3; i++) { c[i] = a[i]; }
	ConnID connId; connId.FromCharArray(c);
	return connId;
}

class PrivatePacket;
typedef std::shared_ptr<PrivatePacket> PrivatePacketPtr;
class PrivatePacket
{
public:
	char _len[2] = { 0 };
	char _acct_machine[9] = { 0 };
	char _passwd_machine[4] = { 0 };
	char _acct[9] = { 0 };
	char _level = { 0 };
	char _ip_csr[4] = { 0 };
	char _port_csr[2] = { 0 };
	char _big_type = { 0 };
	char _lit_type = { 0 };
	char_array _cmd;
	char _crc[4] = { 0 };

public:
	PrivatePacket() {}

	size_t GetLength() const;

	size_t Make(char* pack,
				size_t pack_len,
				char big_type,
				char lit_type,
				const char_array& cmd,
				const char* acct_machine,
				const char* passwd_machine,
				const char* acct_csr,
				char level
				);

	size_t MakeAsc(char* pack,
				   size_t pack_len,
				   char big_type,
				   char lit_type,
				   const char_array& cmd,
				   const char* acct_machine,
				   const char* passwd_machine,
				   const char* acct_csr,
				   char level
				   );

	ParseResult Parse(const char* pack, size_t pack_len, size_t& cbCommited);

	ParseResult ParseAsc(char* pack, size_t pack_len, size_t& cbCommited, size_t& cbNewLength);

	void Copy(const PrivatePacket* rhs)
	{
#define COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(elem) memcpy(&elem, &rhs->elem, sizeof(elem));
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_len);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_acct_machine);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_passwd_machine);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_acct);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_level);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_ip_csr);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_port_csr);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_big_type);
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_lit_type);
		_cmd = rhs->_cmd;
		COPY_MEMORY_FUNCTION_FOR_CLASS_PRIVATE_PACKET(_crc);
	}

protected:
	void CopyData(char* dst, size_t length);

};


inline bool IsCloseEvent(ADEMCO_EVENT ademco_event)
{
	return ademco_event == EVENT_ARM
		|| ademco_event == EVENT_HALFARM;
}

inline bool IsStatusEvent(ADEMCO_EVENT ademco_event)
{
	return ademco_event == EVENT_ARM
		|| ademco_event == EVENT_HALFARM
		|| ademco_event == EVENT_DISARM;
}

inline bool IsMachineTypeEvent(ADEMCO_EVENT ademco_event) {
	return ademco_event == EVENT_I_AM_NET_MODULE
		|| ademco_event == EVENT_I_AM_EXPRESSED_GPRS_2050_MACHINE
		|| ademco_event == EVENT_I_AM_LCD_MACHINE;
}

inline int GetDecDigits(int dec)
{
	int digits = 0;
	while (dec != 0) {
		dec /= 10;
		digits++;
	}
	return digits;
}



// format ascii charactor to integer value.
// e.g. 
// input: '0'
// output: 0
int HexChar2Dec(char hex);

// e.g.
// input: 10
// output: 'A'
char Dec2Hex(char d);
// e.g.
// input: "10"
// output: 16
int HexCharArrayToDec(const char *hex, size_t len);

// e.g.
// input: "10"
// output: 10
int NumStr2Dec(const char* str, size_t str_len);

// e.g.
// input: "10"
// output: "16"
const char* HexCharArrayToStr(const char* hex, size_t len, unsigned char mask = (char)0x0f);

// like upper, but cat result to dst. 
const char* HexCharArrayToStr(char* dst, const char* hex, size_t len,
							  unsigned char mask = (char)0x0f);

// format number to hex char array(max dec is 0x0fff
// e.g.
// input: 16
// output: 0010
void Dec2HexCharArray_4(int dec, char* hex, bool bMax0FFF = true);

// format phone number to hex char array(N bytes)
// e.g.
// input: "18240888101"
// output: 18 24 08 88  10 1f ff ff  ff
void NumStr2HexCharArray_N(const char* str, char* hexarr, size_t max_hex_len = 9);

void ConvertHiLoAsciiToAscii(char* dst, const char* src, size_t len);

unsigned short CalculateCRC(const char* buff, size_t len, unsigned short crc = 0);

/*static bool is_null_data(const char* id, unsigned int len)
{
	return (strncmp(AID_NULL, id, min(len, strlen(AID_NULL))) == 0);
}*/

static bool is_null_data(const char* id)
{
	return (strcmp(AID_NULL, id) == 0)
		|| (strcmp(AID_ACK, id) == 0)
		|| (strcmp(AID_DUH, id) == 0);
}

};

//#include "ademco_func_implementation.h"

#endif /* ___ADEMCO_FUNC_H____ */
