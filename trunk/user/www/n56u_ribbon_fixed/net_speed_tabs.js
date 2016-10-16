
var idTimerPoll = 0;
var tabs = [];
var tabs_desc = [
	[  0, "VPNC",      "<#VPNC_Desc#>" ],
	[  1, "WWAN",      "3G/LTE <#USB_Modem#>" ],
	[  2, "WAN",       "Wired: WAN" ],
	[  3, "LAN",       "Wired: LAN" ],
	[  4, "WLAN2_MII", "Wireless 2.4GHz: iNIC" ],
	[  5, "WLAN2_AP0", "Wireless 2.4GHz: AP Main" ],
	[  6, "WLAN2_AP1", "Wireless 2.4GHz: AP Guest" ],
	[  7, "WLAN2_APC", "Wireless 2.4GHz: AP-Client" ],
	[  8, "WLAN2_WDS", "Wireless 2.4GHz: WDS" ],
	[  9, "WLAN5_AP0", "Wireless 5GHz: AP Main" ],
	[ 10, "WLAN5_AP1", "Wireless 5GHz: AP Guest" ],
	[ 11, "WLAN5_APC", "Wireless 5GHz: AP-Client" ],
	[ 12, "WLAN5_WDS", "Wireless 5GHz: WDS" ],
	[ 13, "ESW_P0",    "Ethernet port: WAN" ],
	[ 14, "ESW_P1",    "Ethernet port: LAN1" ],
	[ 15, "ESW_P2",    "Ethernet port: LAN2" ],
	[ 16, "ESW_P3",    "Ethernet port: LAN3" ],
	[ 17, "ESW_P4",    "Ethernet port: LAN4" ],
	[ 18, "ESW_P5",    "Ethernet port: LAN5" ],
	[ 19, "ESW_P6",    "Ethernet port: LAN6" ],
	[ 20, "ESW_P7",    "Ethernet port: LAN7" ]
];

Highcharts.locale = {
	global : {
		useUTC : false
	},
	lang: {
		months: ['<#MF_Jan#>', '<#MF_Feb#>', '<#MF_Mar#>', '<#MF_Apr#>', '<#MF_May#>', '<#MF_Jun#>', '<#MF_Jul#>', '<#MF_Aug#>', '<#MF_Sep#>', '<#MF_Oct#>', '<#MF_Nov#>', '<#MF_Dec#>'],
		shortMonths: ['<#MS_Jan#>', '<#MS_Feb#>', '<#MS_Mar#>', '<#MS_Apr#>', '<#MS_May#>', '<#MS_Jun#>', '<#MS_Jul#>', '<#MS_Aug#>', '<#MS_Sep#>', '<#MS_Oct#>', '<#MS_Nov#>', '<#MS_Dec#>'],
		weekdays: ['<#WF_Sun#>', '<#WF_Mon#>', '<#WF_Tue#>', '<#WF_Wed#>', '<#WF_Thu#>', '<#WF_Fri#>', '<#WF_Sat#>'],
		rangeSelectorZoom: '<#HSTOCK_Zoom#>'
	}
};

function bytesToIEC(bytes, precision){
	var absval = Math.abs(bytes);
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;
	var petabyte = terabyte * 1024;

	if(absval < kilobyte)
		return bytes + ' B';
	else if(absval < megabyte)
		return (bytes / kilobyte).toFixed(precision) + ' KiB';
	else if(absval < gigabyte)
		return (bytes / megabyte).toFixed(precision) + ' MiB';
	else if(absval < terabyte)
		return (bytes / gigabyte).toFixed(precision) + ' GiB';
	else if(absval < petabyte)
		return (bytes / terabyte).toFixed(precision) + ' TiB';
	else
		return (bytes / petabyte).toFixed(precision) + ' PiB';
}

function bytesToBitrate(bytes, precision){
	var bits = bytes * 8;
	var absval = Math.abs(bits);
	var kilobit = 1000;
	var megabit = kilobit * 1000;
	var gigabit = megabit * 1000;

	if(absval < megabit)
		return (bits / kilobit).toFixed(precision) + ' Kbps';
	else if(absval < gigabit)
		return (bits / megabit).toFixed(precision) + ' Mbps';
	else
		return (bits / gigabit).toFixed(precision) + ' Gbps';
}

function initTab()
{
	E('rx-sel').style.background = '#FF9000';
	E('tx-sel').style.background = '#003EBA';

	E('rx-current').innerHTML = bytesToBitrate(0, 2);
	E('rx-avg').innerHTML = bytesToBitrate(0, 2);
	E('rx-max').innerHTML = bytesToBitrate(0, 2);

	E('tx-current').innerHTML = bytesToBitrate(0, 2);
	E('tx-avg').innerHTML = bytesToBitrate(0, 2);
	E('tx-max').innerHTML = bytesToBitrate(0, 2);

	E('rx-total').innerHTML = bytesToIEC(0, 2);
	E('tx-total').innerHTML = bytesToIEC(0, 2);
}

function updateTab(h)
{
	if (h === undefined)
		return;

	if ((typeof(h.rx) === 'undefined') || (typeof(h.tx) === 'undefined'))
		return;

	E('rx-current').innerHTML = bytesToBitrate(h.rx[h.rx.length - 1], 2);
	E('rx-avg').innerHTML = bytesToBitrate(h.rx_avg, 2);
	E('rx-max').innerHTML = bytesToBitrate(h.rx_max, 2);

	E('tx-current').innerHTML = bytesToBitrate(h.tx[h.tx.length - 1], 2);
	E('tx-avg').innerHTML = bytesToBitrate(h.tx_avg, 2);
	E('tx-max').innerHTML = bytesToBitrate(h.tx_max, 2);

	E('rx-total').innerHTML = bytesToIEC(h.rx_total, 2);
	E('tx-total').innerHTML = bytesToIEC(h.tx_total, 2);
}

function findTab(dev_name)
{
	for(var j=0; j < tabs_desc.length; j++){
		if(tabs_desc[j][1] === dev_name)
			return tabs_desc[j];
	}
	return null;
}

function processTabs()
{
	var tabs_old = tabs;
	var i, t, changed;

	tabs = [];

	for(i in speed_history){
		t = findTab(i);
		if (t)
			tabs.push(['speed-tab-' + i, t[2], t[0]]);
	}

	tabs.sort(function(a, b){
		if (a[2] < b[2]) return -1;
		if (a[2] > b[2]) return 1;
		return 0;
	});

	if(tabs.length == tabs_old.length){
		for (i = tabs.length - 1; i >= 0; --i){
			if (tabs[i][0] != tabs_old[i][0])
				break;
		}
		changed = (i > 0);
	}else
		changed = 1;

	if(changed)
		handleTabs(tabs);
}

function getTabDesc(dev_name){
	var t = findTab(dev_name);
	if (t)
		return t[2];
	return null;
}

function invoke_timer(s){
	idTimerPoll = setTimeout('load_netdevs()', s*1000);
}
