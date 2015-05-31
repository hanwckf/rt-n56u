
var tabs = [];
var cookie_pref = 'n56u_chart_';

function E(e)
{
	return (typeof(e) == 'string') ? document.getElementById(e) : e;
}

var cookie = {
	set: function(key, value, days) {
		document.cookie = cookie_pref + key + '=' + value + '; expires=' +
			(new Date(new Date().getTime() + ((days ? days : 14) * 86400000))).toUTCString() + '; path=/';
	},

	get: function(key) {
		var r = ('; ' + document.cookie + ';').match('; ' + cookie_pref + key + '=(.*?);');
		return r ? r[1] : null;
	},

	unset: function(key) {
		document.cookie = cookie_pref + key + '=; expires=' + (new Date(1)).toUTCString() + '; path=/';
	}
};

function fixInt(n, min, max, def)
{
	if (n === null) return def;
	n *= 1;
	if (isNaN(n)) return def;
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

function bytesToIEC(bytes, precision){
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;
	var petabyte = terabyte * 1024;

	if ((bytes >= 0) && (bytes < kilobyte))
		return bytes + ' B';
	else if ((bytes >= kilobyte) && (bytes < megabyte))
		return (bytes / kilobyte).toFixed(precision) + ' KiB';
	else if ((bytes >= megabyte) && (bytes < gigabyte))
		return (bytes / megabyte).toFixed(precision) + ' MiB';
	else if ((bytes >= gigabyte) && (bytes < terabyte))
		return (bytes / gigabyte).toFixed(precision) + ' GiB';
	else if ((bytes >= terabyte) && (bytes < petabyte))
		return (bytes / terabyte).toFixed(precision) + ' TiB';
	else if  (bytes >= petabyte)
		return (bytes / petabyte).toFixed(precision) + ' PiB';
	else
		return bytes + ' B';
}

function bytesToBitrate(bytes, precision){
	var bits = bytes * 8;
	var kilobit = 1000;
	var megabit = kilobit * 1000;
	var gigabit = megabit * 1000;
	var terabit = gigabit * 1000;

	if ((bits >= 0) && (bits < megabit))
		return (bits / kilobit).toFixed(precision) + ' Kbps';
	else if ((bits >= megabit) && (bits < gigabit))
		return (bits / megabit).toFixed(precision) + ' Mbps';
	else if ((bits >= gigabit) && (bits < terabit))
		return (bits / gigabit).toFixed(precision) + ' Gbps';
	else
		return (bits / terabit).toFixed(precision) + ' Tbps';
}

function _tabCreate(tabs)
{
	var buf = [];
	buf.push('<ul id="tabs" class="nav nav-tabs">');
	for (var i = 0; i < arguments.length; ++i)
		buf.push('<li><a href="javascript:void(0)" id="' + arguments[i][0] + '">' + arguments[i][1] + '</a>');
	buf.push('</ul><div id="tabs-bottom"></div>');
	return buf.join('');
}

function tabCreate(tabs)
{
	document.write(_tabCreate.apply(this, arguments));
}

function tabHigh(id)
{
	var a = E('tabs').getElementsByTagName('A');
	$j(a).parent().removeClass('active');
	$j(E(id)).parent().addClass('active');
}

function showTab(name)
{
	var h,ifdesc;

	ifdesc = name.replace('speed-tab-', '');
	cookie.set(cprefix + 'tab', ifdesc, 14);
	tabHigh(name);

	updateChart(ifdesc);

	h = speed_history[ifdesc];
	if (h === undefined)
		return;

	E('rx-current').innerHTML = bytesToBitrate(h.rx[h.rx.length - 1]/updateDiv, 2);
	E('rx-avg').innerHTML = bytesToBitrate(h.rx_avg/updateDiv, 2);
	E('rx-max').innerHTML = bytesToBitrate(h.rx_max/updateDiv, 2);

	E('tx-current').innerHTML = bytesToBitrate(h.tx[h.tx.length - 1]/updateDiv, 2);
	E('tx-avg').innerHTML = bytesToBitrate(h.tx_avg/updateDiv, 2);
	E('tx-max').innerHTML = bytesToBitrate(h.tx_max/updateDiv, 2);

	E('rx-total').innerHTML = bytesToIEC(h.rx_total, 2);
	E('tx-total').innerHTML = bytesToIEC(h.tx_total, 2);
}

function processTabs()
{
	var old = tabs;
	var i, t, changed, name;

	tabs = [];

	for (i in speed_history) {
		var h = speed_history[i];
		if ((typeof(h.rx) === 'undefined') || (typeof(h.tx) === 'undefined')) {
			delete speed_history[i];
			continue;
		}
		
		t = i;
		if (i == "WLAN5")
			t = 'Wireless <small>(5GHz)</small>';
		else if (i == "WLAN2")
			t = 'Wireless <small>(2.4GHz)</small>';
		else if (i == "LAN")
			t = 'Wired LAN';
		else if (i == "WAN")
			t = ' Wired WAN';
		else if (i == 'WWAN') {
			t = ' 3G/LTE';
		}
		
		if (t != i && i != "")
			tabs.push(['speed-tab-' + i, t]);
	}

	tabs = tabs.sort(function(a, b) {
			if (a[1] < b[1]) return -1;
			if (a[1] > b[1]) return 1;
			return 0;
		});

	if (tabs.length == old.length) {
		for (i = tabs.length - 1; i >= 0; --i){
			if (tabs[i][0] != old[i][0])
				break;
		}
		changed = (i > 0);
	}
	else
		changed = 1;

	name = cookie.get(cprefix + 'tab');

	if (changed) {
		E('tab-area').innerHTML = _tabCreate.apply(this, tabs);
		createCharts(tabs,name);
	}

	if ((name != null) && ((speed_history[name] !== undefined))) {
		showTab('speed-tab-' + name);
	} else if (tabs.length) {
		showTab(tabs[0][0]);
	}
}

function initTabs()
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
