#include <stdio.h>
#include "SIM900A_Driver.h"

//IP  INITIAL
//TCP CLOSE

#define DESTINATION_IP 211.149.208.125
#define DESTINATION_COM 5990

char const AT_CMD[] 						= {"AT"};
char const AT_CIPMODE_CMD[] 		= {"AT+CIPMODE=1"};
char const AT_CIPCCFG_CMD[] 		= {"AT+CIPCCFG=3,2,1000,1"};
char const AT_CSTT_CMD[] 			= {"AT+CSTT=\"CMNET\""};
char const AT_CIICR_CMD[] 			= {"AT+CIICR"};
char const AT_CIFSR_CMD[] 			= {"AT+CIFSR"};
//char code AT_CLPORT_CMD[] 		= {"AT+CLPORT=\"TCP\",\"10000\""};
char const AT_CIPSTART_CMD[] 	= {"AT+CIPSTART=\"TCP\",\"211.149.208.125\",5990"};
char const AT_CIPCLOSE_CMD[] 	= {"AT+CIPCLOSE=1"};
char const AT_CIPSHUT_CMD[] 		= {"AT+CIPSHUT"};
char const AT_CIPSTATUS_CMD[] 	= {"AT+CIPSTATUS"};
char const AT_COMMAND_MODE_CMD[] = "+++";
char const AT_DATA_MODE_CMD[] = "ATO";

char const *P_AT_CMD_LIST[] = 
{
	AT_CMD,
	AT_CIPMODE_CMD,
	AT_CIPCCFG_CMD,
	AT_CSTT_CMD,
	AT_CIICR_CMD,
	AT_CIFSR_CMD,
	//AT_CLPORT_CMD,
	AT_CIPSTART_CMD,
	AT_CIPCLOSE_CMD,
	AT_CIPSHUT_CMD,
	AT_CIPSTATUS_CMD,
	AT_COMMAND_MODE_CMD,
	AT_DATA_MODE_CMD,
};


AP_HAL::UARTDriver *p_ud_Uart;
AP_HAL::Scheduler *p_sch_Scheduler;
uint32_t u32_Timer = 0;
uint32_t ui32_Timer_1 = 0;

SIM900A_GPRS_STATE sgs_Gprs_State;
SIM900A_AT_MISSION sam_AT_Mission;
GPRS_DATA_PACKET_INFO  gdps_Gprs_Packet_Info;

BOOL SIM900A_Init(AP_HAL::UARTDriver *_p_ud_Uart, AP_HAL::Scheduler *_p_sch_Scheduler)
{
	p_sch_Scheduler = _p_sch_Scheduler;
	p_ud_Uart = _p_ud_Uart;

	u32_Timer = 0;
	ui32_Timer_1 = 0;

	sgs_Gprs_State.ui16_Gprs_Init = 0;
	sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
	sgs_Gprs_State.ui16_Server_Hartbeat_State = 1;
	sgs_Gprs_State.ui16_Station_Hartbeat_State = 1;
	sgs_Gprs_State.ui16_Connect_Timeout = 0;
	sgs_Gprs_State.ui16_Server_Hartbeat_Timeout = 0;
	sgs_Gprs_State.ui16_Server_Hartbeat_Timeout_Count = 0;
	sgs_Gprs_State.ui16_First_Connect = 1;
	sgs_Gprs_State.ui16_Recv_New_Server_Hartbeat = 0;
	sgs_Gprs_State.ui16_Gprs_Environment_Init = 0;

	gdps_Gprs_Packet_Info.ui8_Find_E = 0;
	gdps_Gprs_Packet_Info.ui8_Find_ESC = 0;
	gdps_Gprs_Packet_Info.ui8_Recv_End = 0;
	gdps_Gprs_Packet_Info.ui8_Recv_First = 0;
	gdps_Gprs_Packet_Info.ui8_Reserve = 0;
	gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index = 0;

	SIM900A_Update_AT_Mission(AT_COMMAND_MODE_CMD_INDEX, "+++");

	return FALSE;
}

BOOL SIM900A_Gprs_Init(void)
{
	if (sgs_Gprs_State.ui16_Gprs_Environment_Init)
	{
		return TRUE;
	}

	if (SIM900A_Execute_AT_Mission())
	{
		switch (sam_AT_Mission.ui8_AT_Cmd_Index)
		{
		case AT_COMMAND_MODE_CMD_INDEX:
		case AT_CMD_INDEX:
			SIM900A_Update_AT_Mission(AT_CIPSTATUS_CMD_INDEX, "CONNECTING");
			break;
		case AT_CIPSHUT_CMD_INDEX:
			//SIM900A_Send_String("AT_CIPMODE_CMD_INDEX:\n");
			SIM900A_Update_AT_Mission(AT_CIPMODE_CMD_INDEX, "OK");
			break;
		case AT_CIPMODE_CMD_INDEX:
			//SIM900A_Send_String("AT_CIPCCFG_CMD_INDEX:\n");
			SIM900A_Update_AT_Mission(AT_CIPCCFG_CMD_INDEX);
			break;
		case AT_CIPCCFG_CMD_INDEX:
			//SIM900A_Send_String("AT_CSTT_CMD_INDEX:\n");
			SIM900A_Update_AT_Mission(AT_CSTT_CMD_INDEX, "OK");
			break;
		case AT_CSTT_CMD_INDEX:
			//SIM900A_Send_String("AT_CIICR_CMD_INDEX:\n");
			SIM900A_Update_AT_Mission(AT_CIICR_CMD_INDEX, "OK", NULL, NULL, 10000, 0);
			break;
		case AT_CIICR_CMD_INDEX:
			//SIM900A_Send_String("AT_CIFSR_CMD_INDEX:\n");
			SIM900A_Update_AT_Mission(AT_CIFSR_CMD_INDEX,  ".");
			break;
		case AT_CIFSR_CMD_INDEX:
		case AT_CIPCLOSE_CMD_INDEX:
			//sgs_Gprs_State.ui16_Gprs_Init = 1;
			//SIM900A_Send_String("GPRS INIT FINISH\n");
			SIM900A_Update_AT_Mission(AT_CIPSTART_CMD_INDEX, "OK", "CONNECT", NULL, 10000, 0);
			return TRUE;
			break;
		case AT_CIPSTART_CMD_INDEX:
			//SIM900A_Send_String("GPRS INIT FINISH\n");
			break;
		case AT_CIPSTATUS_CMD_INDEX:
			if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CONNECTING"))
			{
				if ((sgs_Gprs_State.ui16_First_Connect) || 
					(sgs_Gprs_State.ui16_Server_Hartbeat_Timeout_Count >= GPRS_SERVER_HARTBEAT_TIMEOUT_ALLOW_TIMES) 
					|| (sgs_Gprs_State.ui16_Connect_Timeout))
				{
					sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
					SIM900A_Update_AT_Mission(AT_CIPCLOSE_CMD_INDEX, "CLOSE", NULL, NULL, 3000);
				}
			}
			else if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CONNECT"))
			{
				if ((sgs_Gprs_State.ui16_First_Connect) || 
					(sgs_Gprs_State.ui16_Server_Hartbeat_Timeout_Count >= GPRS_SERVER_HARTBEAT_TIMEOUT_ALLOW_TIMES) || 
					(sgs_Gprs_State.ui16_Connect_Timeout))
				{
					sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
					SIM900A_Update_AT_Mission(AT_CIPCLOSE_CMD_INDEX, "CLOSE", NULL, NULL, 3000);
				}
				else
				{
					SIM900A_Update_AT_Mission(AT_DATA_MODE_CMD_INDEX, "CONNECT");
					return TRUE;
				}
			}
			else if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CLOSED"))
			{
				//sgs_Gprs_State.ui16_Gprs_Init = 1;
				//SIM900A_Send_String("GPRS INIT FINISH\n");
				SIM900A_Update_AT_Mission(AT_CIPSTART_CMD_INDEX, "OK", "CONNECT", NULL, 10000, 0);
				return TRUE;
			}
			else if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "INITIAL"))
			{
				//sgs_Gprs_State.ui16_Gprs_Init = 0;
				SIM900A_Update_AT_Mission(AT_CIPSHUT_CMD_INDEX, "SHUT", "OK");
			}
			break;
		default:
			break;
		}
	}// end if (SIM900A_Execute_AT_Mission())
	else
	{
		switch (sam_AT_Mission.ui8_AT_Cmd_Index)
		{
		case AT_CMD_INDEX:
			if (sam_AT_Mission.ui8_Timeout_Times == 4)
			{
				SIM900A_Update_AT_Mission(AT_COMMAND_MODE_CMD_INDEX, "+++");
			}
			break;
		case AT_COMMAND_MODE_CMD_INDEX:
			if (sam_AT_Mission.ui8_Timeout_Times == 4)
			{
				SIM900A_Update_AT_Mission(AT_CMD_INDEX, "OK");
			}
			break;
		case AT_CIPSTATUS_CMD_INDEX :
			if (sam_AT_Mission.ui8_Timeout_Times >= 1)
			{
				//SIM900A_Send_String("AT_CIPSTATUS_CMD_INDEX timeout\r\n");
				if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CONNECTING"))
				{
					SIM900A_Update_AT_Mission(AT_CIPSTATUS_CMD_INDEX, "CONNECT");
				}
				else if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CONNECT"))
				{
					SIM900A_Update_AT_Mission(AT_CIPSTATUS_CMD_INDEX, "CLOSED");
				}
				else if (strstr(sam_AT_Mission.p_ch_Result_Match[1], "CLOSED"))
				{
					SIM900A_Update_AT_Mission(AT_CIPSTATUS_CMD_INDEX, "INITIAL");
				}
				else
				{
					sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
					SIM900A_Update_AT_Mission(AT_CIPSHUT_CMD_INDEX, "SHUT", "OK");
				}
			}//  if (sam_AT_Mission.ui8_Timeout_Times >= 1)
			break; // end case AT_CIPSTATUS_CMD_INDEX :
		case AT_CIICR_CMD_INDEX:
			if (sam_AT_Mission.ui8_Timeout_Times >= 1)
			{
				SIM900A_Update_AT_Mission(AT_CMD_INDEX, "OK");
			}
			break;
		default:
			if (sam_AT_Mission.ui8_Timeout_Times >= 3)
			{
				SIM900A_Update_AT_Mission(AT_CMD_INDEX, "OK");
			}
			break;
		}
	}


	return FALSE;
}


int SIM900A_GPRS_State_Check(void)
{
	if (!sgs_Gprs_State.ui16_Gprs_Environment_Init)
	{
		//SIM900A_Send_String("begin Gprs_Environment_Init: \r\n");
		//p_sch_Scheduler->delay(2000);

		if (SIM900A_Gprs_Init())
		{
			//SIM900A_Send_String("finish Gprs_Environment_Init\r\n");
			sgs_Gprs_State.ui16_Gprs_Environment_Init = 1;
			
			sgs_Gprs_State.ui16_Server_Hartbeat_State = 2;
			sgs_Gprs_State.ui16_Connect_Timeout = 0;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout = 0;
			ui32_Timer_1 = p_sch_Scheduler->millis();

			if (sam_AT_Mission.ui8_AT_Cmd_Index == AT_CIPSTART_CMD_INDEX)
			{
				sgs_Gprs_State.ui16_Gprs_Init = 0;
			}
		}
		else
		{
			return 0;
		}
	}

	if (!sgs_Gprs_State.ui16_Gprs_Init)
	{
		//SIM900A_Send_String("begin Gprs_Init: \r\n");
		//p_sch_Scheduler->delay(2000);

		if (SIM900A_Try_Gprs_Connect())
		{
			//SIM900A_Send_String("finish begin Gprs_Init\r\n");
			//sgs_Gprs_State.ui16_Gprs_Connect_State = 1;

			sgs_Gprs_State.ui16_Gprs_Init = 1;
			sgs_Gprs_State.ui16_Server_Hartbeat_State = 0;
			sgs_Gprs_State.ui16_Connect_Timeout = 0;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout = 0;
			ui32_Timer_1 = p_sch_Scheduler->millis();
		}
		else
		{
			return 0;
		}
	}
	
	if (SIM900A_Try_Capture_Gprs_Data_Packet())
	{
		if ((sgs_Gprs_State.ui16_Recv_New_Server_Hartbeat))
		{
			SIM900A_Send_String("RECV HARTBEAT\r\n");
			sgs_Gprs_State.ui16_Gprs_Connect_State = 1;
			sgs_Gprs_State.ui16_First_Connect = 0;
			sgs_Gprs_State.ui16_Server_Hartbeat_State = 2;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout = 0;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout_Count = 0;
			sgs_Gprs_State.ui16_Recv_New_Server_Hartbeat = 0;
			ui32_Timer_1 = p_sch_Scheduler->millis();
		}
	}

	//every GPRS_SERVER_HARTBEAT_TIMEOUT_MS ms to check the GPRS connect state
	if ((!(sgs_Gprs_State.ui16_First_Connect)) && sgs_Gprs_State.ui16_Server_Hartbeat_State >= 1)
	{
		if ((p_sch_Scheduler->millis() - ui32_Timer_1) >= GPRS_SERVER_HARTBEAT_TIMEOUT_MS)
		{
			sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
			ui32_Timer_1 = p_sch_Scheduler->millis();
			sgs_Gprs_State.ui16_Server_Hartbeat_State = 0;
			//SIM900A_Send_String("NO SEND +++\r\n");
			//SIM900A_Send_String("SERVER HARTBEAT TIMEOUT\r\n");
		}
	}
	else
	{
		if ((p_sch_Scheduler->millis() - ui32_Timer_1) >= GPRS_SERVER_HARTBEAT_TIMEOUT_MS)
		{
			ui32_Timer_1 = p_sch_Scheduler->millis();
			sgs_Gprs_State.ui16_Gprs_Environment_Init = 0;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout = 1;
			sgs_Gprs_State.ui16_Server_Hartbeat_Timeout_Count++;
			SIM900A_Update_AT_Mission(AT_COMMAND_MODE_CMD_INDEX, "+++");
			SIM900A_Send_String("SEND +++\r\n");
			SIM900A_Send_String("SERVER HARTBEAT TIMEOUT\r\n");
			return 0;
		}
	}

	return sgs_Gprs_State.ui16_Gprs_Connect_State;
}

BOOL SIM900A_Try_Gprs_Connect(void)
{
	if (SIM900A_Execute_AT_Mission())
	{
		switch (sam_AT_Mission.ui8_AT_Cmd_Index)
		{
		case AT_CIPCLOSE_CMD_INDEX:
			break;
		case AT_CIPSTART_CMD_INDEX:
			return TRUE;
			break;
		case AT_COMMAND_MODE_CMD_INDEX:
			break;
		case AT_CIPSTATUS_CMD_INDEX:
			break;
		case AT_DATA_MODE_CMD_INDEX:
			return TRUE;
			break;
		}
	} // end if (SIM900A_Execute_AT_Mission())
	else// else
	{
		switch (sam_AT_Mission.ui8_AT_Cmd_Index)
		{
		case AT_CIPCLOSE_CMD_INDEX:
			break;
		case AT_CIPSTART_CMD_INDEX:
			if (sam_AT_Mission.ui8_Timeout_Times >= 1)
			{
				return TRUE;
				//SIM900A_Send_String("AT_CIPSTART_CMD_INDEX TIMEOUT");
				//sgs_Gprs_State.ui16_Gprs_Init = 0;
				//sgs_Gprs_State.ui16_Connect_Timeout = 1;
				//sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
				//SIM900A_Update_AT_Mission(AT_COMMAND_MODE_CMD_INDEX, "+++");
				//SIM900A_Update_AT_Mission(AT_CIPCLOSE_CMD_INDEX, "CLOSE", NULL, NULL, 5000);
			}
			break;
		case AT_DATA_MODE_CMD_INDEX:
				if (sam_AT_Mission.ui8_Timeout_Times >= 3)
				{
					sgs_Gprs_State.ui16_Gprs_Init = 0;
					sgs_Gprs_State.ui16_Connect_Timeout = 1;
					sgs_Gprs_State.ui16_Gprs_Connect_State = 0;
					SIM900A_Update_AT_Mission(AT_CIPCLOSE_CMD_INDEX, "CLOSE", NULL, NULL, 5000);
				}
				break;
		case AT_COMMAND_MODE_CMD_INDEX:
			break;
		case AT_CIPSTATUS_CMD_INDEX:
			break;
		default:
			break;
		}// end switch (sam_AT_Mission.ui8_AT_Cmd_Index)
	 }// end if (SIM900A_Execute_AT_Mission()) : else

	return FALSE;
}


SIM900A_AT_MISSION* SIM900A_Get_AT_Mission(void)
{
	return &sam_AT_Mission;
}

void SIM900A_Update_AT_Mission(int8_t i8_AT_Cmd_Index, 
	const char *p_ch_String_To_Match_1,
	const char *p_ch_String_To_Match_2,
	const char *p_ch_String_To_Match_3,
	uint32_t ui32_Timeout_ms,
	uint8_t ui8_Overtime_Retransmision)
{
	sam_AT_Mission.ui8_AT_Cmd_Index = i8_AT_Cmd_Index;
	sam_AT_Mission.ui8_Begin = 1; 
	sam_AT_Mission.ui8_Execute_Finish = 0; 
	sam_AT_Mission.ui8_Execute_Times = 0; 
	sam_AT_Mission.p_ch_Result_Match[0] = P_AT_CMD_LIST[i8_AT_Cmd_Index];
	sam_AT_Mission.p_ch_Result_Match[1] = p_ch_String_To_Match_1;
	sam_AT_Mission.p_ch_Result_Match[2] = p_ch_String_To_Match_2;
	sam_AT_Mission.p_ch_Result_Match[3] = p_ch_String_To_Match_3;
	sam_AT_Mission.ui8_Match_Index = 0;
	sam_AT_Mission.ui8_Match_Num = 0; 
	sam_AT_Mission.ui8_Timeout_Times = 0;
	sam_AT_Mission.ui8_First_Execute_Flag = 3;
	sam_AT_Mission.ui32_Timeout_ms = ui32_Timeout_ms;
	sam_AT_Mission.ui8_Overtime_Retransmision = ui8_Overtime_Retransmision;

	//char chstr[3];
	//SIM900A_Send_String("\r\n");
	//SIM900A_Send_String("Overtime_Retransmision: ");
	//sprintf(chstr, "%d", ui8_Overtime_Retransmision);
	//SIM900A_Send_String(chstr);
	//SIM900A_Send_String("\r\n");

	return;
}


BOOL SIM900A_Execute_AT_Mission(void)
{
	int16_t i16_Bytes = 0;
	int j = 0;
	char ch_Read;
	char ch_Length;
	
	if ((sam_AT_Mission.ui8_First_Execute_Flag > 0))
	{
		if ((i16_Bytes = SIM900A_Available()))
		{
			for (j = 0; j < (int)i16_Bytes; j++)
			{
				SIM900A_Read();
			}
		}

		sam_AT_Mission.ui8_First_Execute_Flag--;
		u32_Timer = p_sch_Scheduler->millis();

		return FALSE;
	}

	if ((sam_AT_Mission.ui8_Execute_Finish))
	{
		//SIM900A_Send_String("NO AT CMD \r\n");
		sam_AT_Mission.ui8_Execute_Times++;
		sam_AT_Mission.ui8_Match_Index = 0;
		sam_AT_Mission.ui8_Begin = 1;
		sam_AT_Mission.ui8_Execute_Finish = 0;
		u32_Timer = p_sch_Scheduler->millis();
	}

	if (!sam_AT_Mission.ui8_Begin)
	{
		if ((p_sch_Scheduler->millis() - u32_Timer) >= sam_AT_Mission.ui32_Timeout_ms)
		{
			u32_Timer = p_sch_Scheduler->millis();
			sam_AT_Mission.ui8_Timeout_Times++;

			if ((sam_AT_Mission.ui8_Overtime_Retransmision))
			{
				sam_AT_Mission.ui8_Begin = 1;
				sam_AT_Mission.ui8_Match_Index = 0;
			}
		}
	}//end if (ch_Global_Begin)

	if (sam_AT_Mission.ui8_Begin)
	{
		sam_AT_Mission.ui8_Begin = 0;
		if (sam_AT_Mission.ui8_Match_Index == 0)
		{
			//SIM900A_Send_String("Begin\r\n");
			SIM900A_Send_String(P_AT_CMD_LIST[sam_AT_Mission.ui8_AT_Cmd_Index]);
			if (P_AT_CMD_LIST[sam_AT_Mission.ui8_AT_Cmd_Index][0] != '+')
			{
				SIM900A_Send_String("\r\n");
			}
		}
	}// end if (sam_AT_Mission.ui8_Begin)

	if ((i16_Bytes = SIM900A_Available()))
	{
		ch_Length = strlen(sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index]);
		//char chstr[3];
		//SIM900A_Send_String("\r\n");
		//SIM900A_Send_String("MATCH: ");
		//SIM900A_Send_String(sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index]);
		//SIM900A_Send_String("\r\n");

		//SIM900A_Send_String("\r\n");
		//SIM900A_Send_String("Recv :");

		for (j = 0; j < (int)i16_Bytes; j++)
		{
			ch_Read = (char)SIM900A_Read();
			//SIM900A_Send_Byte(ch_Read);

			if (ch_Read == sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index][sam_AT_Mission.ui8_Match_Num])
			{
				sam_AT_Mission.ui8_Match_Num++;
				if(ch_Read == sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index][ch_Length - 1])
				{
					sam_AT_Mission.ui8_Match_Num = 0;
					sam_AT_Mission.ui8_Match_Index++;
					if (sam_AT_Mission.ui8_Match_Index >= MATCH_STRING_MAX || sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index] == NULL)
					{
						sam_AT_Mission.ui8_Match_Index = MATCH_STRING_MAX;
						sam_AT_Mission.ui8_Execute_Finish = 1;
						return TRUE;
					}
					ch_Length = strlen(sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index]);
				}
			}// end if (ch_Read == sam_AT_Mission.p_ch_Result_Match_1[sam_AT_Mission.ui8_Match_Num])
			else
			{
				if (ch_Read == sam_AT_Mission.p_ch_Result_Match[sam_AT_Mission.ui8_Match_Index][0])
				{
					sam_AT_Mission.ui8_Match_Num = 1;
				}
				else
				{
					sam_AT_Mission.ui8_Match_Num = 0;
				}
			}
		}// for (j = 0; j < (int)i16_Bytes; j++)
		//SIM900A_Send_String("\r\n");
	}//if (i16_Bytes = SIM900A_Available())

	return FALSE;
}

BOOL SIM900A_Try_Capture_Gprs_Data_Packet(void)
{
	int16_t i16_Bytes = 0;
	int i = 0;
	char ch_Read;

	if ((i16_Bytes = SIM900A_Available()))
	{
		for (i = 0; i < (int)i16_Bytes; i++)
		{
			ch_Read = (char)SIM900A_Read();
			if (!gdps_Gprs_Packet_Info.ui8_Recv_First)
			{
				if (ch_Read == GPRS_DATA_START_CHARACTER)
				{
					gdps_Gprs_Packet_Info.ui8_Recv_First = 1;
					gdps_Gprs_Packet_Info.ui8_Recv_End = 0;
					gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff[gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index] = ch_Read;
				}
				continue;
			}

			if (!gdps_Gprs_Packet_Info.ui8_Find_E && !gdps_Gprs_Packet_Info.ui8_Find_ESC && ch_Read == GPRS_DATA_ESC)
			{
				gdps_Gprs_Packet_Info.ui8_Find_ESC = 1;
				continue;
			}

			gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index++;
			if (gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index >= GPRS_DATA_PACKET_BUFF_SIZE)
			{
				i--;
				gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index = 0;
				gdps_Gprs_Packet_Info.ui8_Recv_End = 0;
				gdps_Gprs_Packet_Info.ui8_Recv_First = 0;
				gdps_Gprs_Packet_Info.ui8_Find_E = 0;
				continue;
			}

			if (gdps_Gprs_Packet_Info.ui8_Find_ESC)
			{
				gdps_Gprs_Packet_Info.ui8_Find_ESC = 0;
				gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff[gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index] = ch_Read;
				continue;
			}

			if (ch_Read == GPRS_DATA_START_CHARACTER && !gdps_Gprs_Packet_Info.ui8_Find_E)
			{
				gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index = 0;
				gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff[gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index] = ch_Read;
				continue;
			}

			if (gdps_Gprs_Packet_Info.ui8_Recv_First)
			{
				gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff[gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index] = ch_Read;
				if (!gdps_Gprs_Packet_Info.ui8_Find_E && ch_Read == GPRS_DATA_END_CHARACTER)
				{
					gdps_Gprs_Packet_Info.ui8_Find_E = 1;
					continue;
				}

				if (gdps_Gprs_Packet_Info.ui8_Find_E && ch_Read != GPRS_DATA_END_CHARACTER)
				{
					SIM900A_Send_String("Recv data:\r\n");
					SIM900A_Send_Chars(gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff, gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index);
					SIM900A_Send_String("\r\n");

					i--;
					gdps_Gprs_Packet_Info.ui8_Gprs_Data_Buff_Index = 0;
					gdps_Gprs_Packet_Info.ui8_Recv_End = 1;
					gdps_Gprs_Packet_Info.ui8_Recv_First = 0;
					gdps_Gprs_Packet_Info.ui8_Find_E = 0;

					SIM900A_Parse_Gprs_Data();

					return TRUE;
				}
			}// end if (n_Recv_First && !n_Recv_End)
		}// for (j = 0; j < (int)i16_Bytes; j++)
	}//if (i16_Bytes = SIM900A_Available())

	return FALSE;
}

P_GPRS_DATA_PACKET_INFO SIM900A_Get_Gprs_Packet_Info(void)
{
	return &gdps_Gprs_Packet_Info;
}

BOOL SIM900A_Parse_Gprs_Data(void)
{
	P_GPRS_DATA_PACKET p_ch_Buff = (P_GPRS_DATA_PACKET)gdps_Gprs_Packet_Info.ch_Gprs_Data_Buff;

	switch (p_ch_Buff->ch_Flag)
	{
	case GPRS_DATA_FLY_MODE:
		break;
	case GPRS_DATA_NWN: //NEXT WAYPOINT NO.
		break;
	case GPRS_DATA_HARTBEAT:
		sgs_Gprs_State.ui16_Recv_New_Server_Hartbeat = 1;
		if (p_ch_Buff->ui8_Data[0] == '1')
		{
			sgs_Gprs_State.ui16_Station_Hartbeat_State = 2;
		}
		break;
	default:
		break;
	}

	return FALSE;
}


BOOL Enter_Data_Mode(void)
{

	return FALSE;
}

BOOL Enter_Cmd_Mode(void)
{

	return FALSE;
}


BOOL Check_IP_Status(void)
{

	return FALSE;
}

void SIM900A_Send_Data_Protol(uint32_t ui32_Data_To_Send, GPRS_DATA_FLAG gpf_Flag)
{
	BYTE ch_Index = 0;
	char ch_Send[12];
	uint16_t ui16_checksum = 0;

	ch_Send[ch_Index++] = 'S';
	ch_Send[ch_Index++] = (BYTE)gpf_Flag;
	ch_Send[ch_Index++] = (BYTE)(ui32_Data_To_Send);
	ch_Send[ch_Index++] = (BYTE)(ui32_Data_To_Send >> 8);
	ch_Send[ch_Index++] = (BYTE)(ui32_Data_To_Send >> 16);
	ch_Send[ch_Index++] = (BYTE)(ui32_Data_To_Send >> 24);
	ch_Send[ch_Index++] = (BYTE)ui16_checksum;
	ch_Send[ch_Index++] = (BYTE)(ui16_checksum >> 8);
	ch_Send[ch_Index++] = 'E';

	//SIM900A_Send_String("AT+CIPSEND\r\n");
	SIM900A_Send_Chars(ch_Send, ch_Index);
	//SIM900A_Send_Byte(0X1A);

	return;
}

void SIM900A_Send_Byte(BYTE by_Data_To_Send)
{
	p_ud_Uart->write(by_Data_To_Send);

	return;
}

void SIM900A_Send_String(const char *p_ch_String)
{
	p_ud_Uart->write(p_ch_String);

	return;
}

void SIM900A_Send_Chars(const char *p_ch_String, uint16_t ui16_Size)
{
	p_ud_Uart->write((const uint8_t *)p_ch_String, (size_t)ui16_Size);

	return;
}

int16_t SIM900A_Available(void)
{
	return p_ud_Uart->available();
}

int16_t SIM900A_Read(void)
{
	return p_ud_Uart->read();
}

void SIM900A_Flush(void)
{
	p_ud_Uart->flush();

	return;
}







