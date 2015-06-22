<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu4_2#> : <#menu4_2_1#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/highcharts.js"></script>
<script type="text/javascript" src="/bootstrap/js/highcharts_theme.js"></script>
<script type="text/javascript" src="/bootstrap/js/network_chart_template.js"></script>
<script type="text/javascript" src="/net_chart_tabs.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

var netChart = [];
var speed_history = [];
var netdev_prev = [];
var last_perf = 0;

var cprefix = 'bw_r';
var updateDiv = 1;
var updateAvgLen = 5;
var idReloadNetDev = 0;

Highcharts.setOptions({
	global : {
		useUTC : false
	}
});

$j(document).ready(function(){
	idReloadNetDev = setTimeout(load_netdev, 100);
});

window.performance = window.performance || {};
performance.now = (function() {
	return performance.now ||
	performance.mozNow ||
	performance.msNow ||
	performance.oNow ||
	performance.webkitNow ||
	function() { return new Date().getTime(); };
})();

function eval_netdev(response,now_perf){
	var c, p, h, t, i, j;
	var netdev = {};
	var x = (new Date()).getTime();
	var diff_time = (now_perf - last_perf);

	if (diff_time <= 0)
		diff_time = 1000;

	last_perf = now_perf;

	x = parseInt(x/1000)*1000;

	try {
		eval(response);
	}
	catch (ex) {
		netdev = {};
	}

	for (i in netdev) {
		c = netdev[i];
		p = netdev_prev[i];
		h = speed_history[i];
		if (h === undefined) {
			speed_history[i] = {};
			h = speed_history[i];
			h.rx = [];
			h.tx = [];
			h.rx_avg = 0;
			h.tx_avg = 0;
			h.rx_max = 0;
			h.tx_max = 0;
			for (j = updateAvgLen; j > 0; --j) {
				h.rx.push(0);
				h.tx.push(0);
			}
		}
		if (p !== undefined) {
			var diff_rx, diff_tx;
			
			if (c.rx < p.rx){
				if (p.rx <= 0xFFFFFFFF && p.rx > 0xE0000000)
					diff_rx = (0xFFFFFFFF - p.rx) + c.rx;
				else
					diff_rx = 0;
			} else {
				diff_rx = (c.rx - p.rx);
			}
			diff_rx = (diff_rx * 1000) / diff_time;
			
			h.rx.splice(0, 1);
			h.rx.push(diff_rx);
			if (diff_rx > h.rx_max)
				h.rx_max = diff_rx;
			
			t = 0;
			for (j = (h.rx.length - updateAvgLen); j < h.rx.length; ++j)
				t += h.rx[j];
			h.rx_avg = t / updateAvgLen;
			
			if (c.tx < p.tx){
				if (p.tx <= 0xFFFFFFFF && p.tx > 0xE0000000)
					diff_tx = (0xFFFFFFFF - p.tx) + c.tx;
				else
					diff_tx = 0;
			} else {
					diff_tx = (c.tx - p.tx);
			}
			
			diff_tx = (diff_tx * 1000) / diff_time;
			h.tx.splice(0, 1);
			h.tx.push(diff_tx);
			if (diff_tx > h.tx_max)
				h.tx_max = diff_tx;
			
			t = 0;
			for (j = (h.tx.length - updateAvgLen); j < h.tx.length; ++j)
				t += h.tx[j];
			h.tx_avg = t / updateAvgLen;
			
			if (netChart[i] !== undefined){
				netChart[i].series[0].addPoint([x, diff_rx*8/1000000], false, false);
				netChart[i].series[1].addPoint([x, diff_tx*8/1000000], false, false);
			}
		}
		h.rx_total = c.rx;
		h.tx_total = c.tx;
		netdev_prev[i] = c;
	}

	processTabs();
}

function load_netdev(){
	clearTimeout(idReloadNetDev);
	$j.ajax({
		type: "get",
		url: "/update.cgi",
		data: {
			output: "netdev"
		},
		dataType: "script",
		cache: true,
		error: function(xhr) {
			idReloadNetDev = setTimeout(load_netdev, 1000);
		},
		success: function(response) {
			var now_perf = performance.now();
			idReloadNetDev = setTimeout(load_netdev, 2000);
			eval_netdev(response,now_perf);
		}
	});
}

function initial(){
	show_banner(0);
	show_menu(5, -1, 0);
	show_footer();

	initTabs();
}

function switchPage(page){
	if(page == "2")
		location.href = "/Main_TrafficMonitor_last24.asp";
	else if(page == "3")
		location.href = "/Main_TrafficMonitor_daily.asp";
	return false;
}

function createCharts(arrTabs,ifdesc){
	var chartIdFirst = null;

	for(var i=0; i < arrTabs.length; i++){
		var chartId = arrTabs[i][0].replace('speed-tab-', '');
		if (i == 0)
			chartIdFirst = chartId;
		if (netChart[chartId] !== undefined)
			continue;
		
		var tWidth = $j('#tab-area').parents('.box').width()-20;
		var div  = '<tr id="tr_'+chartId+'" class="charts" style="display: none;">\n';
		    div += '    <td style="text-align: center; padding:10px; width: 99%">\n';
		    div += '        <div height="500px" style="width:'+tWidth+'px" id="chart_'+chartId+'"></div>\n';
		    div += '    </td>\n';
		    div += '</tr>\n';
		
		$j("#network_chart").append(div);
		
		var nc = {}; // new temp object
		
		// clone object network_chart to nc (true - is recursively)
		$j.extend(true, nc, network_chart_template);
		
		// change id of chart
		nc.chart.renderTo = 'chart_'+chartId;
		
		// get name of tab and set title of chart
		var title = $j(E('speed-tab-'+chartId)).text();
		nc.title.text = "<#menu4#>" + ': ' + title;
		
		// create new charts
		netChart[chartId] = new Highcharts.StockChart(nc);
	}

	$j("#tabs a").click(function(){
		tabSelect(this.id);
	});

	if (ifdesc == null)
		ifdesc = chartIdFirst;
	if (ifdesc != null)
		$j("#tr_"+ifdesc).show();
}

function tabSelect(name){
	var ifdesc;

	showTab(name);
	$j('.charts').hide();
	ifdesc = cookie.get(cprefix + 'tab');
	$j("#tr_"+ifdesc).show();
}

function updateChart(ifdesc){
	if (netChart[ifdesc] !== undefined)
		netChart[ifdesc].redraw();
}

</script>
<style>
    #tabs {margin-bottom: 0px;}
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
    <input type="hidden" name="current_page" value="Main_TrafficMonitor_realtime.asp">
    <input type="hidden" name="next_page" value="Main_TrafficMonitor_realtime.asp">
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
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu4#></h2>
                            <div class="round_bottom">
                                <div id="tabMenu"></div>
                                <div id='rstats'></div>
                                <div>
                                    <div align="right" style="margin: 8px 8px 0px 0px;">
                                       <select onchange="switchPage(this.options[this.selectedIndex].value)" class="top-input" style="width: 150px;">
                                            <option><#switchpage#></option>
                                            <option value="1" selected ><#menu4_2_1#></option>
                                            <option value="2"><#menu4_2_2#></option>
                                            <option value="3"><#menu4_2_3#></option>
                                        </select>
                                    </div>

                                    <div id="tab-area" style="margin-bottom: 0px; margin: -36px 8px 0px 8px;"></div>

                                    <center>
                                        <div style="min-height: 420px;"><table width="100%" id="network_chart"></table></div>
                                    </center>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width='8%'><#Network#></th>
                                            <th width='8%'><#Color#></th>
                                            <th width='8%' style="text-align: right"><#Current#></th>
                                            <th width='8%' style="text-align: right"><#Average#></th>
                                            <th width='8%' style="text-align: right"><#Maximum#></th>
                                            <th width='8%' style="text-align: right"><#Total#></th>
                                        </tr>
                                        <tr>
                                            <td width='8%' style="text-align:center; vertical-align: middle;"><#Downlink#></td>
                                            <td width='10%' style="text-align:center; vertical-align: middle;">
                                                <div id='rx-sel' class="span12" style="border-radius: 5px;"></div>
                                            </td>
                                            <td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='rx-current'></span></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='rx-avg'></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='rx-max'></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='rx-total'></td>
                                        </tr>
                                        <tr>
                                        <td width='8%' style="text-align:center; vertical-align: middle;"><#Uplink#></td>
                                            <td width='10%' style="text-align:center; vertical-align: middle;">
                                                <div id='tx-sel' class="span12" style="border-radius: 5px;"></div>
                                            </td>
                                            <td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='tx-current'></span></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='tx-avg'></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='tx-max'></td>
                                            <td width='15%' align='center' valign='top' style="text-align:right" id='tx-total'></td>
                                        </tr>
                                    </table>

                                </div>
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
