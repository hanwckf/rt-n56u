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
<script type="text/javascript" src="/bootstrap/js/highstock.js"></script>
<script type="text/javascript" src="/bootstrap/js/highchart_theme.js"></script>
<script type="text/javascript" src="/bootstrap/js/network_chart_template.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/tmmenu.js"></script>
<script type="text/javascript" src="/tmcal.js"></script>
<script>
var $j = jQuery.noConflict();

<% nvram("rstats_enable,rstats_colors"); %>

speed_history = {};

var cprefix = 'bw_24';
var updateInt = 60;
var updateDiv = updateInt;
var updateMaxL = 1440;
var updateReTotal = 1;
var hours = 24;
var lastHours = 0;
var debugTime = 0;

var ref = new TomatoRefresh('update.cgi', 'output=bandwidth&arg0=speed');

ref.stop = function() {
	this.timer.start(1000);
}

ref.refresh = function(text)
{
	++updating;
	try {
		this.refreshTime = 1500;
		speed_history = {};
		try {
			eval(text);
			this.refreshTime = (fixInt(speed_history._next, 1, 120, 60) + 2) * 1000;
		}
		catch (ex) {
			speed_history = {};
		}
		if (debugTime) E('dtime').innerHTML = (new Date()) + ' ' + (this.refreshTime / 1000);
		loadData();
	}
	catch (ex) {
//		alert('ex=' + ex);
	}
	--updating;
}


ref.toggleX = function()
{
    //this.toggle();
    //this.showState();
	cookie.set(cprefix + 'refresh', this.running ? 1 : 0);
}

ref.initX = function()
{
	var a;

	a = fixInt(cookie.get(cprefix + 'refresh'), 0, 1, 1);
	if (a) {
		ref.refreshTime = 100;
		ref.toggleX();
	}
}

function initB()
{
	if (nvram.rstats_enable != '1') return;

	initCommon(1, 0, 0, 1);	   //Viz 2010.09
	ref.start();
	ref.initX();
}

function switchPage(page){
	if(page == "1")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(page == "3")
		location.href = "/Main_TrafficMonitor_daily.asp";
	else
		return false;
}

function prepareData(data)
{
    var newData = [];
    var j=0;
    for(var i=(data.length-1); i >= 0; i--)
    {
        var time = parseInt((new Date()).getTime()/1000)*1000;
        newData.unshift([
                            (time - (j++) * updateInt * 1000),
                            bytesToKilobytes(data[i]/updateInt, 2)
                         ]);
    }

    return newData;
}


function createCharts(arrTabs)
{
    for(var i=0; i < arrTabs.length; i++)
    {
        var chartId =  arrTabs[i][0].substring(10, arrTabs[i][0].length);
            chartId = chartId.replace('.', '__'); // replace . (dot) -> __ (uderline two times)

        var tWidth = $j('#tab-area').parents('.box').width()-20;

        // create dom element <tr> for chart
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
        nc.chart.renderTo =  'chart_'+chartId;

        var title = $j(E('speed-tab-'+chartId)).text();
        nc.title.text = "<#menu4#>" + ': ' + title;

        var cID = chartId.replace('__', '.');
        nc.series[0].data = prepareData(speed_history[cID].rx);
        nc.series[1].data = prepareData(speed_history[cID].tx);
        nc.rangeSelector  = {
                                buttons: [{
                                    count: 3*60,
                                    type: 'minute',
                                    text: '3H'
                                }, {
                                    count: 6*60,
                                    type: 'minute',
                                    text: '6H'
                                }, {
                                    count: 12*60,
                                    type: 'minute',
                                    text: '12H'
                                },{
                                    type: 'all',
                                    text: 'All'
                                }],
                                inputEnabled: false,
                                selected: 3
                            };

        // create new charts
        netChart[chartId] = new Highcharts.StockChart(nc);

    }

    var enabledChartId = cookie.get(cprefix + 'tab');
    $j("#tr_"+enabledChartId.replace('.', '__')).show();
}

function bytesToKilobytes(bytes, precision)
{
    return parseFloat((bytes/1024).toFixed(precision));
}

function tabSelect(name)
{
    if (!updating)
    {
        showTab(name);

        $j('.charts').hide();

        var enabledChartId = cookie.get(cprefix + 'tab');
            enabledChartId = enabledChartId.replace('.', '__');
        $j("#tr_"+enabledChartId).show();
    }
}

var netChart = {};

Highcharts.setOptions({
    global : {
        useUTC : false
    }
});

$j(document).ready(function() {
    // fix need to get all enabled ifaces
    var idFindTabs = setInterval(function(){
        if(tabs.length > 0)
        {
            clearInterval(idFindTabs);
            createCharts(tabs);

            $j("#tabs a").click(function(){
                tabSelect(this.id);
            });
        }
    }, 100);


});
</script>
<style>
    #tabs {margin-bottom: 0px;}
</style>
</head>

<body onload="show_banner(0); show_menu(5, -1, 0); show_footer(); initB();" >

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
    <input type="hidden" name="current_page" value="Main_TrafficMonitor_last24.asp">
    <input type="hidden" name="next_page" value="Main_TrafficMonitor_last24.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="zoom" value="3">

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
                                            <option value="1"><#menu4_2_1#></option>
                                            <option value="2" selected><#menu4_2_2#></option>
                                            <option value="3"><#menu4_2_3#></option>
                                        </select>
                                    </div>

                                    <div id="tab-area" style="margin-bottom: 0px; margin: -36px 8px 0px 8px;"></div>

                                    <center>
                                        <div style="min-height: 420px;"><table width="100%" id="network_chart"></table></div>
                                    </center>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <td colspan="2">
                                                <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                                    <tr>
                                                        <th width='8%' style="border-top: 0 none;"><#Network#></th>
                                                        <th width='8%' style="border-top: 0 none;"><#Color#></th>
                                                        <th width='8%' style="border-top: 0 none; text-align: right"><#Current#></th>
                                                        <th width='8%' style="border-top: 0 none; text-align: right"><#Average#></th>
                                                        <th width='8%' style="border-top: 0 none; text-align: right"><#Maximum#></th>
                                                        <th width='8%' style="border-top: 0 none; text-align: right"><#Total#></th>
                                                    </tr>
                                                    <tr>
                                                        <td width='8%' style="text-align:center; vertical-align: middle;"><#Downlink#></td>
                                                        <td width='10%' style="text-align:center; vertical-align: middle;">
                                                            <div id='rx-sel' class="span12" style="border-radius: 5px;">
                                                                <ul id="navigation-1"><b style='border-bottom: 4px solid; display: none;' id='rx-name'></b>
                                                                   <li>
                                                                     <!-- @todo <a title="Color" style="color:#B7E1F7;"><i class="icon icon-chevron-up"></i></a>
                                                                      <ul class="navigation-2">
                                                                         <li><a title="Orange" style="background-color:#FF9000;" onclick="switchColorRX(0)"></a></li>
                                                                         <li><a title="Blue" style="background-color:#003EBA;" onclick="switchColorRX(1)"></a></li>
                                                                         <li><a title="Black" style="background-color:#000000;" onclick="switchColorRX(2)"></a></li>
                                                                         <li><a title="Red" style="background-color:#dd0000;" onclick="switchColorRX(3)"></a></li>
                                                                         <li><a title="Gray" style="background-color:#999999;" onclick="switchColorRX(4)"></a></li>
                                                                         <li><a title="Green" style="background-color:#118811;"onclick="switchColorRX(5)"></a></li>
                                                                      </ul> -->
                                                                   </li>
                                                                </ul>
                                                            </div>
                                                        </td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='rx-current'></span></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='rx-avg'></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='rx-max'></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='rx-total'></td>
                                                    </tr>
                                                    <tr>
                                                        <td width='8%' style="text-align:center; vertical-align: middle;"><#Uplink#></td>
                                                        <td width='10%' style="text-align:center; vertical-align: middle;">
                                                            <div id='tx-sel' class="span12" style="border-radius: 5px;">
                                                                <ul id="navigation-1"><b style='border-bottom: 4px solid; display: none;' id='tx-name'></b>
                                                                   <li>
                                                                      <!-- @todo <a  title="Color" style="color:#B7E1F7;"><i class="icon icon-chevron-up"></i></a>
                                                                      <ul class="navigation-2">
                                                                         <li><a title="Orange" style="background-color:#FF9000;" onclick="switchColorTX(0)"></a></li>
                                                                         <li><a title="Blue" style="background-color:#003EBA;" onclick="switchColorTX(1)"></a></li>
                                                                         <li><a title="Black" style="background-color:#000000;" onclick="switchColorTX(2)"></a></li>
                                                                         <li><a title="Red" style="background-color:#dd0000;" onclick="switchColorTX(3)"></a></li>
                                                                         <li><a title="Gray" style="background-color:#999999;" onclick="switchColorTX(4)"></a></li>
                                                                         <li><a title="Green" style="background-color:#118811;"onclick="switchColorTX(5)"></a></li>
                                                                      </ul> -->
                                                                   </li>
                                                                </ul>
                                                            </div>
                                                        </td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='tx-current'></span></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='tx-avg'></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='tx-max'></td>
                                                        <td width='15%' align='center' valign='top' style="text-align:right" id='tx-total'></td>
                                                    </tr>
                                                 </table>
                                            </td>
                                        </tr>
                                    </table>

                                    <div style="display: none;">
                                        <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                            <thead>
                                                <tr>
                                                    <td colspan="5" id="TriggerList">Display Options</td>
                                                </tr>
                                            </thead>

                                            <div id='bwm-controls'>
                                                <tr>
                                                    <th width='50%'><#Traffic_Hours#>:</th>
                                                        <td>
                                                                <a href='javascript:switchHours(4);' id='hr4'>4</a>,
                                                                <a href='javascript:switchHours(6);' id='hr6'>6</a>,
                                                                <a href='javascript:switchHours(12);' id='hr12'>12</a>,
                                                                <a href='javascript:switchHours(18);' id='hr18'>18</a>,
                                                                <a href='javascript:switchHours(24);' id='hr24'>24</a>
                                                        </td>
                                                    </tr>
                                                <tr>
                                                    <th width='50%'><#Traffic_Avg#>:</th>
                                                    <td>
                                                        <a href='javascript:switchAvg(1)' id='avg1'>Off</a>,
                                                        <a href='javascript:switchAvg(2)' id='avg2'>2x</a>,
                                                        <a href='javascript:switchAvg(4)' id='avg4'>4x</a>,
                                                        <a href='javascript:switchAvg(6)' id='avg6'>6x</a>,
                                                        <a href='javascript:switchAvg(8)' id='avg8'>8x</a>
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <th><#Traffic_Max#></th>
                                                    <td>
                                                        <a href='javascript:switchScale(0)' id='scale0'>Uniform</a>,
                                                        <a href='javascript:switchScale(1)' id='scale1'>Per IF</a>
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <th><#Traffic_SvgDisp#>:</th>
                                                    <td>
                                                            <a href='javascript:switchDraw(0)' id='draw0'>Solid</a>,
                                                            <a href='javascript:switchDraw(1)' id='draw1'>Line</a>
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <th><#Traffic_Color#>:</th>
                                                    <td>
                                                            <a href='javascript:switchColor()' id='drawcolor'>-</a><a href='javascript:switchColor(1)' id='drawrev'><#Traffic_Reverse#></a>
                                                    </td>
                                                </tr>
                                            </div>
                                        </table>
                                    </div>
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


