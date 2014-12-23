<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu2#></title>
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
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('vpns_enable', change_vpns_enabled);

	$j("#tab_vpns_cfg, #tab_vpns_ssl, #tab_vpns_acl, #tab_vpns_cli").click(function(){
		var newHash = $j(this).attr('href').toLowerCase();
		showTab(newHash);
		return false;
	});

	showTab(getHash());
});

</script>
<script>

lan_ipaddr_x = '<% nvram_get_x("", "lan_ipaddr"); %>';
lan_netmask_x = '<% nvram_get_x("", "lan_netmask"); %>';
vpn_ipvnet_x = '<% nvram_get_x("", "vpns_vnet"); %>';
vpn_ipvuse_x = '<% nvram_get_x("", "vpns_vuse"); %>';
fw_enable_x = '<% nvram_get_x("", "fw_enable_x"); %>';
dhcp_enable_x = '<% nvram_get_x("", "dhcp_enable_x"); %>';

var ACLList = [<% get_nvram_list("LANHostConfig", "VPNSACLList", "vpns_pass_x"); %>];

<% openvpn_srv_cert_hook(); %>

var vpn_clients = [];

function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();

	if (!found_app_ovpn())
		document.form.vpns_type.remove(2);

	change_vpns_enabled();

	load_body();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/vpnsrv.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function valid_vpn_subnet(o){
	var ip4v = parse_ipv4_addr(o.value);
	if (ip4v == null){
		alert(o.value + " <#JS_validip#>");
		o.focus();
		o.select();
		return false;
	}

	if (!(ip4v[0] == 192 && ip4v[1] == 168) && !(ip4v[0] == 172 && (ip4v[1] & 240) == 16) && !(ip4v[0] == 10)){
		alert("Please set VPN subnet to 10.x.x.x, 172.[16-31].x.x or 192.168.x.x !");
		o.focus();
		o.select();
		return false;
	}

	var ip4l = parse_ipv4_addr(lan_ipaddr_x);
	if (ip4l != null){
		if ((ip4v[0] == ip4l[0]) && (ip4v[1] == ip4l[1]) && (ip4v[2] == ip4l[2])){
			alert("Please set VPN subnet not equal LAN subnet (" + ip4l[0] + '.' + ip4l[1] + '.' + ip4l[2] + ".x)!");
			o.focus();
			o.select();
			return false;
		}
	}

	ip4v[3] = 0;

	o.value = ip4v[0] + '.' + ip4v[1] + '.' + ip4v[2] + '.' + ip4v[3];

	return true;
}

function valid_rlan_subnet(oa, om){
	var ip4ra = parse_ipv4_addr(oa.value);
	var ip4rm = parse_ipv4_addr(om.value);
	if (ip4ra == null){
		alert(oa.value + " <#JS_validip#>");
		oa.focus();
		oa.select();
		return false;
	}
	if (ip4rm == null || isMask(om.value) <= 0){
		alert(om.value + " <#JS_validmask#>");
		om.focus();
		om.select();
		return false;
	}

	for (i=0;i<4;i++)
		ip4ra[i] = ip4ra[i] & ip4rm[i];
	var r_str = ip4ra[0] + '.' + ip4ra[1] + '.' + ip4ra[2] + '.' + ip4ra[3];

	if (matchSubnet2(oa.value, om.value, lan_ipaddr_x, lan_netmask_x)) {
		alert("Please set remote subnet not equal LAN subnet (" + r_str + ")!");
		oa.focus();
		oa.select();
		return false;
	}

	oa.value = r_str;

	return true;
}

function validForm(){
	if (!document.form.vpns_enable[0].checked)
		return true;

	var mode = document.form.vpns_type.value;
	if (mode == "2") {
		if(!validate_range(document.form.vpns_ov_port, 1, 65535))
			return false;
		
		if (document.form.vpns_ov_mode.value == "1")
			return valid_vpn_subnet(document.form.vpns_vnet);
	}
	else {
		if(!validate_range(document.form.vpns_mtu, 1000, 1460))
			return false;
		if(!validate_range(document.form.vpns_mru, 1000, 1460))
			return false;
		
		if (document.form.vpns_vuse.value != "0")
			return valid_vpn_subnet(document.form.vpns_vnet);
	}

	var o_cli0 = document.form.vpns_cli0;
	var o_cli1 = document.form.vpns_cli1;

	var vpns_cli0_ip = parseInt(o_cli0.value);
	var vpns_cli1_ip = parseInt(o_cli1.value);

	var snet_min = get_subnet_num(lan_ipaddr_x, lan_netmask_x, 0);
	var snet_max = get_subnet_num(lan_ipaddr_x, lan_netmask_x, 1);
	var snet_pool = (snet_max-snet_min) - 1;
	if (snet_pool > 254)
		snet_pool = 254;

	if(vpns_cli0_ip < 1 || vpns_cli0_ip > snet_pool){
		alert("Start IP value should be between 1 and "+snet_pool+"!");
		o_cli0.focus();
		o_cli0.select();
		return false;
	}

	if(vpns_cli1_ip < 2 || vpns_cli1_ip > snet_pool){
		alert("End IP value should be between 2 and "+snet_pool+"!");
		o_cli1.focus();
		o_cli1.select();
		return false;
	}

	if(vpns_cli0_ip > vpns_cli1_ip){
		alert("End IP value should higher or equal than Start IP!");
		o_cli1.focus();
		o_cli1.select();
		return false;
	}

	if((vpns_cli1_ip - vpns_cli0_ip) >= 50){
		alert("VPN server allow max 50 clients!");
		o_cli1.focus();
		o_cli1.select();
		return false;
	}

	return true;
}

function done_validating(action){
	if (action == " Del ")
		refreshpage();
}

function textarea_enabled(v){
	inputCtrl(document.form['ovpnsvr.ca.crt'], v);
	inputCtrl(document.form['ovpnsvr.dh1024.pem'], v);
	inputCtrl(document.form['ovpnsvr.server.crt'], v);
	inputCtrl(document.form['ovpnsvr.server.key'], v);
	inputCtrl(document.form['ovpnsvr.ta.key'], v);
}

function change_vpns_enabled(){
	var v = document.form.vpns_enable[0].checked;

	showhide_div('tab_vpns_cli', v);
	showhide_div('tbl_vpn_config', v);
	showhide_div('tbl_vpn_pool', v);

	if (!v){
		showhide_div('tab_vpns_ssl', 0);
		showhide_div('tab_vpns_acl', 0);
		textarea_enabled(0);
	} else {
		change_vpns_type();
	}
}

function change_vpns_type(){
	var mode = document.form.vpns_type.value;
	var is_ov = (mode == "2") ? 1 : 0;

	var o = document.form.vpns_vuse;
	free_options(o);
	if (!is_ov)
		add_option(o, "<#VPNS_VUse_Item0#>", "0", (vpn_ipvuse_x == '0') ? 1 : 0);
	add_option(o, "<#VPNS_VUse_Item1#>", "1", (vpn_ipvuse_x == '1') ? 1 : 0);
	add_option(o, "<#VPNS_VUse_Item2#>", "2", (vpn_ipvuse_x == '2') ? 1 : 0);

	showhide_div('tab_vpns_ssl', is_ov);

	showhide_div('row_vpns_auth', !is_ov);
	showhide_div('row_vpns_mppe', !is_ov);
	showhide_div('row_vpns_mtu', !is_ov);
	showhide_div('row_vpns_mru', !is_ov);
	showhide_div('row_vpns_script', !is_ov);

	showhide_div('row_vpns_ov_mode', is_ov);
	showhide_div('row_vpns_ov_prot', is_ov);
	showhide_div('row_vpns_ov_port', is_ov);
	showhide_div('row_vpns_ov_mdig', is_ov);
	showhide_div('row_vpns_ov_ciph', is_ov);
	showhide_div('row_vpns_ov_clzo', is_ov);
	showhide_div('row_vpns_ov_atls', is_ov);
	showhide_div('row_vpns_ov_rdgw', is_ov);
	showhide_div('row_vpns_ov_conf', is_ov);

	showhide_div('certs_hint', is_ov && !openvpn_srv_cert_found());

	if (is_ov) {
		showhide_div('row_vpns_cast', 0);
		
		$("col_pass").innerHTML = "";
		$("div_acl_info").innerHTML = "<#VPNS_Accnt_Info2#></br><#VPNS_Accnt_Info3#>";
		inputCtrl(document.form.vpns_pass_x_0, 0);
		document.form.vpns_pass_x_0.value = "";
		
		textarea_enabled(1);
		
		change_vpns_ov_atls();
	}
	else {
		showhide_div('tab_vpns_acl', 1);
		
		$("col_pass").innerHTML = "<#ISP_Authentication_pass#>";
		$("div_acl_info").innerHTML = "<#VPNS_Accnt_Info1#></br><#VPNS_Accnt_Info4#>";
		inputCtrl(document.form.vpns_pass_x_0, 1);
		
		showhide_div('row_vpns_vuse', 1);
		showhide_div('row_vpns_actl', (fw_enable_x == "1"));
		
		textarea_enabled(0);
	}

	change_vpns_vnet_enable();
}

function calc_vpn_addr(vnet_show,is_openvpn){
	var lastdot;
	var lan_part = lan_ipaddr_x;
	var vpn_part = vpn_ipvnet_x;
	var vpn_last = (is_openvpn == 1) ? '254' : '51';

	lastdot = lan_part.lastIndexOf(".");
	if (lastdot > 3)
		lan_part = lan_part.slice(0, lastdot+1);

	lastdot = vpn_part.lastIndexOf(".");
	if (lastdot > 3)
		vpn_part = vpn_part.slice(0, lastdot+1);

	$("lanip1").innerHTML = lan_part;
	$("lanip2").innerHTML = lan_part;

	$("vpnip1").innerHTML = vpn_part + '2';
	$("vpnip2").innerHTML = vpn_part + vpn_last;

	if (vnet_show){
		$("vpnip0").innerHTML = vpn_part + '1';
		$("vpnip3").innerHTML = vpn_part;
	}else{
		$("vpnip0").innerHTML = lan_ipaddr_x;
		$("vpnip3").innerHTML = lan_part;
	}
}

function show_pool_controls(vnet_show,is_openvpn){
	var dhcp_show = 0;

	calc_vpn_addr(vnet_show, is_openvpn);

	showhide_div('row_vpns_vnet', vnet_show);
	showhide_div('row_pool_view', vnet_show);
	showhide_div('row_pool_edit', !vnet_show);

	if (!vnet_show) {
		if (dhcp_enable_x == "1")
			dhcp_show = 1;
	}

	showhide_div('row_lan_dhcp', dhcp_show);
}

function change_vpns_vnet_enable(){
	var vnet_show;
	var rnet_show;
	var is_openvpn = (document.form.vpns_type.value == "2") ? 1 : 0;

	if (is_openvpn){
		vnet_show = (document.form.vpns_ov_mode.value == "1") ? 1 : 0;
		rnet_show = vnet_show;
		showhide_div('tab_vpns_acl', vnet_show);
		showhide_div('row_vpns_vuse', vnet_show);
		showhide_div('row_vpns_actl', (vnet_show && fw_enable_x == "1"));
	}else{
		vnet_show = (document.form.vpns_vuse.value != "0") ? 1 : 0;
		rnet_show = 1;
		showhide_div('row_vpns_cast', !vnet_show);
	}

	if (!rnet_show){
		document.form.vpns_rnet_x_0.value = "";
		document.form.vpns_rmsk_x_0.value = "";
	}

	inputCtrl(document.form.vpns_rnet_x_0, rnet_show);
	inputCtrl(document.form.vpns_rmsk_x_0, rnet_show);

	show_pool_controls(vnet_show, is_openvpn);
	showACLList(vnet_show, rnet_show, is_openvpn);
}

function change_vpns_ov_atls() {
	var v = (document.form.vpns_ov_atls.value == "1") ? 1 : 0;

	showhide_div('row_ta_key', v);
	inputCtrl(document.form['ovpnsvr.ta.key'], v);
}

function markGroupACL(o, c, b) {
	var acl_addr;
	document.form.group_id.value = "VPNSACLList";
	if(b == " Add "){
		if (document.form.vpns_num_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		
		if (document.form.vpns_user_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.vpns_user_x_0.focus();
			document.form.vpns_user_x_0.select();
			return false;
		}
		
		if (document.form.vpns_type.value == "2" && document.form.vpns_addr_x_0.value == "") {
			alert("<#JS_fieldblank#>");
			document.form.vpns_addr_x_0.focus();
			document.form.vpns_addr_x_0.select();
			return false;
		}
		
		acl_addr = parseInt(document.form.vpns_addr_x_0.value);
		if ((document.form.vpns_addr_x_0.value != "") && (acl_addr<2 || acl_addr>254)){
			alert("IP octet value should be between 2 and 254!");
			document.form.vpns_addr_x_0.focus();
			document.form.vpns_addr_x_0.select();
			return false;
		}
		
		if (document.form.vpns_rnet_x_0.value.length > 0 || document.form.vpns_rmsk_x_0.value.length > 0) {
			if (!valid_rlan_subnet(document.form.vpns_rnet_x_0, document.form.vpns_rmsk_x_0))
				return false;
		}
		
		for(i=0; i< ACLList.length; i++){
			if(document.form.vpns_user_x_0.value==ACLList[i][0]) {
				alert('<#JS_duplicate#>' + ' (' + ACLList[i][0] + ')' );
				document.form.vpns_user_x_0.focus();
				document.form.vpns_user_x_0.select();
				return false;
			}
			if((document.form.vpns_addr_x_0.value!="") &&
			   (document.form.vpns_addr_x_0.value==ACLList[i][2])) {
				alert('<#JS_duplicate#>' + ' (' + ACLList[i][0] + ')' );
				document.form.vpns_addr_x_0.focus();
				document.form.vpns_addr_x_0.select();
				return false;
			}
		}
	}
	pageChanged = 0;
	document.form.current_page.value = "/vpnsrv.asp#acl";
	document.form.action_mode.value = b;
	return true;
}

function showACLList(vnet_show,rnet_show,is_openvpn){
	var code;
	var acl_pass;
	var acl_addr;
	var acl_rnet;
	var addr_part = lan_ipaddr_x;
	if (vnet_show)
		addr_part = vpn_ipvnet_x;
	var lastdot = addr_part.lastIndexOf(".");
	if (lastdot > 3)
		addr_part = addr_part.slice(0, lastdot+1);

	code = '<table width="100%" cellspacing="0" cellpadding="3" class="table">';
	if(ACLList.length == 0)
		code +='<tr><td colspan="5" style="text-align: center; padding-bottom: 0px;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
		for(var i = 0; i < ACLList.length; i++){
			acl_pass = "";
			acl_rnet = "";
			if (rnet_show) {
				if (ACLList[i][3] != "" && ACLList[i][4] != "")
					acl_rnet = ACLList[i][3] + " / " + ACLList[i][4];
			}
			
			if (!is_openvpn) {
				acl_pass = "*****";
			}
			
			if (ACLList[i][2] == "")
				acl_addr = "*";
			else
				acl_addr = addr_part + ACLList[i][2];
			
			code +='<tr id="row' + i + '">';
			code +='<td width="20%">' + ACLList[i][0] + '</td>';
			code +='<td width="20%">' + acl_pass + '</td>';
			code +='<td width="20%">' + acl_addr + '</td>';
			code +='<td width="35%">' + acl_rnet + '</td>';
			code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="VPNSACLList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
			code +='</tr>';
		}
		code += '<tr>';
		code += '<td colspan="4" style="padding-bottom: 0px;">&nbsp;</td>'
		code += '<td style="padding-bottom: 0px;"><button class="btn btn-danger" type="submit" onclick="markGroupACL(this, 50, \' Del \');" name="VPNSACLList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';

	$("ACLList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#D9EDF7';
	else
		$("row" + num).style.background='whiteSmoke';
}

function createBodyTable(){
	var t_body = '';
	if(vpn_clients.length > 0)
	{
		try{
			$j.each(vpn_clients, function(i, client){
				t_body += '<tr class="client">\n';
				t_body += '  <td>'+client[0]+'</td>\n';
				t_body += '  <td>'+client[1]+'</td>\n';
				t_body += '  <td>'+client[2]+'</td>\n';
				t_body += '  <td>'+client[3]+'</td>\n';
				t_body += '</tr>\n';
			});
		}
		catch(err){}
	}
	else
	{
		t_body += '<tr class="client">\n';
		t_body += '  <td colspan="4" style="text-align: center;"><div class="alert alert-info"><#Nodata#></div></td>\n';
		t_body += '</tr>\n';
	}
	$j('#wnd_vpns_cli table tr.client').remove();
	$j('#wnd_vpns_cli table:first').append(t_body);
}

var arrHashes = ["cfg", "ssl", "acl", "cli"];

function showTab(curHash){
	if(curHash == ('#'+arrHashes[3])){
		$j.get('/vpn_clients.asp', function(response){
			vpn_clients = eval(response);
			createBodyTable();
		});
	}
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i])){
			$j('#tab_vpns_'+arrHashes[i]).parents('li').addClass('active');
			$j('#wnd_vpns_'+arrHashes[i]).show();
		}else{
			$j('#wnd_vpns_'+arrHashes[i]).hide();
			$j('#tab_vpns_'+arrHashes[i]).parents('li').removeClass('active');
		}
	}
	window.location.hash = curHash;
}

function getHash(){
	var curHash = window.location.hash.toLowerCase();
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i]))
			return curHash;
	}
	return ('#'+arrHashes[0]);
}

</script>

<style>
    .caption-bold {
        font-weight: bold;
    }
</style>

</head>

<body onload="initial();" onunload="unload_body();">
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

    <br>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0" style="position: absolute;"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="current_page" value="vpnsrv.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;PPPConnection;">
    <input type="hidden" name="group_id" value="VPNSACLList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="flag" value="">
    <input type="hidden" name="vpns_num_x_0" value="<% nvram_get_x("", "vpns_num_x"); %>" readonly="1" />

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
                <div class="box well grad_colour_dark_blue">
                    <div id="tabMenu"></div>
                    <h2 class="box_head round_top"><#menu2#></h2>

                    <div class="round_bottom">

                        <div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
                                <li class="active">
                                    <a id="tab_vpns_cfg" href="#cfg"><#Settings#></a>
                                </li>
                                <li>
                                    <a id="tab_vpns_ssl" style="display:none" href="#ssl"><#OVPN_Cert#></a>
                                </li>
                                <li>
                                    <a id="tab_vpns_acl" style="display:none" href="#acl"><#VPNS_Accnt#></a>
                                </li>
                                <li>
                                    <a id="tab_vpns_cli" style="display:none" href="#cli"><#ConClients#></a>
                                </li>
                            </ul>
                        </div>

                        <div id="wnd_vpns_cfg">
                            <div class="alert alert-info" style="margin: 10px;"><#VPNS_Info#></div>
                            <table class="table">
                                <tr>
                                    <th width="50%" style="padding-bottom: 0px; border-top: 0 none;"><#VPNS_Enable#></th>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <div class="main_itoggle">
                                            <div id="vpns_enable_on_of">
                                                <input type="checkbox" id="vpns_enable_fake" <% nvram_match_x("", "vpns_enable", "1", "value=1 checked"); %><% nvram_match_x("", "vpns_enable", "0", "value=0"); %>>
                                            </div>
                                        </div>
                                            <div style="position: absolute; margin-left: -10000px;">
                                            <input type="radio" name="vpns_enable" id="vpns_enable_1" class="input" value="1" onclick="change_vpns_enabled();" <% nvram_match_x("", "vpns_enable", "1", "checked"); %>><#checkbox_Yes#>
                                            <input type="radio" name="vpns_enable" id="vpns_enable_0" class="input" value="0" onclick="change_vpns_enabled();" <% nvram_match_x("", "vpns_enable", "0", "checked"); %>><#checkbox_No#>
                                        </div>
                                    </td>
                                </tr>
                            </table>
                            <table class="table" id="tbl_vpn_config" style="display:none">
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNS_Base#></th>
                                </tr>
                                <tr>
                                    <th width="50%"><#VPNS_Type#></th>
                                    <td>
                                        <select name="vpns_type" class="input" onchange="change_vpns_type();">
                                            <option value="0" <% nvram_match_x("", "vpns_type", "0","selected"); %>>PPTP</option>
                                            <option value="1" <% nvram_match_x("", "vpns_type", "1","selected"); %>>L2TP (w/o IPSec)</option>
                                            <option value="2" <% nvram_match_x("", "vpns_type", "2","selected"); %>>OpenVPN</option>
                                        </select>
                                        <span id="certs_hint" style="display:none" class="label label-warning"><#OVPN_Hint#></span>
                                    </td>
                                </tr>
                                <tr id="row_vpns_auth">
                                    <th><#VPNS_Auth#></th>
                                    <td>
                                        <select name="vpns_auth" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_auth", "0","selected"); %>>MS-CHAPv2 (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpns_auth", "1","selected"); %>>MS-CHAPv2 & CHAP</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_mppe">
                                    <th><#VPNS_Ciph#></th>
                                    <td>
                                        <select name="vpns_mppe" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_mppe", "0","selected"); %>>Auto</option>
                                            <option value="1" <% nvram_match_x("", "vpns_mppe", "1","selected"); %>>MPPE-128 (*)</option>
                                            <option value="2" <% nvram_match_x("", "vpns_mppe", "2","selected"); %>>MPPE-40</option>
                                            <option value="3" <% nvram_match_x("", "vpns_mppe", "3","selected"); %>>No encryption</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_mtu">
                                    <th>MTU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="vpns_mtu" class="input" value="<% nvram_get_x("", "vpns_mtu"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpns_mru">
                                    <th>MRU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="vpns_mru" class="input" value="<% nvram_get_x("", "vpns_mru"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[1000..1460]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_mode" style="display:none">
                                    <th><#OVPN_Mode#></th>
                                    <td>
                                        <select name="vpns_ov_mode" class="input" onchange="change_vpns_vnet_enable();">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_mode", "0","selected"); %>>L2 - TAP (Ethernet)</option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_mode", "1","selected"); %>>L3 - TUN (IP)</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_prot" style="display:none">
                                    <th><#OVPN_Prot#></th>
                                    <td>
                                        <select name="vpns_ov_prot" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_prot", "0","selected"); %>>UDP (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_prot", "1","selected"); %>>TCP</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_port" style="display:none">
                                    <th><#OVPN_Port#></th>
                                    <td>
                                        <input type="text" maxlength="5" size="5" name="vpns_ov_port" class="input" value="<% nvram_get_x("", "vpns_ov_port"); %>" onkeypress="return is_number(this)">
                                        &nbsp;<span style="color:#888;">[ 1194 ]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_mdig" style="display:none">
                                    <th><#VPNS_Auth#></th>
                                    <td>
                                        <select name="vpns_ov_mdig" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_mdig", "0","selected"); %>>[MD5] MD-5, 128 bit</option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_mdig", "1","selected"); %>>[SHA1] SHA-1, 160 bit (*)</option>
                                            <option value="2" <% nvram_match_x("", "vpns_ov_mdig", "2","selected"); %>>[SHA224] SHA-224, 224 bit</option>
                                            <option value="3" <% nvram_match_x("", "vpns_ov_mdig", "3","selected"); %>>[SHA256] SHA-256, 256 bit</option>
                                            <option value="4" <% nvram_match_x("", "vpns_ov_mdig", "4","selected"); %>>[SHA384] SHA-384, 384 bit</option>
                                            <option value="5" <% nvram_match_x("", "vpns_ov_mdig", "5","selected"); %>>[SHA512] SHA-512, 512 bit</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_ciph" style="display:none">
                                    <th><#VPNS_Ciph#></th>
                                    <td>
                                        <select name="vpns_ov_ciph" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_ciph", "0","selected"); %>>[none]</option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_ciph", "1","selected"); %>>[DES-CBC] DES, 64 bit</option>
                                            <option value="2" <% nvram_match_x("", "vpns_ov_ciph", "2","selected"); %>>[DES-EDE-CBC] 3DES, 128 bit</option>
                                            <option value="3" <% nvram_match_x("", "vpns_ov_ciph", "3","selected"); %>>[BF-CBC] Blowfish, 128 bit (*)</option>
                                            <option value="4" <% nvram_match_x("", "vpns_ov_ciph", "4","selected"); %>>[AES-128-CBC] AES, 128 bit</option>
                                            <option value="5" <% nvram_match_x("", "vpns_ov_ciph", "5","selected"); %>>[AES-192-CBC] AES, 192 bit</option>
                                            <option value="6" <% nvram_match_x("", "vpns_ov_ciph", "6","selected"); %>>[DES-EDE3-CBC] 3DES, 192 bit</option>
                                            <option value="7" <% nvram_match_x("", "vpns_ov_ciph", "7","selected"); %>>[DESX-CBC] DES-X, 192 bit</option>
                                            <option value="8" <% nvram_match_x("", "vpns_ov_ciph", "8","selected"); %>>[AES-256-CBC] AES, 256 bit</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_clzo" style="display:none">
                                    <th><#OVPN_CLZO#></th>
                                    <td>
                                        <select name="vpns_ov_clzo" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_clzo", "0","selected"); %>><#btn_Disable#></option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_clzo", "1","selected"); %>><#checkbox_No#></option>
                                            <option value="2" <% nvram_match_x("", "vpns_ov_clzo", "2","selected"); %>><#OVPN_CLZO_Item2#> (*)</option>
                                            <option value="3" <% nvram_match_x("", "vpns_ov_clzo", "3","selected"); %>><#OVPN_CLZO_Item3#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_atls" style="display:none" onchange="change_vpns_ov_atls();">
                                    <th><#OVPN_HMAC#></th>
                                    <td>
                                        <select name="vpns_ov_atls" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_atls", "0","selected"); %>><#checkbox_No#></option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_atls", "1","selected"); %>><#OVPN_HMAC_Item1#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_conf" style="display:none">
                                    <td colspan="2" style="padding-bottom: 15px;">
                                        <a href="javascript:spoiler_toggle('spoiler_vpns_ov_conf')"><span><#OVPN_User#></span></a>
                                        <div id="spoiler_vpns_ov_conf" style="display:none;">
                                            <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpnsvr.server.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.server.conf",""); %></textarea>
                                        </div>
                                    </td>
                                </tr>
                                <tr id="row_vpns_script">
                                    <td colspan="2" style="padding-bottom: 15px;">
                                        <a href="javascript:spoiler_toggle('spoiler_script')"><span><#RunPostVPNS#></span></a>
                                        <div id="spoiler_script" style="display:none;">
                                            <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.vpns_client_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.vpns_client_script.sh",""); %></textarea>
                                        </div>
                                    </td>
                                </tr>
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNS_CTun#></th>
                                </tr>
                                <tr id="row_vpns_vuse" style="display:none">
                                    <th><#VPNS_VUse#></th>
                                    <td>
                                        <select name="vpns_vuse" class="input" onchange="change_vpns_vnet_enable();">
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_vnet" style="display:none">
                                    <th><#VPNS_VNet#></th>
                                    <td>
                                        <input type="text" maxlength="15" class="input" size="15" name="vpns_vnet" value="<% nvram_get_x("", "vpns_vnet"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
                                        &nbsp;<span style="color:#888;">[ 10.8.0.0 ]</span>
                                    </td>
                                </tr>
                                <tr id="row_vpns_cast">
                                    <th><#VPNS_Cast#></th>
                                    <td align="left">
                                        <select name="vpns_cast" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_cast", "0","selected"); %>><#checkbox_No#> (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpns_cast", "1","selected"); %>>LAN to VPN</option>
                                            <option value="2" <% nvram_match_x("", "vpns_cast", "2","selected"); %>>VPN to LAN</option>
                                            <option value="3" <% nvram_match_x("", "vpns_cast", "3","selected"); %>>Both directions</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_ov_rdgw" style="display:none">
                                    <th><#OVPN_RdGw#></th>
                                    <td>
                                        <select name="vpns_ov_rdgw" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_ov_rdgw", "0","selected"); %>><#checkbox_No#> (*)</option>
                                            <option value="1" <% nvram_match_x("", "vpns_ov_rdgw", "1","selected"); %>><#checkbox_Yes#></option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="row_vpns_actl">
                                    <th style="padding-bottom: 0px;"><#VPNS_ACtl#></th>
                                    <td style="padding-bottom: 0px;">
                                        <select name="vpns_actl" class="input">
                                            <option value="0" <% nvram_match_x("", "vpns_actl", "0","selected"); %>><#VPNS_ACtl_Item0#></option>
                                            <option value="1" <% nvram_match_x("", "vpns_actl", "1","selected"); %>><#VPNS_ACtl_Item1#></option>
                                            <option value="2" <% nvram_match_x("", "vpns_actl", "2","selected"); %>><#VPNS_ACtl_Item2#></option>
                                            <option value="3" <% nvram_match_x("", "vpns_actl", "3","selected"); %>><#VPNS_ACtl_Item3#></option>
                                        </select>
                                    </td>
                                </tr>
                            </table>
                            <table class="table" id="tbl_vpn_pool" style="display:none">
                                <tr>
                                    <th colspan="2" style="background-color: #E3E3E3;"><#VPNS_PInfo#></th>
                                </tr>
                                <tr>
                                    <th width="50%"><#VPNS_VAddr#></th>
                                    <td>
                                        <span id="vpnip0"></span>
                                    </td>
                                </tr>
                                <tr id="row_lan_dhcp">
                                    <th><#VPNS_LPool#></th>
                                    <td>
                                        <span><% nvram_get_x("", "dhcp_start"); %></span>
                                        <span>&nbsp;~&nbsp;</span>
                                        <span><% nvram_get_x("", "dhcp_end"); %></span>
                                    </td>
                                </tr>
                                <tr id="row_pool_edit">
                                    <th><#VPNS_VPool#></th>
                                    <td>
                                        <span id="lanip1"></span>
                                        <input type="text" maxlength="3" size="2" name="vpns_cli0" value="<% nvram_get_x("", "vpns_cli0"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                        <span>&nbsp;~&nbsp;</span>
                                        <span id="lanip2"></span>
                                        <input type="text" maxlength="3" size="2" name="vpns_cli1" value="<% nvram_get_x("", "vpns_cli1"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                    </td>
                                </tr>
                                <tr id="row_pool_view">
                                    <th><#VPNS_VPool#></th>
                                    <td>
                                        <span id="vpnip1"></span>
                                        <span>&nbsp;~&nbsp;</span>
                                        <span id="vpnip2"></span>
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                        </div>

                        <div id="wnd_vpns_ssl" style="display:none">
                            <table class="table">
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">ca.crt (Root CA Certificate):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpnsvr.ca.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.ca.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">dh1024.pem (Diffie-Hellman PEM):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="4096" class="span12" name="ovpnsvr.dh1024.pem" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.dh1024.pem",""); %></textarea>
                                    </td>
                                </tr>
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">server.crt (Server Certificate):</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpnsvr.server.crt" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.server.crt",""); %></textarea>
                                    </td>
                                </tr>
                                <tr>
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">server.key (Server Private Key) - secret:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpnsvr.server.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.server.key",""); %></textarea>
                                    </td>
                                </tr>
                                <tr id="row_ta_key">
                                    <td style="padding-bottom: 0px; border-top: 0 none;">
                                        <span class="caption-bold">ta.key (TLS Auth Key) - secret:</span>
                                        <textarea rows="4" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="ovpnsvr.ta.key" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("ovpnsvr.ta.key",""); %></textarea>
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button2" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                        </div>

                        <div id="wnd_vpns_acl" style="display:none">
                            <div id="div_acl_info" class="alert alert-info" style="margin: 10px;"></div>
                            <table class="table">
                                <tr>
                                    <th width="20%" style="border-top: 0 none;"><#VPNS_CName#>:</th>
                                    <th width="20%" id="col_pass" style="border-top: 0 none;"><#ISP_Authentication_pass#></th>
                                    <th width="20%" style="border-top: 0 none;"><#VPNS_FixIP#></th>
                                    <th width="35%" style="border-top: 0 none;"><#VPNS_RNet#></th>
                                    <th width="5%" style="border-top: 0 none;">&nbsp;</th>
                                </tr>
                                <tr>
                                    <td>
                                        <input type="text" size="14" class="span12" autocomplete="off" maxlength="32" name="vpns_user_x_0" onkeypress="return is_string(this)" />
                                    </td>
                                    <td>
                                        <input type="text" size="14" class="span12" autocomplete="off" maxlength="32" name="vpns_pass_x_0" onkeypress="return is_string(this)" />
                                    </td>
                                    <td>
                                        <span id="vpnip3"></span>
                                        <input type="text" size="2" maxlength="3" autocomplete="off" style="width: 25px;" name="vpns_addr_x_0" onkeypress="return is_number(this)" />
                                    </td>
                                    <td>
                                        <input type="text" size="14" maxlength="15" name="vpns_rnet_x_0" style="width: 90px;" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />&nbsp;/
                                        <input type="text" size="14" maxlength="15" name="vpns_rmsk_x_0" style="width: 90px;" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
                                    </td>
                                    <td>
                                        <button class="btn" type="submit" onclick="return markGroupACL(this, 50, ' Add ');" name="VPNSACLList2"><i class="icon icon-plus"></i></button>
                                    </td>
                                </tr>
                                <tr>
                                    <td colspan="5" style="border-top: 0 none; padding: 0px;">
                                        <div id="ACLList_Block"></div>
                                    </td>
                                </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button3" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                        </div>

                        <div id="wnd_vpns_cli" style="display:none">
                            <table class="table" style="width: 100%">
                                <tr>
                                    <th width="25%" style="border-top: 0 none;"><#IPLocal#></th>
                                    <th width="25%" style="border-top: 0 none;"><#IPRemote#></th>
                                    <th width="25%" style="border-top: 0 none;"><#VPNS_CName#></th>
                                    <th width="25%" style="border-top: 0 none;"><#RouterConfig_GWStaticIF_itemname#></th>
                                </tr>
                            </table>
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
