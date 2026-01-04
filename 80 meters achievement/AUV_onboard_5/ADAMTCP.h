#ifdef __cplusplus
     #define EXPORTS extern "C" __declspec (dllexport)
#else
     #define EXPORTS
#endif

/*============================================================================*/
/* This block is data structure                                               */
/*============================================================================*/
struct _StreamData
{
    WORD DIO[8];     // DI/DO data for Slot0, Slot1,...., Slot7
    WORD Slot0[8];   // AI/AO data for slot0
    WORD Slot1[8];   // AI/AO data for slot1
    WORD Slot2[8];   // AI/AO data for slot2
    WORD Slot3[8];   // AI/AO data for slot3
    WORD Slot4[8];   // AI/AO data for slot4
    WORD Slot5[8];   // AI/AO data for slot5
    WORD Slot6[8];   // AI/AO data for slot6
    WORD Slot7[8];   // AI/AO data for slot6
};   //StreamData,*pStreamData;

struct _AlarmInfo
{
	BYTE   bySlot;		   // the Slot of 5000/TCP which cause the alarm change	
	BYTE   byChannel;      // the Channel of 5000/TCP which cause the alarm change	
	BYTE   byAlarmType;    // 0: Low Alarm, 1: High Alarm
	BYTE   byAlarmStatus;  // 0: Alarm Off, 1: Alarm On
	BYTE   byIndexOf5KTCP; // indicate the index 5000/TCP which cause the alarm change, zero-based
	char   szIP[20];       // the IP address which cause the alarm change
	char   szDateTime[48]; // e.x 2001/09/23 10:12:34:567 (Year/Month/Day Hour:Minute:Second:mSecond)
};

/*============================================================================*/
/* This block is for ADAM 5000 TCP only                                       */
/*============================================================================*/
// return code
#define    ADAM5KTCP_NoError                      0
#define    ADAM5KTCP_StartupFailure               (-1)
#define    ADAM5KTCP_SocketFailure                (-2)
#define    ADAM5KTCP_UdpSocketFailure             (-3)
#define    ADAM5KTCP_SetTimeoutFailure            (-4)
#define    ADAM5KTCP_SendFailure                  (-5)
#define    ADAM5KTCP_ReceiveFailure               (-6)
#define    ADAM5KTCP_ExceedMaxFailure             (-7)
#define    ADAM5KTCP_CreateWsaEventFailure        (-8)
#define    ADAM5KTCP_ReadStreamDataFailure        (-9)
#define    ADAM5KTCP_InvalidIP                    (-10)
#define    ADAM5KTCP_ThisIPNotConnected           (-11)
#define    ADAM5KTCP_AlarmInfoEmpty               (-12)

// always receiving sream data in despite of if alarm change
#define    ADAM5KTCP_ReceiveStreamIngoreAlarm     0  
// only receiving sream data when any alarm status change
// for example:    Hi-Alarm On -> Off , Hi-Alarm Off ->On, 
//                 Lo-Alarm On -> Off , Lo-Alarm Off ->On
#define    ADAM5KTCP_ReceiveStreamWhenAlarm       1

// the functions for ADAM-5000/TCP only
EXPORTS int  CALLBACK ADAM5KTCP_Open(void);
EXPORTS void CALLBACK ADAM5KTCP_Close(void);
EXPORTS int  CALLBACK ADAM5KTCP_Connect(char szIP[], unsigned short port,
							                     int iConnectionTimeout, int iSendTimeout, 
							                     int iReceiveTimeout);
EXPORTS void CALLBACK ADAM5KTCP_Disconnect(void);
EXPORTS int  CALLBACK ADAM5KTCP_ModuleDisconnect(char szIP[]);
EXPORTS int  CALLBACK ADAM5KTCP_GetDLLVersion(void);
EXPORTS int  CALLBACK ADAM5KTCP_ReadReg(char szIP[], WORD wIDAddr,  WORD wStartAddress, WORD wCount, WORD wData[]); 
EXPORTS int  CALLBACK ADAM5KTCP_WriteReg(char szIP[], WORD wIDAddr, WORD wStartAddress, WORD wCount, WORD wData[]);
EXPORTS int  CALLBACK ADAM5KTCP_ReadCoil(char szIP[], WORD wIDAddr, WORD wStartAddress, WORD wCount, BYTE byData[]); 
EXPORTS int  CALLBACK ADAM5KTCP_WriteCoil(char szIP[], WORD wIDAddr,WORD wStartAddress, WORD wCount, BYTE byData[]);
EXPORTS int  CALLBACK ADAM5KTCP_SendReceive5KTCPCmd(char szIP[], char szSendToTCP[],  char szReceiveFromTCP[], 
							                        char szModbusSend[], char szModbusReceive[]);

//---- functions for Stream Data ---
EXPORTS int  CALLBACK ADAM5KTCP_Add5KTCPForStream(char szIP[]);
EXPORTS int  CALLBACK ADAM5KTCP_ReadStreamData(char szIP[], struct _StreamData  *pStreamData);
EXPORTS int  CALLBACK ADAM5KTCP_ReadAlarmInfo(struct _AlarmInfo  *pAlarmInfo);
EXPORTS int  CALLBACK ADAM5KTCP_StartStream(HANDLE *EventFromApp );
EXPORTS int  CALLBACK ADAM5KTCP_StopStream( );
EXPORTS int  CALLBACK ADAM5KTCP_SetStreamAlarmState(WORD wStreamAlarmState);
EXPORTS int  CALLBACK ADAM5KTCP_Debug(int *iMatchIndex, int *iReceiveCount, int *iThreadRun, 
										int *iTotalStream, char szFromIP[]);
										
//---- functions for UDP command ---										
EXPORTS int  CALLBACK ADAM5KTCP_UDPOpen(int iSendTimeout, int iReceiveTimeout);
EXPORTS int  CALLBACK ADAM5KTCP_UDPClose();
EXPORTS int  CALLBACK ADAM5KTCP_SendReceiveUDPCmd(char szIP[], char szSend[], char szReceive[]);


/*============================================================================*/
/*============================================================================*/
/* This bloack is for both ADAM 5000/6000 TCP                                 */
/*============================================================================*/
/*============================================================================*/
//--- define Gain -----
#define ADAMTCP_BI_10V			0    // -10V ~ 10V
#define ADAMTCP_BI_5V			1    // -5V ~ 5V 
#define ADAMTCP_BI_2Point5V		2    // -2.5V ~ 2.5V 
#define ADAMTCP_BI_1V			3    // -1V ~ 1V
#define ADAMTCP_BI_500mV		4    // -500mV ~ 500mV
#define ADAMTCP_BI_150mV		5    // -150mV ~ 150mV
#define ADAMTCP_BI_100mV		6    // -100mV ~ 100mV
#define ADAMTCP_BI_50mV			7    // -50mV ~ 50mV
#define ADAMTCP_BI_15mV			8    // -15mV ~ 15mV
#define ADAMTCP_UNI_4TO20mA		9    // 4 ~ 20mA
#define ADAMTCP_UNI_0TO20mA		10   // 0 ~ 20mA
#define ADAMTCP_BI_20mA			11   // -20 ~ 20mA

#define ADAMTCP_UNI_0TO100C		12	// 0~+100 C
#define ADAMTCP_UNI_0TO200C		13	// 0~+200 C
#define ADAMTCP_UNI_0TO400C		14	// 0~+400 C
#define ADAMTCP_BI_30TO120C		15	// -30~+120 C
#define ADAMTCP_BI_40TO160C		16	// -40~+160 C
#define ADAMTCP_BI_50TO150C		17	// -50~+150 C
#define ADAMTCP_BI_80TO120C		18	// -80~+100 C
#define ADAMTCP_BI_200C			19	// -200~+200 C

#define ADAMTCP_TypeB			20   // TypeB 500-1800'C
#define ADAMTCP_TypeE			21   // TypeE 0-1000'C
#define ADAMTCP_TypeJ			22   // TypeJ 0-760'C
#define ADAMTCP_TypeK			23   // TypeK 0-1370'C
#define ADAMTCP_TypeR			24   // TypeR 500-1750'C
#define ADAMTCP_TypeS			25   // TypeS 500-1750'C
#define ADAMTCP_TypeT			26   // TypeT -100 to 400'C

// return code
#define    ADAMTCP_NoError                      0
#define    ADAMTCP_StartupFailure               (-1)
#define    ADAMTCP_SocketFailure                (-2)
#define    ADAMTCP_UdpSocketFailure             (-3)
#define    ADAMTCP_SetTimeoutFailure            (-4)
#define    ADAMTCP_SendFailure                  (-5)
#define    ADAMTCP_ReceiveFailure               (-6)
#define    ADAMTCP_ExceedMaxFailure             (-7)
#define    ADAMTCP_CreateWsaEventFailure        (-8)
#define    ADAMTCP_ReadStreamDataFailure        (-9)
#define    ADAMTCP_InvalidIP                    (-10)
#define    ADAMTCP_ThisIPNotConnected           (-11)
#define    ADAMTCP_AlarmInfoEmpty               (-12)
#define    ADAMTCP_NotSupportModule             (-13)
#define    ADAMTCP_ExceedDONo                   (-14)
#define    ADAMTCP_InvalidRange                 (-15)
#define    ADAMTCP_EventError                   (-100)

// always receiving sream data in despite of if alarm change
#define    ADAMTCP_ReceiveStreamIngoreAlarm     0 

// only receiving sream data when any alarm status change
// for example:    Hi-Alarm On -> Off , Hi-Alarm Off ->On, 
//                 Lo-Alarm On -> Off , Lo-Alarm Off ->On
#define    ADAMTCP_ReceiveStreamWhenAlarm       1

// the functions for ADAM-5000/TCP and ADAM-6000/TCP
EXPORTS int  CALLBACK ADAMTCP_Open(void);
EXPORTS void CALLBACK ADAMTCP_Close(void);
EXPORTS int  CALLBACK ADAMTCP_Connect(char szIP[], unsigned short port,
							                 int iConnectionTimeout, int iSendTimeout, 
							                 int iReceiveTimeout);
EXPORTS void CALLBACK ADAMTCP_Disconnect(void);
EXPORTS int  CALLBACK ADAMTCP_ModuleDisconnect(char szIP[]);
EXPORTS int  CALLBACK ADAMTCP_GetDLLVersion(void);
EXPORTS int  CALLBACK ADAMTCP_ReadReg(char szIP[], WORD wIDAddr,  WORD wStartAddress, WORD wCount, WORD wData[]); 
EXPORTS int  CALLBACK ADAMTCP_WriteReg(char szIP[], WORD wIDAddr, WORD wStartAddress, WORD wCount, WORD wData[]);
EXPORTS int  CALLBACK ADAMTCP_ReadCoil(char szIP[], WORD wIDAddr, WORD wStartAddress, WORD wCount, BYTE byData[]); 
EXPORTS int  CALLBACK ADAMTCP_WriteCoil(char szIP[], WORD wIDAddr,WORD wStartAddress, WORD wCount, BYTE byData[]);
EXPORTS int  CALLBACK ADAMTCP_SendReceive5KTCPCmd(char szIP[],  
							                             char szSendToTCP[],  char szReceiveFromTCP[], 
							                             char szModbusSend[], char szModbusReceive[]);
EXPORTS int  CALLBACK ADAMTCP_SendReceive6KTCPCmd(char szIP[],  
							                             char szSendToTCP[],  char szReceiveFromTCP[], 
							                             char szModbusSend[], char szModbusReceive[]);
EXPORTS int  CALLBACK ADAMTCP_AddTCPForStream(char szIP[]);
EXPORTS int  CALLBACK ADAMTCP_ReadStreamData(char szIP[], struct _StreamData  *pStreamData);
EXPORTS int  CALLBACK ADAMTCP_ReadAlarmInfo(struct _AlarmInfo  *pAlarmInfo);
EXPORTS int  CALLBACK ADAMTCP_StartStream(HANDLE *EventFromApp );
EXPORTS int  CALLBACK ADAMTCP_StopStream( );
EXPORTS int  CALLBACK ADAMTCP_SetStreamAlarmState(WORD wStreamAlarmState);
EXPORTS int  CALLBACK ADAMTCP_Debug(int *iMatchIndex, int *iReceiveCount, int *iThreadRun, 
											   int *iTotalStream, char szFromIP[]);
EXPORTS int  CALLBACK ADAMTCP_UDPOpen(int iSendTimeout, int iReceiveTimeout);
EXPORTS int  CALLBACK ADAMTCP_UDPClose();
EXPORTS int  CALLBACK ADAMTCP_SendReceiveUDPCmd(char szIP[], char szSend[], char szReceive[]);
EXPORTS int  CALLBACK ADAMTCP_SendReceive6KUDPCmd(char szIP[], char szSend[], char szReceive[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KDIO(char szIP[],  WORD wModule,  WORD wIDAddr,
										          BYTE byDI[], BYTE byDO[]);
EXPORTS int  CALLBACK ADAMTCP_Write6KDO(char szIP[], WORD wModule, WORD wIDAddr, 
									             WORD wStartDO, WORD wCount, BYTE byDO[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KAI(char szIP[],  WORD wModule,  WORD wIDAddr, WORD wGain[],
									            WORD wHex[],  double dlValue[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KDIOMode(char szIP[], WORD wModule, WORD wIDAddr, 
                                            BYTE byDIStatus[], BYTE byDOStatus[]);
EXPORTS int  CALLBACK ADAMTCP_Write6KDIOMode(char szIP[], WORD wModule, WORD wIDAddr, 
                                           BYTE byDIStatus[], BYTE byDOStatus[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KSignalWidth(char szIP[], WORD wModule, WORD wIDAddr, 
                                                unsigned long ulLoWidth[], 
                                                unsigned long ulHiWidth[]);
EXPORTS int  CALLBACK ADAMTCP_Write6KSignalWidth(char szIP[], WORD wModule, WORD wIDAddr, 
                                               unsigned long ulLoWidth[], 
                                               unsigned long ulHiWidth[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KCounter(char szIP[], WORD wModule, WORD wIDAddr,  
                                          unsigned long  ulCounterValue[]);
EXPORTS int  CALLBACK ADAMTCP_Clear6KCounter(char szIP[], WORD wIDAddr,  
					                           WORD wChIndex, WORD wData);
EXPORTS int  CALLBACK ADAMTCP_Start6KCounter(char szIP[], WORD wIDAddr,  
					                         WORD wChIndex);
EXPORTS int  CALLBACK ADAMTCP_Stop6KCounter(char szIP[], WORD wIDAddr,  
					                         WORD wChIndex);
EXPORTS int  CALLBACK ADAMTCP_Clear6KDILatch(char szIP[], WORD wIDAddr,  
					                              WORD wChIndex);
EXPORTS int  CALLBACK ADAMTCP_Read6KPulseDelayWidth(char szIP[], 
													WORD wModule, WORD wIDAddr, 
													unsigned long ulLoPulseWidth[],
													unsigned long ulHiPulseWidth[],
													unsigned long ulLoDelayWidth[],
													unsigned long ulHiDelayWidth[]);
EXPORTS int  CALLBACK ADAMTCP_Read6KSinglePulseDelayWidth(char szIP[], 
													WORD wModule, WORD wIDAddr, WORD wChIndex,
													unsigned long *ulLoPulseWidth,
													unsigned long *ulHiPulseWidth,
													unsigned long *ulLoDelayWidth,
													unsigned long *ulHiDelayWidth);
EXPORTS int  CALLBACK ADAMTCP_Write6KPulseDelayWidth(char szIP[],
													WORD wModule, WORD wIDAddr, 
													unsigned long ulLoPulseWidth[], // Pulse Min output signal width at low level
													unsigned long ulHiPulseWidth[], // Pulse Min output signal width at high level
													unsigned long ulLoDelayWidth[], // Delay Min output signal width at low level
													unsigned long ulHiDelayWidth[]); // Delay Min output signal width at high level
EXPORTS int  CALLBACK ADAMTCP_Write6KPulseDelayWidthRuntime(char szIP[],
													WORD wModule, WORD wIDAddr, 
													unsigned long ulLoWidth[],
													unsigned long ulHiWidth[]);
EXPORTS int  CALLBACK ADAMTCP_PulseOutputCount(char szIP[], 
											   WORD wModule, WORD wIDAddr,
											   WORD wChannelIndex, 
											   unsigned long ulPulseCount);
			                                           
///////////////////////////////////////////////////////////////////////
// Function : Event Trigger
// Author   : Tony Liu
// Date     : Aug. 27, 2002
///////////////////////////////////////////////////////////////////////

#define ADAMTCP_TYPE_DIO		 0x1000
#define ADAMTCP_TYPE_AIO		 0x2000
#define ADAMTCP_TYPE_HIALARM	 0x3000
#define ADAMTCP_TYPE_LOALARM	 0x4000
#define ADAMTCP_TYPE_CONNECT	 0xF000
// status
#define ADAMTCP_STATUS_NO_CHANGE 0x000
#define ADAMTCP_STATUS_ON_TO_OFF 0x100
#define ADAMTCP_STATUS_OFF_TO_ON 0x200
#define ADAMTCP_STATUS_LOST		 0xFFF

EXPORTS void  CALLBACK ADAMTCP_InitializeEventTrigger();
EXPORTS void  CALLBACK ADAMTCP_TerminateEventTrigger();

EXPORTS int  CALLBACK ADAMTCP_AddModuleIP(char szIP[], int iAliveInterval);
EXPORTS int  CALLBACK ADAMTCP_StartEventTrigger(HANDLE EventFromApp);
EXPORTS int  CALLBACK ADAMTCP_StopEventTrigger();
EXPORTS int  CALLBACK ADAMTCP_ReadEventInfo(long *lIP, long *lSlot, long *lChannel, 
											long *lType, long *lStatus, long *lValue, long *lDateTime);
EXPORTS int  CALLBACK ADAMTCP_ReadEventPattern(long *lIP, unsigned char szPattern[], int *len);

