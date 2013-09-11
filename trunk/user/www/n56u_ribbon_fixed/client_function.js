var selectedClientOrder;

// ipmonitor: [[IP, MAC, DeviceName, Type, http, printer, iTune], ...]
// wireless: [[MAC, associated, authorized], ...]
// leases: [[hostname, MAC, IP], ...]
// m_dhcp: [[mac, IP, hostname], ...]

function getclients(flag){
	var clients = new Array();
	
	for(var i = 0; i < ipmonitor.length; ++i){
		var j;
		
		clients[i] = new Array(9);
		
		clients[i][0] = ipmonitor[i][2];  // Device name
		
		for(j = leases.length-1; j >= 0; --j){
			if(leases[j][3] == "Expired")
				continue;
			
			if(leases[j][0] == null || leases[j][0].length <= 0)
				continue;
			
			if(ipmonitor[i][0] == leases[j][2]){
				if(clients[i][0] != leases[j][0])
					clients[i][0] = leases[j][0];
				break;
			}
		}
		
		if(clients[i][0] == null || clients[i][0].length <= 0) {
			var mac_up;
			for(j = 0; j < m_dhcp.length; ++j){
				if(m_dhcp[j][2] == null || m_dhcp[j][2].length <= 0)
					continue;
				
				mac_up = mac_add_delimiters(m_dhcp[j][0]);
				if(ipmonitor[i][0] == m_dhcp[j][1] && ipmonitor[i][1] == mac_up){
					clients[i][0] = m_dhcp[j][2];
					break;
				}
			}
		}
		
		clients[i][1] = ipmonitor[i][0];	// IP
		clients[i][2] = ipmonitor[i][1];	// MAC
		clients[i][3] = null;	// if this is a wireless client.
		clients[i][4] = null;	// wireless information.
		clients[i][5] = ipmonitor[i][3];	// TYPE
		clients[i][6] = ipmonitor[i][4];	// if there's the HTTP service.
		clients[i][7] = ipmonitor[i][5];	// if there's the Printer service.
		clients[i][8] = ipmonitor[i][6];	// if there's the iTune service.
		
		for(j = 0; j < wireless.length; ++j){
			if(clients[i][2] == wireless[j][0]){
				clients[i][3] = 10;	// 10 is meant the client is wireless.
				
				clients[i][4] = new Array(2);
				clients[i][4][0] = wireless[j][1];
				clients[i][4][1] = wireless[j][2];
				
				if(clients[i][4][0] == "Yes")
					clients[i][4][0] = "Associated";
				else
					clients[i][4][0] = "Disassociated";
				
				break;
			}
		}
		
		if(clients[i][0] == null || clients[i][0].length <= 0)
			clients[i][0] = "???";
		
		if(flag == 1)
			clients[i][2] = simplyMAC(clients[i][2]);
	}
	
	return clients;
}

function simplyMAC(fullMAC){
	var ptr;
	var tempMAC;
	var pos1, pos2;
	
	ptr = fullMAC;
	tempMAC = "";
	pos1 = pos2 = 0;
	
	for(var i = 0; i < 5; ++i){
		pos2 = pos1+ptr.indexOf(":");
		
		tempMAC += fullMAC.substring(pos1, pos2);
		
		pos1 = pos2+1;
		ptr = fullMAC.substring(pos1);
	}
	
	tempMAC += fullMAC.substring(pos1);
	
	return tempMAC;
}

function mac_add_delimiters(raw_mac) {
	var ret="";
	for (var i=0; i < raw_mac.length; i++) {
		ret += raw_mac.charAt(i);
		if (i % 2 == 1 && i < raw_mac.length-1)
			ret += ":";
	}
	return ret.toUpperCase();
}

function test_all_clients(clients){
	var str = "";
	var Row;
	var Item;
	
	str += clients.length+"\n";
	
	Row = 1;
	for(var i = 0; i < clients.length; ++i){
		if(Row == 1)
			Row = 0;
		else
			str += "\n";
		
		Item = 1;
		for(var j = 0; j < 9; ++j){
			if(Item == 1)
				Item = 0;
			else
				str += ", ";
			
			str += clients[i][j];
		}
		
		str += "\n";
	}
	
	alert(str);
}
