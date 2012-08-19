<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Wireless Router <#Web_Title#> - <#menu5_3_1#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';
wan0_ifname = '<% nvram_get_x("",  "wan0_ifname"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var original_wan_type = wan_proto;
var original_wan_dhcpenable = parseInt('<% nvram_get_x("Layer3Forwarding", "x_DHCPClient"); %>');
var original_dnsenable = parseInt('<% nvram_get_x("IPConnection", "wan_dnsenable_x"); %>');
var client_mac = login_mac_str();

function initial(){
	show_banner(1);
	show_menu(5,3,1);
	show_footer();

	enable_auto_hint(7, 19);

	change_wan_type(document.form.wan_proto.value, 0);
	fixed_change_wan_type(document.form.wan_proto.value);

	if(document.form.wan_pppoe_txonly_x.value == "1")
		document.form.wan_pppoe_idletime_check.checked = true;

	change_nat(sw_mode);

	AuthSelection(document.form.wan_auth_mode.value);
	
	change_stb_port_and_vlan();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		inputCtrl(document.form.x_DHCPClient[0], 1);
		inputCtrl(document.form.x_DHCPClient[1], 1);
		if(!document.form.x_DHCPClient[0].checked){
			inputCtrl(document.form.wan_ipaddr, 1);
			inputCtrl(document.form.wan_netmask, 1);
			inputCtrl(document.form.wan_gateway, 1);
		}
		
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		if(!document.form.wan_dnsenable_x[0].checked){
			inputCtrl(document.form.wan_dns1_x, 1);
			inputCtrl(document.form.wan_dns2_x, 1);
		}
		
		document.form.next_page.value = "";
		document.form.action_mode.value = " Apply ";
		document.form.submit();
	}
}

function validForm(){
	var wan_stb_x = document.form.wan_stb_x.value;
	
	if(!document.form.x_DHCPClient[0].checked){
		if(!validate_ipaddr_final(document.form.wan_ipaddr, 'wan_ipaddr')
				|| !validate_ipaddr_final(document.form.wan_netmask, 'wan_netmask')
				|| !validate_ipaddr_final(document.form.wan_gateway, 'wan_gateway')
				)
			return false;
		
		if(document.form.wan_gateway.value == document.form.wan_ipaddr.value){
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			document.form.wan_gateway.select();
			document.form.wan_gateway.focus();
			return false;
		}

		if(!wan_netmask_check(document.form.wan_netmask))
			return false;
	}
	
	if(!document.form.wan_dnsenable_x[0].checked){
		
		if(!validate_ipaddr_final(document.form.wan_dns1_x, 'wan_dns1_x')){
			document.form.wan_dns1_x.select();
			document.form.wan_dns1_x.focus();
			
			return false;
		}
		if(!validate_ipaddr_final(document.form.wan_dns2_x, 'wan_dns2_x')){
			document.form.wan_dns2_x.select();
			document.form.wan_dns2_x.focus();
			
			return false;
		}
	}
	
	if(document.form.wan_proto.value == "pppoe"
			|| document.form.wan_proto.value == "pptp"
			|| document.form.wan_proto.value == "l2tp"
			){
		if(!validate_string(document.form.wan_pppoe_username)
				|| !validate_string(document.form.wan_pppoe_passwd)
				)
			return false;
		
		if(!validate_range(document.form.wan_pppoe_idletime, 0, 4294967295))
			return false;
	}
	
	if(document.form.wan_proto.value == "pppoe"){
		if(!validate_range(document.form.wan_pppoe_mtu, 576, 1492)
				|| !validate_string(document.form.wan_pppoe_mru, 576, 1492))
			return false;
		
		if(!validate_string(document.form.wan_pppoe_service)
				|| !validate_string(document.form.wan_pppoe_ac))
			return false;
	}
	
	if(document.form.wan_hostname.value.length > 0)
		 if(!validate_string(document.form.wan_hostname))
		 	return false;
	
	if(document.form.wan_hwaddr_x.value.length > 0)
		 if(!validate_hwaddr(document.form.wan_hwaddr_x))
		 	return false;
	
	if(document.form.wan_heartbeat_x.value.length > 0)
		 if(!validate_string(document.form.wan_heartbeat_x))
		 	return false;

	if(document.form.vlan_filter[0].checked)
	{
		if(document.form.vlan_vid_cpu.value.length > 0)
		{
			if(!validate_range(document.form.vlan_vid_cpu, 2, 4094))
				return false;
			if(!validate_range(document.form.vlan_pri_cpu, 0, 7))
				return false;
		}
		
		if (wan_stb_x == "1" || wan_stb_x == "6" || wan_stb_x == "7")
		{
			if(document.form.vlan_vid_lan1.value.length > 0)
			{
				if(!validate_range(document.form.vlan_vid_lan1, 2, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan1, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "2" || wan_stb_x == "6" || wan_stb_x == "7")
		{
			if(document.form.vlan_vid_lan2.value.length > 0)
			{
				if(!validate_range(document.form.vlan_vid_lan2, 2, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan2, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "3" || wan_stb_x == "5" || wan_stb_x == "7")
		{
			if(document.form.vlan_vid_lan3.value.length > 0)
			{
				if(!validate_range(document.form.vlan_vid_lan3, 2, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan3, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "4" || wan_stb_x == "5" || wan_stb_x == "7")
		{
			if(document.form.vlan_vid_lan4.value.length > 0)
			{
				if(!validate_range(document.form.vlan_vid_lan4, 2, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan4, 0, 7))
					return false;
			}
		}
	}
	
	return true;
}

function done_validating(action){
	refreshpage();
}

function change_nat(nat_value){
	if(nat_value != 1){
		$("hw_nat_row").style.display = "none";
		$("sw_nat_row").style.display = "none";
	}
	else {
		$("hw_nat_row").style.display = "";
		$("sw_nat_row").style.display = "";
	}
}

function change_wan_type(wan_type, flag){
	if(typeof(flag) != "undefined")
		change_wan_dhcp_enable(flag);
	else
		change_wan_dhcp_enable(1);
	
	if(wan_type == "pppoe"){
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 1);
		inputCtrl(document.form.wan_pppoe_idletime_check, 1);
		inputCtrl(document.form.wan_pppoe_mtu, 1);
		inputCtrl(document.form.wan_pppoe_mru, 1);
		inputCtrl(document.form.wan_pppoe_service, 1);
		inputCtrl(document.form.wan_pppoe_ac, 1);
		inputCtrl(document.form.wan_heartbeat_x, 0);
		inputCtrl(document.form.wan_auth_mode, 0);
		inputCtrl(document.form.wan_auth_user, 0);
		inputCtrl(document.form.wan_auth_pass, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		
		$("wan_poller_row").style.display = "none";
		$("dhcp_sect").style.display = "";
		$("account_sect").style.display = "";
		$("pppoe_dhcp_x").style.display = "";
	}
	else if(wan_type == "pptp"){
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_auth_mode, 0);
		inputCtrl(document.form.wan_auth_user, 0);
		inputCtrl(document.form.wan_auth_pass, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pptp_options_x, 1);
		// 2008.03 James. patch for Oleg's patch. }
		
		$("wan_poller_row").style.display = "none";
		$("dhcp_sect").style.display = "";
		$("account_sect").style.display = "";
		$("pppoe_dhcp_x").style.display = "none";
	}
	else if(wan_type == "l2tp"){		
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		
		inputCtrl(document.form.wan_pppoe_username, 1);
		inputCtrl(document.form.wan_pppoe_passwd, 1);
		inputCtrl(document.form.wan_pppoe_idletime, 0);
		inputCtrl(document.form.wan_pppoe_idletime_check, 0);
		inputCtrl(document.form.wan_pppoe_mtu, 0);
		inputCtrl(document.form.wan_pppoe_mru, 0);
		inputCtrl(document.form.wan_pppoe_service, 0);
		inputCtrl(document.form.wan_pppoe_ac, 0);
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_auth_mode, 0);
		inputCtrl(document.form.wan_auth_user, 0);
		inputCtrl(document.form.wan_auth_pass, 0);
		
		// 2008.03 James. patch for Oleg's patch. {
		inputCtrl(document.form.wan_pptp_options_x, 0);
		// 2008.03 James. patch for Oleg's patch. }
		
		$("wan_poller_row").style.display = "none";
		$("dhcp_sect").style.display = "";
		$("account_sect").style.display = "";
		$("pppoe_dhcp_x").style.display = "none";
	}
	else if(wan_type == "static"){
		inputCtrl(document.form.wan_dnsenable_x[0], 0);
		inputCtrl(document.form.wan_dnsenable_x[1], 0);
		
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_auth_mode, 1);
		inputCtrl(document.form.wan_auth_user, 1);
		inputCtrl(document.form.wan_auth_pass, 1);
		
		$("wan_poller_row").style.display = "none";
		$("dhcp_sect").style.display = "";
		$("account_sect").style.display = "none";
		$("pppoe_dhcp_x").style.display = "none";
	}
	else{	// Automatic IP
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		
		inputCtrl(document.form.wan_heartbeat_x, 1);
		inputCtrl(document.form.wan_auth_mode, 1);
		inputCtrl(document.form.wan_auth_user, 1);
		inputCtrl(document.form.wan_auth_pass, 1);
		
		$("wan_poller_row").style.display = "";
		$("dhcp_sect").style.display = "none";
		$("account_sect").style.display = "none";
		$("pppoe_dhcp_x").style.display = "none";
	}
}

function fixed_change_wan_type(wan_type){
	var flag = false;
	
	if(!document.form.x_DHCPClient[0].checked){
		if(document.form.wan_ipaddr.value.length == 0)
			document.form.wan_ipaddr.focus();
		else if(document.form.wan_netmask.value.length == 0)
			document.form.wan_netmask.focus();
		else if(document.form.wan_gateway.value.length == 0)
			document.form.wan_gateway.focus();
		else
			flag = true;
	}
	else
		flag = true;
	
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		if(wan_type == original_wan_type){
			document.form.wan_dnsenable_x[0].checked = original_dnsenable;
			document.form.wan_dnsenable_x[1].checked = !original_dnsenable;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', original_dnsenable);
			
			if(flag == true && document.form.wan_dns1_x.value.length == 0 && document.form.wan_dnsenable_x[1].checked == 1)
				document.form.wan_dns1_x.focus();
		}
		else{
			document.form.wan_dnsenable_x[0].checked = 1;
			document.form.wan_dnsenable_x[1].checked = 0;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);
			
			inputCtrl(document.form.wan_dns1_x, 0);
			inputCtrl(document.form.wan_dns2_x, 0);
		}
	}
	else if(wan_type == "static"){
		document.form.wan_dnsenable_x[0].checked = 0;
		document.form.wan_dnsenable_x[1].checked = 1;
		change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);
		
		if(flag == true && document.form.wan_dns1_x.value.length == 0)
			document.form.wan_dns1_x.focus();
	}
	else{	// wan_type == "dhcp"
		
		if(wan_type == original_wan_type){
			document.form.wan_dnsenable_x[0].checked = original_dnsenable;
			document.form.wan_dnsenable_x[1].checked = !original_dnsenable;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', original_dnsenable);
		}
		else{
			document.form.wan_dnsenable_x[0].checked = 1;
			document.form.wan_dnsenable_x[1].checked = 0;
			change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);
			
			inputCtrl(document.form.wan_dns1_x, 0);
			inputCtrl(document.form.wan_dns2_x, 0);
		}
	}
	
	if((document.form.x_DHCPClient[0].checked) || (wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp")){
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
	}
	else{
		inputCtrl(document.form.wan_dnsenable_x[0], 0);
		inputCtrl(document.form.wan_dnsenable_x[1], 0);
	}
}

function change_wan_dhcp_enable(flag){
	var wan_type = document.form.wan_proto.value;
	
	// 2008.03 James. patch for Oleg's patch. {
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		if(flag == 1){
			if(wan_type == original_wan_type){
				document.form.x_DHCPClient[0].checked = original_wan_dhcpenable;
				document.form.x_DHCPClient[1].checked = !original_wan_dhcpenable;
			}
			else{
				document.form.x_DHCPClient[0].checked = 1;
				document.form.x_DHCPClient[1].checked = 0;
			}
		}
		
		inputCtrl(document.form.x_DHCPClient[0], 1);
		inputCtrl(document.form.x_DHCPClient[1], 1);
		
		var wan_dhcpenable = document.form.x_DHCPClient[0].checked;
		
		inputCtrl(document.form.wan_ipaddr, !wan_dhcpenable);
		inputCtrl(document.form.wan_netmask, !wan_dhcpenable);
		inputCtrl(document.form.wan_gateway, !wan_dhcpenable);
	}
	else if(wan_type == "static"){
		document.form.x_DHCPClient[0].checked = 0;
		document.form.x_DHCPClient[1].checked = 1;
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		
		inputCtrl(document.form.wan_ipaddr, 1);
		inputCtrl(document.form.wan_netmask, 1);
		inputCtrl(document.form.wan_gateway, 1);
	}
	else{	// wan_type == "dhcp"
		document.form.x_DHCPClient[0].checked = 1;
		document.form.x_DHCPClient[1].checked = 0;
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		
		inputCtrl(document.form.wan_ipaddr, 0);
		inputCtrl(document.form.wan_netmask, 0);
		inputCtrl(document.form.wan_gateway, 0);
	}
	
	if((document.form.x_DHCPClient[0].checked) || (wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp")){
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
	}
	else{
		document.form.wan_dnsenable_x[0].checked = 0;
		document.form.wan_dnsenable_x[1].checked = 1;
		change_common_radio(document.form.wan_dnsenable_x, 'IPConnection', 'wan_dnsenable_x', 0);
		
		inputCtrl(document.form.wan_dnsenable_x[0], 0);
		inputCtrl(document.form.wan_dnsenable_x[1], 0);
	}
}

function change_stb_port_and_vlan(){
	var wan_stb_x   = document.form.wan_stb_x.value;
	var vlan_filter = document.form.vlan_filter[0].checked;
	
	if (wan0_ifname == "eth2.2"){
		vlan_filter = 0;
		$("wan_vlan_filter").style.display = "none";
	}
	
	if(wan_stb_x == "0" || vlan_filter) {
		$("wan_stb_iso").style.display = "none";
	}
	else {
		$("wan_stb_iso").style.display = "";
	}
	
	if(!vlan_filter) {
		$("vlan_cpu").style.display = "none";
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	
	if(wan_stb_x == "0") {
		if(vlan_filter)
			$("vlan_cpu").style.display = "";
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "1") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan1").style.display = "";
		}
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "2") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan2").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "3") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan3").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "4") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan4").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
	}
	else if(wan_stb_x == "5") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan3").style.display = "";
			$("vlan_lan4").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
	}
	else if(wan_stb_x == "6") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan1").style.display = "";
			$("vlan_lan2").style.display = "";
		}
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "7") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_lan1").style.display = "";
			$("vlan_lan2").style.display = "";
			$("vlan_lan3").style.display = "";
		}
		$("vlan_lan4").style.display = "none";
	}
}

function click_untag_lan(lan_port) {
	if (lan_port == 1) {
		document.form.vlan_tag_lan1.value = (document.form.untag_lan1.checked) ? "0" : "1";
	}
	else if (lan_port == 2) {
		document.form.vlan_tag_lan2.value = (document.form.untag_lan2.checked) ? "0" : "1";
	}
	else if (lan_port == 3) {
		document.form.vlan_tag_lan3.value = (document.form.untag_lan3.checked) ? "0" : "1";
	}
	else if (lan_port == 4) {
		document.form.vlan_tag_lan4.value = (document.form.untag_lan4.checked) ? "0" : "1";
	}
}

function AuthSelection(auth){
	if (auth == "2"){
		$("auth_user_x").style.display = "";
	}
	else{
		$("auth_user_x").style.display = "none";
	}
	
	if (auth != "0"){
		$("auth_pass_x").style.display = "";
	}
	else{
		$("auth_pass_x").style.display = "none";
	}

}

function showMAC(){
	document.form.wan_hwaddr_x.value = simplyMAC(this.client_mac);
}

function simplyMAC(fullMAC){
	var ptr;
	var tempMAC;
	var pos1, pos2;
	
	ptr = fullMAC;
	tempMAC = "";
	pos1 = pos2 = 0;
	
	for(var i = 0; i < 5; ++i){
		pos2 = pos1+ptr.indexOf(":");
		
		tempMAC += fullMAC.substring(pos1, pos2);
		
		pos1 = pos2+1;
		ptr = fullMAC.substring(pos1);
	}
	
	tempMAC += fullMAC.substring(pos1);
	
	return tempMAC;
}


</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(7, 19);return unload_body();">
<script>
	if(sw_mode == 3){
		alert("<#page_not_support_mode_hint#>");
		location.href = "/as.asp";
	}
</script>
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">
<input type="hidden" name="support_cdma" value="<% nvram_get_x("IPConnection", "support_cdma"); %>">
<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;PrinterStatus;WLANConfig11b">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="wan_pppoe_txonly_x" value="<% nvram_get_x("PPPConnection","wan_pppoe_txonly_x"); %>" />
<input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>" />
<input type="hidden" name="lan_netmask" value="<% nvram_get_x("LANHostConfig", "lan_netmask"); %>" />
<input type="hidden" name="vlan_tag_lan1" value="<% nvram_get_x("Layer3Forwarding", "vlan_tag_lan1"); %>" />
<input type="hidden" name="vlan_tag_lan2" value="<% nvram_get_x("Layer3Forwarding", "vlan_tag_lan2"); %>" />
<input type="hidden" name="vlan_tag_lan3" value="<% nvram_get_x("Layer3Forwarding", "vlan_tag_lan3"); %>" />
<input type="hidden" name="vlan_tag_lan4" value="<% nvram_get_x("Layer3Forwarding", "vlan_tag_lan4"); %>" />

<table border="0" class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
	<td height="430" valign="top">
	  <div id="tabMenu" class="submenuBlock"></div><br />
	  
	  <!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top">
			<table width="98%" border="0" align="center" cellpadding="5" cellspacing="0" class="FormTitle">
				<thead>
				<tr>
					<td><#menu5_3#> - <#menu5_3_1#></td>
				</tr>
				</thead>
				
				<tbody>
				<tr>
					<td bgcolor="#FFFFFF"><#Layer3Forwarding_x_ConnectionType_sectiondesc#></td>
				</tr>
				</tbody>	
				
				<tr>
					<th bgcolor="#FFFFFF">
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
							<tr>
								<th width="40%"><#Layer3Forwarding_x_ConnectionType_itemname#></th>
								<td align="left">
									<select class="input" name="wan_proto" onchange="change_wan_type(this.value);fixed_change_wan_type(this.value);">
										<option value="static" <% nvram_match_x("Layer3Forwarding", "wan_proto", "static", "selected"); %>>IPoE: <#BOP_ctype_title5#></option>
										<option value="dhcp" <% nvram_match_x("Layer3Forwarding", "wan_proto", "dhcp", "selected"); %>>IPoE: <#BOP_ctype_title1#></option>
										<option value="pppoe" <% nvram_match_x("Layer3Forwarding", "wan_proto", "pppoe", "selected"); %>>PPPoE</option>
										<option value="pptp" <% nvram_match_x("Layer3Forwarding", "wan_proto", "pptp", "selected"); %>>PPTP</option>
										<option value="l2tp" <% nvram_match_x("Layer3Forwarding", "wan_proto", "l2tp", "selected"); %>>L2TP</option>
									</select>
								</td>
							</tr>
							<tr>
								<th><#Enable_NAT#></th>
								<td style="font-weight:normal;" align="left">
									<input type="radio" name="sw_mode" class="input" value="1" onclick="change_nat(1);" <% nvram_match_x("IPConnection", "sw_mode", "1", "checked"); %>/><#checkbox_Yes#>
									<input type="radio" name="sw_mode" class="input" value="4" onclick="change_nat(4);" <% nvram_match_x("IPConnection", "sw_mode", "4", "checked"); %>/><#checkbox_No#>
								</td>
							</tr>
							<tr id="hw_nat_row">
								<th><#HardwareNAT#> (IPoE/PPPoE):</th>
								<td align="left">
									<select name="hw_nat_mode" class="input">
										<option value="0" <% nvram_match_x("IPConnection", "hw_nat_mode", "0", "selected"); %>>Enable offload for LAN</option>
										<option value="1" <% nvram_match_x("IPConnection", "hw_nat_mode", "1", "selected"); %>>Enable offload for LAN/Wi-Fi</option>
										<option value="2" <% nvram_match_x("IPConnection", "hw_nat_mode", "2", "selected"); %>>Disable (Slow)</option>
									</select>
								</td>
							</tr>
							 <tr id="sw_nat_row">
								<th><#SoftwareNAT#></th>
								<td align="left">
									<select name="sw_nat_mode" class="input">
										<option value="0" <% nvram_match_x("IPConnection", "sw_nat_mode", "0", "selected"); %>>Default NAT</option>
										<option value="1" <% nvram_match_x("IPConnection", "sw_nat_mode", "1", "selected"); %>>Fast NAT path (PPTP/L2TP boost)</option>
									</select>
								</td>
							</tr>
							<tr>
								<th><#Enable_IGD_UPnP#></th>
								<td align="left">
									<select name="upnp_enable" class="input">
										<option value="0" <% nvram_match_x("LANHostConfig","upnp_enable", "0", "selected"); %>><#checkbox_No#></option>
										<option value="1" <% nvram_match_x("LANHostConfig","upnp_enable", "1", "selected"); %>>UPnP</option>
										<option value="2" <% nvram_match_x("LANHostConfig","upnp_enable", "2", "selected"); %>>UPnP/NAT-PMP</option>
									</select>
								</td>
							</tr>
							<tr id="wan_poller_row">
								<th><#WAN_Poller#></th>
								<td style="font-weight:normal;" align="left">
									<input type="radio" name="gw_arp_ping" class="input" value="1" <% nvram_match_x("IPConnection", "gw_arp_ping", "1", "checked"); %>/><#checkbox_Yes#>
									<input type="radio" name="gw_arp_ping" class="input" value="0" <% nvram_match_x("IPConnection", "gw_arp_ping", "0", "checked"); %>/><#checkbox_No#>
								</td>
							</tr>
						</table>
					</td>
				</tr>	
				<tr id="dhcp_sect">
					<td bgcolor="#FFFFFF" id="ip_sect">
						<table  width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<td colspan="2"><#IPConnection_ExternalIPAddress_sectionname#></td>
							</tr>
							</thead>
							<tr>
								<th width="40%"><#Layer3Forwarding_x_DHCPClient_itemname#></th>
								<td style="font-weight:normal;" align="left">
									<input type="radio" name="x_DHCPClient" class="input" value="1" onclick="change_wan_dhcp_enable(0);" <% nvram_match_x("Layer3Forwarding", "x_DHCPClient", "1", "checked"); %>/><#checkbox_Yes#>
									<input type="radio" name="x_DHCPClient" class="input" value="0" onclick="change_wan_dhcp_enable(0);" <% nvram_match_x("Layer3Forwarding", "x_DHCPClient", "0", "checked"); %>/><#checkbox_No#>
								</td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
								<td><input type="text" name="wan_ipaddr" maxlength="15" class="input" size="15" value="<% nvram_get_x("IPConnection","wan_ipaddr"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
								<td><input type="text" name="wan_netmask" maxlength="15" class="input" size="15" value="<% nvram_get_x("IPConnection","wan_netmask"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
							</tr>
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
								<td><input type="text" name="wan_gateway" maxlength="15" class="input" size="15" value="<% nvram_get_x("IPConnection","wan_gateway"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
							</tr>
						</table>
					</td>
	  		</tr>
	  		<tr>
	    		<td bgcolor="#FFFFFF" id="dns_sect">
				<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
          		<thead>
            	<tr>
              <td colspan="2"><#IPConnection_x_DNSServerEnable_sectionname#></td>
            	</tr>
          		</thead>
         		<tr>
            			<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
				<td style="font-weight:normal;" align="left">
  					<input type="radio" name="wan_dnsenable_x" value="1" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', 1)" <% nvram_match_x("IPConnection", "wan_dnsenable_x", "1", "checked"); %>/><#checkbox_Yes#>
  					<input type="radio" name="wan_dnsenable_x" value="0" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', 0)" <% nvram_match_x("IPConnection", "wan_dnsenable_x", "0", "checked"); %>/><#checkbox_No#>
				</td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,13);"><#IPConnection_x_DNSServer1_itemname#></a></th>
            		<td>
            		  <input type="text" maxlength="15" class="input" size="15" name="wan_dns1_x" value="<% nvram_get_x("IPConnection","wan_dns1_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
            		  </td>
          		</tr>
          		<tr>
            		<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,14);"><#IPConnection_x_DNSServer2_itemname#></a></th>
            		<td>
            		   <input type="text" maxlength="15" class="input" size="15" name="wan_dns2_x" value="<% nvram_get_x("IPConnection","wan_dns2_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
            		</td>
          		</tr>
        		</table>
        	</td>
	  		</tr>
	  
	  		<tr id="account_sect">
	    		<td bgcolor="#FFFFFF">
		  	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
            	<thead>
            	<tr>
              	<td colspan="2"><#PPPConnection_UserName_sectionname#></td>
            	</tr>
            	</thead>
            	<tr>
              	<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,4);"><#PPPConnection_UserName_itemname#></a></th>
              	<td>
              	   <input type="text" maxlength="64" class="input" size="32" name="wan_pppoe_username" value="<% nvram_get_x("PPPConnection","wan_pppoe_username"); %>" onkeypress="return is_string(this)"/>
              	</td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,5);"><#PPPConnection_Password_itemname#></a></th>
              	<td><input type="password" maxlength="64" class="input" size="32" name="wan_pppoe_passwd" value="<% nvram_get_x("PPPConnection","wan_pppoe_passwd"); %>"/></td>
            	</tr>
			<tr style="display:none">
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
              	<td>
                	<input type="text" maxlength="10" class="input" size="10" name="wan_pppoe_idletime" value="<% nvram_get_x("PPPConnection","wan_pppoe_idletime"); %>" onkeypress="return is_number(this)"/>
                	<input type="checkbox" style="margin-left:30;display:none;" name="wan_pppoe_idletime_check" value="" onclick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_idletime', '1')"/>
              	</td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
              	<td>
              	    <input type="text" maxlength="5" size="5" name="wan_pppoe_mtu" class="input" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mtu"); %>" onkeypress="return is_number(this)"/>
              	</td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
              	<td>
              	   <input type="text" maxlength="5" size="5" name="wan_pppoe_mru" class="input" value="<% nvram_get_x("PPPConnection", "wan_pppoe_mru"); %>" onkeypress="return is_number(this)"/>
              	</td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
              	<td><input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_service" value="<% nvram_get_x("PPPConnection","wan_pppoe_service"); %>" onkeypress="return is_string(this)"/></td>
            	</tr>
            	<tr>
              	<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
              	<td><input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_ac" value="<% nvram_get_x("PPPConnection","wan_pppoe_ac"); %>" onkeypress="return is_string(this)"/></td>
            	</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,17);"><#PPPConnection_x_PPTPOptions_itemname#></a></th>
			<td>
				<select name="wan_pptp_options_x" class="input">
					<option value="" <% nvram_match_x("Layer3Forwarding","wan_pptp_options_x", "","selected"); %>>Auto</option>
					<option value="nomppe" <% nvram_match_x("Layer3Forwarding","wan_pptp_options_x", "nomppe","selected"); %>>No Encryption/Compression</option>
					<option value="+mppe-40" <% nvram_match_x("Layer3Forwarding","wan_pptp_options_x", "+mppe-40","selected"); %>>Encryption MPPE 40 Bit</option>
					<option value="+mppe-128" <% nvram_match_x("Layer3Forwarding","wan_pptp_options_x", "+mppe-128","selected"); %>>Encryption MPPE 128 Bit</option>
				</select>
			</td>
		</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,18);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
			<td>
				<input type="text" name="wan_pppoe_options_x" value="<% nvram_get_x("PPPConnection", "wan_pppoe_options_x"); %>" class="input" maxlength="255" size="32" onKeyPress="return is_string(this)" onBlur="validate_string(this)"/>
			</td>
		</tr>
		<tr>
			<th><#PPP_AdaptiveLCP#></th>
			<td style="font-weight:normal;" align="left">
				<input type="radio" name="wan_pppoe_lcpa" value="1" <% nvram_match_x("PPPConnection", "wan_pppoe_lcpa", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="wan_pppoe_lcpa" value="0" <% nvram_match_x("PPPConnection", "wan_pppoe_lcpa", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr>
			<th><#PPP_LimitCPU#></th>
			<td>
				<select name="wan_pppoe_cpul" class="input">
					<option value="0" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "0","selected"); %>><#checkbox_No#></option>
					<option value="2500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "2500","selected"); %>>2500 cycles</option>
					<option value="3000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "3000","selected"); %>>3000 cycles</option>
					<option value="3500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "3500","selected"); %>>3500 cycles</option>
					<option value="4000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "4000","selected"); %>>4000 cycles</option>
					<option value="4500" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "4500","selected"); %>>4500 cycles</option>
					<option value="5000" <% nvram_match_x("PPPConnection","wan_pppoe_cpul", "5000","selected"); %>>5000 cycles</option>
				</select>
			</td>
		</tr>
          </table>
	  </td>
	  </tr>
	  <tr>
	    <td bgcolor="#FFFFFF" id="isp_sect">
		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<thead>
		<tr>
			<td colspan="2"><#PPPConnection_x_HostNameForISP_sectionname#></td>
		</tr>
		</thead>
		<tr id="pppoe_dhcp_x" style="display:none;">
			<th width="40%">PPPoE Dual Access:</th>
			<td align="left">
			<select name="pppoe_dhcp_route" class="input">
				<option value="0" <% nvram_match_x("PPPConnection", "pppoe_dhcp_route", "0", "selected"); %>><#checkbox_No#></option>
				<option value="1" <% nvram_match_x("PPPConnection", "pppoe_dhcp_route", "1", "selected"); %>>DHCP or Static</option>
				<option value="2" <% nvram_match_x("PPPConnection", "pppoe_dhcp_route", "2", "selected"); %>>ZeroConf</option>
			</select>
		    </td>
		</tr>
		<tr>
			<th width="40%"><#ISP_Authentication_mode#></th>
			<td align="left">
			<select name="wan_auth_mode" class="input" onChange="AuthSelection(this.value)">
				<option value="0" <% nvram_match_x("Layer3Forwarding", "wan_auth_mode", "0", "selected"); %>><#checkbox_No#></option>
				<option value="1" <% nvram_match_x("Layer3Forwarding", "wan_auth_mode", "1", "selected"); %>>ISP KABiNET</option>
				<option value="2" <% nvram_match_x("Layer3Forwarding", "wan_auth_mode", "2", "selected"); %>>802.1x EAPoL-MD5</option>
			</select>
			</td>
		</tr>
		<tr id="auth_user_x">
			<th><#ISP_Authentication_user#></th>
			<td align="left">
				<input type="text" maxlength="64" class="input" size="32" name="wan_auth_user" value="<% nvram_get_x("Layer3Forwarding","wan_auth_user"); %>" onKeyPress="return is_string(this)"/>
			</td>
		</tr>
		<tr id="auth_pass_x">
			<th><#ISP_Authentication_pass#></th>
			<td align="left">
				<input type="password" maxlength="64" class="input" size="32" name="wan_auth_pass" value="<% nvram_get_x("Layer3Forwarding","wan_auth_pass"); %>" onKeyPress="return is_string(this)"/>
			</td>
		</tr>
		<tr>
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,19);"><#PPPConnection_x_HeartBeat_itemname#></a></th>
			<td>
				<input type="text" name="wan_heartbeat_x" class="input" maxlength="256" size="32" value="<% nvram_get_x("PPPConnection","wan_heartbeat_x"); %>" onKeyPress="return is_string(this)"/>
			</td>
		</tr>
		<tr id="hostname_x">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,15);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
			<td>
				<input type="text" name="wan_hostname" class="input" maxlength="32" size="32" value="<% nvram_get_x("PPPConnection","wan_hostname"); %>" onkeypress="return is_string(this)"/>
			</td>
		</tr>
		<tr id="clone_mac_x">
			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,16);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
			<td>
				<input type="text" name="wan_hwaddr_x" class="input" maxlength="12" size="15" value="<% nvram_get_x("PPPConnection","wan_hwaddr_x"); %>" onKeyPress="return is_hwaddr()"/>
				<input type="button" class="button" onclick="showMAC();" value="<#BOP_isp_MACclone#>"/>
			</td>
		</tr>
	</table>
	</td>
	</tr>
	<tr>
	<td bgcolor="#FFFFFF" id="vlan_sect">
	<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
		<thead>
		<tr>
			<td colspan="2"><#WAN_Bridge#></td>
		</tr>
		</thead>
		<tr>
			<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(7,21);"><#Layer3Forwarding_x_STB_itemname#></a></th>
			<td align="left">
			<select name="wan_stb_x" class="input" onChange="change_stb_port_and_vlan();">
				<option value="0" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "0", "selected"); %>><#checkbox_No#></option>
				<option value="1" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "1", "selected"); %>>LAN1</option>
				<option value="2" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "2", "selected"); %>>LAN2</option>
				<option value="3" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "3", "selected"); %>>LAN3</option>
				<option value="4" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "4", "selected"); %>>LAN4</option>
				<option value="5" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "5", "selected"); %>>LAN3 & LAN4</option>
				<option value="6" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "6", "selected"); %>>LAN1 & LAN2</option>
				<option value="7" <% nvram_match_x("Layer3Forwarding", "wan_stb_x", "7", "selected"); %>>LAN1 & LAN2 & LAN3</option>
			</select>
			</td>
		</tr>
		<tr id="wan_stb_iso">
			<th><#STB_Isolation#></th>
			<td align="left">
			<select name="wan_stb_iso" class="input" onChange="change_stb_port_and_vlan();">
				<option value="0" <% nvram_match_x("Layer3Forwarding", "wan_stb_iso", "0", "selected"); %>><#checkbox_No#></option>
				<option value="1" <% nvram_match_x("Layer3Forwarding", "wan_stb_iso", "1", "selected"); %>><#STB_IsolationItem1#></option>
				<option value="2" <% nvram_match_x("Layer3Forwarding", "wan_stb_iso", "2", "selected"); %>><#STB_IsolationItem2#></option>
			</select>
			</td>
		</tr>
		<tr id="wan_vlan_filter">
			<th><#WAN_FilterVLAN#></th>
			<td style="font-weight:normal;" align="left">
				<input type="radio" name="vlan_filter" value="1" onClick="change_stb_port_and_vlan();" <% nvram_match_x("Layer3Forwarding", "vlan_filter", "1", "checked"); %>/><#checkbox_Yes#>
				<input type="radio" name="vlan_filter" value="0" onClick="change_stb_port_and_vlan();" <% nvram_match_x("Layer3Forwarding", "vlan_filter", "0", "checked"); %>/><#checkbox_No#>
			</td>
		</tr>
		<tr id="vlan_cpu">
			<th>VLAN CPU:</th>
			<td>
				VID&nbsp;<input type="text" name="vlan_vid_cpu" class="input" size="4" maxlength="4" value="<% nvram_get_x("Layer3Forwarding", "vlan_vid_cpu"); %>"/>&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="vlan_pri_cpu" class="input" size="2" maxlength="1" value="<% nvram_get_x("Layer3Forwarding", "vlan_pri_cpu"); %>"/>
			</td>
		</tr>
		<tr id="vlan_lan1">
			<th>VLAN LAN1:</th>
			<td>
				VID&nbsp;<input type="text" name="vlan_vid_lan1" class="input" size="4" maxlength="4" value="<% nvram_get_x("Layer3Forwarding", "vlan_vid_lan1"); %>"/>&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="vlan_pri_lan1" class="input" size="2" maxlength="1" value="<% nvram_get_x("Layer3Forwarding", "vlan_pri_lan1"); %>"/>&nbsp;&nbsp;
				<input type="checkbox" name="untag_lan1" value="" style="margin-left:10;" onclick="click_untag_lan(1);" <% nvram_match_x("Layer3Forwarding", "vlan_tag_lan1", "0", "checked"); %>/><#UntagVLAN#>
			</td>
		</tr>
		<tr id="vlan_lan2">
			<th>VLAN LAN2:</th>
			<td>
				VID&nbsp;<input type="text" name="vlan_vid_lan2" class="input" size="4" maxlength="4" value="<% nvram_get_x("Layer3Forwarding", "vlan_vid_lan2"); %>"/>&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="vlan_pri_lan2" class="input" size="2" maxlength="1" value="<% nvram_get_x("Layer3Forwarding", "vlan_pri_lan2"); %>"/>&nbsp;&nbsp;
				<input type="checkbox" name="untag_lan2" value="" style="margin-left:10;" onclick="click_untag_lan(2);" <% nvram_match_x("Layer3Forwarding", "vlan_tag_lan2", "0", "checked"); %>/><#UntagVLAN#>
			</td>
		</tr>
		<tr id="vlan_lan3">
			<th>VLAN LAN3:</th>
			<td>
				VID&nbsp;<input type="text" name="vlan_vid_lan3" class="input" size="4" maxlength="4" value="<% nvram_get_x("Layer3Forwarding", "vlan_vid_lan3"); %>"/>&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="vlan_pri_lan3" class="input" size="2" maxlength="1" value="<% nvram_get_x("Layer3Forwarding", "vlan_pri_lan3"); %>"/>&nbsp;&nbsp;
				<input type="checkbox" name="untag_lan3" value="" style="margin-left:10;" onclick="click_untag_lan(3);" <% nvram_match_x("Layer3Forwarding", "vlan_tag_lan3", "0", "checked"); %>/><#UntagVLAN#>
			</td>
		</tr>
		<tr id="vlan_lan4">
			<th>VLAN LAN4:</th>
			<td>
				VID&nbsp;<input type="text" name="vlan_vid_lan4" class="input" size="4" maxlength="4" value="<% nvram_get_x("Layer3Forwarding", "vlan_vid_lan4"); %>"/>&nbsp;&nbsp;
				PRIO&nbsp;<input type="text" name="vlan_pri_lan4" class="input" size="2" maxlength="1" value="<% nvram_get_x("Layer3Forwarding", "vlan_pri_lan4"); %>"/>&nbsp;&nbsp;
				<input type="checkbox" name="untag_lan4" value="" style="margin-left:10;" onclick="click_untag_lan(4);" <% nvram_match_x("Layer3Forwarding", "vlan_tag_lan4", "0", "checked"); %>/><#UntagVLAN#>
			</td>
		</tr>
	</table>
	</td>
	</tr>

	<tr>
		<td bgcolor="#FFFFFF" colspan="2" align="right">
			<input name="button" type="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/>
		</td>
	</tr>
</table>
</td>
</form>

					<td id="help_td" style="width:15px;" valign="top">
						<form name="hint_form"></form>
            <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>">
            	<img src="images/help.gif">
            </div>
						<div id="hintofPM" style="display:none;">
							<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
								<thead>
								<tr>
									<td>
										<div id="helpname" class="AiHintTitle"></div>
										<a href="javascript:void(0);" onclick="closeHint()">
											<img src="images/button-close.gif" class="closebutton">
										</a>
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
					</td>
				</tr>
			</table>
		</td>
		<!--===================================Ending of Main Content===========================================-->
	
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
