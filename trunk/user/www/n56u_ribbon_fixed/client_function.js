var selectedClientOrder;

// ipmonitor: [[IP, MAC, DeviceName, Type, http, staled], ...]
// wireless: [MAC, ...] ==Yonsm==> {MAC:RSSI, ...} from ej_wl_auth_list

var sort_mode = parseInt(localStorage.getItem('sortMode'));

function getclients(flag_mac,flag_all){
	var clients = new Array();
	var j = 0;
	for(var i = 0; i < ipmonitor.length; ++i){
		if (flag_all != 1) {
			if (ipmonitor[i][5] == "1")
				continue;
		}
		
		clients[j] = new Array(8);
		
		clients[j][0] = ipmonitor[i][2];	// Device name
		clients[j][1] = ipmonitor[i][0];	// IP
		clients[j][2] = ipmonitor[i][1];	// MAC
		clients[j][3] = null;			// host is a wireless client
		clients[j][4] = null;			// this is a wireless info
		clients[j][5] = ipmonitor[i][3];	// TYPE
		clients[j][6] = ipmonitor[i][4];	// host has a HTTP service
		clients[j][7] = "u";

		for(var mac in wireless){
			if(clients[j][2] == mac){
				clients[j][3] = 10;	// 10 is meant the client is wireless.
				clients[j][4] = wireless[mac]; // By Yonsm: Fetch from wireless
				break;
			}
		}
		
		if(clients[j][0] == null || clients[j][0].length < 1)
			clients[j][0] = "*";
		
		if (flag_mac == 1)
			clients[j][2] = simplyMAC(clients[j][2]);
		++j;
	}

	clients.sort(function(a,b){
		var ret;
		if (sort_mode == 1 || sort_mode == -1) { // Name
			ret = a[0].localeCompare(b[0]);
		} else if (sort_mode == 3 || sort_mode == -3) { // MAC
			ret = a[2].localeCompare(b[2]);
		} else if (sort_mode == 4 || sort_mode == -4) { // RSSI
			ret = (parseInt(a[4])||0) - (parseInt(b[4])||0);
		} else if (sort_mode == 0) { // Type
			ret = a[5].localeCompare(b[5]);
		} else  { // IP
			var aa = a[1].split(".");
			var bb = b[1].split(".");
			var resulta = aa[0]*0x1000000 + aa[1]*0x10000 + aa[2]*0x100 + aa[3]*1;
			var resultb = bb[0]*0x1000000 + bb[1]*0x10000 + bb[2]*0x100 + bb[3]*1;
			ret = resulta-resultb;
		}
		return (sort_mode < 0) ? (ret * -1) : ret;
	});

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

