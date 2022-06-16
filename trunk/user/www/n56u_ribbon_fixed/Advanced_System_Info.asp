<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#></title>
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
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

var arrCharts = [ null, null ];
var arrHashes = ["cpu", "mem"];

var cpu_chart = {
    chart: {
        renderTo: 'cpu_chart',
        zoomType: 'x',
        spacingRight: 15
    },
    title : {
        text : '<#menu5_8_1#> (%)',
        align: 'left'
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
            text: 'CPU %'
        },
        min: 0,
        max: 100,
        minRange: 1,
        opposite: false,
        startOnTick: false,
        showFirstLabel: false
    },
    plotOptions: {
        series: {
            animation: false
        },
        areaspline: {
            lineWidth: 1
        },
        spline: {
            lineWidth: 1
        },
        area: {
            lineWidth: 1
        },
        line: {
            lineWidth: 1
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
        valueSuffix: '%'
    },
    series: [{
        type: 'areaspline',
        name: 'Busy',
        gapSize: 5,
        threshold: null,
        fillColor: {
            linearGradient: {
                x1: 0,
                y1: 0,
                x2: 0,
                y2: 1
            },
            stops: [[0, Highcharts.getOptions().colors[0]], [1, 'rgba(0,0,0,0)']]
        },
        data: (prepare_array_chart)()
    },{
        type: 'spline',
        name: 'User',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)()
    },{
        type: 'spline',
        name: 'System',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)()
    },{
        type: 'spline',
        name: 'Sirq',
        gapSize: 5,
        threshold: null,
        data: (prepare_array_chart)()
    }]
};

var mem_chart = {
    chart: {
        renderTo: 'mem_chart',
        zoomType: 'x',
        spacingRight: 15
    },
    title : {
        text : '<#menu5_8_2#> (MB)',
        align: 'left'
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
            text: '<#HSTOCK_RAM#>'
        },
        min: 0,
        minRange: 16,
        opposite: false,
        startOnTick: false,
        showFirstLabel: false
    },
    plotOptions: {
        series: {
            animation: false
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
        valueSuffix: ' MB',
        valueDecimals: 2
    },
    series: [{
        type: 'spline',
        name: 'Used',
        gapSize: 5,
        data: (prepare_array_chart)()
    },{
        type: 'spline',
        name: 'Buffers',
        gapSize: 5,
        data: (prepare_array_chart)()
    },{
        type: 'spline',
        name: 'Cached',
        gapSize: 5,
        data: (prepare_array_chart)()
    }]
};

Highcharts.setOptions({
    global : {
        useUTC : false
    },
    lang: {
        months: ['<#MF_Jan#>', '<#MF_Feb#>', '<#MF_Mar#>', '<#MF_Apr#>', '<#MF_May#>', '<#MF_Jun#>', '<#MF_Jul#>', '<#MF_Aug#>', '<#MF_Sep#>', '<#MF_Oct#>', '<#MF_Nov#>', '<#MF_Dec#>'],
        shortMonths: ['<#MS_Jan#>', '<#MS_Feb#>', '<#MS_Mar#>', '<#MS_Apr#>', '<#MS_May#>', '<#MS_Jun#>', '<#MS_Jul#>', '<#MS_Aug#>', '<#MS_Sep#>', '<#MS_Oct#>', '<#MS_Nov#>', '<#MS_Dec#>'],
        weekdays: ['<#WF_Sun#>', '<#WF_Mon#>', '<#WF_Tue#>', '<#WF_Wed#>', '<#WF_Thu#>', '<#WF_Fri#>', '<#WF_Sat#>'],
        rangeSelectorZoom: '<#HSTOCK_Zoom#>'
    }
});

$j(document).ready(function(){
    $j("#tab_cpu_chart, #tab_mem_chart").click(function(){
        var newHash = $j(this).attr('href').toLowerCase();
        showChart(newHash,0);
        return false;
    });
    arrCharts[0] = new Highcharts.StockChart(cpu_chart);
    arrCharts[1] = new Highcharts.StockChart(mem_chart);
});

$j(window).bind('hashchange', function(){
    showChart(getHash(),1);
});

function initial(){
    show_banner(0);
    show_menu(6,-1,0);
    show_footer();

    showChart(getHash(),0);
}

function showChart(curHash,rdw){
    for(var i = 0; i < arrHashes.length; i++){
        if(curHash == ('#'+arrHashes[i])){
            $j('#tab_'+arrHashes[i]+'_chart').parents('li').addClass('active');
            $j('#'+arrHashes[i]+'_chart').show();
            if (rdw)
               arrCharts[i].redraw();
        }else{
            $j('#tab_'+arrHashes[i]+'_chart').parents('li').removeClass('active');
            $j('#'+arrHashes[i]+'_chart').hide();
        }
    }
    window.location.hash = curHash.toUpperCase();
}

function prepare_array_chart(){
    var data = [], x = (new Date()).getTime(), p = -450, i;
    x = parseInt(x/1000)*1000;
    for(i = p; i <= 0; i++)
        data.push([x+i*2000, 0]);
    return data;
}

function getHash(){
    var curHash = window.location.hash.toLowerCase();
    return (curHash != '#cpu' && curHash != '#mem') ? '#cpu' : curHash;
}

function bytesToMegabytes(bytes, precision){
    var kilobyte = 1024;
    var megabyte = kilobyte * 1024;
    return parseFloat((bytes / megabyte).toFixed(precision));
}

function getSystemJsonData(cpu,ram){
    if(typeof(cpu) !== 'object' || typeof(ram) !== 'object')
        return;
    var x = (new Date()).getTime();
    x = parseInt(x/1000)*1000;

    arrCharts[0].series[0].addPoint([x, parseInt(cpu.busy)], false, false);
    arrCharts[0].series[1].addPoint([x, parseInt(cpu.user)], false, false);
    arrCharts[0].series[2].addPoint([x, parseInt(cpu.system)], false, false);
    arrCharts[0].series[3].addPoint([x, parseInt(cpu.sirq)], false, false);

    arrCharts[1].yAxis[0].setExtremes(0, bytesToMegabytes(ram.total*1024, 2), false);
    arrCharts[1].series[0].addPoint([x, bytesToMegabytes(ram.used*1024, 2)], false, false);
    arrCharts[1].series[1].addPoint([x, bytesToMegabytes(ram.buffers*1024, 2)], false, false);
    arrCharts[1].series[2].addPoint([x, bytesToMegabytes(ram.cached*1024, 2)], false, false);

    if ($('cpu_chart').style.display == 'none')
        arrCharts[1].redraw();
    else
        arrCharts[0].redraw();
}
</script>
<style>
    #tabs {margin-bottom: 0px;}
</style>
</head>
<body onload="initial();">

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
                            <h2 class="box_head round_top"><#menu5_8#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <div id="tab-area" style="margin-bottom: 0px;">
                                        <ul id="tabs" class="nav nav-tabs">
                                            <li><a href="#CPU" id="tab_cpu_chart"><#menu5_8_1#></a></li>
                                            <li><a href="#MEM" id="tab_mem_chart"><#menu5_8_2#></a></li>
                                        </ul>
                                    </div>

                                    <center>
                                        <table style="width: 100%; margin-top: 6px; margin-bottom: 6px;">
                                            <tr>
                                                <td width="100%" align="center" style="text-align: center">
                                                    <div id="cpu_chart" style="width: 670px; padding-left: 5px;"></div>
                                                    <div id="mem_chart" style="width: 670px; padding-left: 5px;"></div>
                                                </td>
                                            </tr>
                                        </table>
                                    </center>

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
