<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Untitled Document</title>
<link rel="stylesheet" type="text/css" href="aidisk.css"> 

<script type="text/javascript" src="/state.js"></script>
<script>
var ddns_enable_x = parent.getASUSDDNS_enable();
var ddns_server_x = '<% nvram_get_x("LANHostConfig", "ddns_server_x"); %>';
var ddns_hostname_x = '<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>';

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
	
	parent.restore_help_td();	
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
		default:
			alert("System error!");
	}
}

function showDDNS(){
	if('<% nvram_get_x("", "enable_ftp"); %>' == "1"){
		if(this.ddns_enable_x == "1"){
			$("noFTP").style.display = "none";
			$("noDDNS").style.display = "none";
		}
		else{
			$("haveDDNS").style.display = "none";
			$("noFTP").style.display = "none";
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
			//parent.showLoading(compute_work_time(), "waiting");
			parent.document.parameterForm.next_page.value = "/aidisk.asp";

			if(parent.$("dummyShareway").value == "0")
				parent.switchShareMode("ftp", "share");
			else
				parent.initialAccount();//*/
	};
}
</script>
</head>

<body onload="initial();">
<form method="post" name="redirectForm" action=""></form>

<table width="400" border="0" align="center" cellpadding="0" cellspacing="0" >
	<tr>
		<td colspan="3" valign="top">
		<table width="250" border="0" align="center" cellpadding="0" cellspacing="0">
            <tr>
              <td width="64"><img src="../images/aidisk-01.gif" width="64" height="59" /></td>
              <td width="29"><p><img src="../images/aidisk-arrow-1.gif" width="29" height="29" /></p></td>
              <td width="64"><img src="../images/aidisk-02.gif" width="64" height="59" /></td>
              <td width="29"><img src="../images/aidisk-arrow-1.gif" width="29" height="29" /></td>
              <td><img src="../images/aidisk-03_r.gif" width="64" height="59" /></td>
            </tr>
        </table>
		</td>
	</tr>
	<tr>
        <td colspan="3" valign="top"><div class="title"><#Step3_desp#></div></td>
	</tr>
      
      <tr>
      	<td colspan="3" valign="top" class="textbox">
      	<ul>
      	 <li><#yoursharesetting#> <span id="dummyShareStr"></span></li>
         <li>
      	  <div id="haveDDNS">
              <span><#AiDisk_linktoFTP_fromInternet#></span>
              <br>
              <a href="FTP://<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %>" target="_blank">FTP://<% nvram_get_x("LANHostConfig", "ddns_hostname_x"); %></a>
          </div>
          <div id="noDDNS" class="ServerClose">
          	  <#linktoFTP_no_2#>
          </div>
          <div id="noFTP" class="ServerClose"><#linktoFTP_no_1#></div>
		  </li>
		 </ul>
        </td>
      </tr>
      <tr>
        <td align="right" height="50">
		  <div class="short_btn"><a href="Aidisk-3.asp"><#btn_pre#></a></div>		        
		</td>
        <td>
          <div id="finish" class="short_btn"><a href="javascript:;"><#CTL_finish#></a></div>
        </td>
      </tr>
</table>
	</td>
  </tr>
</table>
</body>
</html>
