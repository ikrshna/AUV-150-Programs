//decarations.h
#include <iostream.h>
#include <stdio.h>
#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <process.h>
#include <sys/timeb.h>
#include <time.h>
#include <winsock2.h>
#include <string.h>
#include "adamtcp.h"
#include "driver.h"
#include <math.h>

#define MISS_START 12
#define MISS_ABORT 21
#define LIGHTS_ON 13
#define LIGHTS_OFF 31
#define SYS_HALT 14
#define SYS_REBOOT 41
#define MAN_ACTUATE 15

#define DIVE_SETTLING_TIME 10000

#define STATE int

SYSTEMTIME st;

#define PORT 8888
#define BUFFERSIZE 115


LPDWORD id_co;			
HANDLE co_hwnd;

LPDWORD id_ac;			
HANDLE ac_hwnd;

LPDWORD id_log;			
HANDLE log_hwnd;

LPDWORD id_dc;			
HANDLE dc_hwnd;

HANDLE altiHwnd;
LPDWORD alt_id;

HANDLE hdHwnd;
LPDWORD hd_id;

LPDWORD sg_id;
HANDLE sgHwnd;

LPDWORD id_wt;			
HANDLE wt_hwnd;

LPDWORD id_tt;			
HANDLE tt_hwnd;

LPDWORD id_dvs;
HANDLE dvs_hwnd;

LPDWORD id_batt;
HANDLE batt_hwnd;

LPDWORD id_fls;
HANDLE fls_hwnd;

LPDWORD id_dvl;
HANDLE dvl_hwnd;

DWORD exit_th;

//--------- battery DAQ variables ----------------//
//---  the IP address of 5000/TCP  ---
//--- please check the IP address for your 5000/TCP ---
#define   IPof5KTCP   "192.168.1.31"

//-------- default timeout ------
int     iConnectionTimeout=2000;
int     iSendTimeout=2000;
int     iReceiveTimeout=2000;

char    szSend[80];            // Command sended to 5000/TCP in ASCII format
char    szReceive[80];	       // Response received from 5000/TCP in ASCII format	

////////////////////////////////////////////////////

/// DA variables /////////
#define     MAX_DEVICES     100 

static      PT_AOVoltageOut ptAOVoltageOut;
static		PT_AOConfig		ptAOConfig;
static		PT_DioWriteBit	ptDioWriteBit;
static		PT_DioReadBit	ptDioReadBit;


LONG        DriverHandle = (LONG)NULL;          // driver handle		
//////////////////////////////////

unsigned long _stdcall heading_ctrl(void *);
unsigned long _stdcall surge_ctrl(void *);
//unsigned long _stdcall sway_ctrl(void *);
unsigned long _stdcall depth_Ctrl(void *data);
unsigned long _stdcall traj_timer(void *data);
//unsigned long _stdcall Altimeter_Daq(void *);
unsigned long _stdcall Watchdog_Timer(void *data);// monitoring the timeout of various phases of the mission 
unsigned long _stdcall Communicator(void *data);
unsigned long _stdcall Acoustic_comm(void *data);
//unsigned long _stdcall battMS(void*);
//unsigned long _stdcall fls(void*);
//unsigned long _stdcall dvl(void*);
//unsigned long _stdcall log_data(void *);


struct sensor_struct{ /////////////////// sensor data structure
	long double veh_east_pos;
	long double veh_north_pos;
	float veh_depth;
	float veh_alti;
	float veh_east_spd;
	float veh_north_spd;
	float curr_yaw;
	float veh_surge;
	float veh_sway;
	float veh_pitch;
	float veh_roll;
	float latitude;
	float longitude;
	float obs_rng;
	float obs_brng;
	float srg_err;
	float srg_err_old;
	float bank1;
	float bank2;
	float bank3;
	float bank4;
	float bank5;
	float bank6;
	float curr1;
	float curr2;
	int leak;
	int PS;
}sds;

struct ref_struct{	////////////////// reference data for controllers
	float yaw_des;
	float pitch_des;
	float depth_des;
	float sg_des; ////// total surge desired
	float speed_des;
	float sway_des;
}rds;

struct event_struct{
	bool des_depth_acc;	////// desired depth accomplished
	bool miss_timeout;
	bool int_traj_timeout;
	float bank1_charge;
	float bank2_charge;
	float bank3_charge;
	float bank4_charge;
	float bank5_charge;
	float bank6_charge;
	float leak_occ; /////// leak occurred
}eds;

struct cmd_struct{	////////////////// control signals and commands vector
	float signal;
	float P;
	float D;
	int head_tailtorque;
	int head_nosetorque;
	int sway_tailtorque;
	int sway_nose_torque;
	int depth_tailtorque;
	int depth_nosetorque;
}cds;

struct dvl_data{
	float dvl_roll;
	float dvl_head;
	float dvl_pitch;
	float vel_east;
	float vel_north;
	float vel_Z;
	float disp_east;
	float disp_north;
	float disp_Z;
	float dvl_alti;
}dds;

///// Globals ///////////////////////////
int mission_time;
float traj_sample_time;
bool repeat=true;
bool dive_complete=false;
float start_north;
float start_east;
float veh_res_disp;
float veh_res_disp_phins;
float veh_res_disp_dvl;
float veh_res_disp_dvlspd;
float traj_hold_time;
bool getdatafromfile=true;
int PS;
float depth_err;
float prev_signal;
bool ramp_correction=false;
bool MissionTimedOut;
bool SegmentTimedOut;
bool DiveInfeasible=false;
int msg=0;
char mission[31];
float start_dvleast;
float start_dvlnorth;


int ct_val;
float mt_val;

/// general message buffer
char gen_msg_buffer[100];
//// Acoustic buffer //////
char ac_buffer[100];