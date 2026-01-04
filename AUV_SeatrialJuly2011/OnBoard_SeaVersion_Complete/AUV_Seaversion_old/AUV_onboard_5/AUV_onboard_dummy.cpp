//AUV_onboard_dummy.cpp

#include "AUV_onboard_dummy.h"

int main(){
	
	int command;//command value as received from the operator
	int alti_chk_cnt;
	int count;
	bool alti_ok;
	int alti_prb=0;
	int tda_retval;
	int bda_retval;
	int FLS_retval;
	char fls_name[30];
	
	FILE *fp_missfile;		//file pointer to the mission file
	FILE *fp_UTMfile;		//file pointer to the UTM coords file
	char readline[23];
	char buff[23];
	int index=0;
	FILE *log_dp;
	float veh_depth=0;
	float veh_alti_ALT=0;
	float veh_alti_DVL=0;
	float actual_desired_surge;

	HANDLE FLS_port;

	// Processor related variables
	DWORD dwPriClass;
	SYSTEM_INFO sys_info;
	int proc;
	int no_procs; //number of processors on the system

	//initialise mission related flags
	bool mission_started;		//flag denoting the just-started status of mission
	bool dive_complete;			//flag denoting the status of diving process

	bool obj_found=false;
	bool obj_found_again=false;

	//Create the log files for logging system as well as mission data
		
	GetSystemTime(&st);
	sprintf(sys_filename,"stat%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);
	sprintf(mission_filename,"miss%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);
	sprintf(battery_filename,"batt%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);

	//Check the number of processors on the system
	GetSystemInfo(&sys_info);
	no_procs=sys_info.dwNumberOfProcessors;
	cout<<"MH:Number of processors on the system: "<<no_procs<<endl;
	sprintf(gen_msg_buffer,"MH:Number of processors on the system: %d",no_procs);
	log_missionstatus(sys_filename,gen_msg_buffer);

	//Configure Realtime property of the system
	if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)){
		cout<<"MH:Could not be configured as RealTime Process!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:Could not be configured as RealTime Process!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);	
		cout<<"MH:Program exiting:Premature Termination!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:Program exiting:Premature Termination!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
		return 0;
	}
	else{
		dwPriClass=GetPriorityClass(GetCurrentProcess());
		printf("current priority:0x%x\n",dwPriClass);
		cout<<"MH:Configured as RealTime Process!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:Configured as RealTime Process!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
	}

	//execute the main thread on processor ID: 0
	proc=SetThreadIdealProcessor(GetCurrentThread(),0);
	
	//initialise communication with the Surface Control	
	co_hwnd=CreateThread(NULL,0,Communicator,(void *)&command,NULL,id_co);//start communicator for receiving commands
	ac_hwnd=CreateThread(NULL,0,Acoustic_comm,(void *)&command,NULL,id_ac);//start communicator for receiving commands
	proc=SetThreadIdealProcessor(co_hwnd,0);
	proc=SetThreadIdealProcessor(ac_hwnd,0);

	// Read UTM Zone and validating coordinates
	fp_UTMfile=fopen("utm_coord.txt","r");
	if(fp_UTMfile==NULL){
		sprintf(gen_msg_buffer,"MH:Could not open UTM coords file\n");
		log_missionstatus(sys_filename,gen_msg_buffer);
		ExitThread(exit_th);
	}
	else{
		sprintf(gen_msg_buffer,"MH:Opened UTM coords file\n");
		log_missionstatus(sys_filename,gen_msg_buffer);
		fscanf(fp_UTMfile,"%c %lf %lf",&UTM_zone,&validate_north,&validate_east);
		cout<<validate_north<<validate_east<<endl;
		fclose(fp_UTMfile);
	}

	//########################################### Initialising starts ################################////
	//initialize devices and check for operator's commands
	tda_retval=ThrusterDA_Init();
	if(tda_retval){
		cout<<"MH:USB DA initialisation failed!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:USB DA initialisation failed!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
		ExitThread(exit_th);
	}
	else{
		cout<<"MH:USB DA initialisation complete!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:USB DA initialisation complete!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
		bda_retval=BatteryAD_Init();
		if(bda_retval){
			cout<<"MH:TCP/IP DA initialisation failed!!!"<<endl;
			sprintf(gen_msg_buffer,"MH:TCP/IP DA initialisation failed!!!");
			log_missionstatus(sys_filename,gen_msg_buffer);
			ptAOVoltageOut.OutputValue=0.00 ;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
			DRV_DeviceClose(&DriverHandle);
			ExitThread(exit_th);
		}
		else{
			cout<<"MH:TCP/IP DA initialisation complete!!!"<<endl;
			sprintf(gen_msg_buffer,"MH:TCP/IP DA initialisation complete!!!");
			log_missionstatus(sys_filename,gen_msg_buffer);
		}
	}
	cout<<"MH:Device Initialization complete!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Device Initialization complete!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);

	
	//initialize sensors and check out data acquisition
	while(start_north<=validate_north && start_east<=validate_east){
		Phins_Daq();
		start_north=sds.veh_north_pos;
		start_east=sds.veh_east_pos;
		_sleep(250);
	}
	cout<<start_north<<"\t"<<start_east<<endl;
	cout<<"MH:PHINS initialized and data consistent!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:PHINS initialized and data consistent!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);
	while(veh_depth==0){
		Depth_Daq();
		_sleep(100);
		veh_depth=sds.veh_depth;
	}
	cout<<"MH:Pressure sensor data consistent!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Pressure sensor data consistent!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);
	
	while(veh_alti_ALT==0){
		get_altimetry();
		_sleep(100);
		veh_alti_ALT=sds.veh_alti;
	}
	cout<<"MH:Altimetry data from altimeter consistent!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Altimetry data from altimeter consistent!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);
	/*while(veh_alti_DVL==0){
		dvl_daq();
		_sleep(100);
		veh_alti_DVL=dds.dvl_alti;
		cout<<veh_alti_DVL<<endl;
	}
	cout<<"MH:Altimetry data from DVL consistent!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Altimetry data from DVL consistent!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);*/

	//open FLS port
	FLS_port=FLS_port_config();
	// configure the SONAR
	FLS_retval=FLS_device_config(FLS_port);
	CloseHandle(FLS_port);
	if(!FLS_retval){
		cout<<"MH:FLS device level configuration done!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:FLS device level configuration done!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
	}
	else{
		cout<<"MH:FLS device level configuration not done!!!"<<endl;
		sprintf(gen_msg_buffer,"MH:FLS device level configuration not done!!!");
		log_missionstatus(sys_filename,gen_msg_buffer);
	}

	cout<<"MH:Sensor initialisation and data check complete!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Sensor initialisation and data check complete!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);

	fp_missfile=fopen("mission_file.txt","r");
	if(fp_missfile==NULL){
		cout<<"MH:Mission file could not be opened"<<endl;
		sprintf(gen_msg_buffer,"MH:Mission file could not be opened");
		log_missionstatus(sys_filename,gen_msg_buffer);
		cout<<"MH:Mission Program: Premature Termination"<<endl;
		sprintf(gen_msg_buffer,"MH:Mission File Not Opened: Premature Termination");
		log_missionstatus(sys_filename,gen_msg_buffer);
		ptAOVoltageOut.OutputValue=0.00 ;
		DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
		DRV_DeviceClose(&DriverHandle);
		ADAMTCP_Disconnect();
		ADAMTCP_Close();
		ExitThread(exit_th);
	}
	else{
		cout<<"MH:Mission file opened succesfully"<<endl;
		sprintf(gen_msg_buffer,"MH:Mission file opened succesfully");
		log_missionstatus(sys_filename,gen_msg_buffer);
	}


	cout<<"MH:Ready to send post-launching information: Altimetry and Position to Surface!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Ready to send post-launching information: Altimetry and Position to Surface!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);
	cout<<"MH:Mission ready to be started!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Mission ready to be started!!!");
	log_missionstatus(sys_filename,gen_msg_buffer);

	///// Creating mutexes for synchronising with heading as well as surge controllers
	hmutex_hc=CreateMutex(NULL,FALSE,"mutex_hc");
	hmutex_sc=CreateMutex(NULL,FALSE,"mutex_sc");
	hmutex_sc_phinsread=CreateMutex(NULL,FALSE,"mutex_sc_phins");
	hmutex_hc_phinsread=CreateMutex(NULL,FALSE,"mutex_hc_phins");

Ready_: while(command!=MISS_START){
		//send sensor data to surface control as regards current postion and altimetry
		Phins_Daq();
		Depth_Daq();
		get_altimetry();
		//dvl_daq();
		get_batt_status();
		//cout<<sds.veh_east_pos<<" "<<sds.veh_north_pos<<" "<<sds.latitude<<endl;

		if(command==MISS_ABORT){
			cout<<"MH:Mission Abort Command Received"<<endl;
			sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
			log_missionstatus(sys_filename,gen_msg_buffer);
			cout<<"MH:Mission Program: Premature Termination"<<endl;
			sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
			log_missionstatus(sys_filename,gen_msg_buffer);
			ptAOVoltageOut.OutputValue=0.00 ;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
			DRV_DeviceClose(&DriverHandle);
			ADAMTCP_Disconnect();
			ADAMTCP_Close();
			ExitThread(exit_th);
		}
		if(command==MAN_ACTUATE){
			cds.head_nosetorque=ct_val;
			cds.head_tailtorque=ct_val;
			cds.signal=mt_val;
			tele_actuation();
			command=0;
		}
		_sleep(150);
		cout<<"within Ready"<<endl;
	}
	//########################################################################################//
	//--------------------------- Mission Start command received -----------------------------//
	mission_started=true;
	cout<<"MH:Mission started!!!"<<endl;
	sprintf(gen_msg_buffer,"MH:Mission started!!!\n");
	log_missionstatus(sys_filename,gen_msg_buffer);
	//Create watchdog timer thread
	wt_hwnd=CreateThread(NULL,0,Watchdog_Timer,(void *)NULL,NULL,id_wt);
	//if no Dive command received then 
	//1. do not create the Diving Controller Thread
	//2. directly start Trajectory Following behaviour
	if(rds.depth_des!=0){
		dc_hwnd=CreateThread(NULL,0,depth_Ctrl,(void *)0,0,sg_id);
		if(dc_hwnd==NULL){
			cout<<"MH:Diving could not be initiated"<<endl;
			sprintf(gen_msg_buffer,"MH:Diving could not be initiated");
			log_missionstatus(sys_filename,gen_msg_buffer);
			ptAOVoltageOut.OutputValue=0.00 ;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
			DRV_DeviceClose(&DriverHandle);
			ADAMTCP_Disconnect();
			ADAMTCP_Close();
			cout<<"MH:Dive thread not created: Premature Termination"<<endl;
			sprintf(gen_msg_buffer,"MH:Dive thread not created: Premature Termination");
			log_missionstatus(sys_filename,gen_msg_buffer);	
			ExitThread(exit_th);
		}
		else{
			proc=SetThreadIdealProcessor(dc_hwnd,1);
			dive_complete=false;
		}
	}
	else
		dive_complete=true;
	while(mission_started){		
		if(dive_complete){
			hdHwnd=CreateThread(NULL,0,heading_ctrl,(void *)0,0,hd_id);
			if(hdHwnd==NULL){
				cout<<"MH:Heading Correction could not be initiated"<<endl;
				ptAOVoltageOut.OutputValue=0.00 ;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				DRV_DeviceClose(&DriverHandle);
				ADAMTCP_Disconnect();
				ADAMTCP_Close();
				if(dc_hwnd!=NULL)
					TerminateThread(dc_hwnd,exit_th);
				cout<<"MH:Mission Program: Premature Termination"<<endl;
				sprintf(gen_msg_buffer,"MH:Heading Control not Initiated");
				log_missionstatus(sys_filename,gen_msg_buffer);
				ExitThread(exit_th);
			}
			sgHwnd=CreateThread(NULL,0,surge_ctrl,(void *)0,0,sg_id);
			if(sgHwnd==NULL){
				cout<<"MH:Surge Correction could not be initiated"<<endl;
				ptAOVoltageOut.OutputValue=0.00 ;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				DRV_DeviceClose(&DriverHandle);
				ADAMTCP_Disconnect();
				ADAMTCP_Close();
				if(dc_hwnd!=NULL)
					TerminateThread(dc_hwnd,exit_th);
				cout<<"MH:Mission Program: Premature Termination"<<endl;
				sprintf(gen_msg_buffer,"MH:Surge Control not Initiated");
				log_missionstatus(sys_filename,gen_msg_buffer);
				ExitThread(exit_th);
			}
			sprintf(gen_msg_buffer,"MH:Initiated XY Operation");
			log_missionstatus(sys_filename,gen_msg_buffer);
			//read data from the mission file and update reference data structure
Start_:		if(feof(fp_missfile))
				goto MissionComplete;
			else{
				fscanf(fp_missfile,"%f %f %f",&rds.yaw_des,&rds.sg_des,&traj_hold_time);
				Phins_Daq();
				start_north=sds.veh_north_pos;
				start_east=sds.veh_east_pos;
			}
			sprintf(gen_msg_buffer,"MH:Read trajectory segment");
			log_missionstatus(sys_filename,gen_msg_buffer);
			//release mutex on reference data structure (rds) for controller threads
			//---- ToDo -----//
			ReleaseMutex(hmutex_hc);
			ReleaseMutex(hmutex_sc);
			///////////////////
			//wait till segment time out
			SegmentTimedOut=false;
			MissionTimedOut=false;
			tt_hwnd=CreateThread(NULL,0,traj_timer,(void*)0,NULL,id_tt);
			while(!SegmentTimedOut){
				//// Check for Mission Watchdog Timer timeout ///// 
				if(MissionTimedOut){
					cout<<"MH:Mission Watchdog Timer timed out!!!"<<endl;
					sprintf(gen_msg_buffer,"MH:Mission Watchdog Timer timed out!!!");
					log_missionstatus(sys_filename,gen_msg_buffer);
					TerminateThread(dc_hwnd,exit_th);
					TerminateThread(hdHwnd,exit_th);
					TerminateThread(sgHwnd,exit_th);
					CloseHandle(dc_hwnd);
					CloseHandle(hdHwnd);
					CloseHandle(sgHwnd);
					CloseHandle(hmutex_hc);
					CloseHandle(hmutex_sc);
					ptAOVoltageOut.OutputValue=0.00 ;
					DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
					DRV_DeviceClose(&DriverHandle);
					ADAMTCP_Disconnect();
					ADAMTCP_Close();
					cout<<"MH:Watchdog Timed Out:premature termination!!!"<<endl;
					ExitThread(exit_th);
				}
				if(command==MISS_ABORT){
					cout<<"MH:Mission Abort Command Received"<<endl;
					sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
					cout<<"MH:Mission Program: Premature Termination"<<endl;
					sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
					log_missionstatus(sys_filename,gen_msg_buffer);
					TerminateThread(dc_hwnd,exit_th);
					TerminateThread(hdHwnd,exit_th);
					TerminateThread(sgHwnd,exit_th);
					CloseHandle(dc_hwnd);
					CloseHandle(hdHwnd);
					CloseHandle(sgHwnd);
					CloseHandle(hmutex_hc);
					CloseHandle(hmutex_sc);
					ptAOVoltageOut.OutputValue=0.00 ;
					DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
					DRV_DeviceClose(&DriverHandle);
					ADAMTCP_Disconnect();
					ADAMTCP_Close();
					ExitThread(exit_th);
				}
				Phins_Daq();
				//// get battery status
				get_batt_status();
				//// log battery status
				log_batterystatus(battery_filename);
				// check for obstacles
				//FLS_port=FLS_port_config();
				get_objrange_bearing();
				cout<<"Brng:"<<sds.obs_brng<<"\tRange:"<<sds.obs_rng<<endl;
				if(sds.obs_brng<=182 && sds.obs_brng>=177){
					if(sds.obs_rng<=10.0 && sds.obs_rng>0){
						obj_found=true;
					}
					else{
						obj_found=false;
						obj_found_again=false;
					}
				}
				//// Log sensor data 
				log_systemdata(mission_filename);
				// change ref. surge to 1 meter
				// change Start Position
				if(obj_found && !obj_found_again){
					if(rds.sg_des!=0 && rds.depth_des!=0){
						WaitForSingleObject(hmutex_sc,INFINITE);
						rds.sg_des=1.0;
						start_north=sds.veh_north_pos;
						start_east=sds.veh_east_pos;
						ReleaseMutex(hmutex_sc);
						obj_found_again=true;
						cout<<"Collision Prevention triggered!!!"<<endl;
						sprintf(gen_msg_buffer,"MH:Collision Prevention triggered!!!\n");
						log_missionstatus(sys_filename,gen_msg_buffer);
					}
				}
				/*ptDioReadBit.port=0;
				ptDioReadBit.bit=3;
				DRV_DioReadBit(DriverHandle,&ptDioReadBit);
				if(ptDioReadBit.state){
					sds.leak=1;
					ptDioWriteBit.port  = 0; // output port: 0
					ptDioWriteBit.bit   = 2; // output channel
					ptDioWriteBit.state = 1; // output state
					DRV_DioWriteBit(DriverHandle,&ptDioWriteBit);
					sprintf(gen_msg_buffer,"MH:Leak detected-->Generating ABORT command!!!");
					log_missionstatus(sys_filename,gen_msg_buffer);
					command=MISS_ABORT;
				}*/
				mt_actuation();
				_sleep(150);
			}
			//block controller threads from reading reference data structure (rds)
			//---- ToDo -----//
			WaitForSingleObject(hmutex_hc,INFINITE);
			WaitForSingleObject(hmutex_sc,INFINITE);
			cout<<"MH:Segment Completed!!!"<<endl;
			sprintf(gen_msg_buffer,"MH:Segment Completed!!!\n");
			log_missionstatus(sys_filename,gen_msg_buffer);
			//////////////////
			goto Start_;
		}
		else{
			///// Check for diving completion status /////
			if(abs(depth_err)<=0.1){
				_sleep(DIVE_SETTLING_TIME);
				sprintf(gen_msg_buffer,"MH:Desired Depth Correction Achieved");
				log_missionstatus(sys_filename,gen_msg_buffer);
				cout<<"depth achieved"<<endl;
				hdHwnd=CreateThread(NULL,0,heading_ctrl,(void *)0,0,hd_id);
				if(hdHwnd==NULL){
					cout<<"MH:Heading Correction could not be initiated"<<endl;
					ptAOVoltageOut.OutputValue=0.00 ;
					DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
					DRV_DeviceClose(&DriverHandle);
					ADAMTCP_Disconnect();
					ADAMTCP_Close();
					if(dc_hwnd!=NULL)
						TerminateThread(dc_hwnd,exit_th);
					cout<<"MH:Mission Program: Premature Termination"<<endl;
					sprintf(gen_msg_buffer,"MH:Heading Control not Initiated");
					log_missionstatus(sys_filename,gen_msg_buffer);
					ExitThread(exit_th);
				}
				sgHwnd=CreateThread(NULL,0,surge_ctrl,(void *)0,0,sg_id);
				if(sgHwnd==NULL){
					cout<<"MH:Surge Correction could not be initiated"<<endl;
					ptAOVoltageOut.OutputValue=0.00 ;
					DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
					DRV_DeviceClose(&DriverHandle);
					ADAMTCP_Disconnect();
					ADAMTCP_Close();
					if(dc_hwnd!=NULL)
						TerminateThread(dc_hwnd,exit_th);
					cout<<"MH:Mission Program: Premature Termination"<<endl;
					sprintf(gen_msg_buffer,"MH:Surge Control not Initiated");
					log_missionstatus(sys_filename,gen_msg_buffer);
					ExitThread(exit_th);
				}
				sprintf(gen_msg_buffer,"MH:Initiated XY Operation");
				log_missionstatus(sys_filename,gen_msg_buffer);

				dive_complete=true;
				goto Start_;
			}
			///// if desired depth is inconsistent with current altimetry
			///// declare dive operation to be infeasible and return to the
			///// system Ready state
			/*if(DiveInfeasible){
				CloseHandle(dc_hwnd);
				goto Ready_;
			}*/
			///// Check for Mission Watchdog Timer timeout /////
			if(MissionTimedOut){
				cout<<"MH:Mission Watchdog Timer timed out!!!"<<endl;
				sprintf(gen_msg_buffer,"MH:Watchdog Timed Out");
				log_missionstatus(sys_filename,gen_msg_buffer);
				ptAOVoltageOut.OutputValue=0.00 ;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				DRV_DeviceClose(&DriverHandle);
				ADAMTCP_Disconnect();
				ADAMTCP_Close();
				TerminateThread(dc_hwnd,exit_th);
				CloseHandle(dc_hwnd);
				cout<<"MH:Mission program exiting:premature termination!!!"<<endl;
				ExitThread(exit_th);
			}
			if(command==MISS_ABORT){
				cout<<"MH:Mission Abort Command Received"<<endl;
				sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
				cout<<"MH:Mission Program: Premature Termination"<<endl;
				sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
				log_missionstatus(sys_filename,gen_msg_buffer);
				TerminateThread(dc_hwnd,exit_th);
				CloseHandle(dc_hwnd);
				ptAOVoltageOut.OutputValue=0.00 ;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				DRV_DeviceClose(&DriverHandle);
				ADAMTCP_Disconnect();
				ADAMTCP_Close();
				ExitThread(exit_th);
			}
			/*ptDioReadBit.port=0;
			ptDioReadBit.bit=3;
			DRV_DioReadBit(DriverHandle,&ptDioReadBit);
			if(ptDioReadBit.state){
				sds.leak=1;
				ptDioWriteBit.port  = 0; // output port: 0
				ptDioWriteBit.bit   = 2; // output channel
				ptDioWriteBit.state = 1; // output state
				DRV_DioWriteBit(DriverHandle,&ptDioWriteBit);
				sprintf(gen_msg_buffer,"MH:Leak detected-->Generating ABORT command!!!");
				log_missionstatus(sys_filename,gen_msg_buffer);
				command=MISS_ABORT;
			}*/
			Phins_Daq();
			//// Log sensor data 
			log_systemdata(mission_filename);
			//// get battery status
			get_batt_status();
			//// log battery status
			log_batterystatus(battery_filename);
		}
		_sleep(250);
	}
				
	//############################## Mission Complete #######################################//
	////// Close all the thread handles and reset the devices accordingly //////
MissionComplete:	TerminateThread(dc_hwnd,exit_th);
					TerminateThread(hdHwnd,exit_th);
					TerminateThread(sgHwnd,exit_th);
					CloseHandle(dc_hwnd);
					//CloseHandle(FLS_port);
					CloseHandle(hdHwnd);
					CloseHandle(sgHwnd);
					CloseHandle(hmutex_hc);
					CloseHandle(hmutex_sc);
					ADAMTCP_Disconnect();
					ADAMTCP_Close();
					ptAOVoltageOut.OutputValue=0.00 ;
					DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
					DRV_DeviceClose(&DriverHandle);
					cout<<"MH:Mission successfully completed!!!"<<endl;
					sprintf(gen_msg_buffer,"MH:Mission successfully completed!!!");
					log_missionstatus(sys_filename,gen_msg_buffer);
					ExitThread(exit_th);
}


/////////////// Threads //////////////////////////////////

//###################### COMMUNICATION Threads Block START #######################///////// 
///////////// Communication with Surface //////////////////////

unsigned long _stdcall Communicator(void *data){
	SOCKET hSock;
	SOCKET hClient;
	sockaddr_in sa;
	sockaddr_in saClient;
	bool connected=false;

	char command_header;
	char ser_sendbuf[20];
	char rf_sendbuf[200];
	char recvBuffer[12];
	char refdepth[5];
	char missdur[5];
	char ct_command[5];
	char mt_command[5];
	int reference_depth;
	int mission_duration;

	//// variable declarations for communicating with DVS
	HANDLE dvsPort;
	COMMTIMEOUTS timeout;
	DCB PortDCB;
	DWORD bytesRead;

	char buffer[80];
	int leak_status;
	struct _timeb start;
	struct _timeb stop;
	int timeelapsed;
	DWORD iBytesWritten;



	//------------- Initialize winsock2.2 dll ---------------//
	WSADATA wsaData={0};
	WORD wVersionRequested=MAKEWORD(2,2);
	int nRet=WSAStartup(wVersionRequested,&wsaData);
	if(nRet==SOCKET_ERROR){
		cout<<"ERROR : "<<WSAGetLastError()<<endl;
	}

	//------------- Open a socket --------------------------//
	hSock=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(hSock==INVALID_SOCKET){
		cout<<"Invalid socket,failed to create the socket..."<<endl;
	}

	//-------------- Name the socket -----------------------//
	sa.sin_family=PF_INET;
	sa.sin_port=htons(8888);
	sa.sin_addr.S_un.S_addr=htonl(INADDR_ANY);

	//-----------bind the socket's name-----------------------------------------
	nRet=bind(hSock,(sockaddr*)&sa,sizeof(sa));
	if (nRet==SOCKET_ERROR){
		cout<<"ERROR:"<<WSAGetLastError()<<endl;
	}
		
	
	while(1){
		
		//--------------- Listen -----------------------------//
		cout<<"Listening for connections..."<<endl;
		nRet=listen(hSock,5);	//connection backlog queue set to 5
		if (nRet==SOCKET_ERROR){
			cout<<"ERROR:"<<WSAGetLastError()<<endl;
			closesocket(hSock);
		}
		//--------------- Accept -----------------------------//
		int nSALen=sizeof(sockaddr);
		hClient=accept(hSock,(sockaddr*)&saClient,&nSALen);
		if (hClient==INVALID_SOCKET){
			cout<<"Invalid client socket,connection failed!"<<endl;
			closesocket(hSock);
		}
		else{
			cout<<"Connection established"<<endl;
		}
		for(;;){
			//--------------- Receive & Send ----------------------------//
			cout<<"Ready to receive data"<<endl;
			int inDataLength=recv(hClient,recvBuffer,sizeof(recvBuffer),0);
			recvBuffer[inDataLength]='\0';
			cout<<recvBuffer<<endl;
			sscanf(recvBuffer,"%c ",&command_header);
			//////receive command from client///////////
			switch(command_header)
			{
				case 'S':
					//for START
					*((int *)data)=MISS_START;
					cout<<"START command received!!!"<<endl;
					break;
				case 'A':
					//for ABORT
					PS=1024;
					*((int *)data)=MISS_ABORT;
					cout<<"ABORT command received!!!"<<endl;
					break;
				case 'N':
					//for Light ON
					cout<<"LIGHT ON command received!!!"<<endl;
					dvsPort=CreateFile("COM2",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
					if(dvsPort!=INVALID_HANDLE_VALUE){
						PortDCB.DCBlength = sizeof (DCB); 
						GetCommState(dvsPort,&PortDCB);
						PortDCB.BaudRate = 9600;      
						PortDCB.ByteSize = 8;              
						PortDCB.Parity = NOPARITY;         
						PortDCB.StopBits = ONESTOPBIT;    
						SetCommState(dvsPort,&PortDCB);
						SetCommTimeouts(dvsPort,&timeout);
						WriteFile(dvsPort,"N",1,&iBytesWritten,NULL);
						_ftime(&start);
						ReadFile(dvsPort,buffer,1,&bytesRead,NULL);
						_ftime(&stop);
						timeelapsed=stop.time-start.time;
						if(timeelapsed<=1){
							buffer[bytesRead]='\0';
							leak_status=atoi(buffer);
							sds.leak=leak_status;
						}
						CloseHandle(dvsPort);
					}
					break;
				case 'F':
					//for Light OFF
					cout<<"LIGHT OFF command received!!!"<<endl;
					dvsPort=CreateFile("COM2",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
					if(dvsPort!=INVALID_HANDLE_VALUE){
						PortDCB.DCBlength = sizeof (DCB); 
						GetCommState(dvsPort,&PortDCB);
						PortDCB.BaudRate = 9600;      
						PortDCB.ByteSize = 8;              
						PortDCB.Parity = NOPARITY;         
						PortDCB.StopBits = ONESTOPBIT;    
						SetCommState(dvsPort,&PortDCB);
						SetCommTimeouts(dvsPort,&timeout);
						WriteFile(dvsPort,"F",1,&iBytesWritten,NULL);
						_ftime(&start);
						ReadFile(dvsPort,buffer,1,&bytesRead,NULL);
						_ftime(&stop);
						timeelapsed=stop.time-start.time;
						if(timeelapsed<=1){
							buffer[bytesRead]='\0';
							leak_status=atoi(buffer);
							sds.leak=leak_status;
						}
						CloseHandle(dvsPort);
					}
					break;
				case 'H':
					//for System shutdown
					*((int *)data)=SYS_HALT;
					cout<<"SYSTEM SHUTDOWN command received!!!"<<endl;
					AdjustPriv();
					InitiateSystemShutdown(NULL,NULL,5,true,false);
					//close socket communication
					closesocket(hClient);
					closesocket(hSock);
					ExitThread(exit_th);
				case 'R':
					//for System reboot
					*((int *)data)=SYS_REBOOT;
					cout<<"SYSTEM REBOOT command received!!!"<<endl;
					AdjustPriv();
					InitiateSystemShutdown(NULL,NULL,5,true,true);
					//close socket communication
					closesocket(hClient);
					closesocket(hSock);
					ExitThread(exit_th);
				case 'P':
					//for updates on mission particulars
					sscanf(recvBuffer+1,"%s %s",refdepth,missdur);
					reference_depth=atoi(refdepth);
					mission_duration=atoi(missdur);
					mission_time=mission_duration;
					rds.depth_des =reference_depth;
					break;
				case 'M':
					//for Operating AUV under manual control
					sscanf(recvBuffer+1,"%s %s",ct_command,mt_command);
					ct_val=atoi(ct_command);
					mt_val=atof(mt_command);
					*((int *)data)=MAN_ACTUATE;
					break;
				case 'D':
					//request from Surface Control for sensor-data
					//################!!!!! to be decided !!!!!#################//
					sprintf(rf_sendbuf,"%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %d",
						sds.veh_alti ,dds.dvl_alti,sds.veh_roll,sds.veh_pitch,sds.curr_yaw ,sds.veh_surge,
						sds.bank1,sds.bank2,sds.bank3,sds.bank4,sds.bank5,sds.bank6,
						sds.latitude, sds.longitude, sds.veh_east_pos,sds.veh_north_pos,
						sds.leak);
					cout<<rf_sendbuf<<endl;
					send(hClient,rf_sendbuf,strlen(rf_sendbuf),0);
					break;
				case 'G':
					send(hClient,gen_msg_buffer,strlen(gen_msg_buffer),0);
					break;
				case 'T':
					closesocket(hClient);
					closesocket(hSock);
					ExitThread(exit_th);
					break;

			}
		}
	}
	return 0;
}
unsigned long _stdcall Acoustic_comm(void *data){

	HANDLE hSerial;
	DCB dcb;
	DWORD bytesRead;
	DWORD bytesWritten;
	COMMTIMEOUTS timeout;
	bool port_open=false;
	
	

_PortOpen:	hSerial=CreateFile("\\\\.\\COM13",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

	if(hSerial!=INVALID_HANDLE_VALUE){

		dcb.DCBlength = sizeof(DCB); 
		GetCommState(hSerial,&dcb); 
		dcb.BaudRate = 9600;      
		dcb.ByteSize = 8;              
		dcb.Parity = NOPARITY;         
		dcb.StopBits = ONESTOPBIT;    
		SetCommState(hSerial,&dcb);

		timeout.ReadTotalTimeoutConstant =5000;
		timeout.ReadTotalTimeoutMultiplier =0;
		timeout.WriteTotalTimeoutConstant =0;
		timeout.WriteTotalTimeoutMultiplier =0;
		SetCommTimeouts(hSerial,&timeout);

		port_open=true;
	}
	
	while(1){
		if(port_open){
			ReadFile(hSerial,ac_buffer,8,&bytesRead,NULL);
			if(ac_buffer!=NULL){
				ac_buffer[bytesRead]='\0';
				if(ac_buffer[0]=='X'){
					*((int *)data)=MISS_ABORT;
					sprintf(gen_msg_buffer,"AC:ABORT Command Received from Surface");
					log_missionstatus(sys_filename,gen_msg_buffer);
					CloseHandle(hSerial);
					return 0;
				}
			}
			sprintf(ac_buffer,"$D,%.1f,$A,%.1f,$H,%.1f*",sds.veh_depth,sds.veh_alti,sds.curr_yaw);
			WriteFile(hSerial,ac_buffer,strlen(ac_buffer),&bytesWritten,NULL);
		}
		else
			goto _PortOpen;
	}


	CloseHandle(hSerial);
	return 0;
}
/////////////////////////////////////////////////////////////////
//###################### COMMUNICATION Threads Block END #######################///////// 

//###################### CONTROL Threads Block START #######################///////// 

////////////////// Depth Control /////////////////////////////////////
unsigned long _stdcall depth_Ctrl(void *data){
	
	float err;
	float prev_err=0;

	float err_rate;
	float Kp=5.0;
	float Kd=1.0;

	cout<<"depth control thread running"<<endl;

	PS=63;
	while(1){
		Depth_Daq();
		//get_altimetry();
		//dvl_daq();

		err=rds.depth_des-sds.veh_depth;
		depth_err=err;

		err_rate=(err-prev_err)*4;
		
		cds.depth_tailtorque =-((Kp*err+Kd*err_rate)+50);
		cds.depth_nosetorque =-((Kp*err+Kd*err_rate)+60);

		vt_actuation();
		
		prev_err=err;

		_sleep(100);
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////

/////////////////////// Heading Control ////////////////////////////////////////////
unsigned long _stdcall heading_ctrl(void *data){
	//variable declarations
	float yaw_err_curr;
	float yaw_err_old=123456;
	int torque;

	cout<<"heading controller thread running...."<<endl;
	//open the mutex held by main thread
	hmutex_hc=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_hc");
	//open the mutex over Phins read operation
	//hmutex_hc_phinsread=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_hc_phins");

	while (1){
		// wait till main thread has updated reference data structures
		WaitForSingleObject(hmutex_hc,INFINITE);
		// lock mutex while accessing Phins_Daq
		//WaitForSingleObject(hmutex_hc_phinsread,INFINITE);
		//Phins_Daq();
		// release the mutex over Phins Daq
		//ReleaseMutex(hmutex_hc_phinsread);
		//execute control algorithm
		//cout<<"Yawdes:"<<rds.yaw_des<<"\t"<<"CurrYaw:"<<sds.curr_yaw<<endl;
		yaw_err_curr=LeastSweepAngle(rds.yaw_des,sds.curr_yaw); // Direction of Heading change decided
		torque=-PID_HeadingCtrl(yaw_err_curr,&yaw_err_old);//PID controller O/P calculation
		cds.head_nosetorque=torque;
		cds.head_tailtorque=torque;
		//send commands to the horizontal control thrusters
		ht_actuation();
		// release the lock for the main thread to go for the next trajectory segment
		ReleaseMutex(hmutex_hc);
		_sleep(250);
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////
//////////////////////// Surge Control /////////////////////////////////////////
unsigned long _stdcall surge_ctrl(void *data){
	float veh_east_disp;
	float veh_north_disp;
	float veh_east_spddisp;
	float veh_north_spddisp;
	float veh_eastphins;
	float veh_northphins;
	float old_veh_res_disp=0;
	
	float Kp,Ki,Kd;
	float P,I,D;
	
	Kp=0.4;
	Ki=0.2;
	Kd=2.0;
	I=0.00;

	cout<<"surge control thread running"<<endl;
	//open the mutex held by main thread
	hmutex_sc=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_sc");
	//open the mutex over Phins read operation
	//hmutex_sc_phinsread=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_sc_phins");
	
	while(1){
		
		//---------------------------------------------------------------------------------------------------------//
		/*veh_east_spddisp=start_dvleast-(dds.vel_east*0.25);
		veh_north_spddisp=start_dvlnorth-(dds.vel_north*0.25);
		veh_res_disp_dvlspd=sqrt((veh_east_spddisp*veh_east_spddisp)+(veh_north_spddisp*veh_north_spddisp));
		
		veh_east_disp=start_dvleast-dds.disp_east;
		veh_north_disp=start_dvlnorth-dds.disp_north;
		veh_res_disp_dvl=sqrt((veh_east_disp*veh_east_disp)+(veh_north_disp*veh_north_disp));*/
		//---------------------------------------------------------------------------------------------------------//

		// wait till main thread has updated reference data structures
		WaitForSingleObject(hmutex_sc,INFINITE);
		// lock mutex while accessing Phins_Daq
		//WaitForSingleObject(hmutex_sc_phinsread,INFINITE);
		//Phins_Daq();
		// release the mutex over Phins Daq
		//ReleaseMutex(hmutex_hc_phinsread);

		//execute control algorithm
		veh_eastphins=start_east-sds.veh_east_pos;
		veh_northphins=start_north-sds.veh_north_pos;
		veh_res_disp=sqrt((veh_eastphins*veh_eastphins)+(veh_northphins*veh_northphins));
		if(sds.veh_north_pos<=validate_north || sds.veh_east_pos<=validate_east){
			veh_res_disp=old_veh_res_disp;
			sds.srg_err=0.0;
		}
		else{
			sds.srg_err =rds.sg_des - veh_res_disp;
			old_veh_res_disp=veh_res_disp;
		}
		sds.veh_surge =veh_res_disp;
		if(rds.sg_des!=0){
			cds.P=Kp*sds.srg_err;
			cds.D=Kd*(sds.srg_err-sds.srg_err_old)*4.0;
			cds.signal=cds.P+cds.D;

			if((cds.P+cds.D)<0){
				if((cds.P+cds.D-1.7)<-4.0)
					cds.signal=-4.0;
				else
					cds.signal=cds.P+cds.D-1.7;
			}
			else{
				if((cds.P+cds.D+1.7)>4.0)
					cds.signal=4.0;
				else
					cds.signal=cds.P+cds.D+1.7;
			}
			//cout<<"Surge:"<<veh_res_disp<<"\tError:"<<sds.srg_err<<"\tP:"<<cds.P<<"\tD:"<<cds.D<<"\tS:"<<cds.signal<<endl;
		}
		else{
			//cout<<"No surge"<<endl;
			cds.signal=0.0;
		}
		//cout<<"Main Signal:"<<cds.signal<<endl;
		// send control signal to main thruster
		//mt_actuation();
		// release the lock for the main thread to go for the next trajectory segment
		ReleaseMutex(hmutex_sc);
		_sleep(250);
	}
	
	return 0;
	
}
//////////////////////////////////////////////////////////////////////////////////////
//###################### CONTROL Threads Block END #######################///////// 
		

//###################### TIMER Threads Block START #######################///////// 
//////////////////////////// Watchdog Timer //////////////////////////////////
unsigned long _stdcall Watchdog_Timer(void *data){
	_sleep(1000*60*mission_time);
	PS=2048;
	MissionTimedOut=true;
	return 0;		
}
///////////////////////////// Traject Timer //////////////////////////////////
unsigned long _stdcall traj_timer(void *data){
	_sleep(1000*60*traj_hold_time);
	PS=256;
	SegmentTimedOut=true;
	return 0;
}
//////////////////////////////////////////////////////////////////////////////

//######################### Associated Modules #############################//////
////////////////////////// Actuation Coord //////////////////////////////
void tele_actuation(){
	
	//variable declarations
	CComConfig hor;
	int res_tailtorque;
	int res_nosetorque;
	char command_n[10];
	char command_t[10];


	hor.ComInit ("COM6",1);
	hor.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);

	res_tailtorque=cds.head_tailtorque;
	res_nosetorque=cds.head_nosetorque;
		
	if(res_tailtorque<0){
		if(res_tailtorque<-60)
			res_tailtorque=-60;
		sprintf(command_t,"u37-%d\r",abs(res_tailtorque));
		hor.DeviceWrite (command_t);
	}
	if(res_nosetorque<0){
		if(res_nosetorque<-60)
			res_nosetorque=-60;
		sprintf(command_n,"u17-%d\r",abs(res_nosetorque));
		hor.DeviceWrite (command_n);
	}
	if(res_tailtorque>0){
		if(res_tailtorque>60)
			res_tailtorque=60;
		sprintf(command_t,"u37+%d\r",abs(res_tailtorque));
		hor.DeviceWrite (command_t);
	}
	if(res_nosetorque>0){
		if(res_nosetorque>60)
			res_nosetorque=60;
		sprintf(command_n,"u17+%d\r",abs(res_nosetorque));
		hor.DeviceWrite (command_n);
	}
	hor.EndCom ();
	if(DriverHandle!=NULL){
		ptAOVoltageOut.OutputValue=cds.signal;
		DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
		_sleep(3000);
		ptAOVoltageOut.OutputValue=0.0;
		DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
	}
}

void vt_actuation(void){
		
	// variable declarations
	int tail_cmd;
	int nose_cmd;
	CComConfig tail_vert;
	CComConfig nose_vert;
	char command_nv[10];
	char command_tv[10];
	
	
	tail_vert.ComInit ("COM5",1);
	nose_vert.ComInit ("COM4",1);

	tail_vert.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);
	nose_vert.SetConfigParam (9600,8,NOPARITY,ONESTOPBIT);

	tail_cmd=cds.depth_tailtorque;
	nose_cmd=cds.depth_nosetorque;

	if(tail_cmd<0){
		if(tail_cmd<-76)
			tail_cmd=-76;
		sprintf(command_tv,"u67-%d\r",abs(tail_cmd));
		tail_vert.DeviceWrite (command_tv);
	}
	if(tail_cmd>0){
		if(tail_cmd>76)
			tail_cmd=76;
		sprintf(command_tv,"u67+%d\r",abs(tail_cmd));
		tail_vert.DeviceWrite (command_tv);
	}
	if(nose_cmd<0){
		if(nose_cmd<-76)
			nose_cmd=-76;
		sprintf(command_nv,"u17-%d\r",abs(nose_cmd));
		nose_vert.DeviceWrite (command_nv);
	}
	if(nose_cmd>0){
		if(nose_cmd>76)
			nose_cmd=76;
		sprintf(command_nv,"u17+%d\r",abs(nose_cmd));
		nose_vert.DeviceWrite (command_nv);
	}
	tail_vert.EndCom ();
	nose_vert.EndCom ();
}

void ht_actuation(void){
		
	//variable declarations
	CComConfig hor;
	int res_tailtorque;
	int res_nosetorque;
	char command_n[10];
	char command_t[10];


	hor.ComInit ("COM6",1);
	hor.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);

	res_tailtorque=cds.head_tailtorque;
	res_nosetorque=cds.head_nosetorque;
		
	if(res_tailtorque<0){
		if(res_tailtorque<-60)
			res_tailtorque=-60;
		sprintf(command_t,"u37-%d\r",abs(res_tailtorque));
		hor.DeviceWrite (command_t);
	}
	if(res_tailtorque>0){
		if(res_tailtorque>60)
			res_tailtorque=60;
		sprintf(command_t,"u37+%d\r",abs(res_tailtorque));
		hor.DeviceWrite (command_t);
	}
	_sleep(100);
	if(res_nosetorque<0){
		if(res_nosetorque<-60)
			res_nosetorque=-60;
		sprintf(command_n,"u17-%d\r",abs(res_nosetorque));
		hor.DeviceWrite (command_n);
	}
	if(res_nosetorque>0){
		if(res_nosetorque>60)
			res_nosetorque=60;
		sprintf(command_n,"u17+%d\r",abs(res_nosetorque));
		hor.DeviceWrite (command_n);
	}
	hor.EndCom ();

}

void mt_actuation(void){
		
	float norm_yaw_err;
	float signal;
	float signal_diff;

	norm_yaw_err=abs(rds.yaw_des -sds.curr_yaw )/5.0;
	if(norm_yaw_err>=1)
		norm_yaw_err=1;
	else if(norm_yaw_err<1)
		norm_yaw_err=0;
	
//	signal=prev_signal;
//	signal_diff=abs(cds.signal-prev_signal);
	/*if(abs(norm_yaw_err)<=0.1)
			{if (cds.signal<0)
				cds.signal= (cds.signal-0.80); // Wind compensastion and -ve Nonlinearity Compensation
			if (cds.signal>0.4 && cds.signal<=1.0) 
				cds.signal=1.1;//Low speed Nonlienarity Elimination
			}*/
	/*if((ramp_correction) && (signal_diff>=1)){
		if(cds.signal>prev_signal){
			while(signal<=cds.signal){
				signal=signal+1;
				ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*signal;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				_sleep(20);
			}
			ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*cds.signal;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);

		}
		else if(cds.signal<prev_signal){
			while(signal>=cds.signal){
				signal=signal-1;
				ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*signal;
				DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
				_sleep(20);
			}
			ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*cds.signal;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);

		}
		else if(cds.signal==prev_signal){
			ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*cds.signal;
			DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);

		}
	}
	ramp_correction=false;
	prev_signal=cds.signal;*/
		
	
	/*ptAOVoltageOut.OutputValue=1.5;
	DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);*/
	//ptAOVoltageOut.OutputValue=(1-norm_yaw_err)*cds.signal;
	ptAOVoltageOut.OutputValue=cds.signal;
	DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);

}

///////////////////////////////////////////////////////////////////////////

/////////// Altimetry ////////////////////////////

void get_altimetry(){
	HANDLE am_port;
	char am_pkt[50];
	DWORD bytes_read;
	

	DCB altiDCB;
	char alt[30];
	bool comma = false;
	int i=0;

	
	am_port=CreateFile("COM8",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(am_port==INVALID_HANDLE_VALUE){
		cout<<"could not open the altimeter port successfully"<<endl;
		return;
	}
	else{
		///////////// configuration of port //////////////////////////////
		altiDCB.DCBlength =sizeof(DCB);
		GetCommState(am_port,&altiDCB);
		altiDCB.BaudRate =9600;
		altiDCB.ByteSize =8;
		altiDCB.Parity =NOPARITY;
		altiDCB.StopBits =ONESTOPBIT;
		SetCommState(am_port,&altiDCB);
	}
			
	ReadFile(am_port,am_pkt,50,&bytes_read,NULL);
	CloseHandle(am_port);
	if(am_pkt!=NULL){

		am_pkt[bytes_read]='\0';
		
		while(!comma)
		{
			if (am_pkt[i+10]==','){
				comma=true ;
				continue;
			}
			else
				alt[i]= am_pkt[i+10];
				
			i++;
			
		}
		

		alt[i-1]='\0';
		sds.veh_alti = atof(alt);
	}
}

/////////////////////////////////////////////////////////////////////
/////////////// BattAD ///////////////////////////////////////////

int BatteryAD_Init(){

	int iRetVal;

	//--- Firstly, initialise DLL to working ---
    iRetVal=ADAMTCP_Open();
    if( iRetVal!=0 )
       return 1;
	else{
		//--- try to create a connection to 5000/TCP ---
		iRetVal=ADAMTCP_Connect(IPof5KTCP,502,iConnectionTimeout,iSendTimeout,iReceiveTimeout);
		if( iRetVal<0 )
		{
		   ADAMTCP_Close();
		   return 2;
		}
		return 0;
	}
	////////////////////////////////////////////////////////////////////////////
}
///////////////////////////////////////////////////////////////////////////
////////////////////  battery Daq ////////////////////////////////
void get_batt_status(){
	int iRetVal;
	char line[80];
	int i;
	float ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8;
	float bat1,bat2,bat3,bat4,bat5,bat6,cur1,cur2;
	FILE *log;
	char time_buffer[20];
	
    strcpy(szSend,"#01");
 
    //--- send a command to 5000/TCP and receive response --- 
	
	iRetVal=ADAMTCP_SendReceive5KTCPCmd(IPof5KTCP,szSend,szReceive,NULL,NULL);
	if( iRetVal )
	{
	   printf("ADAMTCP_SendReceive5KTCPCmd() Fail !!!   code=%d\n",iRetVal);
	   //ADAMTCP_Disconnect();
	   //ADAMTCP_Close();
		repeat=false;   
	}
 
	for(i=0;i<(strlen(szReceive)-1);i++){
		szReceive[i]=szReceive[i+1];
	}
	szReceive[i]='\0';
		
	sscanf(szReceive,"%f %f %f %f %f %f %f %f",&ch1,&ch2,&ch3,&ch4,&ch5,&ch6,&ch7,&ch8);
			
	bat1=(float)(22+((ch1-0.839)/0.038));
	bat2=(float)(22+((ch2-0.851)/0.038));
	bat3=(float)(22+((ch3-0.847)/0.038));
	bat4=(float)(22+((ch4-0.855)/0.038));
	bat5=(float)(22+((ch6-0.809)/0.037));
	bat6=(float)(22+((ch7-0.792)/0.038));
	cur1=(float)((ch5-5.936)/0.046);
	cur2=(float)((ch8-5.971)/0.046);
	
	sds.bank1=bat1;
	sds.bank2=bat2;
	sds.bank3=bat3;
	sds.bank4=bat4;
	sds.bank5=bat5;
	sds.bank6=bat6;
	sds.curr1=cur1;
	sds.curr2=cur2;
}//////////////////////////////////////////////////////////////////
///////////////////// depthdaq /////////////////////////////////////
void Depth_Daq(void){
	
	CComConfig dp_port;
	char dp_pkt[100];
	char command[6];	
	char temp[20];
	char temp1[20];

	sprintf(command,"\r");	/// when in RUN mode sensor returns data with \r command
								/// polling mode of data acquisition
	dp_port.ComInit ("COM9",2);
	dp_port.SetConfigParam (9600,8,NOPARITY,ONESTOPBIT);
	
		
	dp_port.DeviceWrite (command);
			
	strcpy(dp_pkt,dp_port.DeviceRead ());
		
	dp_port.EndCom ();

	if(dp_pkt!=NULL){
		sscanf(dp_pkt,"%s %s %f",temp,temp1,&sds.veh_depth);
	}
		
}

////////////////////////////////////////////////////////////////////
////////////////  dvl daq ///////////////////////////////////////
////////////////////////////////// --------------- DVL DAQ ------------------------------- ///////////////////////////////
void dvl_daq(){
	
	HANDLE dvlport;
	DCB portDCB;
	DWORD bytes_read;

	char data[2000],parse1[50],parse2[50],parse3[50],temp1[4],temp2[4],temp3[4],temp4[4],vel_stat[4];
	char status;
	int i=0,j=0,k=0,l=0,m=0,j2=0,j3=0;
	float temp,roll=0,pitch=0,heading=0,vel_E=0,vel_N=0,vel_z=0,dis_E=0,dis_N=0,dis_z=0,bot_rng=0;
	bool be_found=false;
	bool bd_found=false;
	bool sa_found=false;
	int count=0;

	dvlport=CreateFile("COM7",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(dvlport==INVALID_HANDLE_VALUE){
		cout<<"DVL port not opened"<<endl;
			return;
	}
		else{
			///////////// configuration of port //////////////////////////////
			portDCB.DCBlength =sizeof(DCB);
			GetCommState(dvlport,&portDCB);
			portDCB.BaudRate =9600;
			portDCB.ByteSize =8;
			portDCB.Parity =NOPARITY;
			portDCB.StopBits =ONESTOPBIT;
			SetCommState(dvlport,&portDCB);
			//cout<<"port successfully opened!"<<endl;
			
		}
	//_sleep(100);
	ReadFile(dvlport,data,1500,&bytes_read,NULL);
	CloseHandle(dvlport);
	data[bytes_read]='\0';
	cout<<data<<endl;
	//strcpy(data,dvlport.DeviceRead ());
	//cout<<data<<endl;
	for (i=0;i<strlen(data);i++){
				if(data[i]=='S' && data[i+1]=='A'){
					while(data[i]!='\r'){	
						parse1[j++]=data[i];
						i++;
					}
					if(data[i]=='\r')
						parse1[j]='\0';
					for (k=0;k<strlen(parse1);k++){
						if(parse1[k]==',')
							parse1[k]=' ';
					}
					sscanf(parse1,"%s %f %f %f",temp1,&roll,&pitch,&heading);
					sa_found=true;


				}
				if(data[i]=='B' && data[i+1]=='E'){
					while(data[i]!='\r'){	
						parse2[j2++]=data[i];
						i++;
					}
					if(data[i]=='\r')
						parse2[j2]='\0';
					for (k=0;k<strlen(parse2);k++){
						if(parse2[k]==',')
							parse2[k]=' ';

					}
					
					sscanf(parse2,"%s %f %f %f %s",temp1,&vel_E,&vel_N,&vel_z,&status);
					vel_E*=0.001;
					vel_N*=0.001;
					vel_z*=0.001;
					
					be_found=true;
				}
				if(data[i]=='B' && data[i+1]=='D'){
					while(data[i]!='\r'){	
						parse3[j3++]=data[i];
						i++;
					}
					if(data[i]=='\r')
						parse3[j3]='\0';
					for (k=0;k<strlen(parse3);k++){
						if(parse3[k]==',')
							parse3[k]=' ';
					}
					sscanf(parse3,"%s %f %f %f %f %f",temp1,&dis_E,&dis_N,&dis_z,&bot_rng,&temp);
					bd_found=true;

				}
				
			}
			
	
	if(be_found){
		dds.vel_east =vel_E;
		dds.vel_north =vel_N;
		dds.vel_Z =vel_z;
	}
	if(bd_found){
		dds.disp_east =dis_E;
		dds.disp_north =dis_N;
		dds.disp_Z =dis_z;
		dds.dvl_alti=bot_rng;
	}
	if(sa_found){
		dds.dvl_head =heading;
		dds.dvl_pitch =pitch;
		dds.dvl_roll =roll;
	}		

	//cout<<"DVL data----------------------------------------------------------"<<endl;
		
		/*if(sa_found)
			cout<<"\n"<<"roll="<<roll<<"\t"<<"pitch="<<pitch<<"\t"<<"yaw="<<heading<<endl;
		if(be_found)
			cout<<"\n"<<"vel_E="<<vel_E<<"\t"<<"vel_N="<<vel_N<<"\t"<<"vel_Z="<<vel_z<<"\tstatus:"<<status<<endl;*/
		if(bd_found)
			cout<<"DVL altimetry: "<<dds.dvl_alti<<endl;
		
	//cout<<"DVL block---------------------------------------------------------"<<endl;

	

}
////////////////////////////////////// END ////////////////////////////////////////////////////////////////
///////////////////////  flsdaq  ////////////////////////////////////////////
HANDLE FLS_port_config(void){
	HANDLE h_port;
	DCB portDCB;
	
	h_port=CreateFile("\\\\.\\COM12",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(h_port==INVALID_HANDLE_VALUE)
		cout<<"could not open the port successfully"<<endl;
	else{
		///////////// configuration of port //////////////////////////////
		portDCB.DCBlength =sizeof(DCB);
		GetCommState(h_port,&portDCB);
		portDCB.BaudRate =115200;
		portDCB.ByteSize =8;
		portDCB.Parity =NOPARITY;
		portDCB.StopBits =ONESTOPBIT;
		SetCommState(h_port,&portDCB);
		cout<<"port successfully opened!"<<endl;
	}
	return h_port;
}


int FLS_device_config(HANDLE h_port){
	//Configuring FLS with following device settings:
	//Range = 30 meters = 300 dm (0x012C)
	//Number of Bins = 200 (0xC8)
	//VOS = 1460 meters/sec
	//TxPulseLen = 75 (0x4B)
	//ADLow: 60 (0x3C) = 18 dB
	//ADSpan: 47 (0x2F) = 15 dB
	//ADInterval= 214 (0xD6)
	//Step angle: 0.9 degree (1 Grad) Step size= 16 (0x10)
	//Left Limit = 175 degrees = (2520 1/16Grads) = 0x09D8
	//Right Limit = 185 degrees = (2664 1/16Grads) = 0x0A68
	
	char mt_alive[22];//mtAlive message
	char mt_headcommand[82]={0x40,// header @
							0x30,0x30,0x34,0x43,
							0x4C,0x00,
							0xFF,
							0x02,
							0x47,
							0x13,
							0x80,
							0x02,
							0x1D,
							0x81,0x23,
							0x02,
							0x99,0x99,0x99,0x02,
							0x66,0x66,0x66,0x05,
							0xA3,0x70,0x3D,0x06,
							0x70,0x3D,0x0A,0x09,
							0x64,0x00,	// txpulselen
							0x2C,0x01,	// rangescale
							0xD8,0x09,	// leftlimit
							0x68,0x0A,	// rightlimit
							0x2F,	// ADspan
							0x5F,	// ADlow
							0x6B,
							0x6B,
							0x5A,0x00,
							0x7D,0x00,
							0x19,
							0x10,	// Step Size	
							0x41,0x01,	// ADInterval
							0xC8,0x00,	// NoBins
							0xF4,0x01,
							0x64,0x00,
							0x40,0x06,
							0x01,0x00,
							0x00,
							0x00,
							0x50,0x50,
							0x30,0x30,
							0x6B,0x6B,
							0x00,0x00,
							0x5A,0x00,
							0x7D,0x00,
							0x00,0x00,
							0x00,0x00,
							0x0A	// LF
	};//mtHeadCommand

	DWORD bytes_read;
	DWORD bytes_written;

	// write config command to FLS
	WriteFile(h_port,mt_headcommand,82,&bytes_written,NULL);
	_sleep(1000);
	// checking whether FLS configured or not
	ReadFile(h_port,mt_alive,22,&bytes_read,NULL);
	//cout<<mt_alive<<endl;
	if(mt_alive[0]=='@'){
		if(mt_alive[10]==4){
			if(mt_alive[13]==-128)////// check 14th byte : 80(config not done); 00(config done)/////
				return 1;
			else if(mt_alive[13]==0)
				return 0;
		}
	}

}


int FLS_scan_log(HANDLE h_port,char *fname){
	DWORD bytes_read;
	DWORD bytes_written;
	char mt_senddata[18]={0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,0x02,0x07,0x19,0x80,0x02,0xCA,0x64,0xB0,0x03,0x0A};//mtSendData
	char mt_headdata[1000];//mtHeadData
	char bin_data[200];
	int bearing=0;
	int bearing_low;
	int obs_bearing;
	bool left_lim_covered=false;
	bool right_lim_covered=false;

	long int i;
	long int temp=0;
	long int mask=255;
	long int i_rev=0;
	char timestring[4];
	int index=0;

	int count=300;
	int hour;
	int minutes;
	int seconds;
	
	char time_buff[20];

	FILE *fp_logbindata;

	fp_logbindata=fopen(fname,"a+");


SCANLOOP:	GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buff,20);
			for(int j=0;j<strlen(time_buff);j++){
				if(time_buff[j]==':')
					time_buff[j]=' ';
				else
					continue;
			}
			sscanf(time_buff,"%d %d %d",&hour,&minutes,&seconds);
			i=(hour*3600*1000)+(minutes*60*1000)+(seconds*1000);
			for(int k=4,int c=0;k>=1;k--,c++){
				temp=i&mask;
				temp=temp>>(c*8);
				i_rev=i_rev|(temp<<((k-1)*8));
				mask=mask<<8;
				timestring[index++]=temp;
			}
			index=0;
			///////////////////////////////////////////////////////////
			sprintf(mt_senddata+13,"%s",timestring);
			mt_senddata[17]=0x0A;
			WriteFile(h_port,mt_senddata,18,&bytes_written,NULL);
			////// read MtHeadData from FLS containing scanline data /////////
			ReadFile(h_port,mt_headdata,500,&bytes_read,NULL);
			//cout<<mt_headdata[10]<<endl;
			if(mt_headdata[0]=='@'){//check whether it is a data from FLS
				if(mt_headdata[10]==2){//check whether the data is a scanline reply
					bearing=mt_headdata[41];
					bearing=bearing<<8;
					bearing_low=mt_headdata[40];
					bearing_low=bearing_low & 255;
					bearing=bearing | bearing_low;
					obs_bearing=bearing/(16*0.9);
					if(obs_bearing==185)
						right_lim_covered=true;
					if(obs_bearing==175)
						left_lim_covered=true;
					fprintf(fp_logbindata,"%d",obs_bearing);
					for (int m=0;m<200;m++){
						bin_data[m]=mt_headdata[44+m];
						fprintf(fp_logbindata," %d",bin_data[m]);
					}
					fprintf(fp_logbindata,"\n");
				}
			}
			if(right_lim_covered && left_lim_covered)
				goto RETURN_;
			else
				goto SCANLOOP;

RETURN_:	fclose(fp_logbindata);
			return 0;
}


/////////////////////////////////////////////////////////////////////
//////////////////  Log Data   ///////////////////////////////////////////////////
void log_systemdata(char *filename){
	
	FILE *fp_log;
	char time_buffer[20];
	fp_log=fopen(filename,"a+");
		
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,20);
	fprintf(fp_log,"%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
			time_buffer,
			sds.veh_depth,
			dds.disp_Z,
			sds.veh_alti,
			dds.dvl_alti,
			sds.curr_yaw,
			dds.dvl_head,
			sds.veh_pitch,
			dds.dvl_pitch,
			sds.veh_north_pos,
			sds.veh_east_pos,
			sds.latitude,
			sds.longitude,
			sds.veh_surge,
			dds.vel_east,
			dds.vel_north,
			dds.vel_Z,
			sds.obs_rng,
			sds.obs_brng);
	fclose(fp_log);
}
void log_missionstatus(char *filename,char *miss_status){
	
	FILE *fp_log;
	char time_buffer[30];
	fp_log=fopen(filename,"a+");
		
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,30);
	fprintf(fp_log,"%s %s\n",
			time_buffer,
			miss_status);
	fclose(fp_log);
}
void log_batterystatus(char *filename){
	
	FILE *fp_log;
	char time_buffer[20];
	fp_log=fopen(filename,"a+");
		
	GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,20);
	fprintf(fp_log,"%s %f %f %f %f %f %f %f %f\n",
			time_buffer,
			sds.bank1,
			sds.bank2,
			sds.bank3,
			sds.bank4,
			sds.bank5,
			sds.bank6,
			sds.curr1,
			sds.curr2);
	fclose(fp_log);
}

/////////////////////////////////////////////////////////////////////
///////////////////////////  Phins Daq  //////////////////////////////////////////
void Phins_Daq(void){
	CComConfig ph_port;
	char p_cmd[30];

	ph_port.ComInit ("COM1",0);
	ph_port.SetConfigParam (57600,8,ODDPARITY,TWOSTOPBITS);
	sprintf(p_cmd,"$PIXSE,CONFIG,WAKEUP*40%x%x",0x0D,0x0A);
	ph_port.DeviceWrite(p_cmd);
	_sleep(100);

	strcpy(phins_pkt,ph_port.DeviceRead ());
	ph_port.EndCom ();
	position_data(phins_pkt);
	heading_data(phins_pkt);
	speed_data(phins_pkt);
	pitching_data(phins_pkt);
	latlong_data(phins_pkt);
	
		sds.curr_yaw=atof(head_data);	//PHINS 90 deg. misalignment is taken care
		sds.veh_east_pos=atof(pos_X_data);
		sds.veh_north_pos=atof(pos_Y_data);
		sds.veh_east_spd=-atof(north_data);
		sds.veh_north_spd=atof(east_data);
		sds.veh_roll=atof(roll_data);
		sds.veh_pitch=atof(pitch_data);
		sds.latitude=atof(latitude_c);
		sds.longitude=atof(longitude_c);

	sds.srg_err_old=sds.srg_err;

			//cout<<head_data<<endl;

	
}	

//--------------------------- Formatting PHINS data --------------------------------------------------//

void position_data(char *sBuffer){

	STATE ack=0;
	STATE j=0;
	char posX[10],posY[10];
	int k=0,x=0,y=0;
	

for(int i=0;i<strlen(sBuffer);i++){

	if(j==0){
			if(sBuffer[i]==UTM_zone){
			
				if(ack<1){
					ack=1;
					offset=i;
				}
				j=1;
				
			}
			else
				j=0;
			
			continue;
		}
	
		
	if(j==1){
			if(sBuffer[i]==','){
			
				j=2;
				if(ack<2){
					ack=2;
					offset=i-2;
				}
			}
			else
				j=0;

			continue;
		}
		
		
	

	if(j==2){
			if(sBuffer[i]==','){
			
				j=3;
				if(ack<3){
					ack=3;
					offset=i-3;
				}
			}
			else
				j=2;

			continue;
		}
		
		


	if(j==3){
			if(sBuffer[i]==','){
			
				j=4;
				if(ack<4){
					ack=4;
					offset=i-4;
				}
			}
			else
				pos_X_data[x++]=sBuffer[i];

			continue;
		}
		
		

	if(j==4){
			if(sBuffer[i]==','){
			
				j=5;
				if(ack<5){
					ack=5;
					offset=i-5;
				}
			}
			else
				pos_Y_data[y++]=sBuffer[i];

			continue;
		}
		
		
}	
	switch(ack){
	case 0:
		//printf("found nothing\n");
		//printf("data is:%s\n",sBuffer);
		//Sleep(100);
		break;
	case 1:
		//printf("found P at offset: %d\n",offset);
		break;
	case 4:
		posX[x]='\0';
		//printf("found positionX %s at offset: %d\n",pos_X_data,offset);
		break;
	case 5:
		posX[x]='\0';
		//printf("found positionX %s \t",pos_X_data,offset);
		posY[y]='\0';
		//printf("found positionY %s \n",pos_Y_data,offset);
		//sprintf(pack,"# %d %d $",posX,posY);
		//printf("PositionX,Y=%s\n",pack);
		break;
	default:
		//printf("value of ack is: %d\n!!!",ack);
		//printf("!!!!state unrecognised!!!!\n");
		break;
	}

}

void pitching_data(char *sBuffer){

	STATE ack=0;
	STATE j=0;
	int k=0,x=0,y=0;

	

for(int i=0;i<strlen(sBuffer);i++){

	
		if(j==0){
			if(sBuffer[i]=='A'){
			
				if(ack<1){
					ack=1;
					offset=i;
				}
				j=1;
				
			}
			else
				j=0;
			
			continue;
		}
		if(j==1){
			if(sBuffer[i]=='T'){
			
				if(ack<2){
					ack=2;
					offset=i-1;
				}
				j=2;
			}
			else
				j=0;
			
			continue;
		}
		if(j==2){
			if(sBuffer[i]=='I'){
				if(ack<3){
					ack=3;
					offset=i-2;
				}
				j=3;
			}
			else
				j=0;
			
			continue;
		}
		if(j==3){
			if(sBuffer[i]=='T'){
				
				j=4;
				if(ack<4){
					ack=4;
					offset=i-3;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==4){
			if(sBuffer[i]=='U'){
			
				j=5;
				if(ack<5){
					ack=5;
					offset=i-4;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==5){
			if(sBuffer[i]=='D'){
			
				j=6;
				if(ack<6){
					ack=6;
					offset=i-5;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==6){
			if(sBuffer[i]==','){
			
				j=7;
				if(ack<7){
					ack=7;
					offset=i-6;
				}
			}
			else
				j=0;
				
			continue;
		}

	if(j==7){
			if(sBuffer[i]==','){
			
				j=8;
				if(ack<8){
					ack=8;
					offset=i-8;
				}
			}
			else
				roll_data[x++]=sBuffer[i];
				
			continue;
		}
	if(j==8){
			if(sBuffer[i]==','){
			
				j=9;
				if(ack<9){
					ack=9;
					offset=i-9;
				}
			}
			else
				pitch_data[y++]=sBuffer[i];
				
			continue;
		}
		
		
	}
	switch(ack){
	case 0:
		//printf("found nothing\n");
		//printf("data is:%s\n",sBuffer);
		//Sleep(100);
		break;
	
	case 9:
		east_data[x]='\0';
		north_data[y]='\0';

		//printf("found East Speed %s \t",east_data,offset);
		
		//printf("found North Speed %s \n",north_data,offset);
		
		break;
	default:
		//printf("!!!!state unrecognised!!!!\n");
		break;
	}
	j=0;
	ack=0;
	k=0;
	offset=0;
}

void latlong_data(char *sBuffer){

	STATE ack=0;
	STATE j=0;
	int k=0,x=0,y=0;

	

for(int i=0;i<strlen(sBuffer);i++){

	
		if(j==0){
			if(sBuffer[i]=='P'){
			
				if(ack<1){
					ack=1;
					offset=i;
				}
				j=1;
				
			}
			else
				j=0;
			
			continue;
		}
		if(j==1){
			if(sBuffer[i]=='O'){
			
				if(ack<2){
					ack=2;
					offset=i-1;
				}
				j=2;
			}
			else
				j=0;
			
			continue;
		}
		if(j==2){
			if(sBuffer[i]=='S'){
				if(ack<3){
					ack=3;
					offset=i-2;
				}
				j=3;
			}
			else
				j=0;
			
			continue;
		}
		if(j==3){
			if(sBuffer[i]=='I'){
				
				j=4;
				if(ack<4){
					ack=4;
					offset=i-3;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==4){
			if(sBuffer[i]=='T'){
			
				j=5;
				if(ack<5){
					ack=5;
					offset=i-4;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==5){
			if(sBuffer[i]=='I'){
			
				j=6;
				if(ack<6){
					ack=6;
					offset=i-5;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==6){
			if(sBuffer[i]==','){
			
				j=7;
				if(ack<7){
					ack=7;
					offset=i-6;
				}
			}
			else
				j=0;
				
			continue;
		}

	if(j==7){
			if(sBuffer[i]==','){
			
				j=8;
				if(ack<8){
					ack=8;
					offset=i-8;
				}
			}
			else
				latitude_c[x++]=sBuffer[i];
				
			continue;
		}
	if(j==8){
			if(sBuffer[i]==','){
			
				j=9;
				if(ack<9){
					ack=9;
					offset=i-9;
				}
			}
			else
				longitude_c[y++]=sBuffer[i];
				
			continue;
		}
		
		
	}
	switch(ack){
	case 0:
		//printf("found nothing\n");
		//printf("data is:%s\n",sBuffer);
		//Sleep(100);
		break;
	
	case 9:
		latitude_c[x]='\0';
		longitude_c[y]='\0';

		//printf("found East Speed %s \t",east_data,offset);
		
		//printf("found North Speed %s \n",north_data,offset);
		
		break;
	default:
		//printf("!!!!state unrecognised!!!!\n");
		break;
	}
	j=0;
	ack=0;
	k=0;
	offset=0;
}

void heading_data(char *sBuffer){
	//char *head;
	//char head_data[50];
	STATE ack=0;
	STATE j=0;
	int k=0;


	
	
	for(int i=0;i<strlen(sBuffer);i++){

	
		if(j==0){
			if(sBuffer[i]=='H'){
			
				if(ack<1){
					ack=1;
					offset=i;
				}
				j=1;
				
			}
			else
				j=0;
			
			continue;
		}
		if(j==1){
			if(sBuffer[i]=='E'){
			
				if(ack<2){
					ack=2;
					offset=i-1;
				}
				j=2;
			}
			else
				j=0;
			
			continue;
		}
		if(j==2){
			if(sBuffer[i]=='H'){
				//if(c=='H'){
				if(ack<3){
					ack=3;
					offset=i-2;
				}
				j=3;
			}
			else
				j=0;
			
			continue;
		}
		if(j==3){
			if(sBuffer[i]=='D'){
				
				j=4;
				if(ack<4){
					ack=4;
					offset=i-3;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==4){
			if(sBuffer[i]=='T'){
			
				j=5;
				if(ack<5){
					ack=5;
					offset=i-4;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==5){
			if(sBuffer[i]==','){
			
				j=6;
				if(ack<6){
					ack=6;
					offset=i-5;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==6){
			if(sBuffer[i]==','){
			
				j=7;
				if(ack<7){
					ack=7;
					offset=i-6;
				}
			}
			else
				head_data[k++]=sBuffer[i];
				
			continue;
		}
		
		
	}
	switch(ack){
	case 0:
		//printf("found nothing\n");
		//printf("data is:%s\n",sBuffer);
		//Sleep(100);
		break;
	
	case 5:
		printf("!!!!successfully found HEHDT!!!! at offset: %d\n",offset);
		break;
	case 7:
		head_data[k]='\0';
		//printf("found heading %s at offset: %d\n",head_data,offset);
		//head=head_data;
		//return head;
		break;
	default:
		//printf("!!!!state unrecognised!!!!\n");
		break;
	}
	j=0;
	ack=0;
	k=0;
	offset=0;



	
}



void speed_data(char *sBuffer){
	//char *head;
	//char head_data[50];
	STATE ack=0;
	STATE j=0;
	int k=0,x=0,y=0;

	

for(int i=0;i<strlen(sBuffer);i++){

	
		if(j==0){
			if(sBuffer[i]=='S'){
			
				if(ack<1){
					ack=1;
					offset=i;
				}
				j=1;
				
			}
			else
				j=0;
			
			continue;
		}
		if(j==1){
			if(sBuffer[i]=='P'){
			
				if(ack<2){
					ack=2;
					offset=i-1;
				}
				j=2;
			}
			else
				j=0;
			
			continue;
		}
		if(j==2){
			if(sBuffer[i]=='E'){
				if(ack<3){
					ack=3;
					offset=i-2;
				}
				j=3;
			}
			else
				j=0;
			
			continue;
		}
		if(j==3){
			if(sBuffer[i]=='E'){
				
				j=4;
				if(ack<4){
					ack=4;
					offset=i-3;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==4){
			if(sBuffer[i]=='D'){
			
				j=5;
				if(ack<5){
					ack=5;
					offset=i-4;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==5){
			if(sBuffer[i]=='_'){
			
				j=6;
				if(ack<6){
					ack=6;
					offset=i-5;
				}
			}
			else
				j=0;
			
			continue;
		}
		if(j==6){
			if(sBuffer[i]==','){
			
				j=7;
				if(ack<7){
					ack=7;
					offset=i-6;
				}
			}
			else
				j=0;
				
			continue;
		}

	if(j==7){
			if(sBuffer[i]==','){
			
				j=8;
				if(ack<8){
					ack=8;
					offset=i-8;
				}
			}
			else
				east_data[x++]=sBuffer[i];
				
			continue;
		}
	if(j==8){
			if(sBuffer[i]==','){
			
				j=9;
				if(ack<9){
					ack=9;
					offset=i-9;
				}
			}
			else
				north_data[y++]=sBuffer[i];
				
			continue;
		}
		
		
	}
	switch(ack){
	case 0:
		//printf("found nothing\n");
		//printf("data is:%s\n",sBuffer);
		//Sleep(100);
		break;
	
	case 9:
		east_data[x]='\0';
		north_data[y]='\0';

		//printf("found East Speed %s \t",east_data,offset);
		
		//printf("found North Speed %s \n",north_data,offset);
		
		break;
	default:
		//printf("!!!!state unrecognised!!!!\n");
		break;
	}
	j=0;
	ack=0;
	k=0;
	offset=0;

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////  shutdown Reboot //////////////////////////////////////////////////
////////////////////////// System Shutdown and Reboot//////////////////////////////////////////

BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege)   // to enable or disable privilege
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if ( !LookupPrivilegeValue( 
			NULL,            // lookup privilege on local system
			lpszPrivilege,   // privilege to lookup 
			&luid ) )        // receives LUID of privilege
	{
		//log("Could not find privelege" + string(lpszPrivilege));
		return FALSE; 
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	else
		tp.Privileges[0].Attributes = 0;

	// Enable the privilege or disable all privileges.

	if ( !AdjustTokenPrivileges(
		   hToken, 
		   FALSE, 
		   &tp, 
		   sizeof(TOKEN_PRIVILEGES), 
		   (PTOKEN_PRIVILEGES) NULL, 
		   (PDWORD) NULL) )
	{
		//log("Could not adjust the privelege");
		return FALSE; 
	} 

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)

	{
		//log("ERROR_NOT_ALL_ASSIGNED");
		return FALSE;
	} 

	return TRUE;
}


int AdjustPriv()
{
	HANDLE hToken = NULL;

	 if (!OpenProcessToken(GetCurrentProcess(), 
                          TOKEN_QUERY|TOKEN_ADJUST_PRIVILEGES, 
                          &hToken)) 
       {
		  //log("Could not open process token");
		  return 1;
       } 

    // Enable the SE_SHUTDOWN_NAME privilege.
    if(!SetPrivilege(hToken, SE_SHUTDOWN_NAME, TRUE))
	{
		//log("Could not set SE_SHUTDOWN_NAME privelege in token");
	}
    
	// Enable the SE_TCB_NAME privilege.
	/*if(!SetPrivilege(hToken, SE_TCB_NAME, TRUE))
	{
		//log("Could not set SE_TCB_NAME privelege in token");
	}
	
	if(!SetPrivilege(hToken, SE_ASSIGNPRIMARYTOKEN_NAME, TRUE))
	{
		//log("Could not set SE_ASSIGNPRIMARYTOKEN_NAME privelege in token");
	}
	
	if(!SetPrivilege(hToken, SE_INCREASE_QUOTA_NAME, TRUE))
	{
		//log("Could not set SE_INCREASE_QUOTA_NAME privelege in token");
	}
	*/
	CloseHandle(hToken);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//////////////////////  TDA  ///////////////////////////////////////////////
int ThrusterDA_Init(){
	DEVLIST     DeviceList[MAX_DEVICES];
	
	BOOL        bRun = FALSE;                       // flag for running
	float       fBuffer[4] = {5.0f, 2.5f, 1.25f, 0.0f};
	USHORT      gwIndex = 0;
	USHORT      gwDevice = 0, gwSubDevice = 0;      // device index
	USHORT      gwChannel = 0;                      // input channel
	SHORT       gnNumOfDevices;  // number of installed devices
	int         nOutEntries;
	LONG ErrCde;				// error code
	float A_OUT=0.00;

	/// getting the list of devices installed
	DRV_DeviceGetNumOfList(&gnNumOfDevices);

	/// getting info regarding the installed devices
	DRV_DeviceGetList((DEVLIST far *)&DeviceList[0],MAX_DEVICES,(SHORT far *)&nOutEntries);
	/// initialising the installed device
	ErrCde=DRV_DeviceOpen(DeviceList[0].dwDeviceNum ,(LONG far *)&DriverHandle);
	if(ErrCde!=SUCCESS){
		return 1;
	}
	else{
		/// setting AO parameters
		ptAOConfig.chan =gwChannel;
		ptAOConfig.MaxValue =5.00;
		ptAOConfig.MinValue =-5.00;
		ErrCde=DRV_AOConfig(DriverHandle,&ptAOConfig);
		if(ErrCde!=SUCCESS)
			return 2;
		else{
			/// output data
			ptAOVoltageOut.chan =gwChannel;
			return 0;
		}
	}
}
/////////////////////////////////////////////////////////////////////
///////////////////////////  yaw correct modules //////////////////////////////////////////
float LeastSweepAngle(float ref,float curr){
	float err_anti;
	float err_clock;

	if(ref>=curr){
		err_clock=ref-curr;
		err_anti=-(360-err_clock);
	}
	else{
		err_anti=ref-curr;
		err_clock=360-abs(err_anti);
	}
	if(abs(err_anti)>=err_clock)
		return err_clock;
	else
		return err_anti;
	
}
int PID_HeadingCtrl(float yw_er,float *err_old){
 
 int Torque;
 float Kp_head=1.0,Ki_head=0.6,Kd_head=2;	
 float P,I=0.00,D;
 
 if (*err_old=123456) 
	 *err_old=yw_er;
 
 P = Kp_head*yw_er;
 I = I+ Ki_head*yw_er*(250/1000);
 D = Kd_head*(yw_er-*err_old)*(1000/250);
 
 Torque=(int)(P+I+D); //+12;
 
	//else
	 //Torque=(int)(P+I+D); //-15;
 
 
 *err_old=yw_er;
 
 //Limiting Higer and Lower Thruster I/P

 if (Torque >60 ) 
	 Torque =60;
 
 if (Torque <-60)	
     Torque =-60;
 
 if (Torque>=0 && Torque <=15) 
	 Torque =15;

 if (Torque>=-11 && Torque <=-0.00001) 
	 Torque =-11;
 return Torque;

}

/////////////////////////////////////////////////////////////////////
void get_objrange_bearing(){
	//acquiring data from FLS with following gain settings:
	//Range = 30 meters = 300 dm (0x012C)
	//Number of Bins = 200 (0xC8)
	//VOS = 1460 meters/sec
	//TxPulseLen = 75 (0x4B)
	//ADLow: 60 (0x3C) = 18 dB
	//ADSpan: 47 (0x2F) = 15 dB
	//ADInterval= 214 (0xD6)
	//Step angle: 0.9 degree (1 Grad) Step size= 16 (0x10)
	//Left Limit = 175 degrees = (2520 1/16Grads) = 0x09D8
	//Right Limit = 185 degrees = (2664 1/16Grads) = 0x0A68
	HANDLE h_port;
	DCB portDCB;
	
	h_port=CreateFile("\\\\.\\COM12",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
	if(h_port==INVALID_HANDLE_VALUE)
		cout<<"could not open the port successfully"<<endl;
	else{
		///////////// configuration of port //////////////////////////////
		portDCB.DCBlength =sizeof(DCB);
		GetCommState(h_port,&portDCB);
		portDCB.BaudRate =115200;
		portDCB.ByteSize =8;
		portDCB.Parity =NOPARITY;
		portDCB.StopBits =ONESTOPBIT;
		SetCommState(h_port,&portDCB);
	}


	char mt_senddata[18]={0x40,0x30,0x30,0x30,0x43,0x0C,0x00,0xFF,0x02,0x07,0x19,0x80,0x02,0xCA,0x64,0xB0,0x03,0x0A};//mtSendData
	char mt_headdata[1000];//mtHeadData
	char bin_data[200];
	int bin_max;
	int bin_max_val=0;
	int threshold=117;
	int bearing=0;
	int bearing_low;
	float obs_range;
	int obs_bearing;
	int rel_obs_bearing;
	int left_max=0;
	int left_min=0;
	int right_max=0;
	int right_min=0;
	int deviation;
	DWORD bytes_written;
	DWORD bytes_read;
	bool left_lim_covered=false;
	bool right_lim_covered=true;
	char b=15;
	float desired_range;
	int bin_min;

	long int i;
	long int temp=0;
	long int mask=255;
	long int i_rev=0;
	char timestring[4];
	int index=0;

	bool repeat=true;
	int count=300;
	int hour;
	int minutes;
	int seconds;
	
	FILE *fp;
	char time_buff[20];

	FILE *fp_logbindata;

	////////////////////------------ start data acquisition ---------------------------//////////////////////////

	GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buff,20);
	//printf("%s\n",time_buff);
	for(int j=0;j<strlen(time_buff);j++){
		if(time_buff[j]==':')
			time_buff[j]=' ';
		else
			continue;
	}
	sscanf(time_buff,"%d %d %d",&hour,&minutes,&seconds);
	i=(hour*3600*1000)+(minutes*60*1000)+(seconds*1000);
	//printf("%X %d\n",i,sizeof(i));
	for(int k=4,int c=0;k>=1;k--,c++){
		
		//printf("%X\n",mask);
		temp=i&mask;
		temp=temp>>(c*8);
		//printf("%X\n",temp);
		i_rev=i_rev|(temp<<((k-1)*8));
		mask=mask<<8;
		timestring[index++]=temp;
	}
	index=0;
	
	///////////////////////////////////////////////////////////
	sprintf(mt_senddata+13,"%s",timestring);
	mt_senddata[17]=0x0A;
	WriteFile(h_port,mt_senddata,18,&bytes_written,NULL);
	////// read first MtHeadData from FLS containing scanline data /////////
	ReadFile(h_port,mt_headdata,500,&bytes_read,NULL);
	//cout<<mt_headdata[10]<<endl;
	if(mt_headdata[0]=='@'){//check whether it is a data from FLS
		if(mt_headdata[10]==2){//check whether the data is a scanline reply
			bearing=mt_headdata[41];
			bearing=bearing<<8;
			bearing_low=mt_headdata[40];
			bearing_low=bearing_low & 255;
			bearing=bearing | bearing_low;
			obs_bearing=bearing/(16*0.9);
			rel_obs_bearing=obs_bearing-180;
			if(obs_bearing==185)
				right_lim_covered=true;
			if(obs_bearing==175)
				left_lim_covered=true;
			
			for (int m=0;m<200;m++){
				bin_data[m]=mt_headdata[44+m];
			}
			float constraint;
			int no_of_effective_bins=0;
			int eff_bin_total=0;
			int bin_avg=0;
			for (m=0;m<200;m++){
				if(bin_data[m]>=threshold){
					constraint=(30*m)/200;
					if(constraint>0.5){
						bin_min=m;
						break;
					}
				}
					
			}
			obs_range=(30.0*bin_min)/200;
			if(obs_range>0.5){
				sds.obs_rng = obs_range;
				sds.obs_brng = obs_bearing;
			}
		}
	}
	CloseHandle(h_port);
		////////////////////////---------------- end of data acquisition block ----------------------------------------/////////////////////////////////
}



// #########################################################################/////