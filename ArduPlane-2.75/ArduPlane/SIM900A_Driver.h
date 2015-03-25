#ifndef _GPRS_DRIVER_H_
#define _GPRS_DRIVER_H_

#include <AP_HAL.h>
#include <string.h>
#include <stdint.h>


typedef unsigned int uint16;
typedef unsigned char uint8;
typedef int int16;
typedef char int8;

#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef WORD
typedef unsigned int WORD;
#endif

#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define GPRS_DATA_ESC 'A'
#define GPRS_DATA_START_CHARACTER  'S'
#define GPRS_DATA_END_CHARACTER  'E'

#define GPRS_DATA_PACKET_SIZE 9
#define GPRS_DATA_PACKET_BUFF_SIZE  GPRS_DATA_PACKET_SIZE + 2

#define GPRS_SERVER_HARTBEAT_TIMEOUT_MS 10000
#define GPRS_SERVER_HARTBEAT_TIMEOUT_ALLOW_TIMES 3

typedef struct _GPRS_DATA_PACKET_
{
	char ch_Start;
	uint8_t ch_Flag;
	uint8_t ui8_Data[4];
	uint16_t ui16_Checksum;
	char ch_Stop;
}GPRS_DATA_PACKET, *P_GPRS_DATA_PACKET;

typedef struct _GPRS_DATA_PACKET_INFO_
{
	uint8_t ui8_Recv_End : 1;
	uint8_t ui8_Recv_First : 1;
	uint8_t ui8_Find_E : 1;
	uint8_t ui8_Find_ESC : 1;
	uint8_t ui8_Reserve : 4;
	uint8_t ui8_Gprs_Data_Buff_Index;
	char ch_Gprs_Data_Buff[GPRS_DATA_PACKET_BUFF_SIZE];
}GPRS_DATA_PACKET_INFO, *P_GPRS_DATA_PACKET_INFO;

typedef struct _SIM900A_GPRS_STATE_
{
	uint16_t ui16_Gprs_Init : 1;
	uint16_t ui16_Gprs_Connect_State : 3;
	uint16_t ui16_Server_Hartbeat_State : 2;
	uint16_t ui16_Station_Hartbeat_State : 2;
	uint16_t ui16_Connect_Timeout : 1;
	uint16_t ui16_Server_Hartbeat_Timeout : 1;
	uint16_t ui16_Server_Hartbeat_Timeout_Count : 3;
	uint16_t ui16_First_Connect : 1;
	uint16_t ui16_Recv_New_Server_Hartbeat : 1;
	uint16_t ui16_Gprs_Environment_Init : 1;
}SIM900A_GPRS_STATE, *P_SIM900A_GPRS_STATE;

enum GPRS_DATA_FLAG
{
	GPRS_DATA_FLY_MODE = 0,
	GPRS_DATA_CUR,
	GPRS_DATA_BAT,
	GPRS_DATA_VS,
	GPRS_DATA_AS,
	GPRS_DATA_GS,
	GPRS_DATA_CWN, // CURRENT WAYPOINT NO.
	GPRS_DATA_NWN,//NEXT WAYPOINT NO.
	GPRS_DATA_DTW,//DISTANCE TO WAYPOINT
	GPRS_DATA_DTL,//DISTANCE TO LAUNCH
	GPRS_DATA_ALT,//ALTITUDE
	GPRS_DATA_LAT,
	GPRS_DATA_LNG,
	GPRS_DATA_HARTBEAT
};


#define MATCH_STRING_MAX	4
typedef struct _SIM900A_AT_MISSION_
{
	uint8_t ui8_AT_Cmd_Index;
	uint8_t ui8_Previous_AT_Cmd_Index;
	uint8_t ui8_Execute_Finish : 1;
	uint8_t ui8_Begin : 1;
	uint8_t ui8_First_Execute_Flag : 2;
	uint8_t ui8_Match_Index : 4;
	uint8_t ui8_Overtime_Retransmision : 1;
	uint8_t ui8_Reserve : 7;
	uint8_t ui8_Match_Num;
	uint8_t ui8_Execute_Times;//指示本次任务发送了多少次AT指令
	uint8_t ui8_Timeout_Times;
	uint32_t ui32_Timeout_ms;
	const char *p_ch_Result_Match[MATCH_STRING_MAX];
}SIM900A_AT_MISSION, *P_SIM900A_AT_MISSION;

enum _AT_CMD_INDEX_
{
	AT_CMD_INDEX = 0,
	AT_CIPMODE_CMD_INDEX,
	AT_CIPCCFG_CMD_INDEX,
	AT_CSTT_CMD_INDEX,
	AT_CIICR_CMD_INDEX,
	AT_CIFSR_CMD_INDEX,
	//AT_CLPORT_CMD_INDEX,
	AT_CIPSTART_CMD_INDEX,
	AT_CIPCLOSE_CMD_INDEX,
	AT_CIPSHUT_CMD_INDEX,
	AT_CIPSTATUS_CMD_INDEX,
	AT_COMMAND_MODE_CMD_INDEX,
	AT_DATA_MODE_CMD_INDEX
};

BOOL SIM900A_Init(AP_HAL::UARTDriver *_p_ud_Uart, AP_HAL::Scheduler *_p_sch_Scheduler);
BOOL SIM900A_Gprs_Init(void);
void SIM900A_Send_Byte(BYTE by_Data_To_Send);
void SIM900A_Send_String(const char *p_ch_String);
void SIM900A_Send_Chars(const char *p_ch_String, uint16_t ui16_Size);
int16_t SIM900A_Available(void);
int16_t SIM900A_Read(void);
void SIM900A_Flush(void);

void SIM900A_Update_AT_Mission(int8_t i8_AT_Cmd_Index,
	const char *p_ch_String_To_Match_1 = NULL,
	const char *p_ch_String_To_Match_2 = NULL,
	const char *p_ch_String_To_Match_3 = NULL,
	uint32_t ui32_Timeout_ms = 2000,
	uint8_t ui8_Overtime_Retransmision = 1);

void SIM900A_Send_Data_Protol(uint32_t ui32_Data_To_Send, GPRS_DATA_FLAG gpf_Flag);
BOOL SIM900A_Try_Capture_Gprs_Data_Packet(void);
P_GPRS_DATA_PACKET_INFO SIM900A_Get_Gprs_Packet_Info(void);
BOOL SIM900A_Parse_Gprs_Data(void);

BOOL SIM900A_Execute_AT_Mission(void);
SIM900A_AT_MISSION* SIM900A_Get_AT_Mission(void);
BOOL SIM900A_Try_Gprs_Connect(void);
int SIM900A_GPRS_State_Check(void);


#define SIM900A_Recv_Cleanup(SIM900A_Read_Func_Addr) \
for (int _s_r_c_j = 0; _s_r_c_j < 50; _s_r_c_j++)\
{\
	if (SIM900A_Read_Func_Addr() != -1)\
	{\
		_s_r_c_j = 0;\
	}\
}



#endif
