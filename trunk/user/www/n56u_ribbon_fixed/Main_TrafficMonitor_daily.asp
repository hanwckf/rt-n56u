<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu4_2#> : <#menu4_2_3#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/net_chart_tabs.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>

<% nvram("rstats_enable"); %>

<% bandwidth("daily"); %>

var months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
var snames = [' KiB', ' MiB', ' GiB', ' TiB'];
var dateFormat = -1;
var scale = 2;

function initial()
{
	show_banner(0);
	show_menu(5, -1, 0);
	show_footer();

	if (nvram.rstats_enable != '1')
		return;

	var s = cookie.get('daily');
	if (s != null) {
		if (s.match(/^([0-3])$/))
			E('scale').value = scale = RegExp.$1 * 1;
	}
	initDate('ymd');
	daily_history.sort(cmpHist);
	redraw();
}

function save(){
	cookie.set('daily', scale, 31);
}

function pad(n,min){
	n = n.toString();
	while (n.length < min) n = '0' + n;
	return n;
}

function ymdText(yr, mo, da){
	switch (dateFormat) {
	case 1:
		return (mo + 1) + '-' + pad(da,2) + '-' + yr;
	case 2:
		return months[mo] + ' ' + da + ', ' + yr;
	case 3:
		return pad(da,2) + '.' + pad(mo + 1, 2) + '.' + yr;
	}
	return yr + '-' + pad(mo + 1, 2) + '-' + pad(da, 2);
}

function getYMD(n){
	return [(((n >> 16) & 0xFF) + 1900), ((n >>> 8) & 0xFF), (n & 0xFF)];
}

function initDate(c){
	dateFormat = fixInt(cookie.get(c), 0, 3, 0);
	E('dafm').value = dateFormat;
}

function changeDate(e, c){
	dateFormat = e.value * 1;
	cookie.set(c, e.value, 31);
	redraw();
}

function comma(n){
	n = '' + n;
	var p = n;
	while ((n = n.replace(/(\d+)(\d{3})/g, '$1,$2')) != p) p = n;
	return n;
}

function rescale(n, z){
	if ((z) && (n == 0))
		return '-';
	var d = 1024*1024;
	if (scale == 1)
		d = 1024;
	else if (scale == 3)
		d = 1024*1024*1024;
	return (((z) && (n > 0)) ? '+' : '') + comma((n / d).toFixed(2)) + snames[scale];
}

function cmpHist(a, b){
	a = parseInt(a[0], 0);
	b = parseInt(b[0], 0);
	if (a < b) return 1;
	if (a > b) return -1;
	return 0;
}

function changeScale(e){
	scale = e.value * 1;
	redraw();
	save();
}

function makeRow(rclass, rtitle, dl, ul, total){
	return '<tr class="' + rclass + '">' +
		'<td class="rtitle">' + rtitle + '</td>' +
		'<td class="dl">' + dl + '</td>' +
		'<td class="ul">' + ul + '</td>' +
		'<td class="total">' + total + '</td>' +
		'</tr>';
}

function redraw(){
	var h;
	var grid;
	var rows;
	var ymd;
	var d;
	var lastt;
	var lastu, lastd;

	if (daily_history.length-1 > 0) {
		ymd = getYMD(daily_history[0][0]);
		d = new Date((new Date(ymd[0], ymd[1], ymd[2], 12, 0, 0, 0)).getTime() - ((30 - 1) * 86400000));
		E('last-dates').innerHTML = '<br/>(' + ymdText(ymd[0], ymd[1], ymd[2]) + ' ~ ' + ymdText(d.getFullYear(), d.getMonth(), d.getDate()) + ')';
		lastt = ((d.getFullYear() - 1900) << 16) | (d.getMonth() << 8) | d.getDate();
	}

	lastd = 0;
	lastu = 0;
	rows = 0;
	block = '';
	gn = 0;

	grid = '<table class="table" cellspacing="1">';

	grid += "<tr><th width='40%' valign='top' style='text-align:left'><#Date#></th>";
	grid += "<th width='20%' style='text-align:right' valign='top'><#Downlink#></th>";
	grid += "<th width='20%' style='text-align:right' valign='top'><#Uplink#></th>";
	grid += "<th width='20%' style='text-align:right' valign='top'><#Total#></th></tr>";
	
	for (i = 0; i < daily_history.length-1; ++i) {
		h = daily_history[i];
		ymd = getYMD(h[0]);
		grid += makeRow(((rows & 1) ? 'odd' : 'even'), ymdText(ymd[0], ymd[1], ymd[2]), rescale(h[1]), rescale(h[2]), rescale(h[1] + h[2]));
		++rows;

		if (h[0] >= lastt) {
			lastd += h[1];
			lastu += h[2];
		}
	}

	E('bwm-daily-grid').innerHTML = grid + '</table>';
	
	E('last-dn').innerHTML = rescale(lastd);
	E('last-up').innerHTML = rescale(lastu);
	E('last-total').innerHTML = rescale(lastu + lastd);
}

function switchPage(page){
	if(page == "1")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(page == "2")
		location.href = "/Main_TrafficMonitor_last24.asp";
	return false;
}
</script>

<style>
    .table td.dl, .table td.ul, .table td.total {text-align: right;}
    #last-dn, #last-up, #last-total {font-weight: bold;}
</style>

</head>

<body onload="initial();" >

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0" style="position: relative;"></iframe>
    <form method="post" name="form" action="apply.cgi" >
    <input type="hidden" name="current_page" value="Main_TrafficMonitor_daily.asp">
    <input type="hidden" name="next_page" value="Main_TrafficMonitor_daily.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

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
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu4#></h2>
                            <div class="round_bottom">
                                <div id="tabMenu"></div>

                                <div align="right" style="margin: 8px 8px 0px 0px;">
                                    <select onchange='changeDate(this, "ymd")' id='dafm'>
                                        <option value=0><#Date#>:</option>
                                        <option value=0>yyyy-mm-dd</option>
                                        <option value=1>mm-dd-yyyy</option>
                                        <option value=2>mmm dd, yyyy</option>
                                        <option value=3>dd.mm.yyyy</option>
                                    </select>
                                    <select onchange='changeScale(this)' id='scale'>
                                        <option value=0><#Scale#>:</option>
                                        <option value=0>KiB</option>
                                        <option value=1>MiB</option>
                                        <option value=2 selected>GiB</option>
                                        <option value=3>TiB</option>
                                    </select>
                                    <select onchange="switchPage(this.options[this.selectedIndex].value)">
                                        <option><#switchpage#></option>
                                        <option value="1"><#menu4_2_1#></option>
                                        <option value="2"><#menu4_2_2#></option>
                                        <option value="3" selected><#menu4_2_3#></option>
                                    </select>
                                    <div id='bwm-daily-grid'></div>
                                </div>

                                <table width="100%" cellpadding="4" cellspacing="0" class="table table-striped">
                                    <tr>
                                        <td width="40%"><b><#Last30days#> <span id='last-dates'></span></b></td>
                                        <td width="20%" class="dl" id='last-dn'>-</td>
                                        <td width="20%" class="ul" id='last-up'>-</td>
                                        <td width="20%" class="total" id='last-total'>-</td>
                                    </tr>
                                </table>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <div id="footer"></div>
</form>
</div>
</body>
</html>
