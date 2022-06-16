<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_3_1#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('gw_arp_ping');
	init_itoggle('x_DHCPClient', change_wan_dhcp_auto);
	init_itoggle('wan_dnsenable_x', change_wan_dns_auto);
	init_itoggle('vlan_filter', change_stb_port_and_vlan);
});

</script>
<script>

<% login_state_hook(); %>

var client_mac = login_mac_str();

var original_wan_type = wan_proto;
var original_wan_dhcp_auto = parseInt('<% nvram_get_x("", "x_DHCPClient"); %>');
var original_wan_dns_auto = parseInt('<% nvram_get_x("", "wan_dnsenable_x"); %>');
var original_wan_src_phy = '<% nvram_get_x("", "wan_src_phy"); %>';

function initial(){
	show_banner(1);
	show_menu(5,4,1);
	show_footer();

	if (!support_ipv4_ppe()){
		showhide_div('row_hwnat', 0);
	}

	if (support_sfe()){
		showhide_div('row_sfe', 1);
	}

	var o1 = document.form.wan_auth_mode;
	if (!support_peap_ssl()){
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
	}

	var o2 = document.form.wan_stb_x;
	var num_ephy = support_num_ephy();
	if (num_ephy < 5){
		o2.remove(7);
		o2.remove(5);
		o2.remove(4);
	}
	if (num_ephy < 4){
		o2.remove(4);
		o2.remove(3);
	}
	if (num_ephy < 3){
		o2.remove(2);
	}

	var switch_type = support_switch_type();
	if (switch_type == 10 || switch_type == 11){
		document.form.wan_stb_iso.remove(2);
	}

	change_wan_type(document.form.wan_proto.value, 0);
	fixed_change_wan_type(document.form.wan_proto.value);

	AuthSelection(document.form.wan_auth_mode.value);

	change_stb_port_and_vlan();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.next_page.value = "";
		document.form.action_mode.value = " Apply ";
		document.form.submit();
	}
}

function validForm(){
	var lan_addr = document.form.lan_ipaddr.value;
	var lan_mask = document.form.lan_netmask.value;
	var wan_proto = document.form.wan_proto.value;
	var wan_stb_x = document.form.wan_stb_x.value;
	var min_vlan = support_min_vlan();
	var addr_obj;
	var mask_obj;
	var gate_obj;
	var vlan_obj;

	if($("tbl_dhcp_sect").style.display != "none" && !document.form.x_DHCPClient[0].checked){
		addr_obj = document.form.wan_ipaddr;
		mask_obj = document.form.wan_netmask;
		gate_obj = document.form.wan_gateway;
		
		if(!validate_ipaddr_final(addr_obj, 'wan_ipaddr')
				|| !validate_ipaddr_final(mask_obj, 'wan_netmask')
				|| !validate_ipaddr_final(gate_obj, 'wan_gateway')
				)
			return false;
		
		if(gate_obj.value == addr_obj.value){
			alert("<#IPConnection_warning_WANIPEQUALGatewayIP#>");
			gate_obj.select();
			gate_obj.focus();
			return false;
		}
		
		if(matchSubnet2(lan_addr, lan_mask, addr_obj.value, mask_obj.value)){
			alert("<#JS_validsubnet#>");
			mask_obj.focus();
			mask_obj.select();
			return false;
		}
		
		if(!validate_range(document.form.wan_mtu, 1300, 1500))
			return false;
	}

	if(!document.form.wan_dnsenable_x[0].checked){
		if(!validate_ipaddr_final(document.form.wan_dns1_x, 'wan_dns_x'))
			return false;
		if(!validate_ipaddr_final(document.form.wan_dns2_x, 'wan_dns_x'))
			return false;
		if(!validate_ipaddr_final(document.form.wan_dns3_x, 'wan_dns_x'))
			return false;
	}

	if(wan_proto == "pppoe" || wan_proto == "pptp" || wan_proto == "l2tp"){
		if(!validate_string(document.form.wan_pppoe_username)
				|| !validate_string(document.form.wan_pppoe_passwd))
			return false;
	}

	if(wan_proto == "pppoe"){
		if(!validate_range(document.form.wan_pppoe_mtu, 1000, 1492)
				|| !validate_range(document.form.wan_pppoe_mru, 1000, 1492))
			return false;
		
		if(!validate_string(document.form.wan_pppoe_service)
				|| !validate_string(document.form.wan_pppoe_ac))
			return false;
		
		if(!validate_range(document.form.wan_pppoe_idletime, 0, 86400))
			return false;
	}
	else if(wan_proto == "pptp"){
		if(!validate_range(document.form.wan_pptp_mtu, 1000, 1476)
				|| !validate_range(document.form.wan_pptp_mru, 1000, 1500))
			return false;
		
		if(document.form.wan_ppp_peer.value.length > 0)
			if(!validate_string(document.form.wan_ppp_peer))
				return false;
	}
	else if(wan_proto == "l2tp"){
		if(!validate_range(document.form.wan_l2tp_mtu, 1000, 1460)
				|| !validate_range(document.form.wan_l2tp_mru, 1000, 1500))
			return false;
		
		if(document.form.wan_ppp_peer.value.length > 0)
			if(!validate_string(document.form.wan_ppp_peer))
				return false;
	}

	if(document.form.wan_hwaddr_x.value.length > 0)
		if(!validate_hwaddr(document.form.wan_hwaddr_x))
			return false;

	if(document.form.vlan_filter[0].checked){
		vlan_obj = document.form.vlan_vid_cpu;
		if(vlan_obj.value.length > 0){
			if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
				return false;
			if(!validate_range(document.form.vlan_pri_cpu, 0, 7))
				return false;
		}
		
		vlan_obj = document.form.vlan_vid_iptv;
		if(vlan_obj.value.length > 0){
			if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
				return false;
			if(!validate_range(document.form.vlan_pri_iptv, 0, 7))
				return false;
		}
		
		if (wan_stb_x == "1" || wan_stb_x == "6" || wan_stb_x == "7"){
			vlan_obj = document.form.vlan_vid_lan1;
			if(vlan_obj.value.length > 0){
				if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan1, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "2" || wan_stb_x == "6" || wan_stb_x == "7"){
			vlan_obj = document.form.vlan_vid_lan2;
			if(vlan_obj.value.length > 0){
				if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan2, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "3" || wan_stb_x == "5" || wan_stb_x == "7"){
			vlan_obj = document.form.vlan_vid_lan3;
			if(vlan_obj.value.length > 0){
				if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan3, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "4" || wan_stb_x == "5"){
			vlan_obj = document.form.vlan_vid_lan4;
			if(vlan_obj.value.length > 0){
				if(vlan_obj.value!="2" && !validate_range(vlan_obj, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan4, 0, 7))
					return false;
			}
		}
		
		if (document.form.viptv_mode.value == "2"){
			addr_obj = document.form.viptv_ipaddr;
			mask_obj = document.form.viptv_netmask;
			
			if(!validate_ipaddr_final(addr_obj, 'viptv_ipaddr')
				|| !validate_ipaddr_final(mask_obj, 'viptv_netmask')
				)
				return false;
			
			if(matchSubnet2(lan_addr, lan_mask, addr_obj.value, mask_obj.value)){
				alert("<#JS_validsubnet#>");
				mask_obj.focus();
				mask_obj.select();
				return false;
			}
		}
	}

	return true;
}

function done_validating(action){
	refreshpage();
}

function change_wan_type(wan_type, flag){
	change_wan_dhcp_enable(wan_type);
	change_wan_dns_enable(wan_type);

	var is_static = (wan_type == "static") ? 1 : 0;
	var is_pppoe = (wan_type == "pppoe") ? 1 : 0;
	var is_pptp = (wan_type == "pptp") ? 1 : 0;
	var is_l2tp = (wan_type == "l2tp") ? 1 : 0;
	var is_dhcp = !(is_static||is_pppoe||is_pptp||is_l2tp);
	var o_mtu, o_mru;

	if(is_pppoe){
		o_mtu = document.form.wan_pppoe_mtu;
		o_mru = document.form.wan_pppoe_mru;
		if (parseInt(o_mtu.value) > 1492)
			o_mtu.value = "1492";
		if (parseInt(o_mru.value) > 1492)
			o_mru.value = "1492";
	}else if(is_pptp){
		o_mtu = document.form.wan_pptp_mtu;
		o_mru = document.form.wan_pptp_mru;
		if (parseInt(o_mtu.value) > 1476)
			o_mtu.value = "1476";
		if (parseInt(o_mru.value) > 1500)
			o_mru.value = "1500";
	}else if(is_l2tp){
		o_mtu = document.form.wan_l2tp_mtu;
		o_mru = document.form.wan_l2tp_mru;
		if (parseInt(o_mtu.value) > 1460)
			o_mtu.value = "1460";
		if (parseInt(o_mru.value) > 1500)
			o_mru.value = "1500";
	}

	showhide_div("row_wan_poller", is_dhcp);
	showhide_div("row_pppoe_dhcp", is_pppoe);
	showhide_div("row_dhcp_toggle", is_pppoe||is_pptp||is_l2tp);
	showhide_div("row_dns_toggle", !is_static);
	showhide_div("tbl_vpn_control", is_pppoe||is_pptp||is_l2tp);
	showhide_div("row_auth_type", is_static||is_dhcp);

	if(is_pppoe||is_pptp||is_l2tp){
		$("dhcp_sect_desc").innerHTML = "<#WAN_MAN_desc#>";
		$("dhcp_auto_desc").innerHTML = "<#WAN_MAN_DHCP#>";
		
		var dhcp_sect = 1;
		if (is_pppoe && document.form.wan_pppoe_man.value != "1")
			dhcp_sect = 0;
		showhide_div("tbl_dhcp_sect", dhcp_sect);
		
		showhide_div("row_ppp_peer", is_pptp||is_l2tp);
		showhide_div("row_ppp_mppe", is_pptp||is_l2tp);
		showhide_div("row_pppoe_svc", is_pppoe);
		showhide_div("row_pppoe_it", is_pppoe);
		showhide_div("row_pppoe_ac", is_pppoe);
		showhide_div("row_pppoe_mtu", is_pppoe);
		showhide_div("row_pppoe_mru", is_pppoe);
		showhide_div("row_pptp_mtu", is_pptp);
		showhide_div("row_pptp_mru", is_pptp);
		showhide_div("row_l2tp_mtu", is_l2tp);
		showhide_div("row_l2tp_mru", is_l2tp);
		showhide_div("row_l2tp_cli", is_l2tp&&found_app_l2tp());
	}else{
		$("dhcp_sect_desc").innerHTML = "<#IPConnection_ExternalIPAddress_sectionname#>";
		$("dhcp_auto_desc").innerHTML = "<#Layer3Forwarding_x_DHCPClient_itemname#>";
		
		showhide_div("tbl_dhcp_sect", is_static);
	}

	AuthSelection(document.form.wan_auth_mode.value);
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

	change_wan_dns_enable(wan_type);

	if(wan_type == "static"){
		inputRCtrl2(document.form.wan_dnsenable_x, 1);
		$j('#wan_dnsenable_x_on_of').iState(0);
		
		set_wan_dns_auto(0);
		
		if(flag == true && document.form.wan_dns1_x.value.length == 0)
			document.form.wan_dns1_x.focus();
	}
	else{
		var dns_auto = original_wan_dns_auto;
		inputRCtrl2(document.form.wan_dnsenable_x, !dns_auto);
		$j('#wan_dnsenable_x_on_of').iState(dns_auto);
		
		set_wan_dns_auto(dns_auto);
		
		if(flag == true && document.form.wan_dns1_x.value.length == 0 && !document.form.wan_dnsenable_x[0].checked)
			document.form.wan_dns1_x.focus();
	}
}

function set_wan_dns_auto(use_auto){
	inputCtrl(document.form.wan_dns1_x, !use_auto);
	inputCtrl(document.form.wan_dns2_x, !use_auto);
	inputCtrl(document.form.wan_dns3_x, !use_auto);

	showhide_div("row_wan_dns1", !use_auto);
	showhide_div("row_wan_dns2", !use_auto);
	showhide_div("row_wan_dns3", !use_auto);
}

function set_wan_dhcp_auto(use_auto){
	inputCtrl(document.form.wan_ipaddr, !use_auto);
	inputCtrl(document.form.wan_netmask, !use_auto);
	inputCtrl(document.form.wan_gateway, !use_auto);
	inputCtrl(document.form.wan_mtu, !use_auto);

	showhide_div("row_wan_ipaddr", !use_auto);
	showhide_div("row_wan_netmask", !use_auto);
	showhide_div("row_wan_gateway", !use_auto);
	showhide_div("row_wan_mtu", !use_auto);

	var v = use_auto;
	if (document.form.wan_proto.value == "pppoe" && document.form.wan_pppoe_man.value != "1")
		v = 0;

	showhide_div("row_hostname", v);
	showhide_div("row_vci", v);
}

function change_pppoe_man(man_type){
	if(document.form.wan_proto.value == "pppoe"){
		showhide_div("tbl_dhcp_sect", (man_type == "1")?1:0);
		set_wan_dhcp_auto(document.form.x_DHCPClient[0].checked);
	}
}

function change_wan_dhcp_auto(){
	var v = document.form.x_DHCPClient[0].checked;
	set_wan_dhcp_auto(v);
}

function change_wan_dns_auto(use_auto){
	var v = document.form.wan_dnsenable_x[0].checked;
	set_wan_dns_auto(v);
}

function change_wan_dhcp_enable(wan_type){
	if (wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		var dhcp_auto = original_wan_dhcp_auto;
		inputRCtrl2(document.form.x_DHCPClient, !dhcp_auto);
		$j('#x_DHCPClient_on_of').iState(dhcp_auto);
		
		inputCtrl(document.form.x_DHCPClient[0], 1);
		inputCtrl(document.form.x_DHCPClient[1], 1);
		$j('input[name="x_DHCPClient"]').removeAttr('disabled');
		$j('#x_DHCPClient_on_of').iClickable(1);
		
		set_wan_dhcp_auto(dhcp_auto);
	}
	else if(wan_type == "static"){
		inputRCtrl2(document.form.x_DHCPClient, 1);
		$j('#x_DHCPClient_on_of').iState(0);
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		$j('input[name="x_DHCPClient"]').attr('disabled','disabled');
		$j('#x_DHCPClient_on_of').iClickable(0);
		
		set_wan_dhcp_auto(0);
	}
	else{
		inputRCtrl2(document.form.x_DHCPClient, 0);
		$j('#x_DHCPClient_on_of').iState(1);
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		$j('input[name="x_DHCPClient"]').attr('disabled','disabled');
		$j('#x_DHCPClient_on_of').iClickable(0);
		
		set_wan_dhcp_auto(1);
	}
}

function change_wan_dns_enable(wan_type){
	if(wan_type == "static"){
		inputCtrl(document.form.wan_dnsenable_x[0], 0);
		inputCtrl(document.form.wan_dnsenable_x[1], 0);
		$j('input[name="wan_dnsenable_x"]').attr('disabled','disabled');
		$j('#wan_dnsenable_x_on_of').iClickable(0);
	}
	else{
		inputCtrl(document.form.wan_dnsenable_x[0], 1);
		inputCtrl(document.form.wan_dnsenable_x[1], 1);
		$j('input[name="wan_dnsenable_x"]').removeAttr('disabled');
		$j('#wan_dnsenable_x_on_of').iClickable(1);
	}
}

function change_stb_port_and_vlan(){
	var wan_stb_x = parseInt(document.form.wan_stb_x.value);
	var vlan_filter = document.form.vlan_filter[0].checked;
	var vlan_l1 = 0, vlan_l2 = 0, vlan_l3 = 0, vlan_l4 = 0;
	var o_wsp = document.form.wan_src_phy;

	free_options(o_wsp);
	add_option(o_wsp, "WAN", "0", 0);

	showhide_div("wan_stb_iso", (wan_stb_x != 0));
	showhide_div("wan_src_phy", (wan_stb_x != 0));

	if(wan_stb_x == 0) {
		o_wsp.SelectedIndex = 0;
	}
	else if(wan_stb_x == 1) {
		vlan_l1 = vlan_filter;
		add_option(o_wsp, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
	}
	else if(wan_stb_x == 2) {
		vlan_l2 = vlan_filter;
		add_option(o_wsp, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
	}
	else if(wan_stb_x == 3) {
		vlan_l3 = vlan_filter;
		add_option(o_wsp, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
	}
	else if(wan_stb_x == 4) {
		vlan_l4 = vlan_filter;
		add_option(o_wsp, "LAN4", "4", (original_wan_src_phy == 4) ? 1 : 0);
	}
	else if(wan_stb_x == 5) {
		vlan_l3 = vlan_filter;
		vlan_l4 = vlan_filter;
		add_option(o_wsp, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
		add_option(o_wsp, "LAN4", "4", (original_wan_src_phy == 4) ? 1 : 0);
	}
	else if(wan_stb_x == 6) {
		vlan_l1 = vlan_filter;
		vlan_l2 = vlan_filter;
		add_option(o_wsp, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
		add_option(o_wsp, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
	}
	else if(wan_stb_x == 7) {
		vlan_l1 = vlan_filter;
		vlan_l2 = vlan_filter;
		vlan_l3 = vlan_filter;
		add_option(o_wsp, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
		add_option(o_wsp, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
		add_option(o_wsp, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
	}

	showhide_div("vlan_inet", vlan_filter);
	showhide_div("vlan_iptv", vlan_filter);
	showhide_div("vlan_lan1", vlan_l1);
	showhide_div("vlan_lan2", vlan_l2);
	showhide_div("vlan_lan3", vlan_l3);
	showhide_div("vlan_lan4", vlan_l4);

	change_viptv_tag(vlan_filter);
}

function change_viptv_tag(v){
	if (v)
		v = (document.form.vlan_vid_cpu.value !== document.form.vlan_vid_iptv.value);
	showhide_div("viptv_mode", v);
	inputCtrl(document.form.viptv_mode, v);
	change_viptv_mode(v);
}

function change_viptv_mode(v){
	if (v)
		v = (document.form.viptv_mode.value == "2");
	showhide_div("tbl_viptv_sect", v);
	inputCtrl(document.form.viptv_ipaddr, v);
	inputCtrl(document.form.viptv_netmask, v);
	inputCtrl(document.form.viptv_gateway, v);
}

function click_untag_lan(o,lp) {
	var v = (o.checked) ? "0" : "1";
	if (lp == 1)
		document.form.vlan_tag_lan1.value = v;
	else if (lp == 2)
		document.form.vlan_tag_lan2.value = v;
	else if (lp == 3)
		document.form.vlan_tag_lan3.value = v;
	else if (lp == 4)
		document.form.vlan_tag_lan4.value = v;
}

function AuthSelection(auth){
	var wan_type = document.form.wan_proto.value;

	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		showhide_div("row_auth_user", 0);
		showhide_div("row_auth_pass", 0);
		showhide_div("row_auth_host", 0);
		return 0;
	}

	showhide_div("row_auth_host", (auth == "1")?1:0);
	showhide_div("row_auth_user", (auth != "0" && auth != "1")?1:0);
	showhide_div("row_auth_pass", (auth != "0")?1:0);
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
<style>
.wlan_filter {width: 50px;}
</style>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<script>
	if(get_ap_mode()){
		alert("<#page_not_support_mode_hint#>");
		location.href = "/as.asp";
	}
</script>

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

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;WLANConfig11b">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("", "lan_ipaddr"); %>" readonly="1" />
    <input type="hidden" name="lan_netmask" value="<% nvram_get_x("", "lan_netmask"); %>" readonly="1" />
    <input type="hidden" name="vlan_tag_lan1" value="<% nvram_get_x("", "vlan_tag_lan1"); %>" />
    <input type="hidden" name="vlan_tag_lan2" value="<% nvram_get_x("", "vlan_tag_lan2"); %>" />
    <input type="hidden" name="vlan_tag_lan3" value="<% nvram_get_x("", "vlan_tag_lan3"); %>" />
    <input type="hidden" name="vlan_tag_lan4" value="<% nvram_get_x("", "vlan_tag_lan4"); %>" />

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
                            <h2 class="box_head round_top"><#menu5_3#> - <#menu5_3_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Layer3Forwarding_x_ConnectionType_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#Layer3Forwarding_x_ConnectionType_itemname#></th>
                                            <td align="left">
                                                <select class="input" name="wan_proto" onchange="change_wan_type(this.value);fixed_change_wan_type(this.value);">
                                                    <option value="static" <% nvram_match_x("", "wan_proto", "static", "selected"); %>>IPoE: <#BOP_ctype_title5#></option>
                                                    <option value="dhcp" <% nvram_match_x("", "wan_proto", "dhcp", "selected"); %>>IPoE: <#BOP_ctype_title1#></option>
                                                    <option value="pppoe" <% nvram_match_x("", "wan_proto", "pppoe", "selected"); %>>PPPoE</option>
                                                    <option value="pptp" <% nvram_match_x("", "wan_proto", "pptp", "selected"); %>>PPTP</option>
                                                    <option value="l2tp" <% nvram_match_x("", "wan_proto", "l2tp", "selected"); %>>L2TP</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_dhcp" style="display:none;">
                                            <th><#MAN_PPPoE#></th>
                                            <td>
                                                <select name="wan_pppoe_man" class="input" onchange="change_pppoe_man(this.value);">
                                                    <option value="0" <% nvram_match_x("", "wan_pppoe_man", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "wan_pppoe_man", "1", "selected"); %>>DHCP or Static</option>
                                                    <option value="2" <% nvram_match_x("", "wan_pppoe_man", "2", "selected"); %>>ZeroConf (169.254.*.*)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_hwnat">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,23);"><#HardwareNAT#></a></th>
                                            <td>
                                                <select name="hw_nat_mode" class="input">
                                                    <option value="0" <% nvram_match_x("", "hw_nat_mode", "0", "selected"); %>>Offload TCP for LAN</option>
                                                    <option value="1" <% nvram_match_x("", "hw_nat_mode", "1", "selected"); %>>Offload TCP for LAN/WLAN</option>
                                                    <option value="3" <% nvram_match_x("", "hw_nat_mode", "3", "selected"); %>>Offload TCP/UDP for LAN</option>
                                                    <option value="4" <% nvram_match_x("", "hw_nat_mode", "4", "selected"); %>>Offload TCP/UDP for LAN/WLAN</option>
                                                    <option value="2" <% nvram_match_x("", "hw_nat_mode", "2", "selected"); %>>Disable (Slow)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_sfe" style="display:none;">
                                            <th><#WAN_SFE#></a></th>
                                            <td>
                                                <select name="sfe_enable" class="input">
                                                    <option value="0" <% nvram_match_x("", "sfe_enable", "0", "selected"); %>>Disable</option>
                                                    <option value="1" <% nvram_match_x("", "sfe_enable", "1", "selected"); %>>Enable for IPv4/IPv6</option>
                                                    <option value="2" <% nvram_match_x("", "sfe_enable", "2", "selected"); %>>Enable for IPv4/IPv6 and WiFi</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_poller">
                                            <th><#WAN_Poller#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="gw_arp_ping_on_of">
                                                        <input type="checkbox" id="gw_arp_ping_fake" <% nvram_match_x("", "gw_arp_ping", "1", "value=1 checked"); %><% nvram_match_x("", "gw_arp_ping", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="gw_arp_ping" id="gw_arp_ping_1" value="1" <% nvram_match_x("", "gw_arp_ping", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="gw_arp_ping" id="gw_arp_ping_0" value="0" <% nvram_match_x("", "gw_arp_ping", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_dhcp_sect">
                                        <tr>
                                            <th id="dhcp_sect_desc" colspan="2" style="background-color: #E3E3E3;"><#IPConnection_ExternalIPAddress_sectionname#></th>
                                        </tr>
                                        <tr id="row_dhcp_toggle">
                                            <th id="dhcp_auto_desc" width="50%"><#Layer3Forwarding_x_DHCPClient_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="x_DHCPClient_on_of">
                                                        <input type="checkbox" id="x_DHCPClient_fake" <% nvram_match_x("", "x_DHCPClient", "1", "value=1 checked"); %><% nvram_match_x("", "x_DHCPClient", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="x_DHCPClient" id="x_DHCPClient_1" class="input" value="1" onclick="set_wan_dhcp_auto(1);" <% nvram_match_x("", "x_DHCPClient", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="x_DHCPClient" id="x_DHCPClient_0" class="input" value="0" onclick="set_wan_dhcp_auto(0);" <% nvram_match_x("", "x_DHCPClient", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_ipaddr">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
                                            <td><input type="text" name="wan_ipaddr" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_ipaddr"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                        <tr id="row_wan_netmask">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
                                            <td><input type="text" name="wan_netmask" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_netmask"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                        <tr id="row_wan_gateway">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
                                            <td><input type="text" name="wan_gateway" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_gateway"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                        <tr id="row_wan_mtu">
                                            <th>MTU:</th>
                                            <td>
                                                <input type="text" name="wan_mtu" maxlength="4" class="input" size="5" value="<% nvram_get_x("","wan_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1300..1500]</span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#IPConnection_x_DNSServerEnable_sectionname#></th>
                                        </tr>
                                        <tr id="row_dns_toggle">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,12);"><#IPConnection_x_DNSServerEnable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wan_dnsenable_x_on_of">
                                                        <input type="checkbox" id="wan_dnsenable_x_fake" <% nvram_match_x("", "wan_dnsenable_x", "1", "value=1 checked"); %><% nvram_match_x("", "wan_dnsenable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="wan_dnsenable_x" id="wan_dnsenable_x_1" value="1" onclick="set_wan_dns_auto(1);" <% nvram_match_x("", "wan_dnsenable_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="wan_dnsenable_x" id="wan_dnsenable_x_0" value="0" onclick="set_wan_dns_auto(0);" <% nvram_match_x("", "wan_dnsenable_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns1">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,13);"><#IPConnection_x_DNSServer1_itemname#> 1:</a></th>
                                            <td>
                                              <input type="text" maxlength="15" class="input" size="15" name="wan_dns1_x" value="<% nvram_get_x("","wan_dns1_x"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,14);"><#IPConnection_x_DNSServer1_itemname#> 2:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns2_x" value="<% nvram_get_x("","wan_dns2_x"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,15);"><#IPConnection_x_DNSServer1_itemname#> 3:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns3_x" value="<% nvram_get_x("","wan_dns3_x"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_vpn_control">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#PPPConnection_UserName_sectionname#></th>
                                        </tr>
                                        <tr id="row_l2tp_cli" style="display:none">
                                            <th><#PPP_L2TPD#></th>
                                            <td>
                                                <select name="wan_l2tpd" class="input">
                                                    <option value="0" <% nvram_match_x("","wan_l2tpd", "0","selected"); %>>xL2TPD</option>
                                                    <option value="1" <% nvram_match_x("","wan_l2tpd", "1","selected"); %>>RP-L2TP</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ppp_peer">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,20);"><#PPPConnection_x_HeartBeat_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_ppp_peer" class="input" maxlength="256" size="32" value="<% nvram_get_x("","wan_ppp_peer"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,4);"><#PPPConnection_UserName_itemname#></a></th>
                                            <td>
                                               <input type="text" maxlength="64" class="input" size="32" name="wan_pppoe_username" value="<% nvram_get_x("","wan_pppoe_username"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,5);"><#PPPConnection_Password_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="wan_pppoe_passwd" id="wan_pppoe_passwd" style="width: 175px;" value="<% nvram_get_x("","wan_pppoe_passwd"); %>"/>
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('wan_pppoe_passwd')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#VPNS_Auth#></th>
                                            <td>
                                                <select name="wan_ppp_auth" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_ppp_auth", "0","selected"); %>>Auto</option>
                                                    <option value="1" <% nvram_match_x("", "wan_ppp_auth", "1","selected"); %>>PAP</option>
                                                    <option value="2" <% nvram_match_x("", "wan_ppp_auth", "2","selected"); %>>CHAP</option>
                                                    <option value="3" <% nvram_match_x("", "wan_ppp_auth", "3","selected"); %>>MS-CHAPv2</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_ppp_mppe">
                                            <th><#VPNS_Ciph#></th>
                                            <td>
                                                <select name="wan_ppp_mppe" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_ppp_mppe", "0","selected"); %>>No Encryption</option>
                                                    <option value="1" <% nvram_match_x("", "wan_ppp_mppe", "1","selected"); %>>Auto</option>
                                                    <option value="2" <% nvram_match_x("", "wan_ppp_mppe", "2","selected"); %>>MPPE-40</option>
                                                    <option value="3" <% nvram_match_x("", "wan_ppp_mppe", "3","selected"); %>>MPPE-128</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_mtu">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pppoe_mtu" class="input" value="<% nvram_get_x("", "wan_pppoe_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1492]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_mru">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pppoe_mru" class="input" value="<% nvram_get_x("", "wan_pppoe_mru"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1492]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pptp_mtu" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pptp_mtu" class="input" value="<% nvram_get_x("", "wan_pptp_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1476]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pptp_mru" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pptp_mru" class="input" value="<% nvram_get_x("", "wan_pptp_mru"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1500]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_l2tp_mtu" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_l2tp_mtu" class="input" value="<% nvram_get_x("", "wan_l2tp_mtu"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1460]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_l2tp_mru" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_l2tp_mru" class="input" value="<% nvram_get_x("", "wan_l2tp_mru"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1000..1500]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#PPP_LCP#></th>
                                            <td>
                                                <label class="radio inline"><input type="radio" value="1" name="wan_ppp_lcp" class="input" <% nvram_match_x("", "wan_ppp_lcp", "1", "checked"); %>><#checkbox_Yes#></label>
                                                <label class="radio inline"><input type="radio" value="0" name="wan_ppp_lcp" class="input" <% nvram_match_x("", "wan_ppp_lcp", "0", "checked"); %>><#checkbox_No#></label>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#PPP_AdaptiveLCP#></th>
                                            <td>
                                                <label class="radio inline"><input type="radio" value="1" name="wan_ppp_alcp" class="input" <% nvram_match_x("", "wan_ppp_alcp", "1", "checked"); %>><#checkbox_Yes#></label>
                                                <label class="radio inline"><input type="radio" value="0" name="wan_ppp_alcp" class="input" <% nvram_match_x("", "wan_ppp_alcp", "0", "checked"); %>><#checkbox_No#></label>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_svc">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_service" value="<% nvram_get_x("","wan_pppoe_service"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_ac">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_ac" value="<% nvram_get_x("","wan_pppoe_ac"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_it" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="10" class="input" size="32" name="wan_pppoe_idletime" value="<% nvram_get_x("","wan_pppoe_idletime"); %>" onkeypress="return is_number(this,event);"/>
                                               &nbsp;<span style="color:#888;">[0..86400]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,19);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_ppp_pppd" value="<% nvram_get_x("", "wan_ppp_pppd"); %>" class="input" maxlength="255" size="32" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#PPPConnection_x_HostNameForISP_sectionname#></th>
                                        </tr>
                                        <tr id="row_auth_type">
                                            <th><#ISP_Authentication_mode#></th>
                                            <td>
                                                <select name="wan_auth_mode" class="input" onChange="AuthSelection(this.value)">
                                                    <option value="0" <% nvram_match_x("", "wan_auth_mode", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "wan_auth_mode", "1", "selected"); %>>ISP KABiNET</option>
                                                    <option value="2" <% nvram_match_x("", "wan_auth_mode", "2", "selected"); %>>802.1x EAP-MD5</option>
                                                    <option value="3" <% nvram_match_x("", "wan_auth_mode", "3", "selected"); %>>802.1x EAP-TTLS/PAP</option>
                                                    <option value="4" <% nvram_match_x("", "wan_auth_mode", "4", "selected"); %>>802.1x EAP-TTLS/CHAP</option>
                                                    <option value="5" <% nvram_match_x("", "wan_auth_mode", "5", "selected"); %>>802.1x EAP-TTLS/MSCHAP</option>
                                                    <option value="6" <% nvram_match_x("", "wan_auth_mode", "6", "selected"); %>>802.1x EAP-TTLS/MSCHAPv2</option>
                                                    <option value="7" <% nvram_match_x("", "wan_auth_mode", "7", "selected"); %>>802.1x EAP-PEAP/MSCHAPv2</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_auth_host">
                                            <th><#ISP_Authentication_host#></th>
                                            <td>
                                                <input type="text" name="wan_auth_host" class="input" maxlength="15" size="32" value="<% nvram_get_x("","wan_auth_host"); %>" onKeyPress="return is_ipaddr(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_auth_user">
                                            <th><#ISP_Authentication_user#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="32" name="wan_auth_user" value="<% nvram_get_x("","wan_auth_user"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_auth_pass">
                                            <th><#ISP_Authentication_pass#></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="wan_auth_pass" id="wan_auth_pass" style="width: 175px;" value="<% nvram_get_x("","wan_auth_pass"); %>" onKeyPress="return is_string(this,event);"/>
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('wan_auth_pass')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_hostname">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,16);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_hostname" class="input" maxlength="32" size="32" value="<% nvram_get_x("","wan_hostname"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_vci">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,19);"><#PPPConnection_x_HostNameForISP_itemvci#></a></th>
                                            <td>
                                                <input type="text" name="wan_vci" class="input" maxlength="128" size="32" value="<% nvram_get_x("","wan_vci"); %>" onkeypress="return is_string(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,17);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_hwaddr_x" class="input" style="float: left; margin-right: 5px;" maxlength="12" size="15" value="<% nvram_get_x("","wan_hwaddr_x"); %>" onKeyPress="return is_hwaddr(event);"/>
                                                <button type="button" class="btn" onclick="showMAC();"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WAN_TTL_Fix#></th>
                                            <td>
                                                <select name="wan_ttl_fix" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_ttl_fix", "0", "selected"); %>><#WAN_TTL_Item0#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "wan_ttl_fix", "1", "selected"); %>><#WAN_TTL_Item1#></option>
                                                    <option value="2" <% nvram_match_x("", "wan_ttl_fix", "2", "selected"); %>><#WAN_TTL_Item2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#WAN_TTL_Value#></th>
                                            <td>
                                                <select name="wan_ttl_value" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_ttl_value", "0", "selected"); %>><#WAN_TTL_Value_Item0#> (*)</option>
                                                    <option value="64" <% nvram_match_x("", "wan_ttl_value", "64", "selected"); %>><#WAN_TTL_Value_Item1#></option>
                                                    <option value="128" <% nvram_match_x("", "wan_ttl_value", "128", "selected"); %>><#WAN_TTL_Value_Item2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_viptv_sect" style="display:none">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#MAN_VIPTV_desc#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#IPConnection_ExternalIPAddress_itemname#></th>
                                            <td><input type="text" name="viptv_ipaddr" maxlength="15" class="input" size="15" value="<% nvram_get_x("","viptv_ipaddr"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                        <tr>
                                            <th><#IPConnection_x_ExternalSubnetMask_itemname#></th>
                                            <td><input type="text" name="viptv_netmask" maxlength="15" class="input" size="15" value="<% nvram_get_x("","viptv_netmask"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                        <tr>
                                            <th><#IPConnection_x_ExternalGateway_itemname#></th>
                                            <td><input type="text" name="viptv_gateway" maxlength="15" class="input" size="15" value="<% nvram_get_x("","viptv_gateway"); %>" onKeyPress="return is_ipaddr(this,event);"/></td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#WAN_Bridge#></th>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,22);"><#Layer3Forwarding_x_STB_itemname#></a></th>
                                            <td>
                                                <select name="wan_stb_x" class="input" onChange="change_stb_port_and_vlan();">
                                                    <option value="0" <% nvram_match_x("", "wan_stb_x", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "wan_stb_x", "1", "selected"); %>>LAN1</option>
                                                    <option value="2" <% nvram_match_x("", "wan_stb_x", "2", "selected"); %>>LAN2</option>
                                                    <option value="3" <% nvram_match_x("", "wan_stb_x", "3", "selected"); %>>LAN3</option>
                                                    <option value="4" <% nvram_match_x("", "wan_stb_x", "4", "selected"); %>>LAN4</option>
                                                    <option value="5" <% nvram_match_x("", "wan_stb_x", "5", "selected"); %>>LAN3 & LAN4</option>
                                                    <option value="6" <% nvram_match_x("", "wan_stb_x", "6", "selected"); %>>LAN1 & LAN2</option>
                                                    <option value="7" <% nvram_match_x("", "wan_stb_x", "7", "selected"); %>>LAN1 & LAN2 & LAN3</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="wan_src_phy">
                                            <th><#WAN_Source#></th>
                                            <td>
                                                <select name="wan_src_phy" class="input">
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="wan_stb_iso">
                                            <th><#STB_Isolation#></th>
                                            <td>
                                                <select name="wan_stb_iso" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_stb_iso", "0", "selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "wan_stb_iso", "1", "selected"); %>><#STB_IsolationItem1#></option>
                                                    <option value="2" <% nvram_match_x("", "wan_stb_iso", "2", "selected"); %>><#STB_IsolationItem2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#WAN_FilterVLAN#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="vlan_filter_on_of">
                                                        <input type="checkbox" id="vlan_filter_fake" <% nvram_match_x("", "vlan_filter", "1", "value=1 checked"); %><% nvram_match_x("", "vlan_filter", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="vlan_filter" id="vlan_filter_1" value="1" onClick="change_stb_port_and_vlan();" <% nvram_match_x("", "vlan_filter", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="vlan_filter" id="vlan_filter_0" value="0" onClick="change_stb_port_and_vlan();" <% nvram_match_x("", "vlan_filter", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="vlan_inet">
                                            <th>VLAN CPU (Internet):</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_cpu" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_cpu"); %>" onkeypress="return is_number(this,event);" onblur="change_viptv_tag(1);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_cpu" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_cpu"); %>" onkeypress="return is_number(this,event);"/></span>
                                            </td>
                                        </tr>
                                        <tr id="vlan_iptv">
                                            <th>VLAN CPU (IPTV):</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_iptv" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_iptv"); %>" onkeypress="return is_number(this,event);" onblur="change_viptv_tag(1);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_iptv" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_iptv"); %>" onkeypress="return is_number(this,event);"/></span>
                                                <span class="input-prepend">&nbsp;
                                                <select name="viptv_mode" id="viptv_mode" class="input" style="width: 95px;" onchange="change_viptv_mode(1);">
                                                    <option value="0" <% nvram_match_x("", "viptv_mode", "0", "selected"); %>>DHCP (*)</option>
                                                    <option value="1" <% nvram_match_x("", "viptv_mode", "1", "selected"); %>>ZeroConf</option>
                                                    <option value="2" <% nvram_match_x("", "viptv_mode", "2", "selected"); %>>Static IP</option>
                                                </select>
                                                </span>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan1">
                                            <th>VLAN LAN1:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan1" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan1"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan1" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan1"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan1" value="" style="margin-left:10;" onclick="click_untag_lan(this,1);" <% nvram_match_x("", "vlan_tag_lan1", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan2">
                                            <th>VLAN LAN2:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan2" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan2"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan2" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan2"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan2" value="" style="margin-left:10;" onclick="click_untag_lan(this,2);" <% nvram_match_x("", "vlan_tag_lan2", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan3">
                                            <th>VLAN LAN3:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan3" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan3"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan3" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan3"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan3" value="" style="margin-left:10;" onclick="click_untag_lan(this,3);" <% nvram_match_x("", "vlan_tag_lan3", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan4">
                                            <th>VLAN LAN4:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan4" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan4"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan4" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan4"); %>" onkeypress="return is_number(this,event);"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan4" value="" style="margin-left:10;" onclick="click_untag_lan(this,4);" <% nvram_match_x("", "vlan_tag_lan4", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border: 0 none;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                        </tr>
                                    </table>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    </form>

    <div id="footer"></div>
</div>
</body>
</html>
