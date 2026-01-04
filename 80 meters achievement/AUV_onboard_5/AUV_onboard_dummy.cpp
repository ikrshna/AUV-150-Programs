//AUV_onboard_dummy.cpp

#include "AUV_onboard_dummy.h"  // Include custom header file containing function prototypes and definitions

int main(){ 
    
    int command;                // Command value received from operator
    int alti_chk_cnt;           // Counter for altimetry checks
    int count;                  // General purpose counter
    bool alti_ok;               // Flag indicating altimetry status
    int alti_prb=0;             // Altimetry problem counter
    int tda_retval;             // Return value for thruster DA initialization
    int bda_retval;             // Return value for battery DA initialization
    int FLS_retval;             // Return value for FLS (Forward Looking Sonar) configuration
    char fls_name[30];          // File name for FLS data
    
    FILE *fp_missfile;          // File pointer to mission file
    FILE *fp_UTMfile;           // File pointer to UTM coordinates file
    char readline[23];          // Buffer for reading lines
    char buff[23];              // General buffer
    int index=0;                // Index variable
    FILE *log_dp;               // Log file pointer
    float veh_depth=0;          // Vehicle depth
    float veh_alti_ALT=0;       // Vehicle altitude from altimeter
    float veh_alti_DVL=0;       // Vehicle altitude from DVL (Doppler Velocity Log)
    float actual_desired_surge; // Actual desired surge value

    HANDLE FLS_port;            // Handle for FLS serial port

    // Processor related variables for real-time operation
    DWORD dwPriClass;           // Priority class
    SYSTEM_INFO sys_info;       // System information
    int proc;                   // Processor ID
    int no_procs;               // Number of processors on system

    // Initialize mission related flags
    bool mission_started;       // Flag denoting just-started status of mission
    bool dive_complete;         // Flag denoting status of diving process
    bool obj_found=false;       // Flag indicating if object detected
    bool obj_found_again=false; // Flag indicating if object detected again
    depth_err_computed=false;   // Flag indicating depth error computed

    // Create log files for logging system and mission data
    GetSystemTime(&st);  // Get current system time
    
    // Generate log file names with timestamp
    sprintf(sys_filename,"stat%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);
    sprintf(mission_filename,"miss%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);
    sprintf(battery_filename,"batt%d_%d_%d_%d%d%d.txt",st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute,st.wSecond);

    // Check the number of processors on the system
    GetSystemInfo(&sys_info);
    no_procs=sys_info.dwNumberOfProcessors;
    cout<<"MH:Number of processors on the system: "<<no_procs<<endl;
    sprintf(gen_msg_buffer,"MH:Number of processors on the system: %d",no_procs);
    log_missionstatus(sys_filename,gen_msg_buffer);  // Log processor info

    // Configure Realtime property of the system
    if(!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)){
        cout<<"MH:Could not be configured as RealTime Process!!!"<<endl;
        sprintf(gen_msg_buffer,"MH:Could not be configured as RealTime Process!!!");
        log_missionstatus(sys_filename,gen_msg_buffer);    
        cout<<"MH:Program exiting:Premature Termination!!!"<<endl;
        sprintf(gen_msg_buffer,"MH:Program exiting:Premature Termination!!!");
        log_missionstatus(sys_filename,gen_msg_buffer);
        return 0;  // Exit if real-time priority cannot be set
    }
    else{
        dwPriClass=GetPriorityClass(GetCurrentProcess());
        printf("current priority:0x%x\n",dwPriClass);
        cout<<"MH:Configured as RealTime Process!!!"<<endl;
        sprintf(gen_msg_buffer,"MH:Configured as RealTime Process!!!");
        log_missionstatus(sys_filename,gen_msg_buffer);
    }

    // Execute the main thread on processor ID: 0
    proc=SetThreadIdealProcessor(GetCurrentThread(),0);
    
    // Initialize communication with Surface Control
    co_hwnd=CreateThread(NULL,0,Communicator,(void *)&command,NULL,id_co);  // Start communicator for receiving commands
    ac_hwnd=CreateThread(NULL,0,Acoustic_comm,(void *)&command,NULL,id_ac);  // Start acoustic communicator
    proc=SetThreadIdealProcessor(co_hwnd,0);  // Set processor affinity
    proc=SetThreadIdealProcessor(ac_hwnd,0);  // Set processor affinity

    // Read UTM Zone and validating coordinates
    fp_UTMfile=fopen("utm_coord.txt","r");
    if(fp_UTMfile==NULL){
        cout<<"MH:UTM file not opened"<<endl;
        sprintf(gen_msg_buffer,"MH:Could not open UTM coords file\n");
        log_missionstatus(sys_filename,gen_msg_buffer);
        ExitThread(exit_th);  // Exit if UTM file not found
    }
    else{
        sprintf(gen_msg_buffer,"MH:Opened UTM coords file\n");
        log_missionstatus(sys_filename,gen_msg_buffer);
        fscanf(fp_UTMfile,"%c %lf %lf",&UTM_zone,&validate_north,&validate_east);  // Read UTM data
        cout<<validate_north<<validate_east<<endl;
        fclose(fp_UTMfile);
    }

    //########################################### Initialising starts ################################////
    // Initialize devices and check for operator's commands
    tda_retval=ThrusterDA_Init();  // Initialize thruster digital-to-analog converter
    if(tda_retval){
        cout<<"MH:USB DA initialisation failed!!!"<<endl;
        sprintf(gen_msg_buffer,"MH:USB DA initialisation failed!!!");
        log_missionstatus(sys_filename,gen_msg_buffer);
        ExitThread(exit_th);  // Exit if initialization fails
    }
    else{
        cout<<"MH:USB DA initialisation complete!!!"<<endl;
        sprintf(gen_msg_buffer,"MH:USB DA initialisation complete!!!");
        log_missionstatus(sys_filename,gen_msg_buffer);
        bda_retval=BatteryAD_Init();  // Initialize battery analog-to-digital converter
        if(bda_retval){
            cout<<"MH:TCP/IP DA initialisation failed!!!"<<endl;
            sprintf(gen_msg_buffer,"MH:TCP/IP DA initialisation failed!!!");
            log_missionstatus(sys_filename,gen_msg_buffer);
            ptAOVoltageOut.OutputValue=0.00 ;  // Set output voltage to 0
            DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);  // Write to DA
            DRV_DeviceClose(&DriverHandle);  // Close device
            ExitThread(exit_th);  // Exit if initialization fails
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

    // Initialize sensors and check out data acquisition
    while(start_north<=validate_north && start_east<=validate_east){
        Phins_Daq();  // Get PHINS (Position and Heading) data
        start_north=sds.veh_north_pos;  // Update start north position
        start_east=sds.veh_east_pos;    // Update start east position
        _sleep(250);  // Wait 250ms
    }
    cout<<start_north<<"\t"<<start_east<<endl;
    cout<<"MH:PHINS initialized and data consistent!!!"<<endl;
    sprintf(gen_msg_buffer,"MH:PHINS initialized and data consistent!!!");
    log_missionstatus(sys_filename,gen_msg_buffer);
    
    // Check pressure sensor data
    while(veh_depth==0){
        Depth_Daq();  // Get depth data
        _sleep(100);  // Wait 100ms
        veh_depth=sds.veh_depth;  // Update vehicle depth
    }
    cout<<"MH:Pressure sensor data consistent!!!"<<endl;
    sprintf(gen_msg_buffer,"MH:Pressure sensor data consistent!!!");
    log_missionstatus(sys_filename,gen_msg_buffer);

    // Open FLS port and configure SONAR
    FLS_port=FLS_port_config();  // Configure FLS port
    FLS_retval=FLS_device_config(FLS_port);  // Configure FLS device
    CloseHandle(FLS_port);  // Close FLS port handle
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

    // Open mission file
    fp_missfile=fopen("mission_file.txt","r");
    if(fp_missfile==NULL){
        cout<<"MH:Mission file could not be opened"<<endl;
        sprintf(gen_msg_buffer,"MH:Mission file could not be opened");
        log_missionstatus(sys_filename,gen_msg_buffer);
        cout<<"MH:Mission Program: Premature Termination"<<endl;
        sprintf(gen_msg_buffer,"MH:Mission File Not Opened: Premature Termination");
        log_missionstatus(sys_filename,gen_msg_buffer);
        ptAOVoltageOut.OutputValue=0.00 ;  // Reset output voltage
        DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
        DRV_DeviceClose(&DriverHandle);  // Close device
        ADAMTCP_Disconnect();  // Disconnect ADAM TCP
        ADAMTCP_Close();  // Close ADAM TCP
        ExitThread(exit_th);  // Exit thread
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

    // Create mutexes for synchronising with heading and surge controllers
    hmutex_hc=CreateMutex(NULL,FALSE,"mutex_hc");  // Heading control mutex
    hmutex_sc=CreateMutex(NULL,FALSE,"mutex_sc");  // Surge control mutex
    hmutex_sc_phinsread=CreateMutex(NULL,FALSE,"mutex_sc_phins");  // Surge control PHINS mutex
    hmutex_hc_phinsread=CreateMutex(NULL,FALSE,"mutex_hc_phins");  // Heading control PHINS mutex

Ready_: 
    // Wait for mission start command
    while(command!=MISS_START){
        // Send sensor data to surface control
        Phins_Daq();       // Get position and heading
        Depth_Daq();       // Get depth
        get_altimetry();   // Get altitude
        get_batt_status(); // Get battery status

        // Check for abort command
        if(command==MISS_ABORT){
            cout<<"MH:Mission Abort Command Received"<<endl;
            sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
            log_missionstatus(sys_filename,gen_msg_buffer);
            cout<<"MH:Mission Program: Premature Termination"<<endl;
            sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
            log_missionstatus(sys_filename,gen_msg_buffer);
            ptAOVoltageOut.OutputValue=0.00 ;  // Reset output
            DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
            DRV_DeviceClose(&DriverHandle);
            ADAMTCP_Disconnect();
            ADAMTCP_Close();
            ExitThread(exit_th);  // Exit on abort
        }
        
        // Check for manual actuation command
        if(command==MAN_ACTUATE){
            cds.head_nosetorque=ct_val;  // Set nose torque
            cds.head_tailtorque=ct_val;  // Set tail torque
            cds.signal=mt_val;           // Set main thruster signal
            tele_actuation();             // Execute actuation
            command=0;                    // Reset command
        }
        _sleep(150);  // Wait 150ms
        cout<<"within Ready"<<endl;
    }
    
    // Mission start sequence
    cout<<"Mission shall commence within 8 minutes"<<endl;
    sprintf(gen_msg_buffer,"MH:Mission shall commence within 8 minutes!!!\n");
    log_missionstatus(sys_filename,gen_msg_buffer);
    _sleep(8*60*1000);  // Wait 8 minutes
    
    // Mission started
    mission_started=true;
    cout<<"MH:Mission started!!!"<<endl;
    sprintf(gen_msg_buffer,"MH:Mission started!!!\n");
    log_missionstatus(sys_filename,gen_msg_buffer);
    
    // Create watchdog timer thread
    wt_hwnd=CreateThread(NULL,0,Watchdog_Timer,(void *)NULL,NULL,id_wt);
    
    // Create diving controller thread if depth command exists
    if(rds.depth_des!=0){
        dc_hwnd=CreateThread(NULL,0,depth_Ctrl,(void *)0,0,sg_id);
        if(dc_hwnd==NULL){
            cout<<"MH:Diving could not be initiated"<<endl;
            sprintf(gen_msg_buffer,"MH:Diving could not be initiated");
            log_missionstatus(sys_filename,gen_msg_buffer);
            ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
            DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
            DRV_DeviceClose(&DriverHandle);
            ADAMTCP_Disconnect();
            ADAMTCP_Close();
            cout<<"MH:Dive thread not created: Premature Termination"<<endl;
            sprintf(gen_msg_buffer,"MH:Dive thread not created: Premature Termination");
            log_missionstatus(sys_filename,gen_msg_buffer);    
            ExitThread(exit_th);  // Exit if dive thread fails
        }
        else{
            proc=SetThreadIdealProcessor(dc_hwnd,1);  // Set processor affinity
            dive_complete=false;  // Set dive incomplete
            sprintf(gen_msg_buffer,"MH:Diving initiated");
            log_missionstatus(sys_filename,gen_msg_buffer);
        }
    }
    else
        dive_complete=true;  // No dive required
    
    // Main mission loop
    while(mission_started){        
        if(dive_complete){
            // Create heading and surge control threads
            hdHwnd=CreateThread(NULL,0,heading_ctrl,(void *)0,0,hd_id);
            if(hdHwnd==NULL){
                cout<<"MH:Heading Correction could not be initiated"<<endl;
                ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                DRV_DeviceClose(&DriverHandle);
                ADAMTCP_Disconnect();
                ADAMTCP_Close();
                if(dc_hwnd!=NULL)
                    TerminateThread(dc_hwnd,exit_th);  // Terminate dive thread
                cout<<"MH:Mission Program: Premature Termination"<<endl;
                sprintf(gen_msg_buffer,"MH:Heading Control not Initiated");
                log_missionstatus(sys_filename,gen_msg_buffer);
                ExitThread(exit_th);  // Exit on failure
            }
            
            sgHwnd=CreateThread(NULL,0,surge_ctrl,(void *)0,0,sg_id);
            if(sgHwnd==NULL){
                cout<<"MH:Surge Correction could not be initiated"<<endl;
                ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                DRV_DeviceClose(&DriverHandle);
                ADAMTCP_Disconnect();
                ADAMTCP_Close();
                if(dc_hwnd!=NULL)
                    TerminateThread(dc_hwnd,exit_th);  // Terminate dive thread
                cout<<"MH:Mission Program: Premature Termination"<<endl;
                sprintf(gen_msg_buffer,"MH:Surge Control not Initiated");
                log_missionstatus(sys_filename,gen_msg_buffer);
                ExitThread(exit_th);  // Exit on failure
            }
            
            sprintf(gen_msg_buffer,"MH:Initiated XY Operation");
            log_missionstatus(sys_filename,gen_msg_buffer);
            
            // Start reading mission file segments
Start_:     if(feof(fp_missfile))  // Check if end of mission file
                goto MissionComplete;  // Go to mission complete
            else{
                // Read trajectory segment from mission file
                fscanf(fp_missfile,"%f %f %f",&rds.yaw_des,&rds.sg_des,&traj_hold_time);
                Phins_Daq();  // Get current position
                start_north=sds.veh_north_pos;  // Update start position
                start_east=sds.veh_east_pos;
            }
            
            sprintf(gen_msg_buffer,"MH:Read trajectory segment");
            log_missionstatus(sys_filename,gen_msg_buffer);
            
            // Release mutexes for controller threads to access reference data
            ReleaseMutex(hmutex_hc);
            ReleaseMutex(hmutex_sc);
            
            // Wait for segment timeout
            SegmentTimedOut=false;
            MissionTimedOut=false;
            tt_hwnd=CreateThread(NULL,0,traj_timer,(void*)0,NULL,id_tt);  // Create trajectory timer
            
            while(!SegmentTimedOut){
                // Check for mission watchdog timer timeout
                if(MissionTimedOut){
                    cout<<"MH:Mission Watchdog Timer timed out!!!"<<endl;
                    sprintf(gen_msg_buffer,"MH:Mission Watchdog Timer timed out!!!");
                    log_missionstatus(sys_filename,gen_msg_buffer);
                    // Terminate all control threads
                    TerminateThread(dc_hwnd,exit_th);
                    TerminateThread(hdHwnd,exit_th);
                    TerminateThread(sgHwnd,exit_th);
                    CloseHandle(dc_hwnd);
                    CloseHandle(hdHwnd);
                    CloseHandle(sgHwnd);
                    CloseHandle(hmutex_hc);
                    CloseHandle(hmutex_sc);
                    ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                    DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                    DRV_DeviceClose(&DriverHandle);
                    ADAMTCP_Disconnect();
                    ADAMTCP_Close();
                    cout<<"MH:Watchdog Timed Out:premature termination!!!"<<endl;
                    ExitThread(exit_th);  // Exit on timeout
                }
                
                // Check for abort command
                if(command==MISS_ABORT){
                    cout<<"MH:Mission Abort Command Received"<<endl;
                    sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
                    cout<<"MH:Mission Program: Premature Termination"<<endl;
                    sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
                    log_missionstatus(sys_filename,gen_msg_buffer);
                    // Terminate all control threads
                    TerminateThread(dc_hwnd,exit_th);
                    TerminateThread(hdHwnd,exit_th);
                    TerminateThread(sgHwnd,exit_th);
                    CloseHandle(dc_hwnd);
                    CloseHandle(hdHwnd);
                    CloseHandle(sgHwnd);
                    CloseHandle(hmutex_hc);
                    CloseHandle(hmutex_sc);
                    ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                    DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                    DRV_DeviceClose(&DriverHandle);
                    ADAMTCP_Disconnect();
                    ADAMTCP_Close();
                    ExitThread(exit_th);  // Exit on abort
                }
                
                // Read sensor data
                Phins_Daq();       // Get position and heading
                dvl_daq();         // Get DVL data
                get_altimetry();   // Get altitude
                get_batt_status(); // Get battery status
                log_batterystatus(battery_filename);  // Log battery status
                
                // Check for obstacles using FLS
                get_objrange_bearing();  // Get object range and bearing
                cout<<"Brng:"<<sds.obs_brng<<"\tRange:"<<sds.obs_rng<<endl;
                
                // Obstacle detection logic
                if(sds.obs_brng<=182 && sds.obs_brng>=177){
                    if(sds.obs_rng<=6.0 && sds.obs_rng>0){
                        obj_found=true;  // Object detected
                    }
                    else{
                        obj_found=false;
                        obj_found_again=false;
                    }
                }
                
                log_systemdata(mission_filename);  // Log sensor data
                
                // Collision prevention - change reference surge to 1 meter
                if(obj_found && !obj_found_again){
                    if(rds.sg_des!=0 && rds.depth_des!=0){
                        WaitForSingleObject(hmutex_sc,INFINITE);  // Acquire surge control mutex
                        rds.sg_des=1.0;  // Reduce surge to 1 meter
                        start_north=sds.veh_north_pos;  // Update start position
                        start_east=sds.veh_east_pos;
                        ReleaseMutex(hmutex_sc);  // Release mutex
                        obj_found_again=true;  // Mark as detected
                        cout<<"Collision Prevention triggered!!!"<<endl;
                        sprintf(gen_msg_buffer,"MH:Collision Prevention triggered!!!\n");
                        log_missionstatus(sys_filename,gen_msg_buffer);
                    }
                }
                
                mt_actuation();  // Actuate main thruster
                _sleep(150);     // Wait 150ms
            }
            
            // Block controller threads from reading reference data
            WaitForSingleObject(hmutex_hc,INFINITE);
            WaitForSingleObject(hmutex_sc,INFINITE);
            cout<<"MH:Segment Completed!!!"<<endl;
            sprintf(gen_msg_buffer,"MH:Segment Completed!!!\n");
            log_missionstatus(sys_filename,gen_msg_buffer);
            goto Start_;  // Go to next segment
        }
        else{
            // Diving in progress
            sprintf(gen_msg_buffer,"MH:Diving in progress");
            log_missionstatus(sys_filename,gen_msg_buffer);

            // Check for diving completion
            if(depth_err_computed){
                if(abs(depth_err)<=0.1){  // Check if depth error within tolerance
                    _sleep(DIVE_SETTLING_TIME);  // Wait for settling
                    sprintf(gen_msg_buffer,"MH:Desired Depth Correction Achieved");
                    log_missionstatus(sys_filename,gen_msg_buffer);
                    cout<<"depth achieved"<<endl;
                    
                    // Create heading and surge control threads after dive
                    hdHwnd=CreateThread(NULL,0,heading_ctrl,(void *)0,0,hd_id);
                    if(hdHwnd==NULL){
                        cout<<"MH:Heading Correction could not be initiated"<<endl;
                        ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
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
                        ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
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
                    dive_complete=true;  // Mark dive as complete
                    goto Start_;  // Start XY operation
                }
            }
            
            // Check for mission watchdog timer timeout during dive
            if(MissionTimedOut){
                cout<<"MH:Mission Watchdog Timer timed out!!!"<<endl;
                sprintf(gen_msg_buffer,"MH:Watchdog Timed Out");
                log_missionstatus(sys_filename,gen_msg_buffer);
                ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                DRV_DeviceClose(&DriverHandle);
                ADAMTCP_Disconnect();
                ADAMTCP_Close();
                TerminateThread(dc_hwnd,exit_th);  // Terminate dive thread
                CloseHandle(dc_hwnd);
                cout<<"MH:Mission program exiting:premature termination!!!"<<endl;
                ExitThread(exit_th);
            }
            
            // Check for abort command during dive
            if(command==MISS_ABORT){
                cout<<"MH:Mission Abort Command Received"<<endl;
                sprintf(gen_msg_buffer,"MH:Mission Abort Command Received");
                cout<<"MH:Mission Program: Premature Termination"<<endl;
                sprintf(gen_msg_buffer,"MH:ABORT Command Received: Premature Termination");
                log_missionstatus(sys_filename,gen_msg_buffer);
                TerminateThread(dc_hwnd,exit_th);  // Terminate dive thread
                CloseHandle(dc_hwnd);
                ptAOVoltageOut.OutputValue=0.00 ;  // Reset outputs
                DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
                DRV_DeviceClose(&DriverHandle);
                ADAMTCP_Disconnect();
                ADAMTCP_Close();
                ExitThread(exit_th);
            }
            
            // Read sensor data during dive
            Phins_Daq();       // Get position and heading
            dvl_daq();         // Get DVL data
            get_altimetry();   // Get altitude
            log_systemdata(mission_filename);  // Log sensor data
            get_batt_status(); // Get battery status
            log_batterystatus(battery_filename);  // Log battery status
            _sleep(150);       // Wait 150ms
        }
    }
                
    //############################## Mission Complete #######################################//
MissionComplete:
    // Close all thread handles and reset devices
    TerminateThread(dc_hwnd,exit_th);  // Terminate dive control thread
    TerminateThread(hdHwnd,exit_th);   // Terminate heading control thread
    TerminateThread(sgHwnd,exit_th);   // Terminate surge control thread
    CloseHandle(dc_hwnd);              // Close dive thread handle
    CloseHandle(hdHwnd);               // Close heading thread handle
    CloseHandle(sgHwnd);               // Close surge thread handle
    CloseHandle(hmutex_hc);            // Close heading mutex
    CloseHandle(hmutex_sc);            // Close surge mutex
    ADAMTCP_Disconnect();              // Disconnect ADAM TCP
    ADAMTCP_Close();                   // Close ADAM TCP
    ptAOVoltageOut.OutputValue=0.00 ;  // Reset output voltage
    DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);
    DRV_DeviceClose(&DriverHandle);    // Close device
    cout<<"MH:Mission successfully completed!!!"<<endl;
    sprintf(gen_msg_buffer,"MH:Mission successfully completed!!!");
    log_missionstatus(sys_filename,gen_msg_buffer);
    ExitThread(exit_th);  // Exit main thread
}

/////////////// Threads //////////////////////////////////

//###################### COMMUNICATION Threads Block START #######################///////// 
///////////// Communication with Surface //////////////////////

unsigned long _stdcall Communicator(void *data){
    SOCKET hSock;        // Server socket
    SOCKET hClient;      // Client socket
    sockaddr_in sa;      // Server address structure
    sockaddr_in saClient; // Client address structure
    bool connected=false; // Connection status flag

    char command_header;  // First character of received command
    char ser_sendbuf[20]; // Serial send buffer
    char rf_sendbuf[200]; // RF send buffer
    char recvBuffer[12];  // Receive buffer
    char refdepth[5];     // Reference depth buffer
    char missdur[5];      // Mission duration buffer
    char ct_command[5];   // Command torque buffer
    char mt_command[5];   // Main thruster command buffer
    int reference_depth;  // Parsed reference depth
    int mission_duration; // Parsed mission duration

    // Variable declarations for communicating with DVS (Depth Velocity Sensor)
    HANDLE dvsPort;       // DVS serial port handle
    COMMTIMEOUTS timeout; // Communication timeout structure
    DCB PortDCB;          // Device control block
    DWORD bytesRead;      // Bytes read count

    char buffer[80];      // General buffer
    int leak_status;      // Leak detection status
    struct _timeb start;  // Start time for timeout
    struct _timeb stop;   // Stop time for timeout
    int timeelapsed;      // Elapsed time
    DWORD iBytesWritten;  // Bytes written count

    // Initialize winsock2.2 dll
    WSADATA wsaData={0};
    WORD wVersionRequested=MAKEWORD(2,2);
    int nRet=WSAStartup(wVersionRequested,&wsaData);
    if(nRet==SOCKET_ERROR){
        cout<<"ERROR : "<<WSAGetLastError()<<endl;
    }

    // Open a socket
    hSock=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
    if(hSock==INVALID_SOCKET){
        cout<<"Invalid socket,failed to create the socket..."<<endl;
    }

    // Name the socket
    sa.sin_family=PF_INET;
    sa.sin_port=htons(8888);  // Port 8888
    sa.sin_addr.S_un.S_addr=htonl(INADDR_ANY);  // Any IP address

    // Bind the socket's name
    nRet=bind(hSock,(sockaddr*)&sa,sizeof(sa));
    if (nRet==SOCKET_ERROR){
        cout<<"ERROR:"<<WSAGetLastError()<<endl;
    }
        
    
    while(1){
        // Listen for connections
        cout<<"Listening for connections..."<<endl;
        nRet=listen(hSock,5);  // Connection backlog queue set to 5
        if (nRet==SOCKET_ERROR){
            cout<<"ERROR:"<<WSAGetLastError()<<endl;
            closesocket(hSock);
        }
        
        // Accept connection
        int nSALen=sizeof(sockaddr);
        hClient=accept(hSock,(sockaddr*)&saClient,&nSALen);
        if (hClient==INVALID_SOCKET){
            cout<<"Invalid client socket,connection failed!"<<endl;
            closesocket(hSock);
        }
        else{
            cout<<"Connection established"<<endl;
        }
        
        // Communication loop
        for(;;){
            // Receive data from client
            cout<<"Ready to receive data"<<endl;
            int inDataLength=recv(hClient,recvBuffer,sizeof(recvBuffer),0);
            recvBuffer[inDataLength]='\0';
            cout<<recvBuffer<<endl;
            
            // Parse command header
            sscanf(recvBuffer,"%c ",&command_header);
            
            // Process command based on header
            switch(command_header)
            {
                case 'S':
                    // START command
                    *((int *)data)=MISS_START;  // Set mission start flag
                    cout<<"START command received!!!"<<endl;
                    break;
                    
                case 'A':
                    // ABORT command
                    PS=1024;  // Set program status
                    *((int *)data)=MISS_ABORT;  // Set mission abort flag
                    cout<<"ABORT command received!!!"<<endl;
                    break;
                    
                case 'N':
                    // Light ON command
                    cout<<"LIGHT ON command received!!!"<<endl;
                    dvsPort=CreateFile("COM2",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
                    if(dvsPort!=INVALID_HANDLE_VALUE){
                        // Configure serial port
                        PortDCB.DCBlength = sizeof (DCB); 
                        GetCommState(dvsPort,&PortDCB);
                        PortDCB.BaudRate = 9600;      
                        PortDCB.ByteSize = 8;              
                        PortDCB.Parity = NOPARITY;         
                        PortDCB.StopBits = ONESTOPBIT;    
                        SetCommState(dvsPort,&PortDCB);
                        SetCommTimeouts(dvsPort,&timeout);
                        
                        // Send light ON command
                        WriteFile(dvsPort,"N",1,&iBytesWritten,NULL);
                        
                        // Check for leak status with timeout
                        _ftime(&start);
                        ReadFile(dvsPort,buffer,1,&bytesRead,NULL);
                        _ftime(&stop);
                        timeelapsed=stop.time-start.time;
                        if(timeelapsed<=1){
                            buffer[bytesRead]='\0';
                            leak_status=atoi(buffer);
                            sds.leak=leak_status;  // Update leak status
                        }
                        CloseHandle(dvsPort);
                    }
                    break;
                    
                case 'F':
                    // Light OFF command
                    cout<<"LIGHT OFF command received!!!"<<endl;
                    dvsPort=CreateFile("COM2",GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
                    if(dvsPort!=INVALID_HANDLE_VALUE){
                        // Configure serial port
                        PortDCB.DCBlength = sizeof (DCB); 
                        GetCommState(dvsPort,&PortDCB);
                        PortDCB.BaudRate = 9600;      
                        PortDCB.ByteSize = 8;              
                        PortDCB.Parity = NOPARITY;         
                        PortDCB.StopBits = ONESTOPBIT;    
                        SetCommState(dvsPort,&PortDCB);
                        SetCommTimeouts(dvsPort,&timeout);
                        
                        // Send light OFF command
                        WriteFile(dvsPort,"F",1,&iBytesWritten,NULL);
                        
                        // Check for leak status with timeout
                        _ftime(&start);
                        ReadFile(dvsPort,buffer,1,&bytesRead,NULL);
                        _ftime(&stop);
                        timeelapsed=stop.time-start.time;
                        if(timeelapsed<=1){
                            buffer[bytesRead]='\0';
                            leak_status=atoi(buffer);
                            sds.leak=leak_status;  // Update leak status
                        }
                        CloseHandle(dvsPort);
                    }
                    break;
                    
                case 'H':
                    // System shutdown command
                    *((int *)data)=SYS_HALT;  // Set system halt flag
                    cout<<"SYSTEM SHUTDOWN command received!!!"<<endl;
                    AdjustPriv();  // Adjust privileges
                    InitiateSystemShutdown(NULL,NULL,5,true,false);  // Initiate shutdown
                    // Close socket communication
                    closesocket(hClient);
                    closesocket(hSock);
                    ExitThread(exit_th);  // Exit thread
                    
                case 'R':
                    // System reboot command
                    *((int *)data)=SYS_REBOOT;  // Set system reboot flag
                    cout<<"SYSTEM REBOOT command received!!!"<<endl;
                    AdjustPriv();  // Adjust privileges
                    InitiateSystemShutdown(NULL,NULL,5,true,true);  // Initiate reboot
                    // Close socket communication
                    closesocket(hClient);
                    closesocket(hSock);
                    ExitThread(exit_th);  // Exit thread
                    
                case 'P':
                    // Mission particulars update
                    sscanf(recvBuffer+1,"%s %s",refdepth,missdur);  // Parse depth and duration
                    reference_depth=atoi(refdepth);  // Convert to integer
                    mission_duration=atoi(missdur);  // Convert to integer
                    mission_time=mission_duration;   // Set mission time
                    rds.depth_des =reference_depth;  // Set reference depth
                    cout<<rds.depth_des<<endl;
                    sprintf(gen_msg_buffer,"CO:Mission Time: %d Reference Depth: %d",mission_time,reference_depth);
                    log_missionstatus(sys_filename,gen_msg_buffer);  // Log mission details
                    break;
                    
                case 'M':
                    // Manual control command
                    sscanf(recvBuffer+1,"%s %s",ct_command,mt_command);  // Parse torque and thruster commands
                    ct_val=atoi(ct_command);  // Set command torque value
                    mt_val=atof(mt_command);  // Set main thruster value
                    *((int *)data)=MAN_ACTUATE;  // Set manual actuation flag
                    break;
                    
                case 'D':
                    // Sensor data request
                    // Format sensor data into buffer
                    sprintf(rf_sendbuf,"%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %d",
                        sds.veh_alti ,dds.dvl_alti,sds.veh_roll,sds.veh_pitch,sds.curr_yaw ,sds.veh_surge,
                        sds.bank1,sds.bank2,sds.bank3,sds.bank4,sds.bank5,sds.bank6,
                        sds.latitude, sds.longitude, sds.veh_east_pos,sds.veh_north_pos,
                        sds.leak);
                    cout<<rf_sendbuf<<endl;
                    send(hClient,rf_sendbuf,strlen(rf_sendbuf),0);  // Send sensor data
                    break;
                    
                case 'G':
                    // General message request
                    send(hClient,gen_msg_buffer,strlen(gen_msg_buffer),0);  // Send general message
                    break;
                    
                case 'T':
                    // Termination command
                    closesocket(hClient);  // Close client socket
                    closesocket(hSock);    // Close server socket
                    ExitThread(exit_th);   // Exit thread
                    break;
            }
        }
    }
    return 0;
}

// Acoustic communication thread
unsigned long _stdcall Acoustic_comm(void *data){
    HANDLE hSerial;      // Serial port handle
    DCB dcb;             // Device control block
    DWORD bytesRead;     // Bytes read count
    DWORD bytesWritten;  // Bytes written count
    COMMTIMEOUTS timeout; // Timeout structure
    bool port_open=false; // Port open status
    char form_depth[10];  // Formatted depth string
    char form_alti[10];   // Formatted altitude string
    char form_head[10];   // Formatted heading string
    int result;           // Formatting result
    char time_buffer[30]; // Time buffer

    // Try to open serial port
_PortOpen:    
    hSerial=CreateFile("\\\\.\\COM13",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);

    if(hSerial!=INVALID_HANDLE_VALUE){
        // Configure serial port
        dcb.DCBlength = sizeof(DCB); 
        GetCommState(hSerial,&dcb); 
        dcb.BaudRate = 9600;      // 9600 baud rate
        dcb.ByteSize = 8;         // 8 data bits
        dcb.Parity = NOPARITY;    // No parity
        dcb.StopBits = ONESTOPBIT; // 1 stop bit
        SetCommState(hSerial,&dcb);

        timeout.ReadTotalTimeoutConstant =5000;  // 5 second read timeout
        timeout.ReadTotalTimeoutMultiplier =0;
        timeout.WriteTotalTimeoutConstant =0;
        timeout.WriteTotalTimeoutMultiplier =0;
        SetCommTimeouts(hSerial,&timeout);

        port_open=true;  // Port successfully opened
    }
    
    // Main communication loop
    while(1){
        if(port_open){
            // Read data from acoustic modem
            ReadFile(hSerial,ac_buffer,8,&bytesRead,NULL);
            if(ac_buffer!=NULL){
                ac_buffer[bytesRead]='\0';
                if(ac_buffer[0]=='X'){
                    // Abort command received
                    *((int *)data)=MISS_ABORT;  // Set abort flag
                    sprintf(gen_msg_buffer,"AC:ABORT Command Received from Surface");
                    log_missionstatus(sys_filename,gen_msg_buffer);  // Log abort
                    CloseHandle(hSerial);  // Close serial port
                    return 0;  // Exit thread
                }
            }
            
            // Format depth for transmission (2 decimal places with leading zeros)
            result=sds.veh_depth/10;
            if(result==0){
                sprintf(form_depth,"00%.1f",sds.veh_depth);
            }
            else if(result>0 && result<10){
                sprintf(form_depth,"0%.1f",sds.veh_depth);
            }
            else if(result>=10){
                sprintf(form_depth,"%.1f",sds.veh_depth);
            }
            
            // Format altitude for transmission
            result=sds.veh_alti/10;
            if(result==0){
                sprintf(form_alti,"00%.1f",sds.veh_alti);
            }
            else if(result>0 && result<10){
                sprintf(form_alti,"0%.1f",sds.veh_alti);
            }
            else if(result>=10){
                sprintf(form_alti,"%.1f",sds.veh_alti);
            }
            
            // Format heading for transmission
            result=sds.curr_yaw/10;
            if(result==0){
                sprintf(form_head,"00%.1f",sds.curr_yaw);
            }
            else if(result>0 && result<10){
                sprintf(form_head,"0%.1f",sds.curr_yaw);
            }
            else if(result>=10){
                sprintf(form_head,"%.1f",sds.curr_yaw);
            }
            
            // Get current time
            GetTimeFormat(LOCALE_SYSTEM_DEFAULT,TIME_FORCE24HOURFORMAT,NULL,"HH':'mm':'ss",time_buffer,30);
            
            // Format acoustic data packet
            sprintf(ac_buffer,"$T,%s,$D,%s,$A,%s,$H,%s",time_buffer,form_depth,form_alti,form_head);
            
            // Write data to acoustic modem
            WriteFile(hSerial,ac_buffer,strlen(ac_buffer),&bytesWritten,NULL);
        }
        else
            goto _PortOpen;  // Try to open port again
    }

    CloseHandle(hSerial);  // Close serial port
    return 0;
}
/////////////////////////////////////////////////////////////////
//###################### COMMUNICATION Threads Block END #######################///////// 

//###################### CONTROL Threads Block START #######################///////// 

////////////////// Depth Control /////////////////////////////////////
unsigned long _stdcall depth_Ctrl(void *data){
    float err;          // Current depth error
    float prev_err=0;   // Previous depth error
    float err_rate;     // Depth error rate
    float Kp=5.0;       // Proportional gain
    float Kd=1.0;       // Derivative gain

    cout<<"depth control thread running"<<endl;

    PS=63;  // Set program status
    
    while(1){
        Depth_Daq();  // Get current depth
        
        // Calculate depth error
        err=rds.depth_des-sds.veh_depth;  // Desired depth minus current depth
        depth_err=err;  // Store depth error
        depth_err_computed=true;  // Set flag

        err_rate=(err-prev_err)*4;  // Calculate error rate (4 Hz sampling)
        
        // Calculate control outputs for vertical thrusters
        cds.depth_tailtorque =-((Kp*err+Kd*err_rate)+50);  // Tail vertical thruster
        cds.depth_nosetorque =-((Kp*err+Kd*err_rate)+60);  // Nose vertical thruster

        vt_actuation();  // Actuate vertical thrusters
        
        prev_err=err;  // Store current error as previous

        _sleep(100);  // 100ms control loop period (10 Hz)
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////////

/////////////////////// Heading Control ////////////////////////////////////////////
unsigned long _stdcall heading_ctrl(void *data){
    float yaw_err_curr;  // Current yaw error
    float yaw_err_old=123456;  // Previous yaw error (initialized)
    int torque;  // Control torque output

    cout<<"heading controller thread running...."<<endl;
    
    // Open the mutex held by main thread
    hmutex_hc=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_hc");

    while (1){
        // Wait till main thread has updated reference data structures
        WaitForSingleObject(hmutex_hc,INFINITE);
        
        // Execute control algorithm
        // Calculate yaw error using least sweep angle
        yaw_err_curr=LeastSweepAngle(rds.yaw_des,sds.curr_yaw);
        
        // Calculate PID control torque
        torque=-PID_HeadingCtrl(yaw_err_curr,&yaw_err_old);
        
        // Set control outputs for horizontal thrusters
        cds.head_nosetorque=torque;  // Nose horizontal thruster
        cds.head_tailtorque=torque;  // Tail horizontal thruster
        
        // Send commands to horizontal control thrusters
        ht_actuation();
        
        // Release the lock for main thread
        ReleaseMutex(hmutex_hc);
        
        _sleep(250);  // 250ms control loop period (4 Hz)
    }
    return 0;
}
////////////////////////////////////////////////////////////////////////////////

//////////////////////// Surge Control /////////////////////////////////////////
unsigned long _stdcall surge_ctrl(void *data){
    float veh_east_disp;     // Vehicle east displacement
    float veh_north_disp;    // Vehicle north displacement
    float veh_east_spddisp;  // Vehicle east speed displacement
    float veh_north_spddisp; // Vehicle north speed displacement
    float veh_eastphins;     // PHINS east position
    float veh_northphins;    // PHINS north position
    float old_veh_res_disp=0; // Previous resultant displacement
    
    float Kp,Ki,Kd;  // PID gains
    float P,I,D;     // PID terms
    
    // PID gains for surge control
    Kp=0.4;
    Ki=0.2;
    Kd=2.0;
    I=0.00;

    cout<<"surge control thread running"<<endl;
    
    // Open the mutex held by main thread
    hmutex_sc=OpenMutex(MUTEX_ALL_ACCESS,FALSE,"mutex_sc");
    
    while(1){
        // Wait till main thread has updated reference data structures
        WaitForSingleObject(hmutex_sc,INFINITE);

        // Calculate vehicle displacement from PHINS data
        veh_eastphins=start_east-sds.veh_east_pos;    // East displacement
        veh_northphins=start_north-sds.veh_north_pos; // North displacement
        veh_res_disp=sqrt((veh_eastphins*veh_eastphins)+(veh_northphins*veh_northphins)); // Resultant displacement
        
        // Handle boundary conditions
        if(sds.veh_north_pos<=validate_north || sds.veh_east_pos<=validate_east){
            veh_res_disp=old_veh_res_disp;  // Use previous displacement
            sds.srg_err=0.0;  // Zero error
        }
        else{
            sds.srg_err =rds.sg_des - veh_res_disp;  // Calculate surge error
            old_veh_res_disp=veh_res_disp;  // Store current displacement
        }
        
        sds.veh_surge =veh_res_disp;  // Store vehicle surge
        
        if(rds.sg_des!=0){
            // Calculate PID control terms
            cds.P=Kp*sds.srg_err;  // Proportional term
            cds.D=Kd*(sds.srg_err-sds.srg_err_old)*4.0;  // Derivative term (4 Hz)
            
            // Calculate control signal with deadzone compensation
            if((cds.P+cds.D)<0){
                if((cds.P+cds.D-1.7)<-4.0)
                    cds.signal=-4.0;  // Limit negative signal
                else
                    cds.signal=cds.P+cds.D-1.7;  // Negative bias compensation
            }
            else{
                if((cds.P+cds.D+1.7)>4.0)
                    cds.signal=4.0;  // Limit positive signal
                else
                    cds.signal=cds.P+cds.D+1.7;  // Positive bias compensation
            }
        }
        else{
            cds.signal=0.0;  // No surge command
        }
        
        // Release the lock for main thread
        ReleaseMutex(hmutex_sc);
        
        _sleep(250);  // 250ms control loop period (4 Hz)
    }
    
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
//###################### CONTROL Threads Block END #######################///////// 
        

//###################### TIMER Threads Block START #######################///////// 
//////////////////////////// Watchdog Timer //////////////////////////////////
unsigned long _stdcall Watchdog_Timer(void *data){
    _sleep(1000*60*mission_time);  // Sleep for mission duration in minutes
    PS=2048;  // Set program status
    MissionTimedOut=true;  // Set mission timeout flag
    return 0;        
}
///////////////////////////// Traject Timer //////////////////////////////////
unsigned long _stdcall traj_timer(void *data){
    _sleep(1000*60*traj_hold_time);  // Sleep for trajectory hold time in minutes
    PS=256;  // Set program status
    SegmentTimedOut=true;  // Set segment timeout flag
    return 0;
}
//////////////////////////////////////////////////////////////////////////////

//######################### Associated Modules #############################//////
////////////////////////// Actuation Coord //////////////////////////////
// Teleoperation actuation function
void tele_actuation(){
    CComConfig hor;  // Communication configuration for horizontal thrusters
    int res_tailtorque;  // Resultant tail torque
    int res_nosetorque;  // Resultant nose torque
    char command_n[10];  // Nose command buffer
    char command_t[10];  // Tail command buffer

    // Initialize horizontal thruster communication
    hor.ComInit ("COM6",1);
    hor.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);  // 57600 baud, 8N1

    // Get torque values from control data structure
    res_tailtorque=cds.head_tailtorque;
    res_nosetorque=cds.head_nosetorque;
        
    // Send commands to horizontal thrusters based on torque values
    if(res_tailtorque<0){
        if(res_tailtorque<-60) res_tailtorque=-60;  // Limit negative torque
        sprintf(command_t,"u37-%d\r",abs(res_tailtorque));  // Format tail command
        hor.DeviceWrite (command_t);  // Send to tail thruster
    }
    if(res_nosetorque<0){
        if(res_nosetorque<-60) res_nosetorque=-60;  // Limit negative torque
        sprintf(command_n,"u17-%d\r",abs(res_nosetorque));  // Format nose command
        hor.DeviceWrite (command_n);  // Send to nose thruster
    }
    if(res_tailtorque>0){
        if(res_tailtorque>60) res_tailtorque=60;  // Limit positive torque
        sprintf(command_t,"u37+%d\r",abs(res_tailtorque));  // Format tail command
        hor.DeviceWrite (command_t);  // Send to tail thruster
    }
    if(res_nosetorque>0){
        if(res_nosetorque>60) res_nosetorque=60;  // Limit positive torque
        sprintf(command_n,"u17+%d\r",abs(res_nosetorque));  // Format nose command
        hor.DeviceWrite (command_n);  // Send to nose thruster
    }
    
    hor.EndCom ();  // End communication
    
    // Send main thruster signal if driver is available
    if(DriverHandle!=NULL){
        ptAOVoltageOut.OutputValue=cds.signal;  // Set output voltage
        DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);  // Send to main thruster
        _sleep(3000);  // Wait 3 seconds
        ptAOVoltageOut.OutputValue=0.0;  // Reset output
        DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);  // Send zero signal
    }
}

// Vertical thruster actuation function
void vt_actuation(void){
    int tail_cmd;  // Tail vertical thruster command
    int nose_cmd;  // Nose vertical thruster command
    CComConfig tail_vert;  // Tail vertical thruster communication
    CComConfig nose_vert;  // Nose vertical thruster communication
    char command_nv[10];  // Nose vertical command buffer
    char command_tv[10];  // Tail vertical command buffer
    
    // Initialize vertical thruster communications
    tail_vert.ComInit ("COM5",1);  // Tail on COM5
    nose_vert.ComInit ("COM4",1);  // Nose on COM4

    tail_vert.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);  // Tail: 57600 baud, 8N1
    nose_vert.SetConfigParam (9600,8,NOPARITY,ONESTOPBIT);   // Nose: 9600 baud, 8N1

    // Get commands from control data structure
    tail_cmd=cds.depth_tailtorque;
    nose_cmd=cds.depth_nosetorque;

    // Send commands to vertical thrusters
    if(tail_cmd<0){
        if(tail_cmd<-80) tail_cmd=-80;  // Limit negative torque
        sprintf(command_tv,"u67-%d\r",abs(tail_cmd));  // Format tail command
        tail_vert.DeviceWrite (command_tv);  // Send to tail
    }
    if(tail_cmd>0){
        if(tail_cmd>80) tail_cmd=80;  // Limit positive torque
        sprintf(command_tv,"u67+%d\r",abs(tail_cmd));  // Format tail command
        tail_vert.DeviceWrite (command_tv);  // Send to tail
    }
    if(nose_cmd<0){
        if(nose_cmd<-80) nose_cmd=-80;  // Limit negative torque
        sprintf(command_nv,"u17-%d\r",abs(nose_cmd));  // Format nose command
        nose_vert.DeviceWrite (command_nv);  // Send to nose
    }
    if(nose_cmd>0){
        if(nose_cmd>80) nose_cmd=80;  // Limit positive torque
        sprintf(command_nv,"u17+%d\r",abs(nose_cmd));  // Format nose command
        nose_vert.DeviceWrite (command_nv);  // Send to nose
    }
    
    // End communications
    tail_vert.EndCom ();
    nose_vert.EndCom ();
}

// Horizontal thruster actuation function
void ht_actuation(void){
    CComConfig hor;  // Communication configuration
    int res_tailtorque;  // Resultant tail torque
    int res_nosetorque;  // Resultant nose torque
    char command_n[10];  // Nose command buffer
    char command_t[10];  // Tail command buffer

    // Initialize horizontal thruster communication
    hor.ComInit ("COM6",1);
    hor.SetConfigParam (57600,8,NOPARITY,ONESTOPBIT);  // 57600 baud, 8N1

    // Get torque values from control data structure
    res_tailtorque=cds.head_tailtorque;
    res_nosetorque=cds.head_nosetorque;
        
    // Send commands to horizontal thrusters
    if(res_tailtorque<0){
        if(res_tailtorque<-60) res_tailtorque=-60;  // Limit negative torque
        sprintf(command_t,"u37-%d\r",abs(res_tailtorque));  // Format tail command
        hor.DeviceWrite (command_t);  // Send to tail
    }
    if(res_tailtorque>0){
        if(res_tailtorque>60) res_tailtorque=60;  // Limit positive torque
        sprintf(command_t,"u37+%d\r",abs(res_tailtorque));  // Format tail command
        hor.DeviceWrite (command_t);  // Send to tail
    }
    
    _sleep(100);  // Small delay between commands
    
    if(res_nosetorque<0){
        if(res_nosetorque<-60) res_nosetorque=-60;  // Limit negative torque
        sprintf(command_n,"u17-%d\r",abs(res_nosetorque));  // Format nose command
        hor.DeviceWrite (command_n);  // Send to nose
    }
    if(res_nosetorque>0){
        if(res_nosetorque>60) res_nosetorque=60;  // Limit positive torque
        sprintf(command_n,"u17+%d\r",abs(res_nosetorque));  // Format nose command
        hor.DeviceWrite (command_n);  // Send to nose
    }
    
    hor.EndCom ();  // End communication
}

// Main thruster actuation function
void mt_actuation(void){
    float norm_yaw_err;  // Normalized yaw error
    
    // Calculate normalized yaw error (0-1 range)
    norm_yaw_err=abs(rds.yaw_des -sds.curr_yaw )/5.0;
    if(norm_yaw_err>=1)
        norm_yaw_err=1;
    else if(norm_yaw_err<1)
        norm_yaw_err=0;
    
    // Send main thruster command
    ptAOVoltageOut.OutputValue=cds.signal;  // Set output voltage
    DRV_AOVoltageOut(DriverHandle,(LPT_AOVoltageOut)&ptAOVoltageOut);  // Send to main thruster
}

///////////////////////////////////////////////////////////////////////////

/////////// Altimetry ////////////////////////////

// Get altitude from altimeter
void get_altimetry(){
    HANDLE am_port;      // Altimeter port handle
    char am_pkt[50];     // Altimeter packet buffer
    DWORD bytes_read;    // Bytes read count
    

    DCB altiDCB;         // Device control block
    char alt[30];        // Altitude string buffer
    bool comma = false;  // Comma detection flag
    int i=0;             // Index counter

    
    // Open altimeter serial port
    am_port=CreateFile("COM8",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(am_port==INVALID_HANDLE_VALUE){
        cout<<"could not open the altimeter port successfully"<<endl;
        return;
    }
    else{
        // Configure serial port
        altiDCB.DCBlength =sizeof(DCB);
        GetCommState(am_port,&altiDCB);
        altiDCB.BaudRate =9600;      // 9600 baud rate
        altiDCB.ByteSize =8;         // 8 data bits
        altiDCB.Parity =NOPARITY;    // No parity
        altiDCB.StopBits =ONESTOPBIT; // 1 stop bit
        SetCommState(am_port,&altiDCB);
    }
            
    // Read data from altimeter
    ReadFile(am_port,am_pkt,50,&bytes_read,NULL);
    CloseHandle(am_port);  // Close port
    
    if(am_pkt!=NULL){
        am_pkt[bytes_read]='\0';  // Null terminate
        
        // Parse altitude data (look for comma delimiter)
        while(!comma)
        {
            if (am_pkt[i+10]==','){
                comma=true ;  // Found comma
                continue;
            }
            else
                alt[i]= am_pkt[i+10];  // Copy altitude character
                
            i++;
        }
        
        alt[i-1]='\0';  // Null terminate altitude string
        sds.veh_alti = atof(alt);  // Convert to float and store
    }
}

/////////////////////////////////////////////////////////////////////
/////////////// BattAD ///////////////////////////////////////////

// Initialize battery analog-to-digital converter
int BatteryAD_Init(){
    int iRetVal;  // Return value

    // Initialize DLL for ADAM TCP communication
    iRetVal=ADAMTCP_Open();
    if( iRetVal!=0 )
       return 1;  // Return error if DLL init fails
    else{
        // Create connection to 5000/TCP
        iRetVal=ADAMTCP_Connect(IPof5KTCP,502,iConnectionTimeout,iSendTimeout,iReceiveTimeout);
        if( iRetVal<0 )
        {
           ADAMTCP_Close();  // Close DLL on error
           return 2;  // Return connection error
        }
        return 0;  // Success
    }
}
///////////////////////////////////////////////////////////////////////////
////////////////////  battery Daq ////////////////////////////////
// Get battery status from ADAM module
void get_batt_status(){
    int iRetVal;        // Return value
    char line[80];      // Line buffer
    int i;              // Index
    float ch1,ch2,ch3,ch4,ch5,ch6,ch7,ch8;  // Raw channel readings
    float bat1,bat2,bat3,bat4,bat5,bat6,cur1,cur2;  // Calculated values
    FILE *log;          // Log file pointer
    char time_buffer[20];  // Time buffer
    
    strcpy(szSend,"#01");  // Command to read all channels
 
    // Send command to ADAM module and receive response
    iRetVal=ADAMTCP_SendReceive5KTCPCmd(IPof5KTCP,szSend,szReceive,NULL,NULL);
    if(!iRetVal){
        // Process response (remove leading character)
        for(i=0;i<(strlen(szReceive)-1);i++){
            szReceive[i]=szReceive[i+1];
        }
        szReceive[i]='\0';  // Null terminate
            
        // Parse channel values
        sscanf(szReceive,"%f %f %f %f %f %f %f %f",&ch1,&ch2,&ch3,&ch4,&ch5,&ch6,&ch7,&ch8);
                
        // Calculate battery voltages and currents (calibrated)
        bat1=(float)(22+((ch1-0.839)/0.038));  // Battery 1 voltage
        bat2=(float)(22+((ch2-0.851)/0.038));  // Battery 2 voltage
        bat3=(float)(22+((ch3-0.847)/0.038));  // Battery 3 voltage
        bat4=(float)(22+((ch4-0.855)/0.038));  // Battery 4 voltage
        bat5=(float)(22+((ch6-0.809)/0.037));  // Battery 5 voltage
        bat6=(float)(22+((ch7-0.792)/0.038));  // Battery 6 voltage
        cur1=(float)((ch5-5.936)/0.046);       // Current 1
        cur2=(float)((ch8-5.971)/0.046);       // Current 2
        
        // Store in sensor data structure
        sds.bank1=bat1;
        sds.bank2=bat2;
        sds.bank3=bat3;
        sds.bank4=bat4;
        sds.bank5=bat5;
        sds.bank6=bat6;
        sds.curr1=cur1;
        sds.curr2=cur2;
    }
    else
    {
        cout<<"ADAMTCP_SendReceive5KTCPCmd() Fail !!!"<<endl;  // Log error
    }
}
//////////////////////////////////////////////////////////////////
///////////////////// depthdaq /////////////////////////////////////
// Get depth data from pressure sensor
void Depth_Daq(void){
    CComConfig dp_port;      // Depth port configuration
    char dp_pkt[100];        // Depth packet buffer
    char command[6];         // Command string    
    char temp[20];           // Temporary buffer
    char temp1[20];          // Temporary buffer

    sprintf(command,"\r");  // Command to read data in RUN mode
    
    // Initialize depth sensor communication
    dp_port.ComInit ("COM9",2);
    dp_port.SetConfigParam (9600,8,NOPARITY,ONESTOPBIT);  // 9600 baud, 8N1
    
    // Send read command
    dp_port.DeviceWrite (command);
            
    // Read response
    strcpy(dp_pkt,dp_port.DeviceRead ());
        
    dp_port.EndCom ();  // End communication

    if(dp_pkt!=NULL){
        // Parse depth value from response
        sscanf(dp_pkt,"%s %s %f",temp,temp1,&sds.veh_depth);
    }
}

////////////////////////////////////////////////////////////////////
////////////////  dvl daq ///////////////////////////////////////
////////////////////////////////// --------------- DVL DAQ ------------------------------- ///////////////////////////////
// Get data from Doppler Velocity Log
void dvl_daq(){
    HANDLE dvlport;            // DVL port handle
    DCB portDCB;               // Device control block
    DWORD bytes_read;          // Bytes read count

    char data[2000];           // Raw data buffer
    char parse1[50];           // Parsed string 1 (SA message)
    char parse2[50];           // Parsed string 2 (BE message)
    char parse3[50];           // Parsed string 3 (BD message)
    char temp1[4];             // Temporary buffer
    char temp2[4];             // Temporary buffer
    char temp3[4];             // Temporary buffer
    char temp4[4];             // Temporary buffer
    char vel_stat[4];          // Velocity status
    char status;               // Status character
    int i=0,j=0,k=0,l=0,m=0,j2=0,j3=0;  // Index counters
    float temp;                // Temporary float
    float roll=0;              // DVL roll
    float pitch=0;             // DVL pitch
    float heading=0;           // DVL heading
    float vel_E=0;             // East velocity
    float vel_N=0;             // North velocity
    float vel_z=0;             // Vertical velocity
    float dis_E=0;             // East displacement
    float dis_N=0;             // North displacement
    float dis_z=0;             // Vertical displacement
    float bot_rng=0;           // Bottom range (altitude)
    bool be_found=false;       // BE message found flag
    bool bd_found=false;       // BD message found flag
    bool sa_found=false;       // SA message found flag
    int count=0;               // Counter

    // Open DVL serial port
    dvlport=CreateFile("COM7",GENERIC_READ,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(dvlport==INVALID_HANDLE_VALUE){
        cout<<"DVL port not opened"<<endl;
        return;
    }
    else{
        // Configure serial port
        portDCB.DCBlength =sizeof(DCB);
        GetCommState(dvlport,&portDCB);
        portDCB.BaudRate =9600;      // 9600 baud rate
        portDCB.ByteSize =8;         // 8 data bits
        portDCB.Parity =NOPARITY;    // No parity
        portDCB.StopBits =ONESTOPBIT; // 1 stop bit
        SetCommState(dvlport,&portDCB);
    }
    
    // Read data from DVL
    ReadFile(dvlport,data,1500,&bytes_read,NULL);
    CloseHandle(dvlport);  // Close port
    
    if(data!=NULL){
        data[bytes_read]='\0';  // Null terminate
        
        // Parse DVL data messages
        for (i=0;i<strlen(data);i++){
            // SA message: Attitude data (roll, pitch, heading)
            if(data[i]=='S' && data[i+1]=='A'){
                while(data[i]!='\r'){    
                    parse1[j++]=data[i];  // Copy SA message
                    i++;
                }
                if(data[i]=='\r')
                    parse1[j]='\0';  // Null terminate
                    
                // Replace commas with spaces for parsing
                for (k=0;k<strlen(parse1);k++){
                    if(parse1[k]==',')
                        parse1[k]=' ';
                }
                
                // Parse attitude data
                sscanf(parse1,"%s %f %f %f",temp1,&roll,&pitch,&heading);
                sa_found=true;  // Set flag
            }
            
            // BE message: Velocity data (East, North, Vertical)
            if(data[i]=='B' && data[i+1]=='E'){
                while(data[i]!='\r'){    
                    parse2[j2++]=data[i];  // Copy BE message
                    i++;
                }
                if(data[i]=='\r')
                    parse2[j2]='\0';  // Null terminate
                    
                // Replace commas with spaces for parsing
                for (k=0;k<strlen(parse2);k++){
                    if(parse2[k]==',')
                        parse2[k]=' ';
                }
                
                // Parse velocity data (convert mm/s to m/s)
                sscanf(parse2,"%s %f %f %f %s",temp1,&vel_E,&vel_N,&vel_z,&status);
                vel_E*=0.001;  // Convert to m/s
                vel_N*=0.001;  // Convert to m/s
                vel_z*=0.001;  // Convert to m/s
                
                be_found=true;  // Set flag
            }
            
            // BD message: Displacement data
            if(data[i]=='B' && data[i+1]=='D'){
                while(data[i]!='\r'){    
                    parse3[j3++]=data[i];  // Copy BD message
                    i++;
                }
                if(data[i]=='\r')
                    parse3[j3]='\0';  // Null terminate
                    
                // Replace commas with spaces for parsing
                for (k=0;k<strlen(parse3);k++){
                    if(parse3[k]==',')
                        parse3[k]=' ';
                }
                
                // Parse displacement data
                sscanf(parse3,"%s %f %f %f %f %f",temp1,&dis_E,&dis_N,&dis_z,&bot_rng,&temp);
                bd_found=true;  // Set flag
            }
        }
        
        // Store parsed data in DVL data structure
        if(be_found){
            dds.vel_east =vel_E;    // East velocity
            dds.vel_north =vel_N;   // North velocity
            dds.vel_Z =vel_z;       // Vertical velocity
        }
        if(bd_found){
            dds.disp_east =dis_E;   // East displacement
            dds.disp_north =dis_N;  // North displacement
            dds.disp_Z =dis_z;      // Vertical displacement
            dds.dvl_alti=bot_rng;   // Bottom range (altitude)
        }
        if(sa_found){
            dds.dvl_head =heading;  // DVL heading
            dds.dvl_pitch =pitch;   // DVL pitch
            dds.dvl_roll =roll;     // DVL roll
        }
    }
}
////////////////////////////////////// END ////////////////////////////////////////////////////////////////
///////////////////////  flsdaq  ////////////////////////////////////////////
// Configure FLS (Forward Looking Sonar) serial port
HANDLE FLS_port_config(void){
    HANDLE h_port;  // Port handle
    DCB portDCB;    // Device control block
    
    // Open FLS serial port
    h_port=CreateFile("\\\\.\\COM12",GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
    if(h_port==INVALID_HANDLE_VALUE)
        cout<<"could not open the port successfully"<<endl;
    else{
        // Configure serial port
        portDCB.DCBlength =sizeof(DCB);
        GetCommState(h_port,&portDCB);
        portDCB.BaudRate =115200;     // 115200 baud rate
        portDCB.ByteSize =8;          // 8 data bits
        portDCB.Parity =NOPARITY;     // No parity
        portDCB.StopBits =ONESTOPBIT; // 1 stop bit
        SetCommState(h_port,&portDCB);
        cout<<"port successfully opened!"<<endl;
    }
    return h_port;  // Return port handle
}

// Configure FLS device with specific settings
int FLS_device_config(HANDLE h_port){
    // FLS configuration with following settings:
    // Range = 30 meters = 300 dm (0x012C)
    // Number of Bins = 200 (0xC8)
    // VOS = 1460 meters/sec (speed of sound)
    // TxPulseLen = 75 (0x4B) (transmit pulse length)
    // ADLow: 60 (0x3C) = 18 dB (analog-digital low threshold)
    // ADSpan: 47 (0x2F) = 15 dB (analog-digital span)
    // ADInterval= 214 (0xD6) (analog-digital interval)
    // Step angle: 0.9 degree (1 Grad) Step size= 16 (0x10)
    // Left Limit = 175 degrees = (2520 1/16Grads) = 0x09D8
    // Right Limit = 185 degrees = (2664 1/16Grads) = 0x0A68
    
    char mt_alive[22];  // mtAlive message buffer
    // mtHeadCommand: Configuration command packet
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
                            0x64,0x00,    // txpulselen = 100 (0x64)
                            0x2C,0x01,    // rangescale = 300 (0x012C)
                            0xD8,0x09,    // leftlimit = 2520 (0x09D8)
                            0x68,0x0A,    // rightlimit = 2664 (0x0A68)
                            0x2F,        // ADspan = 47 (0x2F)
                            0x5F,        // ADlow = 95 (0x5F)
                            0x6B,
                            0x6B,
                            0x5A,0x00,
                            0x7D,0x00,
                            0x19,
                            0x10,        // Step Size = 16 (0x10)
                            0x41,0x01,    // ADInterval = 321 (0x0141)
                            0xC8,0x00,    // NoBins = 200 (0xC8)
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
                            0x0A        // LF (line feed)
    };
    
    DWORD bytes_read;     // Bytes read count
    DWORD bytes_written;  // Bytes written count

    // Write configuration command to FLS
    WriteFile(h_port,mt_headcommand,82,&bytes_written,NULL);
    _sleep(1000);  // Wait 1 second for configuration
    
    // Check if FLS configured successfully
    ReadFile(h_port,mt_alive,22,&bytes_read,NULL);
    
    // Parse response to check configuration status
    if(mt_alive[0]=='@'){  // Check if valid response
        if(mt_alive[10]==4){  // Check message type
            // Check configuration status byte (14th byte)
            if(mt_alive[13]==-128)  // 0x80 = config not done
                return 1;  // Configuration failed
            else if(mt_alive[13]==0)  // 0x00 = config done
                return 0;  // Configuration successful
        }
    }
    return 1;  // Default to failure
}
