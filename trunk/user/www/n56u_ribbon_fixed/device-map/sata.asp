<!DOCTYPE html>
<html>
<head>
<title></title>
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
<script>
var $j = jQuery.noConflict();

var virt_port_id = "1000";
var disk_ports_array = parent.foreign_disk_interface_names();

function initial(){
	var i, total_mounted, total_devices;
	var code = '';

	total_devices = 0;
	total_mounted = 0;

	for(i = 0; i < disk_ports_array.length; i++){
		if(disk_ports_array[i] == virt_port_id){
			var model_name = parent.getDiskModelName(i);
			var size_total = parent.getDiskTotalSize(i);
			var mounted_num = parent.getDiskMountedNum(i);
			code +='<div class="alert alert-info alert-header"><#SATA_Storage#></div>\n';
			code +='<table width="100%" cellpadding="4" cellspacing="0" class="table">\n';
			code +='  <tr>\n';
			code +='    <th width="50%" style="border: 0 none;"><#Modelname#>:</th>\n';
			code +='    <td style="border: 0 none;">'+model_name+'</td>\n';
			code +='  </tr>\n';
			code +='  <tr>\n';
		    if (mounted_num > 0) {
				var alertPercentbar = 'progress-info';
				var size_free = simpleNum2(parent.computeallpools(i, "size")-parent.computeallpools(i, "size_in_use"));
				var percentbar = Math.round(100-simpleNum2((size_free)/size_total*100));
				var percentstr = (percentbar > 10 ? (percentbar + '%') : '');
				if (percentbar > 65 && percentbar < 85)
					alertPercentbar = 'progress-warning';
				else if (percentbar >= 85)
					alertPercentbar = 'progress-danger';
			code +='    <th><#Totalspace#> / <#Availdisk#>:</th>\n';
			code +='    <td>'+size_total+' GB / '+size_free+' GB</td>\n';
			code +='  </tr>\n';
			code +='  <tr>\n';
			code +='    <th><#DiskUsage#>:</th>\n';
			code +='    <td><div style="margin-bottom: 2px; width:250px; float: left;" class="progress '+alertPercentbar+'"><div class="bar" style="width:'+percentbar+'%">'+percentstr+'</div></div></td>\n';
			code +='  </tr>\n';
			code +='  <tr>\n';
			code +='    <th><#Safelyremovedisk_title#>:</th>\n';
			code +='    <td><input type="button" class="btn btn-success span2" onclick="remove_disk('+i+');" value="<#btn_remove#>"></td>\n';
			total_mounted++;
		    } else {
			code +='    <th><#Totalspace#>:</th>\n';
			code +='    <td>'+size_total+' GB</td>\n';
			code +='  </tr>\n';
			code +='  <tr>\n';
			code +='    <th><#Safelyremovedisk_title#>:</th>\n';
			code +='    <td><span class="label label-success"><#Safelyremovedisk#></span></td>\n';
		    }
			code +='  </tr>\n';
			code +='</table>\n';
			total_devices++;
		}
	}

	if(total_devices < 1){
		code +='<div class="alert alert-info alert-header" style="text-align: center;"><#SATA_Empty#></div>\n';
	}

	$j('#sata_pool').append(code);
}

function remove_disk(disk_idx){
	var str = "<#Safelyremovedisk_confirm#>";

	if(confirm(str)){
		parent.showLoading();
		
		document.diskForm.action = "safely_remove_disk.asp";
		document.diskForm.port.value = virt_port_id;
		document.diskForm.devn.value = "";
		if (disk_idx >= 0)
			document.diskForm.devn.value = parent.getDiskDevice(disk_idx);
		document.diskForm.submit();
	}
}

</script>

<style>
    .table {margin-bottom: 0px;}
    .table th, .table td{vertical-align: middle;}
    .alert-header {margin: 3px 0px 0px 0px; padding: 5px 5px 5px 8px;}
    .progress {
        background-image: -moz-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -ms-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -webkit-gradient(linear, 0 0, 0 100%, from(#f3f3f3), to(#dddddd));
        background-image: -webkit-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: -o-linear-gradient(top, #f3f3f3, #dddddd);
        background-image: linear-gradient(top, #f3f3f3, #dddddd);
        filter: progid:dximagetransform.microsoft.gradient(startColorstr='#f3f3f3', endColorstr='#dddddd', GradientType=0);
    }
</style>
</head>

<body class="body_iframe" onload="initial();">

<div id="sata_pool"></div>

<form method="post" name="diskForm" action="">
<input type="hidden" name="port" value="">
<input type="hidden" name="devn" value="">
</form>

</body>
</html>
