<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router</title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/bootstrap/js/highstock.js"></script>
<script language="JavaScript" type="text/javascript" src="/bootstrap/js/highchart_theme.js"></script>
<script language="JavaScript" type="text/javascript" src="/bootstrap/js/mem_chart.js"></script>

<script>
    function initial(){
    	show_banner(2);
        show_menu(6,8,2);
        show_footer();
    }
</script>


<script>
var $j = jQuery.noConflict();

var chartMem;

Highcharts.setOptions({
    global : {
        useUTC : false
    }
});

$j(document).ready(function() {
	chartMem = new Highcharts.StockChart(mem_chart);
});

function bytesToMegabytes(bytes, precision)
{
    var kilobyte = 1024;
    var megabyte = kilobyte * 1024;

    return parseFloat((bytes / megabyte).toFixed(precision));
}

function getSystemJsonData(jsonData)
{
    var MEMORY = {};
    chartMem.yAxis[0].setExtremes(0, bytesToMegabytes(jsonData.ram.total, 2), true, true);

    MEMORY.used = chartMem.series[0];
    MEMORY.used.addPoint([MEMORY.used.data[MEMORY.used.data.length-1].x + 2000, bytesToMegabytes(jsonData.ram.used, 2)], true);

    MEMORY.buffer = chartMem.series[1];
    MEMORY.buffer.addPoint([MEMORY.buffer.data[MEMORY.buffer.data.length-1].x + 2000, bytesToMegabytes(jsonData.ram.buffer, 2)], true);

    MEMORY.shared = chartMem.series[2];
    MEMORY.shared.addPoint([MEMORY.shared.data[MEMORY.shared.data.length-1].x + 2000, bytesToMegabytes(jsonData.ram.shared, 2)], true);
}
</script>

<body onload="initial();">


    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span2"><center><div id="logo"></div></center></div>
            <div class="span10" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">

    <div class="container-fluid">
        <div class="row-fluid">
            <div class="span2">
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

            <div class="span10">
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu5_8#> - <#menu5_8_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <center>
                                        <table style="width: 97%; margin-top: 10px; margin-bottom: 30px;">
                                            <tr>
                                                <td>
                                                    <div id="memory_chart" style="width: 100%;">&nbsp;</div>
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

    <!--==============Beginning of hint content=============-->
    <div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
        <form name="hint_form"></form>
        <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

        <div id="hintofPM" style="display:none;">
            <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
            <thead>
                <tr>
                    <td>
                        <div id="helpname" class="AiHintTitle"></div>
                        <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
                    </td>
                </tr>
            </thead>

                <tr>
                    <td valign="top" >
                        <div class="hint_body2" id="hint_body"></div>
                        <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                    </td>
                </tr>
            </table>
        </div>
    </div>
    <!--==============Ending of hint content=============-->

<div id="footer"></div>
</body>
</html>