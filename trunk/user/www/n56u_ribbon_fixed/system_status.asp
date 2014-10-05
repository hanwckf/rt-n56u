<!DOCTYPE html>
<html>
<head>
<title>System Information</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<style>
#cpu_info, #mem_info, .adv_info{text-decoration: none; border-bottom: 1px dotted #005580;}
#cpu_info:hover, #mem_info:hover, .adv_info:hover{cursor: pointer; text-decoration: none;}
</style>

<script type="text/javascript" src="/jquery.js"></script>
<script>

var idRefresh;
var timeToRefresh = 2;

function bytesToSize(bytes, precision)
{
	var kilobyte = 1024;
	var megabyte = kilobyte * 1024;
	var gigabyte = megabyte * 1024;
	var terabyte = gigabyte * 1024;

	if ((bytes >= 0) && (bytes < kilobyte))
		return bytes + ' B';
	else if ((bytes >= kilobyte) && (bytes < megabyte))
		return (bytes / kilobyte).toFixed(precision) + ' KB';
	else if ((bytes >= megabyte) && (bytes < gigabyte))
		return (bytes / megabyte).toFixed(precision) + ' MB';
	else if ((bytes >= gigabyte) && (bytes < terabyte))
		return (bytes / gigabyte).toFixed(precision) + ' GB';
	else if (bytes >= terabyte)
		return (bytes / terabyte).toFixed(precision) + ' TB';
	else
		return bytes + ' B';
}

function getLALabelStatus(num)
{
	var la = parseFloat(num);
	return la > 0.9 ? 'danger' : (la > 0.5 ? 'warning' : 'info');
}

function getSystemInfo()
{
	clearTimeout(idRefresh);
	$.getJSON('/system_status_data.asp', function(data){
		setSystemInfo(data);
		
		if(typeof parent.getRadioBandStatus === 'function' && typeof data.wifi2 != 'undefined'){
			var objWifi = {wifi2: data.wifi2, wifi5: data.wifi5};
			parent.getRadioBandStatus(objWifi);
		}
		
		if(typeof parent.setLogStamp === 'function' && typeof data.logmt != 'undefined')
			parent.setLogStamp(data.logmt);
		
		if(typeof parent.getSystemJsonData === 'function')
			parent.getSystemJsonData(data);
	});
}

function setSystemInfo(jsonData)
{
	if(typeof jsonData === 'object'){
		var lavg = jsonData.lavg;
		var uptime = jsonData.uptime;
		var ram = jsonData.ram;
		var swap = jsonData.swap;
		var cpu = jsonData.cpu;
		var arrLA = lavg.split(' ');

		uptime.hours = uptime.hours < 10 ? ('0'+uptime.hours) : uptime.hours;
		uptime.minutes = uptime.minutes < 10 ? ('0'+uptime.minutes) : uptime.minutes;

		$("#la_info").html('<span class="label label-'+getLALabelStatus(arrLA[0])+'">'+arrLA[0]+'</span>&nbsp;<span class="label label-'+getLALabelStatus(arrLA[1])+'">'+arrLA[1]+'</span>&nbsp;<span class="label label-'+getLALabelStatus(arrLA[2])+'">'+arrLA[2]+'</span>');
		$("#cpu_info").html(cpu.busy + '%');
		$("#mem_info").html(bytesToSize(ram.free*1024, 2) + " / " + bytesToSize(ram.total*1024, 2));
		$("#uptime_info").html(uptime.days + "<#Day#>".substring(0,1) + " " + uptime.hours+"<#Hour#>".substring(0,1) + " " + uptime.minutes+"<#Minute#>".substring(0,1));

		$("#cpu_usage tr:nth-child(1) td:first").html('busy: '+cpu.busy+'%');
		$("#cpu_usage tr:nth-child(2) td:first").html('user: '+cpu.user+'%');
		$("#cpu_usage tr:nth-child(2) td:last").html('system: '+cpu.system+'%');
		$("#cpu_usage tr:nth-child(3) td:first").html('sirq: '+cpu.sirq+'%');
		$("#cpu_usage tr:nth-child(3) td:last").html('irq: '+cpu.irq+'%');
		$("#cpu_usage tr:nth-child(4) td:first").html('idle: '+cpu.idle+'%');
		$("#cpu_usage tr:nth-child(4) td:last").html('nice: '+cpu.nice+'%');

		$("#mem_usage tr:nth-child(1) td:first").html('total: '+bytesToSize(ram.total*1024, 2));
		$("#mem_usage tr:nth-child(2) td:first").html('free: '+bytesToSize(ram.free*1024, 2));
		$("#mem_usage tr:nth-child(2) td:last").html('used: '+bytesToSize(ram.used*1024, 2));
		$("#mem_usage tr:nth-child(3) td:first").html('cached: '+bytesToSize(ram.cached*1024, 2));
		$("#mem_usage tr:nth-child(3) td:last").html('buffers: '+bytesToSize(ram.buffers*1024, 2));
		$("#mem_usage tr:nth-child(4) td:first").html('swap: '+bytesToSize(swap.total*1024, 2));
		$("#mem_usage tr:nth-child(4) td:last").html('swap used: '+bytesToSize(swap.used*1024, 2));
	}

	idRefresh = setTimeout('getSystemInfo()', timeToRefresh*1000);
}

$(document).ready(function() {
	getSystemInfo();

	$("#cpu_info").click(function(){
		$("#main_info").hide();
		$("#cpu_usage").show();
	});

	$("#mem_info").click(function(){
		$("#main_info").hide();
		$("#mem_usage").show();
	});
});

</script>

</head>
<body class="body_iframe" style="background-color: transparent">
    <div id="main_info">
        <table class="table table-condensed" width="100%" style="margin-bottom: 0px;">
            <tbody>
                <tr>
                    <td width="43%" style="border: 0 none;"><#SI_LoadAvg#></td>
                    <td style="border: 0 none; min-width: 115px;">
                        <div id="la_info"> -- -- -- </div>
                    </td>
                </tr>
                <tr>
                    <td style="height: 20px;"><a class="adv_info" href="javascript:void(0)" onclick='parent.location.href="/Advanced_System_Info.asp#CPU"'><#SI_LoadCPU#></a></td>
                    <td>
                        <span id="cpu_info"> -- % </span>
                    </td>
                </tr>
                <tr>
                    <td><a class="adv_info" href="javascript:void(0)" onclick='parent.location.href="/Advanced_System_Info.asp#MEM"'><#SI_FreeMem#></a></td>
                    <td>
                        <span id="mem_info"> -- MB / -- MB </span>
                    </td>
                </tr>
                 <tr>
                    <td><#SI_Uptime#></td>
                    <td>
                        <span id="uptime_info">&nbsp;</span>
                    </td>
                </tr>
            </tbody>
        </table>
    </div>

    <div id="cpu_usage" style="display: none;">
        <table class="table table-condensed" width="100%" style="margin-bottom: 0px;">
            <tbody>
                <tr>
                    <td style="text-align:left; border: 0 none;"></td>
                    <td style="border: 0 none; text-align:right;"><a class="label" href="javascript:void(0)" onclick='$("#cpu_usage").hide(); $("#main_info").show();'>hide</a></td>
                </tr>
                <tr>
                    <td width="43%"></td>
                    <td></td>
                </tr>
                <tr>
                    <td></td>
                    <td></td>
                </tr>
                <tr>
                    <td></td>
                    <td></td>
                </tr>
            </tbody>
        </table>
    </div>

    <div id="mem_usage" style="display: none;">
        <table class="table table-condensed" width="100%" style="margin-bottom: 0px;">
            <tbody>
                <tr>
                    <td style="text-align:left; border: 0 none;"></td>
                    <td style="border: 0 none; text-align:right;"><a class="label" href="javascript:void(0)" onclick='$("#mem_usage").hide(); $("#main_info").show();'>hide</a></td>
                </tr>
                <tr>
                    <td width="43%"></td>
                    <td></td>
                </tr>
                <tr>
                    <td></td>
                    <td></td>
                </tr>
                <tr>
                    <td></td>
                    <td></td>
                </tr>
            </tbody>
        </table>
    </div>
</body>
</html>