<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu4_2#> : <#menu4_2_2#></title>
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
<% bandwidth("speed", ""); %>
var netChart;
var cprefix = "bw_24_";

var net_chart_24 = {
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
        minRange: 300*1000,
        title: {
            text: null
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
            type: 'hour',
            text: '1H'
        },{
            count: 3,
            type: 'hour',
            text: '3H'
        },{
            count: 6,
            type: 'hour',
            text: '6H'
        },{
            count: 12,
            type: 'hour',
            text: '12H'
        },{
            type: 'all',
            text: 'All'
        }],
        inputEnabled: false,
        selected: 4
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
        data: (prepare_array_chart)(0)
    },{
        type: 'areaspline',
        name: '<#Uplink#>',
        color: '#003EBA',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)(1)
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
	netChart = new Highcharts.StockChart(net_chart_24);
});

function initial(){
	show_banner(0);
	show_menu(5, -1, 0);
	show_footer();

	initTab();
	processTabs();
	updateTab(speed_history[netdev]);
	setChartTitle(netdev);

	invoke_timer(poll_next);
}

function getNextTime(){
	var x = (new Date()).getTime();
	return parseInt(x/1000)*1000-(data_period-poll_next)*1000;
}

function redraw_speed(){
	var x = getNextTime();

	for(var i in speed_history){
		var h = speed_history[i];
		if ((typeof(h.rx) === 'undefined') || (typeof(h.tx) === 'undefined'))
			continue;
		
		netChart.series[0].setData(prepareData(x, h.rx), false);
		netChart.series[1].setData(prepareData(x, h.tx), false);
		
		if (netdev !== i){
			netdev = i;
			setChartTitle(i);
			E('sel_netif').value = 'speed-tab-' + i;
		}
		
		break;
	}

	processTabs();
	updateTab(speed_history[netdev]);

	invoke_timer(poll_next);

	netChart.redraw();
}

function eval_netdevs(response){
	speed_history = {};

	try {
		eval(response);
	}
	catch (ex) {
		speed_history = {};
	}

	redraw_speed();
}

function load_netdevs(){
	clearTimeout(idTimerPoll);
	$j.ajax({
		type: "get",
		url: "/update.cgi",
		data: {
			output: "bandwidth",
			arg0: "speed",
			arg1: netdev
		},
		dataType: "script",
		cache: true,
		error: function(xhr){
			invoke_timer(5);
		},
		success: function(response){
			eval_netdevs(response);
		}
	});
}

function prepareData(x,data){
	var newData = [];
	var i, j = 0, p = data_period;
	for(i=(data.length-1); i >= 0; i--)
		newData.unshift([(x - (j++) * p * 1000), (data[i]*8/1000000)]);
	return newData;
}

function prepare_array_chart(id){
	var x = getNextTime();
	var h = speed_history[netdev];
	if (h !== undefined){
		if(id==1){
			if (typeof(h.tx) !== 'undefined')
				return prepareData(x, h.tx);
		}else{
			if (typeof(h.rx) !== 'undefined')
				return prepareData(x, h.rx);
		}
	}

	var data = [], p = 1-parseInt(86400/data_period);
	for(i = p; i <= 0; i++)
		data.push([x+i*data_period*1000, 0]);
	return data;
}

function setChartTitle(ifdesc){
	var title = getTabDesc(ifdesc);
	if (title)
		netChart.setTitle({text: title}, null, false);
}

function setChartData(ifdesc){
	if (!ifdesc)
		return false;
	if(netdev != ifdesc){
		netdev = ifdesc;
		setChartTitle(ifdesc);
		load_netdevs();
	}
}

function handleTabs(arrTabs){
	var o = E('sel_netif');
	var tabName = 'speed-tab-' + netdev;
	free_options(o);
	for(var i=0; i<arrTabs.length; i++)
		add_option(o, arrTabs[i][1], arrTabs[i][0], arrTabs[i][0] === tabName);
}

function tabSelect(tabName){
	var ifdesc = tabName.replace('speed-tab-', '');
	setChartData(ifdesc);
}

function switchPage(id){
	if(id == "tab_bw_rt")
		location.href = "/Main_TrafficMonitor_realtime.asp";
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
                            <h2 class="box_head round_top"><#menu4#> - <#menu4_2_2#></h2>
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
                                            <li class="active"><a href="javascript:void(0)" id="tab_bw_24"><#menu4_2_2#></a></li>
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
