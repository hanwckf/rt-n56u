/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
	struct variable variables_Storage_SharedList[] = {
			{"sh_path", "45", validate_string, ARGV("32"), FALSE, 0x0},
			{"sh_name", "34", validate_string, ARGV("16"), FALSE, 0x0},
			{"sh_nameb", "", validate_string, ARGV("16"), FALSE, 0x0},
			{"sh_comment", "", validate_string, ARGV(""), FALSE, FALSE},
			{"sh_commentb", "", validate_string, ARGV(""), FALSE, FALSE},
			{"sh_acc_onlist_num", "", validate_string, ARGV("0","12"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};
	
	struct variable variables_Storage_UserList[] = {
			{"acc_username", "41", validate_string, ARGV("28"), FALSE, FALSE},
			{"acc_password", "38", validate_string, ARGV("24"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};
	
      //2005.12.16 Yau add
      struct variable variables_Storage_UserRights_List[] = {
		{"sh_acc_user", "34", validate_string, ARGV(""), FALSE, FALSE},
		{"sh_rright", "28", validate_string, ARGV(""), FALSE, FALSE},
		{"sh_wright", "28", validate_string, ARGV(""), FALSE, FALSE},
		{0,0,0,0,0,0} //Viz changed 2010.08
      };

	struct variable variables_Language[] = {
			{"preferred_lang", "", validate_choice, ARGV(
					"EN:English",
					"TW:Traditional Chinese",
					"CN:Simple Chinese",
					"KR:Korean",
				0), FALSE, FALSE}
		};
	
	struct variable variables_Storage[] = {
			{"st_samba_mode", "", validate_choice, ARGV(
					"0:Disable",
					"1:Share all partitions in disk",
					"2:Apply rules in shared node list",
				0), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"computer_name", "", validate_string, ARGV("32"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"computer_nameb", "", validate_string, ARGV("32"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"st_samba_workgroup", "", validate_string, ARGV("32"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"st_samba_workgroupb", "", validate_string, ARGV("32"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"st_ftp_mode", "", validate_choice, ARGV(
					"0:Disable",
					"1:Login to first partition",
					"2:Login to first matched shared node",
				0), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"ftp_lang", "", validate_choice, ARGV("EN", "TW", "CN", "KR"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"st_max_user", "", validate_range, ARGV("1", "10"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			{"usb_vid_allow", "", validate_range, ARGV("0","FFFF"), FALSE, RESTART_REBOOT},
			{"usb_index", "", validate_choice, ARGV(
					"0:Auto Copy",
					"1:Sharing USB Disk",
				0), FALSE, FALSE},
			{"run_prog", "", validate_string, ARGV("32"), FALSE, FALSE},
			{"sh_num", "", validate_range, ARGV("0","65535"), FALSE, FALSE},
			{"acc_num", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
			
		//2005.12.16 Yau add
			{"sh_acc_num", "", validate_range, ARGV("0","65535"), FALSE, FALSE},
			
			{"sh_name_x", "", validate_string, ARGV("16"), FALSE, FALSE},
			{"sh_nameb_x", "", validate_string, ARGV("16"), FALSE, FALSE},
			{"sh_comment_x", "", validate_string, ARGV(""), FALSE, FALSE},
			{"sh_commentb_x", "", validate_string, ARGV(""), FALSE, FALSE},
			{"sh_acc_user_x", "", validate_string, ARGV(""), FALSE, FALSE},
			{"sh_acc_onlist_num_x", "", validate_string, ARGV("0","12"), FALSE, FALSE},
	      	{"x_PartType", "", validate_choice, ARGV("0:IDE","1:USB",0), FALSE, FALSE},
		
      		{"x_PartAll", "Status", NULL, ARGV("Storage","x_PartAll"), FALSE, FALSE},

		{"x_Part1", "", validate_string, ARGV(""), FALSE, FALSE}, 
	    
		{"x_Part2", "", validate_string, ARGV(""), FALSE, FALSE}, 
	    
		{"x_Part3", "", validate_string, ARGV(""), FALSE, FALSE}, 
	    
		{"x_Part4", "", validate_string, ARGV(""), FALSE, FALSE}, 
		 
      		{"x_StorageModel", "Status", NULL, ARGV("Storage","StorageModel"), FALSE, FALSE},
     
      		{"x_StoragePart1", "Status", NULL, ARGV("Storage","StoragePart1"), FALSE, FALSE},
     
      		{"x_StoragePart2", "Status", NULL, ARGV("Storage","StoragePart2"), FALSE, FALSE},
     
      		{"x_StoragePart3", "Status", NULL, ARGV("Storage","StoragePart3"), FALSE, FALSE},
     
      		{"x_StoragePart4", "Status", NULL, ARGV("Storage","StoragePart4"), FALSE, FALSE},
     
      		{"x_StorageStatus", "Status", NULL, ARGV("Storage","StorageStatus"), FALSE, FALSE},
					
		{"x_StorageAction", "", validate_string, ARGV(""), FALSE, FALSE},														   

      		{"x_USBModel", "Status", NULL, ARGV("Storage","StorageModel"), FALSE, FALSE},

      		{"st_usbpart1_t", "Status", NULL, ARGV("Storage","USBPart1"), FALSE, FALSE},
     
      		{"st_usbpart2_t", "Status", NULL, ARGV("Storage","USBPart2"), FALSE, FALSE},
     
      		{"st_usbpart3_t", "Status", NULL, ARGV("Storage","USBPart3"), FALSE, FALSE},
     
      		{"st_usbpart4_t", "Status", NULL, ARGV("Storage","USBPart4"), FALSE, FALSE},
     
      		{"st_usb_status_t", "Status", NULL, ARGV("Storage","USBStatus"), FALSE, FALSE},

		{"x_USBAction", "", validate_string, ARGV(""), FALSE, FALSE},
		
		{"apps_dl", "", validate_range, ARGV("0","1"), FALSE, FALSE},
		
		{"apps_dl_share", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
		
		{"apps_dl_share_port_from", "", validate_range, ARGV("1024","60000"), FALSE, FALSE},
		
		{"apps_dl_share_port_to", "", validate_range, ARGV("1024","60000"), FALSE, FALSE},
		
		{"apps_dl_seed", "", validate_range, ARGV("0","168"), FALSE, FALSE},
		
		{"apps_seed", "", validate_range, ARGV("0","1"), FALSE, RESTART_REBOOT},

		{"apps_upload_max", "", validate_range, ARGV("0","999"), FALSE, RESTART_REBOOT},

		{"apps_dms", "", validate_range, ARGV("0","1"), FALSE, RESTART_DMS},	// 2007.10 James

		{"apps_itunes", "", validate_range, ARGV("0","1"), FALSE, RESTART_ITUNES},

//		{"dms_comp_mode", "", validate_range, ARGV("0","1"), FALSE, RESTART_DMS},
		
		{"apps_dms_usb_port", "", validate_range, ARGV("1","2"), FALSE, FALSE},
		
      		{"Storage_SharedList", "Group", validate_group, ARGV(variables_Storage_SharedList, "32", "79", "sh_num"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James
       
      		{"Storage_UserList", "Group", validate_group, ARGV(variables_Storage_UserList, "16", "79", "acc_num"), FALSE, RESTART_FTPSAMBA},	// 2007.10 James

		//2005.12.16 Yau
		{"Storage_UserRight_List", "Group", validate_group, ARGV(variables_Storage_UserRights_List, "", "79", "sh_acc_num"),
		  FALSE, FALSE},

// *** Changes by Padavan ***
		{"achk_enable", "", validate_range, ARGV("0","1"), FALSE, FALSE},
		{"nfsd_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_NFS},
		{"optw_enable", "", validate_range, ARGV("0","1"), FALSE, FALSE},
		{"dlna_source", "", validate_string, ARGV("255"), FALSE, RESTART_DMS},
		{"dlna_rescan", "", validate_range, ARGV("0","2"), FALSE, FALSE},
		{"trmd_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_TORRENT},
		{"trmd_pport", "", validate_range, ARGV("1024","65535"), FALSE, RESTART_TORRENT},
		{"trmd_rport", "", validate_range, ARGV("1024","65535"), FALSE, RESTART_TORRENT},
		{"hdd_spindt", "", validate_range, ARGV("0","9"), FALSE, RESTART_HDDTUNE},
		{"hdd_apmoff", "", validate_range, ARGV("0","1"), FALSE, RESTART_HDDTUNE},
// *** Changes by Padavan ***

      		{ 0, 0, 0, 0, 0, 0}
      };

      struct variable variables_IPConnection_ExposedIPList[] = {	  
     
      {0,0,0,0,0,0} //Viz changed 2010.08
      };      
	
	struct variable variables_IPConnection_VSList[] = {
			{"vts_port_x", "12", validate_portrange, NULL, FALSE, FALSE},
      {"vts_ipaddr_x", "16", validate_ipaddr, NULL, FALSE, FALSE},
			{"vts_lport_x", "7", validate_range, ARGV("0","4294927695"), FALSE, FALSE},
			{"vts_proto_x", "8", validate_choice, ARGV(
					"TCP",
					"UDP",
					"BOTH",
					"OTHER",
					0), FALSE, FALSE},

			{"vts_protono_x", "12", validate_range, ARGV("0", "255", ""), FALSE , FALSE},

			{"vts_desc_x", "20", validate_string, ARGV("20"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};
	
	struct variable variables_IPConnection_TriggerList[] = {	  
			{"autofw_outport_x", "12", validate_portrange, NULL, FALSE, FALSE},
			{"autofw_outproto_x", "7", validate_choice, ARGV(		
					"TCP",
					"UDP",
					0), FALSE, FALSE},
			{"autofw_inport_x", "12", validate_portrange, NULL, FALSE, FALSE},
			{"autofw_inproto_x", "7", validate_choice, ARGV(		
					"TCP",
					"UDP",
					0), FALSE, FALSE},
			{"autofw_desc_x", "18", validate_string, ARGV("18"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};      
	
      struct variable variables_PPPConnection_PPPoERouteList[] = {	 
      {0,0,0,0,0,0} //Viz changed 2010.08
      };      
	
      struct variable variables_FirewallConfig_WLFilterList[] = {	  
      
		{"filter_wl_srcip_x", "16", validate_ipaddr, NULL, FALSE, FALSE},
	    
		{"filter_wl_srcport_x", "12", validate_portrange, NULL, FALSE, FALSE},
	    
		{"filter_wl_dstip_x", "16", validate_ipaddr, NULL, FALSE, FALSE},
	    
		{"filter_wl_dstport_x", "12", validate_portrange, NULL, FALSE, FALSE},
	    		      
	      {"filter_wl_proto_x", "8", validate_choice, ARGV(		
	      
		   "TCP",
	      
		   "TCP ALL",
	      
		   "TCP SYN",
	      
		   "TCP ACK",
	      
		   "TCP FIN",
	      
		   "TCP RST",
	      
		   "TCP URG",
	      
		   "TCP PSH",
	      
		   "UDP",
	      
	      0), FALSE, FALSE},
			   
      {0,0,0,0,0,0} //Viz changed 2010.08
      };      
	
	struct variable variables_FirewallConfig_LWFilterList[] = {	  
			{"filter_lw_srcip_x", "16", validate_ipaddr, NULL, FALSE, FALSE},
	  
			{"filter_lw_srcport_x", "12", validate_portrange, NULL, FALSE, FALSE},
			{"filter_lw_dstip_x", "16", validate_ipaddr, NULL, FALSE, FALSE},

			{"filter_lw_dstport_x", "12", validate_portrange, NULL, FALSE, FALSE},
			{"filter_lw_proto_x", "8", validate_choice, ARGV(		
					"TCP",
					"TCP ALL",
					"TCP SYN",
					"TCP ACK",
					"TCP FTN",
					"TCP RST",
					"TCP URG",
					"TCP PSH",
					"UDP",
					0), FALSE, FALSE},
			{"filter_lw_desc_x", "20", validate_string, ARGV("20"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};      
	
	struct variable variables_FirewallConfig_UrlList[] = {
			{"url_keyword_x", "36", validate_string, ARGV("32"), FALSE, RESTART_FIREWALL},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};

	struct variable variables_FirewallConfig_KeywordList[] = {
			{"keyword_keyword_x", "36", validate_string, ARGV("32"), FALSE, RESTART_FIREWALL},
			{0,0,0,0,0,0}
		};

	struct variable variables_FirewallConfig_MFList[] = {
			{"macfilter_list_x", "32", validate_hwaddr, NULL, FALSE, FALSE},
		        {0,0,0,0,0,0} //Viz changed 2010.08
		};

	struct variable variables_RouterConfig_GWStatic[] = {	  
			{"sr_ipaddr_x", "17", validate_ipaddr, NULL, FALSE, FALSE},
			{"sr_netmask_x", "17", validate_ipaddr, NULL, FALSE, FALSE},
			{"sr_gateway_x", "17", validate_ipaddr, NULL, FALSE, FALSE},
			{"sr_matric_x", "5", validate_ipaddr, NULL, FALSE, FALSE},
			{"sr_if_x", "3", validate_choice, ARGV(		
					"LAN",
					"WAN",
					0), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};      
      
	struct variable variables_LANHostConfig_ManualDHCPList[] = {	  
			{"dhcp_staticmac_x", "14", validate_hwaddr, NULL, FALSE, FALSE},
			{"dhcp_staticip_x", "17", validate_ipaddr, NULL, FALSE, FALSE},
			{"dhcp_staticname_x", "24", validate_string, ARGV("24"), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};      

	struct variable variables_DeviceSecurity11a_ACLList[] = {	  
			{"wl_maclist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};
      
  struct variable variables_DeviceSecurity11a_rt_ACLList[] = {
      {"rt_maclist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
      {0,0,0,0,0,0} //Viz changed 2010.08
    };	

	struct variable variables_WLANConfig11b_RBRList[] = {	  
			{"wl_wdslist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
			//{"rt_wdslist_x", "32", validate_hwaddr, NULL, FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
		};      

        struct variable variables_WLANConfig11b_rt_RBRList[] = {
                        //{"wl_wdslist_x", "32", validate_hwaddr, NULL, FALSE, FALSE},
                        {"rt_wdslist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
                        {0,0,0,0,0,0} //Viz changed 2010.08
                };
	
	struct variable variables_DeviceSecurity11b_ACLList[] = {	        
		{"wl_maclist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
      			{0,0,0,0,0,0} //Viz changed 2010.08
                };  
    
        struct variable variables_DeviceSecurity11b_rt_ACLList[] = {
          	{"rt_maclist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
      			{0,0,0,0,0,0} //Viz changed 2010.08
                };


#ifndef NOQOS 
	struct variable variables_PrinterStatus_x_USRRuleList[] = {
			{"qos_service_name_x", "16", validate_string, ARGV("32"), FALSE, FALSE},
			{"qos_ip_x", "16", validate_ipaddr, NULL, FALSE, FALSE},
			{"qos_port_x", "12", validate_range, ARGV("0","65535"), FALSE, FALSE},
			{"qos_port_y", "12", validate_range, ARGV("0","65535"), FALSE, FALSE},
			{"qos_prio_x", "3", validate_choice, ARGV(
					"1",
					"2",
					"3",
					"4",
					"5",
					"6",
					"7",
					"8",
					0), FALSE, FALSE},
			{0,0,0,0,0,0} //Viz changed 2010.08
			};
#endif
      
/*
 * actions_$ServiceId[] = { $ActionName, $InArgu, $OutArgu, Callback}
 * Used in GetVarRequest
 *  	1. Find serviceId from svcLinks
 *	2. Get value from varaibles$ServiceId by means of nvram_get_x
 * 
 * Used in ActionRequest
 *      1. Find serviceId from svcLinks
 *	2. If $callback is not null, excute it
 * 	   else goto 3
 *	3. Check validate from $Validate_Func of variables_$ServiceId, if not validate return error
 *      4. Set value by means of nvram_set_x or nvram_add_list_x or nvram_del_list_x
 *      5. return
 *
 * variables_$ServiceId[] = { $Name, $Validate_Func, $Argu, $NullOk, $Event}
 * For writable variable
 *     $Validate_Func : check if the variable is ok
 *     $Argu : arguments list pass into $Validate_Func
 *     $NullOk: if variable is allowed as NULL
 *     $Event: if event is sent once variable is changed
 * For read only variable
 *     $Validate_Func == NULL
 *     $Argu : file and field to get value
 *     $NullOk: if variable is allowed as NULL
 *     $Event: if event is sent once variable is changed
 */
/* 
 * Variables are set in order (put dependent variables later). Set
 * nullok to TRUE to ignore zero-length values of the variable itself.
 * For more complicated validation that cannot be done in one pass or
 * depends on additional form components or can throw an error in a
 * unique painful way, write your own validation routine and assign it
 * to a hidden variable (e.g. filter_ip).
 *
 * This data structure is generated automatically by script, maybe not 
 * easy to read. But it really work!!!
*/
	
	struct variable variables_General[] = {
//2007.10 James {		
		{"x_Setting", "", validate_range, ARGV("0","1"), FALSE, FALSE},

		{"w_Setting", "", validate_range, ARGV("0","1"), FALSE, FALSE},
		
		{"machine_name", "", validate_string, ARGV("32"), FALSE, RESTART_REBOOT},
//2007.10 James }
		
		{"http_username", "", validate_string, ARGV("32"), FALSE, FALSE},
									 
		{"http_passwd", "", validate_string, ARGV("32"), FALSE, FALSE},
				 
		{"x_SystemUpTime", "Status", NULL, ARGV("general.log","UpTime"), FALSE, FALSE},
		
		{"x_ProductID", "Status", NULL, ARGV("general.log","ProductID"), FALSE, FALSE},
		
		{"x_FirmwareVersion", "Status", NULL, ARGV("general.log","FirmwareVer"), FALSE, FALSE},
		
		{"x_HardwareVersion", "Status", NULL, ARGV("general.log","HardwareVer"), FALSE, FALSE},
		
		{"x_BootloaderVersion", "Status", NULL, ARGV("general.log","BootloaderVer"), FALSE, FALSE},
		
		{"modem_enable", "", validate_range, ARGV("0","4"), FALSE, RESTART_NETWORKING},

		{"Dev3G", "", validate_choice, ARGV(

                   "AUTO:AUTO",

                   "ASUS-T500:ASUS-T500",

                   "BandLuxe-C120:BandLuxe-C120",

                   "BandLuxe-C270:BandLuxe-C270",

                   "HUAWEI-E160G:HUAWEI-E160G",

                   "HUAWEI-E169:HUAWEI-E169",

                   "HUAWEI-E176:HUAWEI-E176",

                   "HUAWEI-E180:HUAWEI-E180",

                   "HUAWEI-E220:HUAWEI-E220",

                   "MU-Q101:MU-Q101",

                   "OPTION-ICON225:OPTION-ICON225",

                   "Sony-Ericsson-W910i:Sony-Ericsson-W910i",

                   "Alcatel-X200:Alcatel-X200",
                   "Huawei-K3520:Huawei-K3520",
                   "Huawei-E161:Huawei-E161",
                   "Huawei-E1550:Huawei-E1550",
                   "Huawei-EC122:Huawei-EC122",
                   "Huawei-EC306_AEAUTO:Huawei-EC306_AEAUTO",
                   "Sierra-U598:Sierra-U598",
                   "AnyData-ADU-510A_AEAUTO:AnyData-ADU-510A_AEAUTO",
                   "AnyData-ADU-500A_AEAUTO:AnyData-ADU-500A_AEAUTO",
                   "Alcatel-Oune-touch-X220S_AEAUTO:Alcatel-Oune-touch-X220S_AEAUTO",
                   "Onda-MT833UP_AEAUTO:Onda-MT833UP_AEAUTO",
                   "Onda-MW833UP_AEAUTO:Onda-MW833UP_AEAUTO",
                   "Huawei-E1800_AEAUTO:Huawei-E1800_AEAUTO",
                   "Huawei-K4505_AEAUTO:Huawei-K4505_AEAUTO",
                   "Huawei-E172_AEAUTO:Huawei-E172_AEAUTO",
                   "Huawei-E372_AEAUTO:Huawei-E372_AEAUTO",
                   "Huawei-E122_AEAUTO:Huawei-E122_AEAUTO",
                   "Huawei-E160E_AEAUTO:Huawei-E160E_AEAUTO",
                   "Huawei-E1552_AEAUTO:Huawei-E1552_AEAUTO",
                   "Huawei-E173_AEAUTO:Huawei-E173_AEAUTO",
                   "Huawei-E1823_AEAUTO:Huawei-E1823_AEAUTO",
                   "Huawei-E1762_AEAUTO:Huawei-E1762_AEAUTO",
                   "Huawei-K4505_AEAUTO:Huawei-K4505_AEAUTO",
                   "Huawei-E1750C_AEAUTO:Huawei-E1750C_AEAUTO",
                   "Huawei-E1752Cu_AEAUTO:Huawei-E1752Cu_AEAUTO",
                   "Huawei-E172_AEAUTO:Huawei-E172_AEAUTO",
                   "ZTE-AC5710_AEAUTO:ZTE-AC5710_AEAUTO",
                   "ZTE-MF100_AEAUTO:ZTE-MF100_AEAUTO",
                   "ZTE-MF636_AEAUTO:ZTE-MF636_AEAUTO",
                   "ZTE-MF622_AEAUTO:ZTE-MF622_AEAUTO",
                   "ZTE-MF626_AEAUTO:ZTE-MF626_AEAUTO",
                   "ZTE-MF632_AEAUTO:ZTE-MF632_AEAUTO",
                   "ZTE-MF112_AEAUTO:ZTE-MF112_AEAUTO",
                   "ZTE-MFK3570-Z_AEAUTO:ZTE-MFK3570-Z_AEAUTO",
                   "BandLuxe-C339_AEAUTO:BandLuxe-C339_AEAUTO",
                   "CS15_AEAUTO:CS15_AEAUTO",
                   "CS17_AEAUTO:CS17_AEAUTO",
                   "ICON401_AEAUTO:ICON401_AEAUTO",

		0), FALSE, RESTART_NETWORKING},       // 3g device

		{"modem_country", "", validate_choice, ARGV(

		   "AU:AU",

		   "CA:CA",

		   "CN:CN",
 
		   "US:US",

		   "IT:IT",

		   "PO:PO",
 
		   "UK:UK",

		   "IN:IN",

		   "MA:MA",

		   "SG:SG",

		   "PH:PH",

		   "SA:SA",

		   "HK:HK",

		   "TW:TW",

		   "EG:EG",

		   "DR:DR",

		   "ES:ES",

		   "BR:BR",

		   "NE:NE",

		   "NO:NO",

		   "RU:RU",

		0), FALSE, RESTART_NETWORKING},       // 3g device

		{"modem_isp", "", validate_choice, ARGV(

		   "Telstra:Telstra",

		   "Optus:Optus",

		   "Bigpond:Bigpond",
 
		   "Hutchison 3G:Hutchison 3G",

		   "Vodafone:Vodafone",

		   "iburst:iburst",
 
		   "Exetel:Exetel",

		   "Internode:Internode",

		   "Three:Three",

		   "Three PrePaid:Three PrePaid",

		   "TPG:TPG",

		   "Virgin:Virgin",

		   "Rogers:Rogers",

		   "China Telecom:China Telecom",

		   "China Mobile:China Mobile",

		   "China Unicom:China Unicom",

		   "Telecom Italia Mobile:Telecom Italia Mobile",

		   "Bell Mobility:Bell Mobility",

		   "Cellular One:Cellular One",

		   "Cincinnati Bell:Cincinnati Bell",

		   "T-Mobile (T-Zone):T-Mobile (T-Zone)",

		   "T-Mobile (Internet):T-Mobile (Internet)",

		   "Verizon:Verizon",

		   "OTelecom Italia Mobile:OTelecom Italia Mobile",

		   "Optimus:Optimus",

		   "Orangenet:Orangenet",

		   "Vodafone:Vodafone",

		   "O2:O2",

		   "T-mobile:T-mobile",

		   "IM2:IM2",

		   "INDOSAT:INDOSAT",

		   "XL:XL",

		   "Telkomsel Flash:Telkomsel Flash",

		   "3:3",

		   "Celcom:Celcom",

		   "Maxis:Maxis",

		   "M1:M1",

		   "Singtel:Singtel",

		   "StarHub:StarHub",

		   "Power Grid:Power Grid",

		   "Globe:Globe",

		   "Smart:Smart",

		   "Sun Cellula:Sun Cellula",

		   "Vodacom:Vodacom",

		   "MTN:MTN",

		   "Cell-c:Cell-c",

		   "SmarTone-Vodafone:SmarTone-Vodafone",

		   "3 Hong Kong:3 Hong Kong",

		   "One2Free:One2Free",

		   "PCCW mobile:PCCW mobile",

		   "All 3G ISP support:All 3G ISP support",

		   "New World:New World",

		   "3HK:3HK",

		   "CSL:CSL",

		   "People:People",

		   "Sunday:Sunday",

		   "Far Eastern:Far Eastern",

		   "Far Eastern(fetims):Far Eastern(fetims)",

		   "Chunghua Telecom:Chunghua Telecom",

		   "Taiwan Mobile:Taiwan Mobile",

		   "Vibo:Vibo",

		   "Taiwan Cellular:Taiwan Cellular",

		   "Etisalat:Etisalat",

		   "Telmex:Telmex",

		   "Claro:Claro",

		   "T-Mobile:T-Mobile",

		   "KPN:KPN",

		   "Telfort:Telfort",

		   "Vodafone:Vodafone",

		   "Telenor Mobile:Telenor Mobile",

		   "Netcom Mobile:Netcom Mobile",

		   "BeeLine:BeeLine",

		   "Megafon:Megafon",

		   "MTS:MTS",

		   "PrimTel:PrimTel",

		   "Not Support yet:Not Support yet",

		0), FALSE, RESTART_NETWORKING},       // 3g device

		{"modem_apn", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},
		{"modem_dialnum", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},
		{"modem_user", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},
		{"modem_pass", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},
		{"modem_node", "", validate_range, ARGV("0","8"), FALSE, RESTART_NETWORKING},
		{"modem_port", "", validate_range, ARGV("0","9"), FALSE, RESTART_NETWORKING},
		{"wan_3g_pin", "", validate_string, ARGV("8"), FALSE, RESTART_NETWORKING},

		{"help_enable", "", validate_range, ARGV("0","1"), FALSE, FALSE}, // Padavan
		{"ez_action_short", "", validate_range, ARGV("0","9"), FALSE, FALSE}, // Padavan
		{"ez_action_long", "", validate_range, ARGV("0","10"), FALSE, FALSE}, // Padavan
		{"u2ec_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_SPOOLER},
		{"lprd_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_SPOOLER},
		{"rawd_enable", "", validate_range, ARGV("0","2"), FALSE, RESTART_SPOOLER},

		{ 0, 0, 0, 0, 0, 0}
	};
	
      struct variable variables_Layer3Forwarding[] = {
					      
							{"", "", validate_string, ARGV(""), FALSE, FALSE},														   
							
							{"x_DHCPClient", "", validate_choice, ARGV("0", "1"), FALSE, RESTART_NETWORKING},	// 2008.03 James
							
							{"wan_pptp_options_x", "", validate_string, ARGV("255"), FALSE, RESTART_NETWORKING},	// 2008.03 James
							
	      {"wan_proto", "", validate_choice, ARGV(	      
	      
		   "dhcp:Automatic IP",
	      
		   "static:Static IP",
	      
		   "pppoe:PPPoE",
	      
		   "pptp:PPTP",
		
		   "l2tp:L2TP",
		
	      0), FALSE, RESTART_NETWORKING},	// 2007.10 James
	   	
	      {"wan_mode_x", "", validate_choice, ARGV(	      
	      
		   "0:Disabled",
	      
		   "1:Enabled",
	      
		   "2:Auto",
	      
	      0), FALSE, RESTART_NETWORKING},	// 2007.10 James
	   	
	      {"wan_etherspeed_x", "", validate_choice, ARGV(	      
	      
		   "auto:Auto negotiation",
	      
		   "10half:10Mbps half-duplex",
	      
		   "10full:10Mbps full-duplex",
	      
		   "100half:100Mpbs half-duplex",
	      
		   "100full:100Mpbs full-duplex",
	      
	      0), FALSE, RESTART_NETWORKING},	// 2007.10 James

		{"pppoe_dhcp_route", "", validate_range, ARGV("0", "2"), FALSE, RESTART_NETWORKING},

		{"wan_stb_x", "", validate_range, ARGV("0", "7"), FALSE, RESTART_NETWORKING},	// 2008.03 James

		{"vlan_isp", "", validate_choice, ARGV( 
						"none", 
						"russia", 
						"unifi_home", 
						"unifi_biz", 
						"singtel_mio", 
						"singtel_others", 
						"manual", 
						"vfiltered", 
						0), FALSE, RESTART_NETWORKING}, //Cherry Cho added in 2011/7/20

		{"internet_vid", "", validate_range, ARGV("2", "4094"), FALSE, RESTART_NETWORKING}, 

		{"iptv_vid", "", validate_range, ARGV("2", "4094"), FALSE, RESTART_NETWORKING}, 

		{"voip_vid", "", validate_range, ARGV("2", "4094"), FALSE, RESTART_NETWORKING}, 

		{"internet_prio", "", validate_range, ARGV("0", "7"), FALSE, RESTART_NETWORKING}, 

		{"iptv_prio", "", validate_range, ARGV("0", "7"), FALSE, RESTART_NETWORKING}, 

		{"voip_prio", "", validate_range, ARGV("0", "7"), FALSE, RESTART_NETWORKING}, 
// *** Changes by Padavan ***
		{"wan_auth_mode", "", validate_range, ARGV("0","2"), FALSE, RESTART_NETWORKING},
		{"wan_auth_user", "", validate_string, ARGV(""), FALSE, RESTART_NETWORKING},
		{"wan_auth_pass", "", validate_string, ARGV(""), FALSE, RESTART_NETWORKING},
// *** Changes by Padavan ***
		{ 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_WANCommonInterface[] = {
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_IPConnection[] = {
      	
	      {"wan_route_x", "", validate_choice, ARGV(	      
	      
		   "Unconfigured",
	      
		   "IP_Routed",
	      
		   "IP_Bridged",
	      
	      0), FALSE, RESTART_REBOOT},	// 2007.10 James
	   	
	      {"", "", validate_choice, ARGV(	      
	      
		   "Unconfigured",
	      
		   "IP_Routed",
	      
		   "IP_Bridged",
	      
	      0), FALSE, FALSE},
	   
		{"ConnectionStatus", "Status", NULL, ARGV("wan.log","IPLink"), FALSE, FALSE},

		{"Uptime", "Status", NULL, ARGV("wan.log","Uptime"), FALSE, FALSE},

		{"", "", validate_choice, ARGV("ERROR_NONE", 0), FALSE, FALSE},
			    
		{"wan_nat_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		
		{"hw_nat_mode", "", validate_range, ARGV("0","2"), FALSE, RESTART_FIREWALL},	// Padavan
		{"sw_nat_mode", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// Padavan
		
		{"sw_mode", "", validate_range, ARGV("0","1"), FALSE, RESTART_REBOOT},
			       
		{"", "", validate_range, ARGV("0","1"), FALSE, FALSE},						       
	      
		{"wan_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
		{"wan_netmask", "", validate_ipaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
		{"wan_gateway", "", validate_ipaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
			     
		{"wan_dnsenable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
	      
		{"wan_dns1_x", "", validate_ipaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
		{"wan_dns2_x", "", validate_ipaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
		{"IPTablesInfo", "Status", NULL, ARGV("iptable.log",""), FALSE, FALSE},
			 
		{"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},
		     
		{"", "", validate_range, ARGV("0","1"), FALSE, FALSE},						       

		{"dmz_ip", "", validate_ipaddr, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
			     
		{"sp_battle_ips", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James

		{"fw_pt_pptp", "", validate_range, ARGV("0","1"), FALSE, RESTART_VPN},

		{"fw_pt_l2tp", "", validate_range, ARGV("0","1"), FALSE, RESTART_VPN},

		{"fw_pt_ipsec", "", validate_range, ARGV("0","1"), FALSE, RESTART_VPN},
				       
		{"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    				     
		{"vts_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
				       
		{"vts_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James
		
		{"port_ftp", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},
				     
		{"autofw_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
				       
		{"autofw_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James
       
      {"ExposedIPList", "Group", validate_group, ARGV(variables_IPConnection_ExposedIPList, "32", "52", "ExposedIPCount"), FALSE, FALSE},     
       
      {"VSList", "Group", validate_group, ARGV(variables_IPConnection_VSList, "24", "75", "vts_num_x"), FALSE, RESTART_FIREWALL},	// 2008.01 James.
       
      {"TriggerList", "Group", validate_group, ARGV(variables_IPConnection_TriggerList, "10", "56", "autofw_num_x"), FALSE, RESTART_FIREWALL},	// 2008.01 James.
			
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_PPPConnection[] = {
						   
		    {"wan_pppoe_username", "", validate_string, ARGV("64"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		 
		{"wan_pppoe_passwd", "", validate_string, ARGV("64"), FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
      {"ConnectionStatus", "Status", NULL, ARGV("wan.log","WANLink"), FALSE, FALSE},

		       {"wan_pppoe_idletime", "", validate_range, ARGV("0","4294927695"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		    
		{"wan_pppoe_txonly_x", "", validate_string, ARGV(""), FALSE, RESTART_NETWORKING},	// 2007.10 James
		
		{"wan_pppoe_options_x", "", validate_string, ARGV("255"), FALSE, RESTART_NETWORKING},	// 2008.03 James
			 
	     {"wan_pppoe_mtu", "", validate_range, ARGV("576", "1492", ""), FALSE, RESTART_NETWORKING},	// 2007.10 James
		     
	     {"wan_pppoe_mru", "", validate_range, ARGV("576", "1492", ""), FALSE, RESTART_NETWORKING},	// 2007.10 James
			       
		       {"", "", validate_range, ARGV("0","4294927695"), FALSE, FALSE},							     
							    
		    {"wan_pppoe_service", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},	// 2007.10 James
							 
		    {"wan_pppoe_ac", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		    {"wan_pppoe_lcpa", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
				  
		 {"wan_pppoe_relay_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
						      
		    {"wan_hostname", "", validate_string, ARGV("32"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		 
		{"wan_hwaddr_x", "", validate_hwaddr, NULL, FALSE, RESTART_NETWORKING},	// 2007.10 James
						    
		    {"wan_heartbeat_x", "", validate_string, ARGV(""), FALSE, RESTART_NETWORKING},	// 2007.10 James
	    
      {"x_WANType", "Status", NULL, ARGV("wan.log", "wan_proto_t"), FALSE, FALSE},

      {"x_WANIPAddress", "Status", NULL, ARGV("wan.log","wan_ipaddr_t"), FALSE, FALSE},

      {"x_WANSubnetMask", "Status", NULL, ARGV("wan.log","wan_netmask_t"), FALSE, FALSE},

      {"x_WANGateway", "Status", NULL, ARGV("wan.log","wan_gateway_t"), FALSE, FALSE},

      {"x_WANDNSServer", "Status", NULL, ARGV("wan.log","wan_dns_t"), FALSE, FALSE},

      {"x_WANLink", "Status", NULL, ARGV("wan.log","wan_status_t"), FALSE, FALSE},
					
		    {"", "", validate_string, ARGV(""), FALSE, FALSE},														   
		 
      {"x_DDNSStatus", "Status", NULL, ARGV("ddns.log","DDNSStatus"), FALSE, FALSE},
			
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_EthernetLink[] = {
			     
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_FirewallConfig[] = {
		 {"fw_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James

		 {"fw_dos_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2008.03 James
	      	
	      {"fw_log_x", "", validate_choice, ARGV(	      
	      
		   "none:None",
	      
		   "drop:Dropped",
	      
		   "accept:Accepted",
	      
		   "both:Both",
	      
	      0), FALSE, RESTART_FIREWALL},	// 2007.10 James
			    
		 {"misc_natlog_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
			       
		 {"misc_http_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
			   
	     {"misc_httpport_x", "", validate_range, ARGV("1024", "65535", ""), FALSE, RESTART_FIREWALL},	// 2007.10 James
			 
		 {"misc_lpr_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
			       
		 {"misc_ping_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
			       
		 {"fw_wl_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},// 2007.10 James
	      
		{"filter_wl_date_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
	    
		{"filter_wl_time_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
	    	
	      {"filter_wl_default_x", "", validate_choice, ARGV(	      
	      
		   "DROP",
	      
		   "ACCEPT",
	      
	      0), FALSE, RESTART_FIREWALL},	// 2007.10 James
	   
		{"filter_wl_icmp_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
	    
		{"", "", validate_portrange, NULL, FALSE, FALSE},
		

		
		{"fw_lw_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},// 2007.10 James

		{"fw_lw_enable_x_1", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	      
		{"filter_lw_date_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
	    
		{"filter_lw_time_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James

		{"filter_lw_time_x_1", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},
	    	
	      {"filter_lw_default_x", "", validate_choice, ARGV(	      
	      
		   "DROP",
	      
		   "ACCEPT",
	      
	      0), FALSE, RESTART_FIREWALL},	// 2007.10 James
	   
		{"filter_lw_icmp_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
	    
      {"FirewallLog", "Status", NULL, ARGV("firewall.log",""), FALSE, FALSE},

      {"SystemLog", "Status", NULL, ARGV("syslog.log",""), FALSE, FALSE},

      {"SystemCmd", "Status", NULL, ARGV("syscmd.log",""), FALSE, FALSE},
		 
		{"url_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// 2007.10 James
		{"url_enable_x_1", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},	// jerry5 added for n56u
		{"url_date_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
		{"url_time_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// 2007.10 James
		{"url_time_x_1", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},	// jerry5 added for n56u

		{"keyword_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
		{"keyword_enable_x_1", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
		{"keyword_date_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},
		{"keyword_time_x", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},
		{"keyword_time_x_1", "", validate_portrange, NULL, FALSE, RESTART_FIREWALL},

	      {"", "", validate_choice, ARGV(	      
	      
		   "DROP",
	      
		   "ACCEPT",
	      
	      0), FALSE, FALSE},
				    
		       {"url_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James

		       {"keyword_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},
					     
		       {"filter_wl_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James
					     
		       {"filter_lw_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James
		    	
	      {"macfilter_enable_x", "", validate_choice, ARGV(	      
	      
		   "0:Disable",
	      
		   "1:Accept",
	      
		   "2:Reject",
	      
	      0), FALSE, RESTART_FIREWALL},	// 2007.10 James
				    
		       {"macfilter_num_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_FIREWALL},	// 2007.10 James

	{"WLFilterList", "Group", validate_group, ARGV(variables_FirewallConfig_WLFilterList, "32", "63", "filter_wl_num_x"), FALSE, RESTART_FIREWALL},	// 2007.10 James

	{"LWFilterList", "Group", validate_group, ARGV(variables_FirewallConfig_LWFilterList, "32", "63", "filter_lw_num_x"), FALSE, RESTART_FIREWALL},	// 2007.11 James

	{"UrlList", "Group", validate_group, ARGV(variables_FirewallConfig_UrlList, "128", "36", "url_num_x"), FALSE, RESTART_FIREWALL},	// 2007.11 James

	{"KeywordList", "Group", validate_group, ARGV(variables_FirewallConfig_KeywordList, "128", "36", "keyword_num_x"), FALSE, RESTART_FIREWALL},

	{"MFList", "Group", validate_group, ARGV(variables_FirewallConfig_MFList, "16", "32", "macfilter_num_x"), FALSE, RESTART_FIREWALL},	// 2007.11 James

// *** Changes by Padavan ***
	{"sshd_wopen", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	{"sshd_wport", "", validate_range, ARGV("22","65535"), FALSE, RESTART_FIREWALL},
	{"trmd_ropen", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	{"fw_syn_cook", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	{"nf_nat_type", "", validate_range, ARGV("0","2"), FALSE, RESTART_FIREWALL},
	{"nf_nat_loop", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	{"nf_max_conn", "", validate_range, ARGV("8192","200000"), FALSE, RESTART_FIREWALL},
	{"nf_alg_h323", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
	{"nf_alg_sip", "", validate_range, ARGV("0","1"), FALSE, RESTART_FIREWALL},
// *** Changes by Padavan ***

	{ 0, 0, 0, 0, 0, 0}
	};

      struct variable variables_RouterConfig[] = {
		       
		{"sr_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		
		{"dr_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2008.03 James
		
		{"mr_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2008.03 James

	      {"sr_rip_x", "", validate_choice, ARGV(	      
	      
		   "0:Disabled",
	      
		   "1:LAN",
	      
		   "2:WAN",
	      
		   "3:BOTH",
	      
	      0), FALSE, RESTART_NETWORKING},	// 2007.10 James
				    
		       {"sr_num_x", "", validate_range, ARGV("0","32"), FALSE, RESTART_NETWORKING},	// 2007.10 James
				     
		 {"dr_static_rip_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
			   
	     {"dr_static_matric_x", "", validate_range, ARGV("1", "16", "1"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		
	      {"dr_default_x", "", validate_choice, ARGV(	      
	      
		   "0:be redistributed",
	      
		   "1:not be redistributed",
	      
	      0), FALSE, },	// 2007.10 James
	   
		      {"RouteInfo", "Status", NULL, ARGV("route.log",""), FALSE, FALSE},
			 
		       {"dr_static_rip_x", "", validate_range, ARGV("0","65535"), FALSE, },	// 2007.10 James
					     
		       {"dr_staticnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_NETWORKING},	// 2007.10 James
		     
      {"GWStatic", "Group", validate_group, ARGV(variables_RouterConfig_GWStatic, "6", "59", "sr_num_x"), FALSE, RESTART_NETWORKING},	// 2008.01 James.
 
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_LANHostConfig[] = {
		{"wcn_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_REBOOT},	// 2007.10 James
			       
		{"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
				     
		{"lan_proto_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_REBOOT},	// 2007.10 James
	      
		{"lan_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_REBOOT},	// 2007.10 James
	    
		{"lan_netmask", "", validate_ipaddr, NULL, FALSE, RESTART_REBOOT},	// 2007.10 James

                {"controlrate_unknown_unicast", "", validate_range, ARGV("0", "1024"), FALSE, RESTART_SWITCH},
                {"controlrate_unknown_multicast", "", validate_range, ARGV("0", "1024"), FALSE, RESTART_SWITCH},
                {"controlrate_multicast", "", validate_range, ARGV("0", "1024"), FALSE, RESTART_SWITCH},
                {"controlrate_broadcast", "", validate_range, ARGV("0", "1024"), FALSE, RESTART_SWITCH},

		{"udpxy_enable_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_NETWORKING},
	    
      {"x_LANIPAddress", "Status", NULL, ARGV("lan.log","lan_ipaddr_t"), FALSE, FALSE},

      {"x_LANSubnetMask", "Status", NULL, ARGV("lan.log","lan_netmask_t"), FALSE, FALSE},

      {"x_LANGateway", "Status", NULL, ARGV("lan.log","lan_gateway_t"), FALSE, FALSE},
					
		    {"lan_hostname", "", validate_string, ARGV("32"), FALSE, RESTART_REBOOT},	// 2007.10 James
		 
		{"lan_gateway", "", validate_ipaddr, NULL, FALSE, RESTART_REBOOT},	// 2007.10 James
			     
		 {"dhcp_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_DHCPD},	// 2007.10 James
						    
		    {"lan_domain", "", validate_string, ARGV("32"), FALSE, RESTART_DHCPD},	// 2007.10 James
		 
		{"dhcp_start", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},	// 2007.10 James
	    
		{"dhcp_end", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},	// 2007.10 James
			 
	     {"dhcp_lease", "", validate_range, ARGV("120", "604800", ""), FALSE, RESTART_DHCPD},	// 2007.10 James
	
		{"dhcp_gateway_x", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},	// 2007.10 James
	    
		{"dhcp_dns1_x", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},	// 2007.10 James
		{"dhcp_dns2_x", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},

      {"x_LDNSServer2", "Status", NULL, ARGV("LANHostConfig","lan_ipaddr"), FALSE, RESTART_REBOOT},	// 2007.10 James

		{"dhcp_wins_x", "", validate_ipaddr, NULL, FALSE, RESTART_DHCPD},	// 2007.10 James
			     
		 {"dhcp_static_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_DHCPD},	// 2007.10 James
				       
		       {"dhcp_staticnum_x", "", validate_range, ARGV("0","32"), FALSE, RESTART_DHCPD},	// 2007.10 James
					     
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},
		    
      {"DHCPLog", "Status", NULL, ARGV("leases.log",""), FALSE, FALSE},

      {"DmzDHCPLog", "Status", NULL, ARGV("dleases.log",""), FALSE, FALSE},
		 
	{"upnp_enable", "", validate_range, ARGV("0","2"), FALSE, RESTART_UPNP},	// 2007.10 James

// *** Changes by Padavan ***
	{"telnetd", "", validate_range, ARGV("0","1"), FALSE, RESTART_TERMINAL},
	{"sshd_enable", "", validate_range, ARGV("0","2"), FALSE, RESTART_TERMINAL},
	{"ether_led0", "", validate_range, ARGV("0","10"), FALSE, RESTART_SWITCH},
	{"ether_led1", "", validate_range, ARGV("0","10"), FALSE, RESTART_SWITCH},
	{"ether_jumbo", "", validate_range, ARGV("0","1"), FALSE, RESTART_SWITCH},
	{"ether_green", "", validate_range, ARGV("0","1"), FALSE, RESTART_SWITCH},
	{"ether_link_wan",  "", validate_range, ARGV("0","5"), FALSE, RESTART_SWITCH},
	{"ether_link_lan1", "", validate_range, ARGV("0","5"), FALSE, RESTART_SWITCH},
	{"ether_link_lan2", "", validate_range, ARGV("0","5"), FALSE, RESTART_SWITCH},
	{"ether_link_lan3", "", validate_range, ARGV("0","5"), FALSE, RESTART_SWITCH},
	{"ether_link_lan4", "", validate_range, ARGV("0","5"), FALSE, RESTART_SWITCH},
	{"ether_flow_wan",  "", validate_range, ARGV("0","2"), FALSE, RESTART_SWITCH},
	{"ether_flow_lan1", "", validate_range, ARGV("0","2"), FALSE, RESTART_SWITCH},
	{"ether_flow_lan2", "", validate_range, ARGV("0","2"), FALSE, RESTART_SWITCH},
	{"ether_flow_lan3", "", validate_range, ARGV("0","2"), FALSE, RESTART_SWITCH},
	{"ether_flow_lan4", "", validate_range, ARGV("0","2"), FALSE, RESTART_SWITCH},
	{"pptpd_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_POPTOP},
	{"pptpd_cast", "", validate_range, ARGV("0","3"), FALSE, RESTART_POPTOP},
	{"pptpd_auth", "", validate_range, ARGV("0","1"), FALSE, RESTART_POPTOP},
	{"pptpd_mppe", "", validate_range, ARGV("0","4"), FALSE, RESTART_POPTOP},
	{"pptpd_mtu", "", validate_range, ARGV("512","1460"), FALSE, RESTART_POPTOP},
	{"pptpd_mru", "", validate_range, ARGV("512","1460"), FALSE, RESTART_POPTOP},
	{"pptpd_clib", "", validate_range, ARGV("2","254"), FALSE, RESTART_POPTOP},
	{"pptpd_clie", "", validate_range, ARGV("2","254"), FALSE, RESTART_POPTOP},
// *** Changes by Padavan ***

	{"log_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_SYSLOG},	// 2007.10 James

     	{"rstats_path", "", validate_string, ARGV("0","48"), FALSE, RESTART_RSTATS},	 // 2010.08 Viz

      	{"rstats_enable", "" ,validate_range, ARGV("0", "1"), FALSE, RESTART_RSTATS},	 // 2010.08 Viz

	{"rstats_stime", "" , validate_range, ARGV("1", "168"), FALSE, RESTART_RSTATS},	// 2010.08 Viz

      	{"rstats_sshut", "" , validate_range, ARGV("0", "1"), FALSE, RESTART_RSTATS},	// 2010.08 Viz

	{"rstats_bak", "" , validate_range, ARGV("0", "1"), FALSE, RESTART_RSTATS},	// 2010.08 Viz

	{"rstats_new", "" , validate_range, ARGV("0", "1"), FALSE, RESTART_RSTATS},	// 2010.08 Viz

	{"time_zone", "", validate_choice, 
							NULL
							, FALSE, RESTART_TIME},	// 2007.10 James
						   
		    {"time_interval", "", validate_string, ARGV(""), FALSE, RESTART_REBOOT},	// 2007.10 James
							 
		    {"ntp_server0", "", validate_string, ARGV(""), FALSE, RESTART_NTPC},	// 2007.10 James
							 
		    {"ntp_server1", "", validate_string, ARGV(""), FALSE, RESTART_NTPC},	// 2007.10 James
			     
		 {"ddns_enable_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_DDNS},	// 2007.10 James
		 
		 {"asusddns_tos_agreement", "", validate_range, ARGV("0","1"), FALSE, FALSE},	// 2007.12 James added.
	      	
	      {"ddns_server_x", "", validate_choice, ARGV(	      
	      
		   "WWW.ASUS.COM",//2007.03.20 Yau add for asus ddns

		   "WWW.DYNDNS.ORG",
	      
		   "WWW.DYNDNS.ORG(CUSTOM)",
	      
		   "WWW.DYNDNS.ORG(STATIC)",
	      
		   "WWW.TZO.COM",
	      
		   "WWW.ZONEEDIT.COM",
	      
	      0), FALSE, RESTART_DDNS},	// 2007.10 James
						   
		{"ddns_username_x", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_passwd_x", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_hostname_x", "", validate_string, ARGV("64"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_hostname2_x", "", validate_string, ARGV("64"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_hostname3_x", "", validate_string, ARGV("64"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_wildcard_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_DDNS},	// 2007.10 James
	      
		//2007.03.20 Yau add for asus ddns
		{"ddns_timeout", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_return_code", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_old_name", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		{"ddns_suggest_name", "", validate_string, ARGV("32"), FALSE, RESTART_DDNS},	// 2007.10 James
		//End of Yau add

      {"x_DDNSStatus", "Status", NULL, ARGV("ddns.log","DDNSStatus"), FALSE, FALSE},
 
      {"ManualDHCPList", "Group", validate_group, ARGV(variables_LANHostConfig_ManualDHCPList, "8", "55", "dhcp_staticnum_x"), FALSE, RESTART_DHCPD},	// 2007.11 James
      
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_WLANConfig11a[] = {

      {"WirelessLog", "Status", NULL, ARGV("wlan11a.log",""), FALSE, FALSE},
		  
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_DeviceSecurity11a[] = {
			       
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    	
	      {"wl_macmode", "", validate_choice, ARGV(	      
	      
		   "disabled:Disable",
	      
		   "allow:Accept",
	      
		   "deny:Reject",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   	
              {"rt_macmode", "", validate_choice, ARGV(
                                                "disabled:Disable",
                                                "allow:Accept",
                                                "deny:Reject",
                                                0), FALSE, RESTART_WIFI},
				    
		       {"wl_macnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},	// 2007.10 James
					     
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    
		{"", "", validate_string, ARGV(""), FALSE, FALSE}, 
	     
      {"ACLList", "Group", validate_group, ARGV(variables_DeviceSecurity11a_ACLList, "64", "32", "wl_macnum_x"), FALSE, RESTART_WIFI},	// 2008.04 James.
      {"rt_ACLList", "Group", validate_group, ARGV(variables_DeviceSecurity11a_rt_ACLList, "64", "32", "rt_macnum_x"), FALSE, RESTART_WIFI}, 
			
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_WLANAuthentication11a[] = {
			       
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    
		{"wl_radius_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
				     
		       {"wl_radius_port", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},	// 2007.10 James
		    
		{"wl_radius_key", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},	// 2007.10 James
			
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_WLANConfig11b[] = {

			/* n56u start */
			{"rt_ssid", validate_string, ARGV("32"), FALSE, RESTART_WIFI},
			{"rt_ssid2", "", validate_string, ARGV("32"), FALSE, RESTART_WIFI},
			{"rt_gmode", "", validate_range, ARGV("0","5"), FALSE, RESTART_WIFI},
			{"rt_channel", "", validate_wlchannel, NULL, FALSE, RESTART_WIFI},
			{"rt_rateset", "", validate_choice, ARGV(
		   				"default:Default",
		 				"all:All",
						"12:1, 2 Mbps", 0), FALSE, RESTART_WIFI},
			{"rt_bcn", "", validate_range, ARGV("1", "65535", ""), FALSE, RESTART_WIFI},
			{"rt_dtim", "", validate_range, ARGV("1", "255", ""), FALSE, RESTART_WIFI},
			{"rt_gmode_protection", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},
			{"rt_gmode_protection_x", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},
			{"rt_rts", "", validate_range, ARGV("0", "2347", ""), FALSE, RESTART_WIFI},
			{"rt_frag", "", validate_range, ARGV("256", "2346", ""), FALSE, RESTART_WIFI},
			{"rt_ap_isolate", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
			{"rt_closed", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
			/*{"rt_macmode", "", validate_choice, ARGV(	      	      
						"disabled:Disable",
						"allow:Accept",
						"deny:Reject",
						0), FALSE, RESTART_WIFI},*/
			{"rt_IgmpSnEnable", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
			{"rt_mcastrate", "", validate_range, ARGV("0","9"), FALSE, RESTART_WIFI},
			{"rt_wsc_config_state", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
//			{"rt_secret_code", "0", 0 },
			{"rt_wme", "", validate_choice, ARGV(
						"0:Disabled",
						"1:Enabled", 0), FALSE, RESTART_WIFI},
			{"rt_wme_no_ack", "", validate_choice, ARGV(
						"off:Disabled",
						"on:Enabled", 0), FALSE, RESTART_WIFI},	
//			{"rt_GreenAP", "0", 0 },

			{"rt_auth_mode", "", validate_choice, ARGV(
						"open:Open System or Shared Key",
						"shared:Shared Key",
						"psk:WPA-PSK",
						"wpa:WPA",
						"radius:Radius with 802.1x", 0), FALSE, RESTART_WIFI},
			{"rt_crypto", "", validate_choice, ARGV(
						"tkip:TKIP",
						"aes:AES",
						"tkip+aes:TKIP+AES", 0), FALSE, RESTART_WIFI},
			{"rt_wpa_psk", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},
			{"rt_wpa_gtk_rekey", "", validate_range, ARGV("0", "86400", ""), FALSE, RESTART_WIFI},
			{"rt_key", "", validate_choice, ARGV(
						"1:Key1",
						"2:Key2",
						"3:Key3",
						"4:Key4", 0), FALSE, RESTART_WIFI},
			{"rt_key_type", "", validate_choice, ARGV(
						"0:HEX",
						"1:ASCII", 0), FALSE, RESTART_WIFI},
			{"rt_key1", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},
			{"rt_key2", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},
			{"rt_key3", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},
			{"rt_key4", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},
			{"rt_lazywds", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
			{"rt_radius_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_WIFI},
			{"rt_radius_port", "", validate_ipaddr, NULL, FALSE, RESTART_WIFI},
			{"rt_radius_key", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},
			{"rt_radio_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
//			{"rt_mode", "ap", 0 },
			{"rt_mode_x", "", validate_choice, ARGV(
						"0:AP Only",
						"1:WDS Only",
						"2:Hybrid", 0), FALSE, RESTART_WIFI},
			{"rt_wdsapply_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
			{"rt_wdsnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},
			{"rt_wep_x", "", validate_choice, ARGV(
						"0:None",
						"1:WEP-64bits",
						"2:WEP-128bits", 0), FALSE, RESTART_WIFI},
			{"rt_phrase_x", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},
			{"rt_radio_date_x", "", validate_portrange, NULL, FALSE, RESTART_WIFI},
			{"rt_radio_time_x", "", validate_portrange, NULL, FALSE, RESTART_WIFI},
			{"rt_macapply_x", "", validate_choice, ARGV(
						"Both",
						"802.11a only",
						"802.11g only", 0), FALSE, RESTART_WIFI},
			{"rt_macnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},
			{"rt_maclist_x", "32", validate_hwaddr, NULL, FALSE, RESTART_WIFI},
			{"rt_wpa_mode", "", validate_choice, ARGV(
						"0:WPA-Auto-Personal",
						"1:WPA-Personal",
						"2:WPA2-Personal",
						"3:WPA-Enterprise",
						"4:WPA-Auto-Enterprise", 0), FALSE, RESTART_WIFI},
			{"rt_PktAggregate", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI},		// 2008.08 magic
			{"rt_TxBurst", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI},		//2008.08 magic
			{"rt_APSDCapable", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI},		// 2008.08 magic
			{"rt_DLSCapable", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI},		// 2008.08 magic
			{"rt_HT_OpMode", "", validate_choice, ARGV(
						"0:Mixed Mode",
						"1:Green Field", 0), FALSE, RESTART_WIFI},	// 2008.08 magic
			{"rt_HT_BW", "", validate_choice, ARGV(
						"0:20",
						"1:20/40", 0), FALSE, RESTART_WIFI},		// 2008.08 magic
			{"rt_HT_EXTCHA", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},

			{"TxPower", "", validate_range, ARGV("0", "100"), FALSE, RESTART_WIFI},	// 2010.4 ASUS

			{"rt_TxPower", "", validate_range, ARGV("0", "100"), FALSE, RESTART_WIFI},	// 2010.4 ASUS

			{"wps_band", "", validate_choice, ARGV(
						"0:5GHz",
						"1:2.4GHz", 0), FALSE, RESTART_WPS},
			/* n56u end */     

		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
					     
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
			
			{"wl_wsc_mode", "", validate_string, ARGV("enabled","disabled"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_mode", "", validate_string, ARGV("enabled","disabled"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_config_state", "", validate_range, ARGV("0","1"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_proc_status", "", validate_range, ARGV("0","4"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_method", "", validate_range, ARGV("0","2"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_config_command", "", validate_range, ARGV("0","2"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_pbc_force", "", validate_range, ARGV("0","2"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"wsc_sta_pin", "", validate_string, ARGV("8"), FALSE, RESTART_WPS},	// 2008.01 James.
			{"pbc_overlap", "", validate_range, ARGV("0", "1"), FALSE, RESTART_WPS},	// 2008.02 James.
			{"wsc_client_role", "", validate_string, ARGV("registrar", "enrollee"), FALSE, FALSE},	// 2008.06 James.
			
			{"rt_country_code", "", validate_string, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
			{"wl_country_code", "", validate_string, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
			
			{"wl_bss_enabled", "", validate_choice, ARGV("0","1"), FALSE, RESTART_REBOOT},    // 2008.06 James
			{"wl_ssid", "", validate_string, ARGV("32"), FALSE, RESTART_WIFI},	// 2007.10 James
			{"wl_ssid2", "", validate_string, ARGV("32"), FALSE, RESTART_WIFI},	// 2007.10 James
			{"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    	
	      {"wl_mode_x", "", validate_choice, ARGV(	      
	      
		   "0:AP Only",
	      
		   "1:WDS Only",
	      
		   "2:Hybrid",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   
		{"wl_channel", "", validate_wlchannel, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
			     
		 {"wl_wdsapply_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
			       
		 {"wl_lazywds", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
				       
		 {"wl_wdsnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},	// 2007.10 James
		    	
		 {"wl_maxassoc", "", validate_range, ARGV("1","128"), FALSE, RESTART_WIFI},	// 2008.06 James

		 {"wl_bss_maxassoc", "", validate_range, ARGV("1","128"), FALSE, RESTART_WIFI},	// 2008.06 James
				 
	   	{"wl_gmode", "", validate_choice, ARGV(
						"2:Auto",
						"1:b/g Mixed",
						"3:n Only",
						"4:g Only",
						"0:b Only", 0), FALSE, RESTART_WIFI},//2008.08 magic
					
	    {"wl_gmode_protection_x", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
				
				{"TxBurst", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI},//2008.08 magic
				
				{"PktAggregate", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI}, //2008.08 magic
	    			
				{"wps_enable", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI}, //2008.08 magic

				{"wps_mode", "", validate_choice, ARGV(
						"1:PIN Method",
						"2:PBC Method", 0), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_OpMode", "", validate_choice, ARGV(
						"0:Mixed Mode",
						"1:Green Field", 0), FALSE, RESTART_WIFI}, //2008.08 magic
	
				{"HT_BW", "", validate_choice, ARGV(
						"0:20",
						"1:20/40", 0), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_GI", "", validate_choice, ARGV(
						"0:Long",
						"1:Auto", 0), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_MCS", "", validate_range, ARGV("0","33"), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_RDG", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_AMSDU", "", validate_choice, ARGV(
						"0:Disable",
						"1:Enable", 0), FALSE, RESTART_WIFI}, //2008.08 magic
				
				{"HT_EXTCHA", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_TxStream", "", validate_range, ARGV("1","3"), FALSE, RESTART_WIFI}, //2008.08 magic

				{"HT_RxStream", "", validate_range, ARGV("1","3"), FALSE, RESTART_WIFI}, //2008.08 magic

				{"wl_txbf", "", validate_choice, ARGV("0:Disable", "1:Enable", 0), FALSE, RESTART_WIFI},
    
				{"wl_auth_mode", "", validate_choice, ARGV(	      
	      
		   "open:Open System or Shared Key",
	      
		   "shared:Shared Key",
	      
		   "psk:WPA-PSK",
	      
		   "wpa:WPA",
	      
		   "radius:Radius with 802.1x",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James

	      {"wl_wpa_mode", "", validate_choice, ARGV(	      
	      
		   "0:WPA-Auto-Personal",
	      
		   "1:WPA-Personal",
	      
		   "2:WPA2-Personal",
		
		   "3:WPA-Enterprise",
	
		   "4:WPA-Auto-Enterprise",

	      0), FALSE, RESTART_WIFI},	// 2007.10 James

	      {"wl_crypto", "", validate_choice, ARGV(	      
	      
		   "tkip:TKIP",
	      
		   "aes:AES",
	      
		   "tkip+aes:TKIP+AES",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   
		{"wl_wpa_psk", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},	// 2007.10 James
	    	
	      {"wl_wep_x", "", validate_choice, ARGV(	      
	      
		   "0:None",
	      
		   "1:WEP-64bits",
	      
		   "2:WEP-128bits",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
				
				{"wep_key_type", "", validate_choice, ARGV(
						"0:ASCII digits",
						"1:Hex digits",
						0), FALSE, RESTART_WIFI},	// 2008.01 James
				
		{"wl_phrase_x", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},	// 2007.10 James
	    
		{"wl_key1", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
	    
		{"wl_key2", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
	    
		{"wl_key3", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
	    
		{"wl_key4", "", validate_wlkey, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
	    	
	      {"wl_key", "", validate_choice, ARGV(	      
	      
		   "1:Key1",
	      
		   "2:Key2",
	      
		   "3:Key3",
	      
		   "4:Key4",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James

				{"wl_key_type", "", validate_choice, ARGV(    
						   "0:HEX",
							"1:ASCII", 0), FALSE, RESTART_WIFI}, //2008.11 magic
			
	     {"wl_wpa_gtk_rekey", "", validate_range, ARGV("0", "86400", ""), FALSE, RESTART_WIFI},	// 2007.10 James
				 
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},
		    	
	      {"wl_afterburner", "", validate_choice, ARGV(	      
	      
		   "off:Disabled",
	      
		   "auto:Enabled",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James

			  {"wl_nbw_cap", "", validate_choice, ARGV("0", "1", "2"), FALSE, RESTART_WIFI},   // 2008.06 James
			  
			  {"wl_txstreams", "", validate_range, ARGV("0", "1", "2"), FALSE, RESTART_WIFI},   // 2008.06 James
			  
			  {"wl_rxstreams", "", validate_range, ARGV("0", "1", "2"), FALSE, RESTART_WIFI},   // 2008.06 James
			    
		 {"wl_closed", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
			       
		 {"wl_ap_isolate", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
				 
		 {"wl_radarthrs", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2008.06 James
	      	
	      {"wl_rate", "", validate_choice, ARGV(	      
	      
		   "0:Auto",
	      
		   "1000000:1",
	      
		   "2000000:2",
	      
		   "5500000:5.5",
	      
		   "6000000:6",
	      
		   "9000000:9",
	      
		   "11000000:11",
	      
		   "12000000:12",
	      
		   "18000000:18",
	      
		   "24000000:24",
	      
		   "36000000:36",
	      
		   "48000000:48",
	      
		   "54000000:54",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
		{"wl_IgmpSnEnable", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},
		{"wl_mcastrate", "", validate_range, ARGV("0","8"), FALSE, RESTART_WIFI},
	   	
	      {"wl_rateset", "", validate_choice, ARGV(	      
	      
		   "default:Default",
	      
		   "all:All",
	      
		   "12:1, 2 Mbps",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
			
	     {"wl_frag", "", validate_range, ARGV("256", "2346", ""), FALSE, RESTART_WIFI},	// 2007.10 James
		     
	     {"wl_rts", "", validate_range, ARGV("0", "2347", ""), FALSE, RESTART_WIFI},	// 2007.10 James
		     
	     {"wl_dtim", "", validate_range, ARGV("1", "255", ""), FALSE, RESTART_WIFI},	// 2007.10 James
		     
	     {"wl_bcn", "", validate_range, ARGV("1", "65535", ""), FALSE, RESTART_WIFI},	// 2007.10 James
		
	      {"wl_frameburst", "", validate_choice, ARGV(	      
	      
		   "off:Disabled",
	      
		   "on:Enabled",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   	
	      {"wl_mode_ex", "", validate_choice, ARGV(	      
	      
		   "ap:AP or WDS",
	      
		   "sta:Station",
	      
		   "wet:Ethernet Bridge",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
			    
		 {"wl_radio_x", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
	      
		{"wl_radio_date_x", "", validate_portrange, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
	    
		{"wl_radio_time_x", "", validate_portrange, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
			 
	     {"wl_radio_power_x", "", validate_range, ARGV("1", "84", ""), FALSE, RESTART_WIFI},	// 2007.10 James
	
      {"WirelessLog", "Status", NULL, ARGV("wlan11b.log",""), FALSE, FALSE},
					
		    {"", "", validate_string, ARGV(""), FALSE, FALSE},														   
		 	
	      {"wl_wme", "", validate_choice, ARGV(	      
	      
		   "0:Disabled",
	      
		   "1:Enabled",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   	
	      {"wl_wme_bss_disable", "", validate_choice, ARGV("0", "1"), FALSE, RESTART_WIFI}, // 2008.06 James
			  
	      {"wl_wme_no_ack", "", validate_choice, ARGV(	      
	      
		   "off:Disabled",
	      
		   "on:Enabled",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James

					{"APSDCapable", "", validate_choice, ARGV(
							"0:Disable",
							"1:Enable", 0), FALSE, RESTART_WIFI},	// 2008.08 magic
					{"DLSCapable", "", validate_choice, ARGV(
							"0:Disable",
							"1:Enable", 0), FALSE, RESTART_WIFI},	// 2008.08 magic
						   
		    {"wl_wme_ap_bk", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_ap_be", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_ap_vi", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_ap_vo", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_sta_bk", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_sta_be", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_sta_vi", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James
							 
		    {"wl_wme_sta_vo", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James

					{"wl_wme_txp_be", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},    // 2008.06 James

					{"wl_wme_txp_bk", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},    // 2008.06 James

					{"wl_wme_txp_vi", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},    // 2008.06 James

					{"wl_wme_txp_vo", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},    // 2008.06 James

		 {"wl_preauth", "", validate_range, ARGV("0","1"), FALSE, RESTART_WIFI},	// 2007.10 James
						      
		    {"wl_net_reauth", "", validate_string, ARGV(""), FALSE, RESTART_WIFI},	// 2007.10 James

{ "sta_ssid", "", 0, 0, 0, RESTART_APCLI},
{ "sta_bssid", "", 0, 0, 0, RESTART_APCLI},
{ "sta_auth_mode", "", 0, 0, 0, RESTART_APCLI},
{ "sta_wep_x", "", 0, 0, 0, RESTART_APCLI},
{ "sta_wpa_mode", "", 0, 0, 0, RESTART_APCLI},
{ "sta_crypto", "", 0, 0, 0, RESTART_APCLI},
{ "sta_wpa_psk", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key_type", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key1", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key2", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key3", "", 0, 0, 0, RESTART_APCLI},
{ "sta_key4", "", 0, 0, 0, RESTART_APCLI},
{ "sta_check_ha", "", 0, 0, 0, RESTART_APCLI},
{ "sta_authorized", "", 0, 0, 0, FALSE},

{"wl_guest_enable", "0", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_ssid_1", "guest", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_auth_mode_1", "open", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_crypto_1", "0", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_wpa_psk_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_wep_x_1", "0", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_phrase_x_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_key1_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_key2_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_key3_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_key4_1", "", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_key_1", "1", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"wl_guest_wpa_gtk_rekey_1", "0", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"lan1_ipaddr", "192.168.2.1", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"lan1_netmask", "255.255.255.0", 0, 0, 0, RESTART_REBOOT},	// 2007.10 James
{"dhcp1_enable_x", "1", 0, 0, 0, RESTART_DHCPD},	// 2007.10 James
{"dhcp1_start", "192.168.2.2", 0, 0, 0, RESTART_DHCPD},	// 2007.10 James
{"dhcp1_end", "192.168.2.254", 0, 0, 0, RESTART_DHCPD},	// 2007.10 James
{"lan1_lease", "86400", 0, 0, 0, RESTART_DHCPD},	// 2007.10 James

	{"RBRList", "Group", validate_group, ARGV(variables_WLANConfig11b_RBRList, "16", "32", "wl_wdsnum_x"), FALSE, RESTART_WIFI},	// 2008.01 James.

	{"rt_RBRList", "Group", validate_group, ARGV(variables_WLANConfig11b_rt_RBRList, "16", "32", "rt_wdsnum_x"), FALSE, RESTART_WIFI},
			
	{ 0, 0, 0, 0, 0, 0}

	};
   
      struct variable variables_DeviceSecurity11b[] = {
			       
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    	
	      {"wl_macmode", "", validate_choice, ARGV(	      
	      
		   "disabled:Disable",
	      
		   "allow:Accept",
	      
		   "deny:Reject",
	      
	      0), FALSE, RESTART_WIFI},	// 2007.10 James
	   	
              {"rt_macmode", "", validate_choice, ARGV(
                                                "disabled:Disable",
                                                "allow:Accept",
                                                "deny:Reject",
                                                0), FALSE, RESTART_WIFI},
				    
		       {"wl_macnum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},	// 2007.10 James
					     
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    
		{"", "", validate_string, ARGV(""), FALSE, FALSE}, 
	     
      {"ACLList", "Group", validate_group, ARGV(variables_DeviceSecurity11b_ACLList, "64", "32", "wl_macnum_x"), FALSE, RESTART_WIFI},	// 2008.01 James.
      {"rt_ACLList", "Group", validate_group, ARGV(variables_DeviceSecurity11b_rt_ACLList, "64", "32", "rt_macnum_x"), FALSE, RESTART_WIFI},
     
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_WLANAuthentication11b[] = {
			       
		       {"", "", validate_range, ARGV("0","65535"), FALSE, FALSE},											    
		    
		{"wl_radius_ipaddr", "", validate_ipaddr, NULL, FALSE, RESTART_WIFI},	// 2007.10 James
				     
		       {"wl_radius_port", "", validate_range, ARGV("0","65535"), FALSE, RESTART_WIFI},	// 2007.10 James
		    
		{"wl_radius_key", "", validate_string, ARGV("64"), FALSE, RESTART_WIFI},	// 2007.10 James
			
      { 0, 0, 0, 0, 0, 0}
      };
   
      struct variable variables_PrinterStatus[] = {
      
      {"x_PrinterModel", "Status", NULL, ARGV("printer_status.log","printer_model_t"), FALSE, FALSE},

      {"x_PrinterStatus", "Status", NULL, ARGV("printer_status.log","printer_status_t"), FALSE, FALSE},

      {"x_PrinterUser", "Status", NULL, ARGV("printer_status.log","printer_user_t"), FALSE, FALSE},
					
		    {"", "", validate_string, ARGV(""), FALSE, FALSE},														   
#ifndef NOQOS 
//      {"qos_urulenum_x", "", validate_range, ARGV("0","65535"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_rulenum_x", "8", validate_range, ARGV("0","8"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_global_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_service_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_tos_prio", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_pshack_prio", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_shortpkt_prio", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_dfragment_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_dfragment_size", "", validate_range, ARGV("0","100"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_service_ubw", "", validate_range, ARGV("0","100"), FALSE, RESTART_QOS},	// 2007.10 James

      {"qos_manual_ubw", "", validate_range, ARGV("0","1073741824"), FALSE, RESTART_QOS},	// 2007.10 James
// Viz add 2010.08
      {"qos_orules", "", validate_string, ARGV(""), FALSE, RESTART_QOS }, // 2009.12 Jerry5

      {"qos_default", "", validate_range, ARGV("0","5"), FALSE, RESTART_QOS}, // 2009.12 Jerry5

      {"qos_orates",	"", validate_string, ARGV(""), FALSE, RESTART_QOS}, // 2009.12 Jerry5
			
      {"qos_user_enable", "", validate_range, ARGV("0","1"), FALSE, RESTART_QOS}, // 2009.12 Jerry5

      {"qos_inuse", "", validate_range, ARGV("0","1024"), FALSE, RESTART_QOS}, // 2009.12 Jerry5
// end Viz

      {"x_USRRuleList", "Group", validate_group, ARGV(variables_PrinterStatus_x_USRRuleList, "8", "47", "qos_rulenum_x"), FALSE, RESTART_QOS},	// 2008.01 James.

//      {"x_QRuleList", "Group", validate_group, ARGV(variables_PrinterStatus_x_QRuleList, "8", "44", "qos_rulenum_x"), FALSE, FALSE},

//      {"x_UQRuleList", "Group", validate_group, ARGV(variables_PrinterStatus_x_UQRuleList, "8", "28", "qos_urulenum_x"), FALSE, FALSE},
#endif			
      { 0, 0, 0, 0, 0, 0}
      };
     
struct action actions_Layer3Forwarding[];
struct action actions_LANHostConfig[];
struct action actions_EthernetLink[];
struct action actions_IPConnection[];
struct action actions_PPPConnection[];
struct action actions_WANCommonInterface[];
struct action actions_General[];
struct action actions_FirewallConfig[];
struct action actions_RouterConfig[];
struct action actions_WLANConfig[];
struct action actions_DeviceSecurity[];
struct action actions_WLANAuthentication[];
struct action actions_PrinterStatus[];
struct variable variables_WLANAuthentication11b[];
struct action actions_Storage[];
struct action actions_Language[];

struct svcLink svcLinks[] = {	    
	   {"General", 	"urn:schemas-upnp-org:service:General:1", variables_General, actions_General},
	   {"LANHostConfig", "urn:schemas-upnp-org:service:Layer3Forwarding:1", variables_LANHostConfig, actions_LANHostConfig},
	   {"Layer3Forwarding", "urn:schemas-upnp-org:service:LANHostConfigManagement:0.8", variables_Layer3Forwarding, actions_Layer3Forwarding},
	   {"WANCommonInterface", "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1", variables_WANCommonInterface, actions_WANCommonInterface},	   
	   {"IPConnection", "urn:schemas-upnp-org:service:WANIPConnection:1", variables_IPConnection, actions_IPConnection},
	   {"PPPConnection", "urn:schemas-upnp-org:service:WANPPPConnection:1", variables_PPPConnection, actions_PPPConnection},
	   {"EthernetLink", "urn:schemas-upnp-org:service:WANEthernetLinkConfig:1", variables_EthernetLink, actions_EthernetLink},
	   {"FirewallConfig", "urn:schemas-upnp-org:service:FirewallConfig:1", variables_FirewallConfig, actions_FirewallConfig},
	   {"RouterConfig", "urn:schemas-upnp-org:service:RouterConfig:1", variables_RouterConfig, actions_RouterConfig},	   
	   {"WLANConfig11a", "urn:schemas-upnp-org:service:WLANConfiguration:1", variables_WLANConfig11a, actions_WLANConfig},
	   {"DeviceSecurity11a", "urn:schemas-upnp-org:service:DeviceSecurity:1", variables_DeviceSecurity11a, actions_DeviceSecurity},
	   {"WLANAuthentication11a", "urn:schemas-upnp-org:service:WLANAuthentication:1", variables_WLANAuthentication11a, actions_WLANAuthentication},
	   {"WLANConfig11b", "urn:schemas-upnp-org:service:WLANConfiguration:1", variables_WLANConfig11b, actions_WLANConfig},
	   {"DeviceSecurity11b", "urn:schemas-upnp-org:service:DeviceSecurity:1", variables_DeviceSecurity11b, actions_DeviceSecurity},
	   {"WLANAuthentication11b", "urn:schemas-upnp-org:service:WLANAuthentication:1", variables_WLANAuthentication11b, actions_WLANAuthentication},	 
	   {"PrinterStatus", "urn:schemas-upnp-org:service:PrinterStatus:1", variables_PrinterStatus, actions_PrinterStatus},
	   {"Storage", "urn:schemas-upnp-org:service:Storage:1", variables_Storage, actions_Storage},
	   {"LANGUAGE", "urn:schemas-upnp-org:service:LANGUAGE:1", variables_Language, actions_Language},
	   {0, 0, 0}
};
	
