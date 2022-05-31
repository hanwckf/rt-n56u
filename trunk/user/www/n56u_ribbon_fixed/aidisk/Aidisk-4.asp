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
<script type="text/javascript" src="/state.js"></script>
<script>
var ddns_enable_x = parent.getASUSDDNS_enable();
var ddns_server_x = '<% nvram_get_x("", "ddns_server_x"); %>';
var ddns_hostname_x = '<% nvram_get_x("", "ddns_hostname_x"); %>';

var ddns_hostname_title;

var SAFE_TIME = 30;
var FOLDER_WORK_TIME = 2;

function initial(){
	parent.show_help_iframe(4);
	parent.hideLoading();
	
	parent.get_account_parameter();
	
	show_dummyshareway();
	
	showDDNS();
	
	clickevent();
	parent.openHint(15, 4);
}

function show_dummyshareway(){
	switch(parent.$("dummyShareway").value){
		case "0":
			showtext($("dummyShareStr"), "\"<#Step2_method1#>\"");
			break;
		case "1":
			showtext($("dummyShareStr"), "\"<#Step2_method2#>\"");
			break;
		case "2":
			showtext($("dummyShareStr"), "\"<#Step2_method3#>\"");
			break;
	}
}

function showDDNS(){
	var enable_ftp = '<% nvram_get_x("", "enable_ftp"); %>';

	if(enable_ftp == "1"){
		$("noFTP").style.display = "none";
		if(this.ddns_enable_x == "1"){
			$("noDDNS").style.display = "none";
		}
		else{
			$("haveDDNS").style.display = "none";
		}
	}
	else{
		$("noDDNS").style.display = "none";
		$("haveDDNS").style.display = "none";
	}
}

function go_pre_page(){
	document.redirectForm.action = "/aidisk/Aidisk-3.asp";
	document.redirectForm.submit();
}

function compute_work_time(){
	var total_folder_number = 0;
	
	for(var i = 0; i < parent.pool_names().length; ++i){
		if(parent.pool_names()[i].indexOf("part") < 0)
			continue;
		total_folder_number += parent.get_sharedfolder_in_pool(parent.pool_names()[i]).length;
	}
	
	if(parent.$("dummyShareway").value == "1")
		return FOLDER_WORK_TIME*total_folder_number*2+SAFE_TIME;
	else if(parent.$("dummyShareway").value == "2")
		return FOLDER_WORK_TIME*total_folder_number+SAFE_TIME;
	else
		return SAFE_TIME;
}

function clickevent(){
	$("finish").onclick = function(){
			parent.showLoading();
			parent.document.parameterForm.next_page.value = "/aidisk.asp";
			if(parent.$("dummyShareway").value == "0")
				parent.switchShareMode("ftp", "share");
			else
				parent.initialAccount();
	};
}
</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}

    table.inside th{border-top: 0 none;}

    td.steps .badge {font-size: 300%; margin-left: 10px; padding: 2px 15px 3px 15px; border-radius: 26px;}
</style>
</head>

<body class="body_iframe" onload="initial();">
<form method="post" name="redirectForm" action=""></form>

    <table width="100%" class="table" style="margin-top: 31px;" cellpadding="0" cellspacing="0" >
        <tr>
            <td colspan="2" style="border-top: 0 none; text-align: center" class="steps">
                <span class="badge">1</span>
                <span class="badge" style="margin-left: 30px;">2</span>
                <span class="badge badge-info" style="margin-left: 30px;">3</span>
            </td>
        </tr>
        <tr>
            <td colspan="3" style="border-top: 0 none; padding-top: 25px;"><div class="alert alert-info" style="margin-top: 10px;"><#Step3_desp#></div></td>
        </tr>

        <tr>
            <td colspan="3" class="textbox">
                <ul>
                    <li><#yoursharesetting#> <span id="dummyShareStr"></span></li>
                    <li>
                        <div id="haveDDNS">
                            <span><#AiDisk_linktoFTP_fromInternet#></span>
                            <br>
                            <a href="FTP://<% nvram_get_x("", "ddns_hostname_x"); %>" target="_blank">FTP://<% nvram_get_x("", "ddns_hostname_x"); %></a>
                        </div>
                        <div id="noDDNS" class="ServerClose"><#linktoFTP_no_2#></div>
                        <div id="noFTP" class="ServerClose"><#linktoFTP_no_1#></div>
                    </li>
                 </ul>
            </td>
        </tr>

        <tr>
            <td width="50%" style="text-align: right; border-top: 1px dashed #ddd">
                <a class="btn" style="min-width: 170px;" href="Aidisk-3.asp"><#btn_pre#></a>
            </td>
            <td>
                <a id="finish" class="btn btn-primary" style="width: 170px;" href="javascript:void(0);"><#CTL_finish#></a>
            </td>
        </tr>
    </table>

</body>
</html>
