<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title></title>
<style type="text/css"></style>

<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script>

<% login_state_hook(); %>

var ipmonitor = [<% get_static_client(); %>];
var wireless = [<% wl_auth_list(); %>];
var leases = [<% dhcp_leases(); %>];
var m_dhcp = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

var list_of_BlockedClient = [<% get_nvram_list("FirewallConfig", "MFList"); %>];
var networkmap_fullscan = '<% nvram_match_x("", "networkmap_fullscan", "0", "done"); %>';

var clients = getclients(1);
var unblocked_clients = new Array();
var blocked_clients = new Array();
var page_modified = 0;

function initial(){
	flash_button();
	parent.show_client_status(clients.length);
	isFullscanDone();
	
	set_client_is_blocked();
	show_clients();
	parent.hideLoading();
}

function isFullscanDone(){
	if(networkmap_fullscan == "done"){
		$("LoadingBar").style.display = "none";
		$("refresh_list").disabled = false;
	}
	else{
		$("LoadingBar").style.display = "block";
		setTimeout("location.href='clients.asp';",5000);
		$("refresh_list").disabled = true;
	}
}

function set_client_is_blocked(){
	if(list_type == '1'){
		for(var i = 0; i < clients.length; ++i){
			if(checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][9] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][9] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "block";
	}
	else if(list_type == '0'){
		for(var i = 0; i < clients.length; ++i){
			clients[i][9] = "u";
			unblocked_clients[i] = i;
		}
		$("alert_block").style.display = "none";
	}
	else{
		for(var i = 0; i < clients.length; ++i){
			if(!checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][9] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][9] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "none";
	}
}

function simplyName(orig_name, index){
	if(orig_name != null){
		if(orig_name.length > 17)
			shown_client_name = orig_name.substring(0, 15) + "...";
		else
			shown_client_name = orig_name;
	}
	else
		shown_client_name = clients[index][2];
}

var DEVICE_TYPE = ["", "<#Device_type_01_PC#>", "<#Device_type_02_RT#>", "<#Device_type_03_AP#>", "<#Device_type_04_NAS#>", "<#Device_type_05_IC#>", "<#Device_type_06_OD#>"];
var shown_client_name = "";

function show_clients(){
	var addClient, clientType, clientName, clientIP, clientMAC, clientBlock;
	
	for(var j=0, i=0, k=0; j < clients.length; j++){
		if(clients[j][9] == "u" || sw_mode == "3"){
			var isPrt = "";
			addClient = $('Clients_table').insertRow(k+2);
			clientType = addClient.insertCell(0);
			clientName = addClient.insertCell(1);
			clientIP = addClient.insertCell(2);
			clientMAC = addClient.insertCell(3);
			clients[j][5] = (clients[j][5] == undefined)?6:clients[j][5];
			switch(clients[j][7]){
				case "1":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(LPR)";
					break;
				case "2":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(RAW)";
					break;
				case "3":
					isPrt = "<br/><strong><#Device_service_Printer#> </strong>YES(SMB)";
					break;
				default:
					;
			};
			
			var isITu = (clients[j][8] == "1")?"<br/><strong><#Device_service_iTune#> </strong>YES":"";
			var isWL = (clients[j][3] == "10")?"<br/><strong><#Device_service_Wireless#> </strong>YES":"";
			clientType.style.textAlign = "center";
			clientType.innerHTML = "<img title='"+ DEVICE_TYPE[clients[j][5]]+"' src='/bootstrap/img/wl_device/" + clients[j][5] +".gif'>";
			clientName.innerHTML = "<div class='"+(j == 0 ? 'popover_bottom' : 'popover_top' ) + "' data-original-title='<font size=-1>MAC: " + clients[j][2] + isPrt + isITu + isWL+ "</font>' data-content='"+("Name: " + clients[j][0])+"'>" + clients[j][0] + "</div>";
			clientIP.innerHTML = (clients[j][6] == "1") ? "<a href=http://" + clients[j][1] + " target='blank'>" + clients[j][1] + "</a>" : clients[j][1];
			clientMAC.innerHTML = "<a target='_blank' href='http://standards.ieee.org/cgi-bin/ouisearch?" + clients[j][2].substr(0,6) + "'>" + clients[j][2] + "</a>";
			
			if(list_type != "1" && sw_mode != "3"){
				clientBlock = addClient.insertCell(4);
				clientBlock.style.textAlign = "center";
				clientBlock.innerHTML = '<div class="icon icon-remove" id=unblock_client'+j+' onClick="blockClient('+j+')" style="cursor:pointer;"></div>\n';
			}
			k++;
		}
		else if(clients[j][9] == "b"){
			simplyName(clients[j][0], j);
			
			add_xClient = $('xClients_table').insertRow(i+2); //here use i for create row
			xClientType = add_xClient.insertCell(0);
			xClientName = add_xClient.insertCell(1);
			xClientIP = add_xClient.insertCell(2);
			xClientMAC = add_xClient.insertCell(3);
			
			var isPrt = (clients[j][7] == "1")?"<br/><strong><#Device_service_Printer#> </strong>YES":"";
			var isITu = (clients[j][8] == "1")?"<br/><strong><#Device_service_iTune#> </strong>YES":"";
			var isWL = (clients[j][3] == "10")?"<br/><strong><#Device_service_Wireless#> </strong>YES":"";
			
			clients[j][5] = (clients[j][5] == undefined)?6:clients[j][5];
			clients[j][0] = (clients[j][0] == null)?"":clients[j][0];
			
			xClientType.style.textAlign = "center";
			xClientType.innerHTML = "<img title='" +DEVICE_TYPE[clients[j][5]]+"' src='/bootstrap/img/wl_device/" + clients[j][5] +".gif'>";
			xClientName.innerHTML = "<div class='"+(j == 0 ? 'popover_bottom' : 'popover_top' ) + "' data-original-title='<font size=-1>MAC: " + clients[j][2] + isPrt + isITu + isWL+ "</font>' data-content='"+("Name: " + clients[j][0])+"'>" + clients[j][0] + "</div>";
			
			xClientIP.innerHTML = clients[j][1];
			xClientMAC.innerHTML = "<a target='_blank' href='http://standards.ieee.org/cgi-bin/ouisearch?" + clients[j][2].substr(0,6) + "'>" + clients[j][2] + "</a>";
			
			if(list_type != "1"){
				xClientunBlock = add_xClient.insertCell(4);
				xClientunBlock.style.textAlign = "center";
				xClientunBlock.innerHTML = '<div class="icon icon-plus" onClick="unBlockClient('+j+')" style="cursor:pointer;"></div>\n'
			}
			i++;
		}
	}
}

function blockClient(unBlockedClient_order){
	var str = "";
	
	if(list_type == "1"){
		alert("<#macfilter_alert_str1#>");
		return;
	}
	this.selectedClientOrder = unBlockedClient_order;
	
	str += '<#block_Comfirm1#>" ';
	str += (clients[unBlockedClient_order][0] == null)?clients[unBlockedClient_order][2]:clients[unBlockedClient_order][0];
	str += '" ?\n';
	str += '<#block_Comfirm2#>';
	
	if(confirm(str)){
		set_filter_rule("add");
		do_block_client();
	}
}

function unBlockClient(blockedClient_order){
	var str = "";
	
	if(list_type == "1"){
		alert("<#macfilter_alert_str1#>");
		return;
	}
	
	this.selectedClientOrder = blockedClient_order;
	
	str += '<#unblock_Comfirm1#>" ';
	str += (clients[blockedClient_order][0] == null)?clients[blockedClient_order][2]:clients[blockedClient_order][0];
	str += ' "<#unblock_Comfirm2#>';
	
	if(confirm(str)){
		set_filter_rule("del");
		do_unblock_client();
	}
}

function do_block_client(){
	parent.showLoading();
	if(!checkDuplicateName(clients[this.selectedClientOrder][2], list_of_BlockedClient)){
		document.macfilterForm.action_mode.value = " Add ";
		document.macfilterForm.macfilter_list_x_0.value = clients[this.selectedClientOrder][2];
	}
	else{
		document.macfilterForm.target = "";
		document.macfilterForm.action_mode.value = " Restart ";
		document.macfilterForm.macfilter_list_x_0.disabled = true;
	}
	document.macfilterForm.submit();
}

function do_unblock_client(){
	parent.showLoading();
	document.macfilterForm.action_mode.value = " Del ";
	
	document.macfilterForm.submit();
}

function set_filter_rule(action){
	if(action == "add")
		;
	else if(action == "del"){
		for(var i = 0; i < list_of_BlockedClient.length; ++i){
			if(list_of_BlockedClient[i][0] == clients[this.selectedClientOrder][2]){
				free_options($("MFList_s"));
				add_option($("MFList_s"), null, i, 1);
			}
		}
	}
}

function done_validating(action, group_id){
	$("applyFrame").src = "";
	page_modified = 1;
	if(group_id == "MFList"){
		if(action == " Add ")
			refreshpage();
		else if(action == " Del ")
			refreshpage();
	}
}

function submit_macfilter(){
	document.macfilterForm.target = "";
	document.macfilterForm.action_mode.value = " Restart ";
	document.macfilterForm.next_page.value = location.pathname;
	document.macfilterForm.submit();
}


function build_submitrule(){
	if(document.macfilterForm.modified.value == "1"){
		if(list_type != "1"){
			submit_macfilter();
		}
		else
			refreshpage();
	}
	else
		refreshpage();
}


function applyRule(){
	parent.showLoading();
	build_submitrule();
}

function showLoading(restart_time2){
	parent.showLoading(restart_time2);
}

function networkmap_update(s){
	document.form.action_mode.value = "";
	document.form.action_script.value = s;
	document.form.flag.value = "nodetect";
	document.form.submit();
}

</script>

<style>
    .table th, .table td{vertical-align: middle; text-align: center;}
</style>

</head>

<body class="body_iframe" >

<iframe name="applyFrame" id="applyFrame" src="" width="0" height="0" frameborder="0" scrolling="no" style="position: absolute;"></iframe>

<div id="LoadingBar" style="display: none; "><span class="label label-info"><#Device_Searching#></span>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="loadingBarBlock" class="loadingBarBlock" align="center" style="display: none;">
</div>

<form method="post" name="macfilterForm" id="macfilterForm" action="/start_apply.htm" target="applyFrame" style="position: absolute; margin-left: -10000px;">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;General;">
<input type="hidden" name="group_id" value="MFList">
<input type="hidden" name="current_page" value="/device-map/clients.asp">
<input type="hidden" name="next_page" value="/device-map/clients.asp">
<input type="hidden" name="modified" value="<% nvram_get_x("", "MFList"); %>">
<!-- for enable rule in MACfilter -->
<input type="hidden" name="macfilter_enable_x" value="2">
<!-- for add rule in MACfilter -->
<input type="hidden" name="macfilter_list_x_0" value="">
<input type="hidden" name="macfilter_time_x_0" value="00002359">
<input type="hidden" name="macfilter_date_x_0" value="1111111">
<!-- for del rule in MACfilter -->
<select name="MFList_s" id="MFList_s" multiple="true" style="visibility:hidden; width:0px; height:0px;"></select>
</form>

<div id="unBlockedClients_table"></div>

<table id="Clients_table" width="100%" align="center" cellpadding="1" class="table">
    <thead>
        <tr>
            <th colspan="5" style="text-align: center;"><#ConnectedClient#></th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>
<br />

<div id="blockedClients_table"></div>
<table id="xClients_table" width="100%" align="center" class="table">
    <thead>
        <tr>
            <th colspan="5" style="text-align: center;"><#BlockedClient#></th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>

<center>
    <input type="button" id="applyClient" class="btn btn-primary span2" onclick="applyRule();" value="<#CTL_apply#>" >
    <input type="button" id="refresh_list" class="btn btn-info span2" onclick="networkmap_update('networkmap_refresh');" value="<#CTL_refresh#>">
</center>

<p><div id="alert_block" class="alert alert-danger" style="margin-top:40px; display: none;">
	<a href="/Advanced_MACFilter_Content.asp" target="_parent"><#menu5_5_3#></a> <#macfilter_alert_str1#>
</div></p>


<form method="post" name="form" id="refreshForm" action="/start_apply.htm" target="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="current_page" value="/device-map/clients.asp">
<input type="hidden" name="next_page" value="/device-map/clients.asp">
<input type="hidden" name="flag" value="">
</form>

<script>
	var $j = jQuery.noConflict();

	$j(document).ready(function() {
		$j('.popover_top').popover({placement: 'right'});
		$j('.popover_bottom').popover({placement: 'right'});
	});

	var th  = "<th width='6%'><#Type#></th>";
	th += "<th width='35%'><#Computer_Name#></th>";
	th += "<th width='22%'><#LAN_IP#></th>";
	th += "<th width='22%'><#MAC_Address#></th>";

	if (sw_mode != "3") {
		var list_type = '<% nvram_get_x("", "macfilter_enable_x"); %>';
		var list_of_BlockedClient = [<% get_nvram_list("FirewallConfig", "MFList"); %>];
		$j("#Clients_table thead").append("<tr>" + th + (list_type != "1" ? '<th><#Block#></th>' : '') + "</tr>");
		$j("#xClients_table thead").append("<tr>" + th + (list_type != "1" ? '<th><#unBlock#></th>' : '') + "</tr>");
		if(list_of_BlockedClient.length == 0)
			$j("#xClients_table tbody").append("<tr><td colspan='5'><div class='alert alert-info'><#Nodata#></div></td></tr>");
	} else {
		$("applyClient").style.display = "none";
		$("xClients_table").style.display = "none";
		$j("#Clients_table thead").append("<tr>" + th + "</tr>");
	}
</script>

<script>
	initial();

	function loading() {
		$("loadingBarBlock").style.display = "none";
	}
	if (window.attachEvent) {
		window.attachEvent('onload', loading);
	} else {
		window.addEventListener('load', loading, false);
	}
</script>
</body>
</html>
