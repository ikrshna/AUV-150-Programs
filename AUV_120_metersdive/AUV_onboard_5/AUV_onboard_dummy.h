#include "declarations.h"
#include "ComConfig.h"

#pragma comment(lib,"user32.lib")

#pragma comment (lib,"ws2_32.lib")

HANDLE hmutex_hc;
HANDLE hmutex_sc;
HANDLE hmutex_hc_phinsread;
HANDLE hmutex_sc_phinsread;

char sys_filename[120];
char mission_filename[120];
char battery_filename[120];

void tele_actuation();
void vt_actuation(void);
void ht_actuation(void);
void mt_actuation(void);

void get_altimetry(void);

int BatteryAD_Init(void);

void get_batt_status(void);

void Depth_Daq(void);

void dvl_daq(void);

HANDLE FLS_port_config(void);
int FLS_device_config(HANDLE);
int FLS_scan_log(HANDLE,char *);
void get_objrange_bearing();

void log_systemdata(char *);
void log_missionstatus(char *,char *);
void log_batterystatus(char *);

char phins_pkt[1500];
char head_data[20];
char pos_X_data[50];
char pos_Y_data[50];
char east_data[50];
char north_data[50];
char pitch_data[20];
char roll_data[20];
char latitude_c[20];
char longitude_c[20];

char UTM_zone;
double validate_north;
double validate_east;

int offset;

void Phins_Daq(void);
void position_data(char *);
void pitching_data(char *);
void latlong_data(char *);
void heading_data(char *);
void speed_data(char *);

BOOL SetPrivilege(HANDLE,LPCTSTR,BOOL);  
int AdjustPriv(void);

int ThrusterDA_Init(void);

float LeastSweepAngle(float,float);
int PID_HeadingCtrl(float,float*);

bool depth_err_computed;
