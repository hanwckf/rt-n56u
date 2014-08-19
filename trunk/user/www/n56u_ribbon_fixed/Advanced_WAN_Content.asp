<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>Wireless Router <#Web_Title#> - <#menu5_3_1#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>

<script>
    var $j = jQuery.noConflict();

    $j(document).ready(function() {
        $j('#wan_nat_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wan_nat_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#wan_nat_x_1").attr("checked", "checked");
                $j("#wan_nat_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#wan_nat_x_fake").removeAttr("checked").attr("value", 0);
                $j("#wan_nat_x_0").attr("checked", "checked");
                $j("#wan_nat_x_1").removeAttr("checked");
            }
        });
        $j("#wan_nat_x_on_of label.itoggle").css("background-position", $j("input#wan_nat_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#gw_arp_ping_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#gw_arp_ping_fake").attr("checked", "checked").attr("value", 1);
                $j("#gw_arp_ping_1").attr("checked", "checked");
                $j("#gw_arp_ping_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#gw_arp_ping_fake").removeAttr("checked").attr("value", 0);
                $j("#gw_arp_ping_0").attr("checked", "checked");
                $j("#gw_arp_ping_1").removeAttr("checked");
            }
        });
        $j("#gw_arp_ping_on_of label.itoggle").css("background-position", $j("input#gw_arp_ping_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#x_DHCPClient_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#x_DHCPClient_fake").attr("checked", "checked").attr("value", 1);
                $j("#x_DHCPClient_1").attr("checked", "checked");
                $j("#x_DHCPClient_0").removeAttr("checked");
                change_wan_dhcp_auto(1);
            },
            onClickOff: function(){
                $j("#x_DHCPClient_fake").removeAttr("checked").attr("value", 0);
                $j("#x_DHCPClient_0").attr("checked", "checked");
                $j("#x_DHCPClient_1").removeAttr("checked");
                change_wan_dhcp_auto(0);
            }
        });
        $j("#x_DHCPClient_on_of label.itoggle").css("background-position", $j("input#x_DHCPClient_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#wan_dnsenable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#wan_dnsenable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#wan_dnsenable_x_1").attr("checked", "checked");
                $j("#wan_dnsenable_x_0").removeAttr("checked");
                change_wan_dns_auto(1);
            },
            onClickOff: function(){
                $j("#wan_dnsenable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#wan_dnsenable_x_0").attr("checked", "checked");
                $j("#wan_dnsenable_x_1").removeAttr("checked");
                change_wan_dns_auto(0);
            }
        });
        $j("#wan_dnsenable_x_on_of label.itoggle").css("background-position", $j("input#wan_dnsenable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#vlan_filter_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#vlan_filter_fake").attr("checked", "checked").attr("value", 1);
                $j("#vlan_filter_1").attr("checked", "checked");
                $j("#vlan_filter_0").removeAttr("checked");
                change_stb_port_and_vlan();
            },
            onClickOff: function(){
                $j("#vlan_filter_fake").removeAttr("checked").attr("value", 0);
                $j("#vlan_filter_0").attr("checked", "checked");
                $j("#vlan_filter_1").removeAttr("checked");
                change_stb_port_and_vlan();
            }
        });
        $j("#vlan_filter_on_of label.itoggle").css("background-position", $j("input#vlan_filter_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });

</script>

<script>

<% login_state_hook(); %>

var client_mac = login_mac_str();

var original_wan_type = wan_proto;
var original_wan_dhcpenable = parseInt('<% nvram_get_x("", "x_DHCPClient"); %>');
var original_dnsenable = parseInt('<% nvram_get_x("", "wan_dnsenable_x"); %>');
var original_wan_src_phy = '<% nvram_get_x("", "wan_src_phy"); %>';

function initial(){
	show_banner(1);
	show_menu(5,4,1);
	show_footer();

	if (!support_peap_ssl()){
		var o1 = document.form.wan_auth_mode;
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
		o1.remove(3);
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
	var wan_proto = document.form.wan_proto.value;
	var wan_stb_x = document.form.wan_stb_x.value;
	var min_vlan = support_min_vlan();

	if(!document.form.x_DHCPClient[0].checked){
		var addr_obj = document.form.wan_ipaddr;
		var mask_obj = document.form.wan_netmask;
		var gate_obj = document.form.wan_gateway;
		
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
		
		var lan_addr = document.form.lan_ipaddr.value;
		var lan_mask = document.form.lan_netmask.value;
		
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

	if(document.form.wan_hostname.value.length > 0)
		if(!validate_string(document.form.wan_hostname))
			return false;

	if(document.form.wan_hwaddr_x.value.length > 0)
		if(!validate_hwaddr(document.form.wan_hwaddr_x))
			return false;

	if(document.form.vlan_filter[0].checked){
		if(document.form.vlan_vid_cpu.value.length > 0){
			if(!validate_range(document.form.vlan_vid_cpu, min_vlan, 4094))
				return false;
			if(!validate_range(document.form.vlan_pri_cpu, 0, 7))
				return false;
		}
		
		if(document.form.vlan_vid_iptv.value.length > 0){
			if(!validate_range(document.form.vlan_vid_iptv, min_vlan, 4094))
				return false;
			if(!validate_range(document.form.vlan_pri_iptv, 0, 7))
				return false;
		}
		
		if (wan_stb_x == "1" || wan_stb_x == "6" || wan_stb_x == "7"){
			if(document.form.vlan_vid_lan1.value.length > 0){
				if(!validate_range(document.form.vlan_vid_lan1, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan1, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "2" || wan_stb_x == "6" || wan_stb_x == "7"){
			if(document.form.vlan_vid_lan2.value.length > 0){
				if(!validate_range(document.form.vlan_vid_lan2, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan2, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "3" || wan_stb_x == "5" || wan_stb_x == "7"){
			if(document.form.vlan_vid_lan3.value.length > 0){
				if(!validate_range(document.form.vlan_vid_lan3, min_vlan, 4094))
					return false;
				if(!validate_range(document.form.vlan_pri_lan3, 0, 7))
					return false;
			}
		}
		
		if (wan_stb_x == "4" || wan_stb_x == "5"){
			if(document.form.vlan_vid_lan4.value.length > 0){
				if(!validate_range(document.form.vlan_vid_lan4, min_vlan, 4094))
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

function change_wan_type(wan_type, flag){
	change_wan_dhcp_enable(wan_type);
	change_wan_dns_enable(wan_type);

	if(wan_type == "pppoe"){
		
		$("dhcp_sect_desc").innerHTML = "<#WAN_MAN_desc#>";
		$("dhcp_auto_desc").innerHTML = "<#WAN_MAN_DHCP#>";
		
		if (parseInt(document.form.wan_pppoe_mtu.value) > 1492)
			document.form.wan_pppoe_mtu.value = "1492";
		if (parseInt(document.form.wan_pppoe_mru.value) > 1492)
			document.form.wan_pppoe_mru.value = "1492";
		
		if (document.form.wan_pppoe_man.value == "1")
			$("tbl_dhcp_sect").style.display = "";
		else
			$("tbl_dhcp_sect").style.display = "none";
		
		$("row_wan_poller").style.display = "none";
		$("row_dhcp_toggle").style.display = "";
		$("row_dns_toggle").style.display = "";
		$("tbl_vpn_control").style.display = "";
		$("row_pppoe_dhcp").style.display = "";
		$("row_pppoe_svc").style.display = "";
		$("row_pppoe_it").style.display = "";
		$("row_pppoe_ac").style.display = "";
		$("row_pppoe_mtu").style.display = "";
		$("row_pppoe_mru").style.display = "";
		$("row_pptp_mtu").style.display = "none";
		$("row_pptp_mru").style.display = "none";
		$("row_l2tp_mtu").style.display = "none";
		$("row_l2tp_mru").style.display = "none";
		$("row_l2tp_cli").style.display = "none";
		$("row_ppp_peer").style.display = "none";
		$("row_ppp_mppe").style.display = "none";
		$("row_auth_type").style.display = "none";
	}
	else if(wan_type == "pptp"){
		
		$("dhcp_sect_desc").innerHTML = "<#WAN_MAN_desc#>";
		$("dhcp_auto_desc").innerHTML = "<#WAN_MAN_DHCP#>";
		
		if (parseInt(document.form.wan_pptp_mtu.value) > 1476)
			document.form.wan_pptp_mtu.value = "1476";
		if (parseInt(document.form.wan_pptp_mru.value) > 1500)
			document.form.wan_pptp_mru.value = "1500";
		
		$("row_wan_poller").style.display = "none";
		$("tbl_dhcp_sect").style.display = "";
		$("row_dhcp_toggle").style.display = "";
		$("row_dns_toggle").style.display = "";
		$("tbl_vpn_control").style.display = "";
		$("row_pppoe_dhcp").style.display = "none";
		$("row_pppoe_svc").style.display = "none";
		$("row_pppoe_it").style.display = "none";
		$("row_pppoe_ac").style.display = "none";
		$("row_pppoe_mtu").style.display = "none";
		$("row_pppoe_mru").style.display = "none";
		$("row_pptp_mtu").style.display = "";
		$("row_pptp_mru").style.display = "";
		$("row_l2tp_mtu").style.display = "none";
		$("row_l2tp_mru").style.display = "none";
		$("row_l2tp_cli").style.display = "none";
		$("row_ppp_peer").style.display = "";
		$("row_ppp_mppe").style.display = "";
		$("row_auth_type").style.display = "none";
	}
	else if(wan_type == "l2tp"){
		
		$("dhcp_sect_desc").innerHTML = "<#WAN_MAN_desc#>";
		$("dhcp_auto_desc").innerHTML = "<#WAN_MAN_DHCP#>";
		
		if (parseInt(document.form.wan_l2tp_mtu.value) > 1460)
			document.form.wan_l2tp_mtu.value = "1460";
		if (parseInt(document.form.wan_l2tp_mru.value) > 1500)
			document.form.wan_l2tp_mru.value = "1500";
		
		$("row_wan_poller").style.display = "none";
		$("tbl_dhcp_sect").style.display = "";
		$("row_dhcp_toggle").style.display = "";
		$("row_dns_toggle").style.display = "";
		$("tbl_vpn_control").style.display = "";
		$("row_pppoe_dhcp").style.display = "none";
		$("row_pppoe_svc").style.display = "none";
		$("row_pppoe_it").style.display = "none";
		$("row_pppoe_ac").style.display = "none";
		$("row_pppoe_mtu").style.display = "none";
		$("row_pppoe_mru").style.display = "none";
		$("row_pptp_mtu").style.display = "none";
		$("row_pptp_mru").style.display = "none";
		$("row_l2tp_mtu").style.display = "";
		$("row_l2tp_mru").style.display = "";
		$("row_l2tp_cli").style.display = "";
		$("row_ppp_peer").style.display = "";
		$("row_ppp_mppe").style.display = "";
		$("row_auth_type").style.display = "none";
	}
	else if(wan_type == "static"){
		
		$("dhcp_sect_desc").innerHTML = "<#IPConnection_ExternalIPAddress_sectionname#>";
		$("dhcp_auto_desc").innerHTML = "<#Layer3Forwarding_x_DHCPClient_itemname#>";
		
		$("row_wan_poller").style.display = "none";
		$("tbl_dhcp_sect").style.display = "";
		$("row_pppoe_dhcp").style.display = "none";
		$("row_dhcp_toggle").style.display = "none";
		$("row_dns_toggle").style.display = "none";
		$("tbl_vpn_control").style.display = "none";
		$("row_auth_type").style.display = "";
	}
	else{	// Automatic IP
		
		$("dhcp_sect_desc").innerHTML = "<#IPConnection_ExternalIPAddress_sectionname#>";
		$("dhcp_auto_desc").innerHTML = "<#Layer3Forwarding_x_DHCPClient_itemname#>";
		
		$("row_wan_poller").style.display = "";
		$("tbl_dhcp_sect").style.display = "none";
		$("row_pppoe_dhcp").style.display = "none";
		$("row_dhcp_toggle").style.display = "none";
		$("row_dns_toggle").style.display = "";
		$("tbl_vpn_control").style.display = "none";
		$("row_auth_type").style.display = "";
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
		
		change_wan_dns_auto(0);
		
		if(flag == true && document.form.wan_dns1_x.value.length == 0)
			document.form.wan_dns1_x.focus();
	}
	else{	// dhcp, pppoe, pptp, l2tp
		inputRCtrl2(document.form.wan_dnsenable_x, !original_dnsenable);
		$j('#wan_dnsenable_x_on_of').iState(original_dnsenable);
		
		change_wan_dns_auto(original_dnsenable);
		
		if(flag == true && document.form.wan_dns1_x.value.length == 0 && document.form.wan_dnsenable_x[1].checked)
			document.form.wan_dns1_x.focus();
	}
}

function change_wan_dns_auto(use_auto){
	inputCtrl(document.form.wan_dns1_x, !use_auto);
	inputCtrl(document.form.wan_dns2_x, !use_auto);
	inputCtrl(document.form.wan_dns3_x, !use_auto);

	showhide_div("row_wan_dns1", !use_auto);
	showhide_div("row_wan_dns2", !use_auto);
	showhide_div("row_wan_dns3", !use_auto);
}

function change_wan_dhcp_auto(use_auto){
	inputCtrl(document.form.wan_ipaddr, !use_auto);
	inputCtrl(document.form.wan_netmask, !use_auto);
	inputCtrl(document.form.wan_gateway, !use_auto);
	inputCtrl(document.form.wan_mtu, !use_auto);

	showhide_div("row_wan_ipaddr", !use_auto);
	showhide_div("row_wan_netmask", !use_auto);
	showhide_div("row_wan_gateway", !use_auto);
	showhide_div("row_wan_mtu", !use_auto);
}

function change_wan_dhcp_enable(wan_type){
	if (wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		inputRCtrl2(document.form.x_DHCPClient, !original_wan_dhcpenable);
		$j('#x_DHCPClient_on_of').iState(original_wan_dhcpenable);
		
		inputCtrl(document.form.x_DHCPClient[0], 1);
		inputCtrl(document.form.x_DHCPClient[1], 1);
		$j('input[name="x_DHCPClient"]').removeAttr('disabled');
		$j('#x_DHCPClient_on_of').iClickable(1);
		
		change_wan_dhcp_auto(original_wan_dhcpenable);
	}
	else if(wan_type == "static"){
		inputRCtrl2(document.form.x_DHCPClient, 1);
		$j('#x_DHCPClient_on_of').iState(0);
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		$j('input[name="x_DHCPClient"]').attr('disabled','disabled');
		$j('#x_DHCPClient_on_of').iState(0).iClickable(0);
		
		change_wan_dhcp_auto(0);
	}
	else{	// "dhcp"
		inputRCtrl2(document.form.x_DHCPClient, 0);
		$j('#x_DHCPClient_on_of').iState(1);
		
		inputCtrl(document.form.x_DHCPClient[0], 0);
		inputCtrl(document.form.x_DHCPClient[1], 0);
		$j('input[name="x_DHCPClient"]').attr('disabled','disabled');
		$j('#x_DHCPClient_on_of').iState(1).iClickable(0);
		
		change_wan_dhcp_auto(1);
	}
	
	if((document.form.x_DHCPClient[0].checked) || (wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp")){
		$j('input[name="x_DHCPClient"]').removeAttr('disabled');
		$j('#x_DHCPClient_on_of').iClickable(1);
	}
	else{
		$j('input[name="x_DHCPClient"]').attr('disabled','disabled');
		$j('#x_DHCPClient_on_of').iState(0).iClickable(0);
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

function change_pppoe_man(man_type){
	if(document.form.wan_proto.value == "pppoe"){
		if (man_type == "1")
			$("tbl_dhcp_sect").style.display = "";
		else
			$("tbl_dhcp_sect").style.display = "none";
	}
}

function change_stb_port_and_vlan(){
	var wan_stb_x   = document.form.wan_stb_x.value;
	var vlan_filter = document.form.vlan_filter[0].checked;
	
	free_options(document.form.wan_src_phy);
	add_option(document.form.wan_src_phy, "WAN", "0", 0);
	
	if(wan_stb_x == "0" || vlan_filter) {
		$("wan_stb_iso").style.display = "none";
	}
	else {
		$("wan_stb_iso").style.display = "";
	}
	
	if(wan_stb_x == "0") {
		$("wan_src_phy").style.display = "none";
		document.form.wan_src_phy.SelectedIndex = 0;
	}
	else {
		$("wan_src_phy").style.display = "";
	}
	
	if(!vlan_filter) {
		$("vlan_cpu").style.display = "none";
		$("vlan_iptv").style.display = "none";
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	
	if(wan_stb_x == "0") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
	}
	else if(wan_stb_x == "1") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan1").style.display = "";
		}
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
	}
	else if(wan_stb_x == "2") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan2").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
	}
	else if(wan_stb_x == "3") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan3").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan4").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
	}
	else if(wan_stb_x == "4") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan4").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		$("vlan_lan3").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN4", "4", (original_wan_src_phy == 4) ? 1 : 0);
	}
	else if(wan_stb_x == "5") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan3").style.display = "";
			$("vlan_lan4").style.display = "";
		}
		$("vlan_lan1").style.display = "none";
		$("vlan_lan2").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
		add_option(document.form.wan_src_phy, "LAN4", "4", (original_wan_src_phy == 4) ? 1 : 0);
	}
	else if(wan_stb_x == "6") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan1").style.display = "";
			$("vlan_lan2").style.display = "";
		}
		$("vlan_lan3").style.display = "none";
		$("vlan_lan4").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
		add_option(document.form.wan_src_phy, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
	}
	else if(wan_stb_x == "7") {
		if(vlan_filter) {
			$("vlan_cpu").style.display = "";
			$("vlan_iptv").style.display = "";
			$("vlan_lan1").style.display = "";
			$("vlan_lan2").style.display = "";
			$("vlan_lan3").style.display = "";
		}
		$("vlan_lan4").style.display = "none";
		add_option(document.form.wan_src_phy, "LAN1", "1", (original_wan_src_phy == 1) ? 1 : 0);
		add_option(document.form.wan_src_phy, "LAN2", "2", (original_wan_src_phy == 2) ? 1 : 0);
		add_option(document.form.wan_src_phy, "LAN3", "3", (original_wan_src_phy == 3) ? 1 : 0);
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
	var wan_type = document.form.wan_proto.value;
	
	if(wan_type == "pppoe" || wan_type == "pptp" || wan_type == "l2tp"){
		$("row_auth_user").style.display = "none";
		$("row_auth_pass").style.display = "none";
		$("row_auth_host").style.display = "none";
		return 0;
	}
	
	if (auth == "1")
		$("row_auth_host").style.display = "";
	else
		$("row_auth_host").style.display = "none";
	
	if (auth != "0" && auth != "1")
		$("row_auth_user").style.display = "";
	else
		$("row_auth_user").style.display = "none";
	
	if (auth != "0")
		$("row_auth_pass").style.display = "";
	else
		$("row_auth_pass").style.display = "none";
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
            <div class="span9" >
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
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
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
                                        <tr>
                                            <th><#Enable_NAT#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="wan_nat_x_on_of">
                                                        <input type="checkbox" id="wan_nat_x_fake" <% nvram_match_x("", "wan_nat_x", "1", "value=1 checked"); %><% nvram_match_x("", "wan_nat_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="wan_nat_x" id="wan_nat_x_1" class="input" value="1" <% nvram_match_x("", "wan_nat_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="wan_nat_x" id="wan_nat_x_0" class="input" value="0" <% nvram_match_x("", "wan_nat_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
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
                                                    <input type="radio" name="x_DHCPClient" id="x_DHCPClient_1" class="input" value="1" onclick="change_wan_dhcp_auto(1);" <% nvram_match_x("", "x_DHCPClient", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="x_DHCPClient" id="x_DHCPClient_0" class="input" value="0" onclick="change_wan_dhcp_auto(0);" <% nvram_match_x("", "x_DHCPClient", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_ipaddr">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,1);"><#IPConnection_ExternalIPAddress_itemname#></a></th>
                                            <td><input type="text" name="wan_ipaddr" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_ipaddr"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
                                        </tr>
                                        <tr id="row_wan_netmask">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,2);"><#IPConnection_x_ExternalSubnetMask_itemname#></a></th>
                                            <td><input type="text" name="wan_netmask" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_netmask"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
                                        </tr>
                                        <tr id="row_wan_gateway">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,3);"><#IPConnection_x_ExternalGateway_itemname#></a></th>
                                            <td><input type="text" name="wan_gateway" maxlength="15" class="input" size="15" value="<% nvram_get_x("","wan_gateway"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);"/></td>
                                        </tr>
                                        <tr id="row_wan_mtu">
                                            <th>MTU:</th>
                                            <td>
                                                <input type="text" name="wan_mtu" maxlength="4" class="input" size="5" value="<% nvram_get_x("","wan_mtu"); %>" onkeypress="return is_number(this)"/>
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
                                                    <input type="radio" name="wan_dnsenable_x" id="wan_dnsenable_x_1" value="1" onclick="change_wan_dns_auto(1);" <% nvram_match_x("", "wan_dnsenable_x", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="wan_dnsenable_x" id="wan_dnsenable_x_0" value="0" onclick="change_wan_dns_auto(0);" <% nvram_match_x("", "wan_dnsenable_x", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns1">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,13);"><#IPConnection_x_DNSServer1_itemname#> 1:</a></th>
                                            <td>
                                              <input type="text" maxlength="15" class="input" size="15" name="wan_dns1_x" value="<% nvram_get_x("","wan_dns1_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns2">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,14);"><#IPConnection_x_DNSServer1_itemname#> 2:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns2_x" value="<% nvram_get_x("","wan_dns2_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_wan_dns3">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,15);"><#IPConnection_x_DNSServer1_itemname#> 3:</a></th>
                                            <td>
                                               <input type="text" maxlength="15" class="input" size="15" name="wan_dns3_x" value="<% nvram_get_x("","wan_dns3_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_vpn_control">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#PPPConnection_UserName_sectionname#></th>
                                        </tr>
                                        <tr id="row_l2tp_cli">
                                            <th width="50%"><#PPP_L2TPD#></th>
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
                                                <input type="text" name="wan_ppp_peer" class="input" maxlength="256" size="32" value="<% nvram_get_x("","wan_ppp_peer"); %>" onKeyPress="return is_string(this)"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,4);"><#PPPConnection_UserName_itemname#></a></th>
                                            <td>
                                               <input type="text" maxlength="64" class="input" size="32" name="wan_pppoe_username" value="<% nvram_get_x("","wan_pppoe_username"); %>" onkeypress="return is_string(this)"/>
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
                                            <th><#VPNS_MPPE#></th>
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
                                                <input type="text" maxlength="4" size="5" name="wan_pppoe_mtu" class="input" value="<% nvram_get_x("", "wan_pppoe_mtu"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1492]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_mru">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pppoe_mru" class="input" value="<% nvram_get_x("", "wan_pppoe_mru"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1492]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pptp_mtu" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pptp_mtu" class="input" value="<% nvram_get_x("", "wan_pptp_mtu"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1476]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_pptp_mru" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_pptp_mru" class="input" value="<% nvram_get_x("", "wan_pptp_mru"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1500]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_l2tp_mtu" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,7);"><#PPPConnection_x_PPPoEMTU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_l2tp_mtu" class="input" value="<% nvram_get_x("", "wan_l2tp_mtu"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1460]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_l2tp_mru" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,8);"><#PPPConnection_x_PPPoEMRU_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="4" size="5" name="wan_l2tp_mru" class="input" value="<% nvram_get_x("", "wan_l2tp_mru"); %>" onkeypress="return is_number(this)"/>
                                                &nbsp;<span style="color:#888;">[1000..1500]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#PPP_AdaptiveLCP#></th>
                                            <td>
                                                <!--select name="wan_ppp_alcp" class="input">
                                                    <option value="0" <% nvram_match_x("", "wan_ppp_alcp", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("", "wan_ppp_alcp", "1","selected"); %>><#checkbox_Yes#></option>
                                                </select-->
                                                <label class="radio inline"><input type="radio" value="1" name="wan_ppp_alcp" class="input" <% nvram_match_x("", "wan_ppp_alcp", "1", "checked"); %>><#checkbox_Yes#></label>
                                                <label class="radio inline"><input type="radio" value="0" name="wan_ppp_alcp" class="input" <% nvram_match_x("", "wan_ppp_alcp", "0", "checked"); %>><#checkbox_No#></label>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_svc">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,9);"><#PPPConnection_x_ServiceName_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_service" value="<% nvram_get_x("","wan_pppoe_service"); %>" onkeypress="return is_string(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_ac">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,10);"><#PPPConnection_x_AccessConcentrator_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="wan_pppoe_ac" value="<% nvram_get_x("","wan_pppoe_ac"); %>" onkeypress="return is_string(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_pppoe_it" style="display:none">
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,6);"><#PPPConnection_IdleDisconnectTime_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="10" class="input" size="32" name="wan_pppoe_idletime" value="<% nvram_get_x("","wan_pppoe_idletime"); %>" onkeypress="return is_number(this)"/>
                                               &nbsp;<span style="color:#888;">[0..86400]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,19);"><#PPPConnection_x_AdditionalOptions_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_ppp_pppd" value="<% nvram_get_x("", "wan_ppp_pppd"); %>" class="input" maxlength="255" size="32" onKeyPress="return is_string(this)" onBlur="validate_string(this)"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#PPPConnection_x_HostNameForISP_sectionname#></th>
                                        </tr>
                                        <tr id="row_auth_type">
                                            <th width="50%"><#ISP_Authentication_mode#></th>
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
                                            <th width="50%"><#ISP_Authentication_host#></th>
                                            <td>
                                                <input type="text" name="wan_auth_host" class="input" maxlength="15" size="32" value="<% nvram_get_x("","wan_auth_host"); %>"  onKeyUp="change_ipaddr(this);"/>
                                            </td>
                                        </tr>
                                        <tr id="row_auth_user">
                                            <th width="50%"><#ISP_Authentication_user#></th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="32" name="wan_auth_user" value="<% nvram_get_x("","wan_auth_user"); %>" onKeyPress="return is_string(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="row_auth_pass">
                                            <th width="50%"><#ISP_Authentication_pass#></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" maxlength="64" class="input" size="32" name="wan_auth_pass" id="wan_auth_pass" style="width: 175px;" value="<% nvram_get_x("","wan_auth_pass"); %>" onKeyPress="return is_string(this)"/>
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('wan_auth_pass')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="hostname_x">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,16);"><#PPPConnection_x_HostNameForISP_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_hostname" class="input" maxlength="32" size="32" value="<% nvram_get_x("","wan_hostname"); %>" onkeypress="return is_string(this)"/>
                                            </td>
                                        </tr>
                                        <tr id="clone_mac_x">
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,17);"><#PPPConnection_x_MacAddressForISP_itemname#></a></th>
                                            <td>
                                                <input type="text" name="wan_hwaddr_x" class="input" style="float: left; margin-right: 5px;" maxlength="12" size="15" value="<% nvram_get_x("","wan_hwaddr_x"); %>" onKeyPress="return is_hwaddr()"/>
                                                <button type="button" class="btn" onclick="showMAC();"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#WAN_Bridge#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,7,22);"><#Layer3Forwarding_x_STB_itemname#></a></th>
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
                                            <th><#WAN_FilterVLAN#></th>
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
                                        <tr id="vlan_cpu">
                                            <th>VLAN CPU (Internet):</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_cpu" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_cpu"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_cpu" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_cpu"); %>"/></span>
                                            </td>
                                        </tr>
                                        <tr id="vlan_iptv">
                                            <th>VLAN CPU (IPTV):</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_iptv" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_iptv"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_iptv" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_iptv"); %>"/></span>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan1">
                                            <th>VLAN LAN1:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan1" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan1"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan1" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan1"); %>"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan1" value="" style="margin-left:10;" onclick="click_untag_lan(1);" <% nvram_match_x("", "vlan_tag_lan1", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan2">
                                            <th>VLAN LAN2:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan2" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan2"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan2" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan2"); %>"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan2" value="" style="margin-left:10;" onclick="click_untag_lan(2);" <% nvram_match_x("", "vlan_tag_lan2", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan3">
                                            <th>VLAN LAN3:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan3" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan3"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan3" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan3"); %>"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan3" value="" style="margin-left:10;" onclick="click_untag_lan(3);" <% nvram_match_x("", "vlan_tag_lan3", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr id="vlan_lan4">
                                            <th>VLAN LAN4:</th>
                                            <td>
                                                <span class="input-prepend"><span class="add-on">VID</span><input type="text" name="vlan_vid_lan4" class="wlan_filter" size="4" maxlength="4" value="<% nvram_get_x("", "vlan_vid_lan4"); %>"/>&nbsp;&nbsp;</span>
                                                <span class="input-prepend"><span class="add-on">PRIO</span><input type="text" name="vlan_pri_lan4" class="wlan_filter" size="2" maxlength="1" value="<% nvram_get_x("", "vlan_pri_lan4"); %>"/>&nbsp;&nbsp;</span>
                                                <label class="checkbox inline"><input type="checkbox" name="untag_lan4" value="" style="margin-left:10;" onclick="click_untag_lan(4);" <% nvram_match_x("", "vlan_tag_lan4", "0", "checked"); %>/><#UntagVLAN#></label>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2" style="border-top: 0 none;">
                                                <br/>
                                                <center><input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" /></center>
                                            </td>
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
