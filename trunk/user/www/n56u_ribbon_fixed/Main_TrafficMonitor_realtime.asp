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
<script type="text/javascript" src="/net_speed_tabs.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();
<% netdev(); %>
var netdevs_prev = {};

var netChart;
var speed_rx = {};
var speed_tx = {};
var speed_history = {};
var netdev = "LAN";
var last_perf = 0;
var last_netdev = "";

var cprefix = "bw_rt_";
var updateAvgLen = 5;

var net_chart_rt = {
    chart: {
        renderTo: 'net_chart',
        zoomType: 'x',
        spacingRight: 15
    },
    title : {
        text: 'Wired: LAN',
        align: 'center'
    },
    xAxis: {
        type: 'datetime',
        minRange: 10*1000,
        title: {
            text: null
        },
        labels: {
            format: '{value:%H:%M:%S}'
        }
    },
    yAxis: {
        title: {
            text: '<#HSTOCK_Bandwidth#>'
        },
        min: 0,
        minRange: 0.04,
        opposite: false,
        startOnTick: false,
        showFirstLabel: false
    },
    plotOptions: {
        series: {
            animation: false
        },
        areaspline: {
            lineWidth: 1,
            fillOpacity: 0.3
        }
    },
    legend: {
        enabled: true,
        verticalAlign: 'top',
        floating: true,
        align: 'right'
    },
    rangeSelector: {
        buttons: [{
            count: 1,
            type: 'minute',
            text: '1M'
        },{
            count: 5,
            type: 'minute',
            text: '5M'
        },{
            count: 15,
            type: 'minute',
            text: '15M'
        },{
            type: 'all',
            text: 'All'
        }],
        inputEnabled: false,
        selected: 1
    },
    tooltip:{
        xDateFormat: '%H:%M:%S',
        valueSuffix: ' Mbps',
        valueDecimals: 2
    },
    series: [{
        type: 'areaspline',
        name: '<#Downlink#>',
        color: '#FF9000',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)()
    },{
        type: 'areaspline',
        name: '<#Uplink#>',
        color: '#003EBA',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)()
    }]
};

Highcharts.setOptions(Highcharts.locale);

$j(document).ready(function(){
	$j("#tabs a").click(function(){
		switchPage(this.id);
		return false;
	});
	if(get_ap_mode()){
		$j("#tab_tr_dy").parents('li').hide();
		$j("#tab_tr_mo").parents('li').hide();
	}
	netChart = new Highcharts.StockChart(net_chart_rt);
	invoke_timer(2);
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

function initial(){
	show_banner(0);
	show_menu(5, -1, 0);
	show_footer();

	var ifdesc = cookie.get(cprefix + 'tab');
	if (ifdesc)
		netdev = ifdesc;

	initTab();
	calc_speed(0);
}

function calc_speed(now_perf){
	var c, p, h, t, i, j, ifdesc, ifdesc_1st;
	var x = (new Date()).getTime();
	var diff_time = (now_perf - last_perf);

	if (diff_time <= 0)
		diff_time = 1000;

	last_perf = now_perf;
	x = parseInt(x/1000)*1000;

	ifdesc = null;
	ifdesc_1st = null;

	for(i in netdevs){
		c = netdevs[i];
		p = netdevs_prev[i];
		h = speed_history[i];
		if(speed_rx[i] === undefined)
			speed_rx[i] = prepare_array(x);
		if(speed_tx[i] === undefined)
			speed_tx[i] = prepare_array(x);
		if(h === undefined){
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
		if(p !== undefined){
			var diff_rx = 0, diff_tx = 0;
			
			if(p.id != c.id) {
				p.id = c.id;
				p.rx = 0;
				p.tx = 0;
			}
			
			if(c.rx < p.rx){
				if (p.rx <= 0xFFFFFFFF && p.rx > 0xE0000000)
					diff_rx = (0xFFFFFFFF - p.rx) + c.rx;
			} else {
				diff_rx = (c.rx - p.rx);
			}
			diff_rx = diff_rx * 1000 / diff_time;
			
			h.rx.splice(0, 1);
			h.rx.push(diff_rx);
			if (diff_rx > h.rx_max)
				h.rx_max = diff_rx;
			
			t = 0;
			for (j = (h.rx.length - updateAvgLen); j < h.rx.length; ++j)
				t += h.rx[j];
			h.rx_avg = t / updateAvgLen;
			
			if(c.tx < p.tx){
				if (p.tx <= 0xFFFFFFFF && p.tx > 0xE0000000)
					diff_tx = (0xFFFFFFFF - p.tx) + c.tx;
			} else {
				diff_tx = (c.tx - p.tx);
			}
			
			diff_tx = diff_tx * 1000 / diff_time;
			h.tx.splice(0, 1);
			h.tx.push(diff_tx);
			if (diff_tx > h.tx_max)
				h.tx_max = diff_tx;
			
			t = 0;
			for (j = (h.tx.length - updateAvgLen); j < h.tx.length; ++j)
				t += h.tx[j];
			h.tx_avg = t / updateAvgLen;
			
			diff_rx = diff_rx*8/1000000;
			diff_tx = diff_tx*8/1000000;
			
			speed_rx[i].push([x, diff_rx]);
			speed_tx[i].push([x, diff_tx]);
			
			if(i === last_netdev){
				netChart.series[0].addPoint([x, diff_rx], false, false);
				netChart.series[1].addPoint([x, diff_tx], false, false);
			}
		}
		h.rx_total = c.rx;
		h.tx_total = c.tx;
		netdevs_prev[i] = c;
		if (!ifdesc_1st)
			ifdesc_1st = i;
		if (!ifdesc && i === netdev)
			ifdesc = i;
	}

	if (!ifdesc && ifdesc_1st){
		ifdesc = ifdesc_1st;
		setChartData(ifdesc);
		E('sel_netif').value = 'speed-tab-' + ifdesc;
	}

	processTabs();
	updateTab(speed_history[netdev]);
}

function eval_netdevs(response){
	var now_perf = performance.now();

	netdevs = {};

	try {
		eval(response);
	}
	catch (ex) {
		netdevs = {};
	}

	calc_speed(now_perf);
	netChart.redraw();
}

function prepare_array(x){
	var data = [], p = -450;
	for (var i = p; i <= 0; i++)
		data.push([x+i*2000, 0]);
	return data;
}

function prepare_array_chart(){
	var x = (new Date()).getTime();
	x = parseInt(x/1000)*1000;
	return prepare_array(x);
}

function load_netdevs(){
	clearTimeout(idTimerPoll);
	$j.ajax({
		type: "get",
		url: "/update.cgi",
		data: {
			output: "netdev"
		},
		dataType: "script",
		cache: true,
		error: function(xhr){
			invoke_timer(2);
		},
		success: function(response){
			invoke_timer(2);
			eval_netdevs(response);
		}
	});
}

function prepareData(data){
	var newData = [];
	for(var i=0; i<data.length; i++)
		newData.push(data[i]);
	return newData;
}

function setChartTitle(ifdesc){
	var title = getTabDesc(ifdesc);
	if (title)
		netChart.setTitle({text: title}, null, false);
}

function setChartData(ifdesc){
	if (!ifdesc)
		return false;
	if (netdev != ifdesc){
		netdev = ifdesc;
		cookie.set(cprefix + 'tab', ifdesc, 14);
	}
	if (last_netdev != ifdesc && speed_rx[ifdesc] !== undefined && speed_tx[ifdesc] !== undefined){
		if (last_netdev != ""){
			netChart.series[0].setData(prepareData(speed_rx[ifdesc]), false);
			netChart.series[1].setData(prepareData(speed_tx[ifdesc]), false);
		}
		last_netdev = ifdesc;
		setChartTitle(ifdesc);
	}
}

function handleTabs(arrTabs){
	var o = E('sel_netif');
	var ifdesc = netdev;
	var tabName = 'speed-tab-' + ifdesc;
	free_options(o);
	for(var i=0; i<arrTabs.length; i++)
		add_option(o, arrTabs[i][1], arrTabs[i][0], arrTabs[i][0] === tabName);
	setChartData(ifdesc);
}

function tabSelect(tabName){
	var ifdesc = tabName.replace('speed-tab-', '');
	setChartData(ifdesc);
	updateTab(speed_history[ifdesc]);

	netChart.redraw();
}

function switchPage(id){
	if(id == "tab_bw_24")
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
.table-stat td {padding: 4px 8px;}
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
                            <h2 class="box_head round_top"><#menu4#> - <#menu4_2_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <div align="right" style="margin: 3px 8px 0px 0px;">
                                        <select id="sel_netif" style="width: 200px; margin-bottom: 5px;" onchange="tabSelect(this.value);">
                                        </select>
                                    </div>

                                    <div style="margin-bottom: 0px; margin: -36px 0px 0px 0px;">
                                        <ul id="tabs" class="nav nav-tabs">
                                            <li class="active"><a href="javascript:void(0)" id="tab_bw_rt"><#menu4_2_1#></a></li>
                                            <li><a href="javascript:void(0)" id="tab_bw_24"><#menu4_2_2#></a></li>
                                            <li><a href="javascript:void(0)" id="tab_tr_dy"><#menu4_2_3#></a></li>
                                            <li><a href="javascript:void(0)" id="tab_tr_mo"><#menu4_2_4#></a></li>
                                        </ul>
                                    </div>

                                    <center>
                                        <table style="width: 100%; margin-top: 6px; margin-bottom: 6px;">
                                            <tr>
                                                <td width="100%" align="center" style="text-align: center">
                                                    <div id="net_chart" style="width: 670px; padding-left: 5px;"></div>
                                                </td>
                                            </tr>
                                        </table>
                                    </center>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table table-stat">
                                        <tr>
                                            <th width="9%"><#Color#></th>
                                            <th width="11%"><#Network#></th>
                                            <th width="20%" style="text-align: right"><#Current#></th>
                                            <th width="20%" style="text-align: right"><#Average#></th>
                                            <th width="20%" style="text-align: right"><#Maximum#></th>
                                            <th width="20%" style="text-align: right"><#Total#></th>
                                        </tr>
                                        <tr>
                                            <td width="9%" style="text-align:center; vertical-align: middle;">
                                                <div id="rx-sel" class="span12" style="border-radius: 5px;"></div>
                                            </td>
                                            <td width="11%"><#Downlink#></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right;font-weight: bold;"><span id="rx-current"></span></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id="rx-avg"></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id="rx-max"></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id="rx-total"></td>
                                        </tr>
                                        <tr>
                                            <td width="9%" style="text-align:center; vertical-align: middle;">
                                                <div id="tx-sel" class="span12" style="border-radius: 5px;"></div>
                                            </td>
                                            <td width="11%"><#Uplink#></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right;font-weight: bold;"><span id="tx-current"></span></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id='tx-avg'></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id='tx-max'></td>
                                            <td width="20%" align="center" valign="top" style="text-align:right" id='tx-total'></td>
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
</div>

</body>
</html>
