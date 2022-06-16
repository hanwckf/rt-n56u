<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu4#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();
<% bandwidth("history", ""); %>
var scale = 2;
var dateFormat = 2;
var cprefix = "hist_";
var idTimerPoll = 0;

var months_s = ['<#MS_Jan#>', '<#MS_Feb#>', '<#MS_Mar#>', '<#MS_Apr#>', '<#MS_May#>', '<#MS_Jun#>', '<#MS_Jul#>', '<#MS_Aug#>', '<#MS_Sep#>', '<#MS_Oct#>', '<#MS_Nov#>', '<#MS_Dec#>'];
var months_f = ['<#MF_Jan#>', '<#MF_Feb#>', '<#MF_Mar#>', '<#MF_Apr#>', '<#MF_May#>', '<#MF_Jun#>', '<#MF_Jul#>', '<#MF_Aug#>', '<#MF_Sep#>', '<#MF_Oct#>', '<#MF_Nov#>', '<#MF_Dec#>'];
var snames = [' KiB', ' MiB', ' GiB', ' TiB'];
var tabs_desc = [
	[ "WAN",  "Wired: WAN" ],
	[ "WISP", "Wireless: WISP" ],
	[ "WWAN", "3G/LTE <#USB_Modem#>" ]
];

$j(document).ready(function(){
	$j("#tabs a").click(function(){
		switchPage(this.id);
		return false;
	});
});

$j(window).bind('hashchange', function(){
	redraw_history();
});

function initial(){
	show_banner(0);
	show_menu(5, -1, 0);
	show_footer();

	initDate();
	initScale();

	handleTabs();
	sort_history();
	redraw_history();

	invoke_timer(poll_next);
}

function getHash(){
	var curHash = window.location.hash.toLowerCase();
	if (curHash != '#dy' && curHash != '#mo')
		curHash = '#dy';
	return curHash;
}

function pad(n,min){
	n = n.toString();
	while (n.length < min) n = '0' + n;
	return n;
}

function getYMD(n){
	return [(((n >> 16) & 0xFF) + 1900), ((n >>> 8) & 0xFF), (n & 0xFF)];
}

function comma(n){
	n = '' + n;
	var p = n;
	while ((n = n.replace(/(\d+)(\d{3})/g, '$1,$2')) != p) p = n;
	return n;
}

function fixInt(n, min, max, def){
	if (n === null) return def;
	n *= 1;
	if (isNaN(n)) return def;
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

function rescale(n, z){
	if ((z) && (n == 0))
		return '-';
	var d = 1;
	if (scale == 1)
		d = 1024;
	else if (scale == 2)
		d = 1024*1024;
	else if (scale == 3)
		d = 1024*1024*1024;
	return (((z) && (n > 0)) ? '+' : '') + comma((n / d).toFixed(2)) + snames[scale];
}

function makeRow(rtitle, dl, ul, total){
	return '<tr>' +
		'<td class="rtitle">' + rtitle + '</td>' +
		'<td class="dl">' + dl + '</td>' +
		'<td class="ul">' + ul + '</td>' +
		'<td class="total">' + total + '</td>' +
		'</tr>';
}

function getTabDesc(idx){
	for(var j=0; j < tabs_desc.length; j++){
		if (tabs_desc[j][0] == idx)
			return tabs_desc[j][1];
	}
	return idx;
}

function tabSelect(tabVal){
	if(netdev != tabVal){
		netdev = tabVal;
		load_history();
	}
}

function handleTabs(){
	var o = E('sel_netif');
	if (o.length != netdevs.length){
		free_options(o);
		for(var i=0; i<netdevs.length; i++)
			add_option(o, getTabDesc(netdevs[i]), netdevs[i], netdevs[i] == netdev);
	}
}

function initScale(){
	scale = fixInt(cookie.get(cprefix + 'scale'), 0, 3, 2);
	E('scale').value = scale;
}

function changeScale(e){
	scale = e.value * 1;
	cookie.set(cprefix + 'scale', e.value, 31);
	redraw_history();
}

function initDate(){
	dateFormat = fixInt(cookie.get(cprefix + 'ymd'), 0, 6, 2);
	E('dafm').value = dateFormat;
}

function changeDate(e){
	dateFormat = e.value * 1;
	cookie.set(cprefix + 'ymd', e.value, 31);
	redraw_history();
}

function cmpHist(a, b){
	a = parseInt(a[0], 0);
	b = parseInt(b[0], 0);
	if (a < b) return 1;
	if (a > b) return -1;
	return 0;
}

function sort_history(){
	daily_history.sort(cmpHist);
	monthly_history.sort(cmpHist);
}

function invoke_timer(s){
	idTimerPoll = setTimeout('load_history()', s*1000);
}

function eval_history(response){
	netdevs.length = 0;
	daily_history.length = 0;
	monthly_history.length = 0;

	try {
		eval(response);
	}
	catch (ex) {
		daily_history.length = 0;
		monthly_history.length = 0;
	}

	handleTabs();
	sort_history();
	redraw_history();

	invoke_timer(poll_next);
}

function load_history(){
	clearTimeout(idTimerPoll);
	$j.ajax({
		type: "get",
		url: "/update.cgi",
		data: {
			output: "bandwidth",
			arg0: "history",
			arg1: netdev
		},
		dataType: "script",
		cache: true,
		error: function(xhr){
			invoke_timer(5);
		},
		success: function(response){
			eval_history(response);
		}
	});
}

function ymdText(yr, mo, da){
	switch (dateFormat) {
	case 1:
		return (mo + 1) + '-' + pad(da,2) + '-' + yr;
	case 2:
		return months_s[mo] + ' ' + da + ', ' + yr;
	case 3:
		return months_f[mo] + ' ' + da + ', ' + yr;
	case 4:
		return pad(da,2) + '.' + pad(mo + 1, 2) + '.' + yr;
	case 5:
		return pad(da,2) + '-' + months_s[mo] + '-' + yr;
	case 6:
		return da + ' ' + months_s[mo] + ' ' + yr;
	}
	return yr + '-' + pad(mo + 1, 2) + '-' + pad(da, 2);
}

function ymText(yr, mo){
	return months_f[mo] + ', ' + yr;
}

function redraw_history(){
	var i,h,ymd,grid,hash;

	grid =  '<table class="table table-striped" width="100%" cellspacing="1">';
	grid += '<tr><th valign="top" style="text-align:left"><#Date#></th>';
	grid += '<th width="25%" style="text-align:right" valign="top"><#Downlink#></th>';
	grid += '<th width="25%" style="text-align:right" valign="top"><#Uplink#></th>';
	grid += '<th width="25%" style="text-align:right" valign="top"><#Total#></th></tr>';

	hash = getHash();
	if (hash == '#dy'){
		$j('#tab_tr_mo').parents('li').removeClass('active');
		$j('#tab_tr_dy').parents('li').addClass('active');
		E('bx_label').innerHTML = '<#menu4#> - <#menu4_2_3#>';
		E('dafm').disabled = 0;
		for (i = 0; i < daily_history.length; ++i){
			h = daily_history[i];
			ymd = getYMD(h[0]);
			grid += makeRow(ymdText(ymd[0], ymd[1], ymd[2]), rescale(h[1]), rescale(h[2]), rescale(h[1] + h[2]));
		}
	}else{
		$j('#tab_tr_dy').parents('li').removeClass('active');
		$j('#tab_tr_mo').parents('li').addClass('active');
		E('bx_label').innerHTML = '<#menu4#> - <#menu4_2_4#>';
		E('dafm').disabled = 1;
		for (i = 0; i < monthly_history.length; ++i){
			h = monthly_history[i];
			ymd = getYMD(h[0]);
			grid += makeRow(ymText(ymd[0], ymd[1]), rescale(h[1]), rescale(h[2]), rescale(h[1] + h[2]));
		}
	}

	E('bwm-daily-grid').innerHTML = grid + '</table>';
}

function switchPage(id){
	if(id == "tab_bw_rt")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(id == "tab_bw_24")
		location.href = "/Main_TrafficMonitor_last24.asp";
	else if(id == "tab_tr_dy")
		location.href = "/Main_TrafficMonitor_daily.asp#DY";
	else if(id == "tab_tr_mo")
		location.href = "/Main_TrafficMonitor_daily.asp#MO";
	return false;
}

</script>
<style>
#tabs {margin-bottom: 0px;}
.table-striped td {padding: 4px 8px;}
.table td.dl, .table td.ul, .table td.total {text-align: right;}
</style>
</head>

<body onload="initial();" >

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9">
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <div class="container-fluid">
        <div class="row-fluid">
            <div class="span3">
                <!--Sidebar content-->
                <!--=====Beginning of Main Menu=====-->
                <div class="well sidebar-nav side_nav" style="padding: 0px;">
                    <ul id="mainMenu" class="clearfix"></ul>
                    <ul class="clearfix">
                        <li>
                            <div id="subMenu" class="accordion"></div>
                        </li>
                    </ul>
                </div>
            </div>

            <div class="span9">
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 id="bx_label" class="box_head round_top"><#menu4#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <div align="right" style="margin: 3px 8px 0px 0px;">
                                        <select id="sel_netif" style="width: 200px; margin-bottom: 5px;" onchange="tabSelect(this.value);">
                                        </select>
                                    </div>

                                    <div style="margin-bottom: 0px; margin: -36px 0px 0px 0px;">
                                        <ul id="tabs" class="nav nav-tabs">
                                            <li><a href="javascript:void(0)" id="tab_bw_rt"><#menu4_2_1#></a></li>
                                            <li><a href="javascript:void(0)" id="tab_bw_24"><#menu4_2_2#></a></li>
                                            <li><a href="#DY" id="tab_tr_dy"><#menu4_2_3#></a></li>
                                            <li><a href="#MO" id="tab_tr_mo"><#menu4_2_4#></a></li>
                                        </ul>
                                    </div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table table-striped" style="margin-bottom: 0px;">
                                        <tr>
                                            <td width="50%" style="border-top: 0 none;">
                                                <#Date#>:&nbsp;
                                                <select id="dafm" style="width: 200px;" onchange="changeDate(this);">
                                                    <option value=0>yyyy-mm-dd</option>
                                                    <option value=1>m-dd-yyyy</option>
                                                    <option value=2>mmm d, yyyy</option>
                                                    <option value=3>mmmm d, yyyy</option>
                                                    <option value=4>dd.mm.yyyy</option>
                                                    <option value=5>dd-mmm-yyyy</option>
                                                    <option value=6>d mmm yyyy</option>
                                                </select>
                                            </td>
                                            <td align="right" style="border-top: 0 none; text-align: right;">
                                                <#Scale#>:&nbsp;
                                                <select id="scale" style="width: 200px;" onchange="changeScale(this);">
                                                    <option value=0>KiB</option>
                                                    <option value=1>MiB</option>
                                                    <option value=2>GiB</option>
                                                    <option value=3>TiB</option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <div id="bwm-daily-grid"></div>

                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div id="footer"></div>
</div>

</body>
</html>
