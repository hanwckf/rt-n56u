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
<script type="text/javascript" src="/client_function.js"></script>
<script>
var $j = jQuery.noConflict();

var ipmonitor = [<% get_static_client(); %>];
var wireless = {<% wl_auth_list(); %>};

var list_type = '<% nvram_get_x("", "macfilter_enable_x"); %>';
var list_of_BlockedClient = [<% get_nvram_list("FirewallConfig", "MFList"); %>];
var m_dhcp = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

var nmap_fullscan = '<% nvram_get_x("", "networkmap_fullscan"); %>';

var DEVICE_TYPE = ["", "<#Device_type_01_PC#>", "<#Device_type_02_RT#>", "<#Device_type_03_AP#>", "<#Device_type_04_NAS#>", "<#Device_type_05_IC#>", "<#Device_type_06_OD#>"];

var clients = getclients(1,0);
var unblocked_clients = new Array();
var blocked_clients = new Array();
var page_modified = 0;

function initial(){
	if (sw_mode == "3") {
		list_type = '0';
	}
	flash_button();
	prepare_clients();
	show_clients();
	check_full_scan_done();
}

function prepare_clients(){
	var i, j, k;

	if(typeof parent.show_client_status === 'function')
		parent.show_client_status(clients.length);

	if(list_type == '0'){
		for(i = 0; i < clients.length; ++i){
			clients[i][7] = "u";
			unblocked_clients[i] = i;
		}
		$("alert_block").style.display = "none";
	}
	else if(list_type == '1'){
		for(i = 0; i < clients.length; ++i){
			if(checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][7] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][7] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "block";
	}
	else{
		for(i = 0; i < list_of_BlockedClient.length; ++i){
			if(!checkDuplicateName(list_of_BlockedClient[i][0], clients)){
				k = clients.length;
				clients[k] = new Array(8);
				
				clients[k][0] = "*";
				clients[k][1] = "*";
				clients[k][2] = list_of_BlockedClient[i][0];
				clients[k][3] = null;
				clients[k][4] = null;
				clients[k][5] = "6";
				clients[k][6] = "0";
				clients[k][7] = "b";
				
				var mac_up = list_of_BlockedClient[i][0].toUpperCase();
				
				for(j = 0; j < m_dhcp.length; ++j){
					if (mac_up == m_dhcp[j][0].toUpperCase()){
						if (m_dhcp[j][2] != null && m_dhcp[j][2].length > 0)
							clients[k][0] = m_dhcp[j][2];
						clients[k][1] = m_dhcp[j][1];
						break;
					}
				}
			}
		}
		
		for(i = 0; i < clients.length; ++i){
			if(!checkDuplicateName(clients[i][2], list_of_BlockedClient)){
				clients[i][7] = "u";
				unblocked_clients[i] = i;
			}
			else{
				clients[i][7] = "b";
				blocked_clients[i] = i;
			}
		}
		$("alert_block").style.display = "none";
	}
}

function check_full_scan_done(){
	if(nmap_fullscan != "1"){
		$("LoadingBar").style.display = "none";
		$("refresh_list").disabled = false;
		if (sw_mode == "3") {
			$j('.popover_top').popover({placement: 'top'});
			$j('.popover_bottom').popover({placement: 'bottom'});
		}else {
			$j('.popover_top').popover({placement: 'right'});
			$j('.popover_bottom').popover({placement: 'right'});
		}
	}else{
		$("LoadingBar").style.display = "block";
		$("refresh_list").disabled = true;
		setTimeout("update_clients();", 2000);
	}
}

function update_clients(e) {
	$j.ajax({
		url: '/lan_clients.asp',
		dataType: 'script',
		error: function(xhr) {
			;
		},
		success: function(response) {
			ipmonitor = ipmonitor_last;
			nmap_fullscan = nmap_fullscan_last;
			clients = [];
			clients = getclients(1,0);
			prepare_clients();
			show_clients();
			check_full_scan_done();
		}
	});
}

function add_client_row(table, atIndex, client, blocked, j){
	var row = table.insertRow(atIndex);
	var typeCell = row.insertCell(0);
	var nameCell = row.insertCell(1);
	var ipCell = row.insertCell(2);
	var macCell = row.insertCell(3);
	var rssiCell = row.insertCell(4);
	var blockCell = row.insertCell(5);

	typeCell.style.textAlign = "center";
	typeCell.innerHTML = "<img title='"+ DEVICE_TYPE[client[5]]+"' src='/bootstrap/img/wl_device/" + client[5] +".gif'>";
	nameCell.innerHTML = (client[6] == "1") ? "<a href=http://" + client[0] + " target='blank'>" + client[0] + "</a>" : client[0];
	ipCell.innerHTML = (client[6] == "1") ? "<a href=http://" + client[1] + " target='blank'>" + client[1] + "</a>" : client[1];
	macCell.innerHTML = "<a target='_blank' href='https://services13.ieee.org/RST/standards-ra-web/rest/assignments/?registry=MAC&text=" + client[2].substr(0,6) + "'>" + mac_add_delimiters(client[2]) + "</a>";
	if (client[3] == 10){
		rssiCell.innerHTML = client[4].toString();
	}
	if(list_type != "1" && sw_mode != "3"){
		blockCell.style.textAlign = "center";
		blockCell.innerHTML = "<div class='icon icon-" + (blocked ? "plus" : "remove") + "' onClick='" + (blocked ? "unBlock" : "block") + "Client("+j+")' style='cursor:pointer;'></div>\n";
	}
}

function show_clients(){
	var i, j, k;
	var table1, table2;
	var addClient, clientType, clientName, clientIP, clientMAC, clientBlock;
	
	table1 = $('Clients_table');
	table2 = $('xClients_table');
	
	while (table1.rows.length > 2)
		table1.deleteRow(-1);
	while (table2.rows.length > 2)
		table2.deleteRow(-1);
	
	var hasBlocked = false;
	for(j=0, i=0, k=0; j < clients.length; j++){
		if(clients[j][7] == "u" || sw_mode == "3"){
			add_client_row(table1, k+2, clients[j], false, j);
			k++;
		}
		else if(clients[j][7] == "b"){
			add_client_row(table2, i+2, clients[j], true, j);
			hasBlocked = true;
			i++;
		}
	}

	table2.style.display = hasBlocked ? "inline" : "none";

	var NDRow = "<tr><td colspan='6'><div class='alert alert-info'><#Nodata#></div></td></tr>";

	if (table1.rows.length < 3)
		$j("#Clients_table tbody").append(NDRow);

	if (table2.rows.length < 3 && sw_mode != "3")
		$j("#xClients_table tbody").append(NDRow);
}

function sort(mode) {
	sort_mode = (sort_mode > 0) ? (mode * -1) : mode;
	localStorage.setItem('sortMode', sort_mode);
	clients = getclients(1,0);
	prepare_clients();
	show_clients();
}

function blockClient(unBlockedClient_order){
	var str = "";

	if(list_type == "1"){
		alert("<#macfilter_alert_str1#>");
		return;
	}
	this.selectedClientOrder = unBlockedClient_order;

	str += '<#block_Comfirm1#> "';
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

	str += '<#unblock_Comfirm1#> "';
	str += (clients[blockedClient_order][0] == null)?clients[blockedClient_order][2]:clients[blockedClient_order][0];
	str += '" "<#unblock_Comfirm2#>';

	if(confirm(str)){
		set_filter_rule("del");
		do_unblock_client();
	}
}

function do_block_client(){
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
		if(list_type != "1")
			submit_macfilter();
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
<input type="hidden" name="macfilter_enable_x" value="2">
<input type="hidden" name="macfilter_list_x_0" value="">
<input type="hidden" name="macfilter_time_x_0" value="00002359">
<input type="hidden" name="macfilter_date_x_0" value="1111111">
<select name="MFList_s" id="MFList_s" multiple="true" style="visibility:hidden; width:0px; height:0px;"></select>
</form>

<div id="unBlockedClients_table"></div>

<table id="Clients_table" width="100%" align="center" cellpadding="1" class="table">
    <thead>
        <tr>
            <th colspan="5" style="text-align: center;"><#ConnectedClient#></th>
        </tr>
        <tr>
            <th width="8%"><a href="javascript:sort(0)"><#Type#></a></th>
            <th><a href="javascript:sort(1)"><#Computer_Name#></a></th>
            <th width="20%"><a href="javascript:sort(2)">IP</a></th>
            <th width="24%"><a href="javascript:sort(3)">MAC</a></th>
            <th width="8%" id="col_rssi"><a href="javascript:sort(4)">RSSI</a></th>
            <th width="0%" id="col_block"></th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>

<div id="blockedClients_table"></div>
<table id="xClients_table" width="100%" align="center" class="table">
    <thead>
        <tr>
            <th colspan="5" style="text-align: center;"><#BlockedClient#></th>
        </tr>
        <tr>
            <th width="8%"><#Type#></th>
            <th><#Computer_Name#></th>
            <th width="20%">IP</th>
            <th width="24%">MAC</th>
            <th width="8%" id="col_unrssi">RSSI</th>
            <th width="0%" id="col_unblock"></th>
        </tr>
    </thead>
    <tbody>
    </tbody>
</table>

<center>
    <input type="button" id="applyClient" class="btn btn-primary span2" onclick="applyRule();" value="<#CTL_apply#>" />
    <input type="button" id="refresh_list" class="btn btn-info span2" onclick="networkmap_update('networkmap_refresh');" value="<#CTL_refresh#>" />
</center>

<div id="alert_block" class="alert alert-danger" style="margin-top:40px; display: none;">
    <p><a href="/Advanced_MACFilter_Content.asp" target="_parent"><#menu5_5_3#></a> <#macfilter_alert_str1#></p>
</div>

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
	if (!support_2g_radio() && !support_5g_radio()) {
		$("col_rssi").width = "0%";
		$("col_rssi").innerHTML = "";
		$("col_unrssi").width = "0%";
		$("col_unrssi").innerText = "";
	}
	if (sw_mode != "3") {
		if (list_type != "1") {
			$("col_block").width = "12%";
			$("col_unblock").width = "12%";
			$("col_block").innerHTML = "<#Block#>";
			$("col_unblock").innerHTML = "<#unBlock#>";
		}
	} else {
		$("applyClient").style.display = "none";
		$("xClients_table").style.display = "none";
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



