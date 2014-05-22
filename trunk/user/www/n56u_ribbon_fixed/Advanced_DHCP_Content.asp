<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_2#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/help.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#dhcp_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#dhcp_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#dhcp_enable_x_1").attr("checked", "checked");
                $j("#dhcp_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#dhcp_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#dhcp_enable_x_0").attr("checked", "checked");
                $j("#dhcp_enable_x_1").removeAttr("checked");
            }
        });
        $j("#dhcp_enable_x_on_of label.itoggle").css("background-position", $j("input#dhcp_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#dhcp_static_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#dhcp_static_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#dhcp_static_x_1").attr("checked", "checked");
                $j("#dhcp_static_x_0").removeAttr("checked");
                change_dhcp_static_enabled();
            },
            onClickOff: function(){
                $j("#dhcp_static_x_fake").removeAttr("checked").attr("value", 0);
                $j("#dhcp_static_x_0").attr("checked", "checked");
                $j("#dhcp_static_x_1").removeAttr("checked");
                change_dhcp_static_enabled();
            }
        });
        $j("#dhcp_static_x_on_of label.itoggle").css("background-position", $j("input#dhcp_static_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

<script>

var ipmonitor = [<% get_static_client(); %>];
var wireless = [<% wl_auth_list(); %>];
var m_dhcp = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

var clients_info = getclients(1,0);

var isMenuopen = 0;

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,3,2);
	show_footer();
	showtext($("LANIP"), '<% nvram_get_x("", "lan_ipaddr"); %>');

	if((inet_network(document.form.lan_ipaddr.value)>=inet_network(document.form.dhcp_start.value))&&(inet_network(document.form.lan_ipaddr.value)<=inet_network(document.form.dhcp_end.value)))
		$('router_in_pool').style.display="";

	showMDHCPList();
	showLANIPList();

	change_dhcp_static_enabled();

	if (!support_ipv6()){
		document.form.dhcp_verbose.remove(2);
		document.form.dhcp_verbose.remove(2);
	}

	load_body();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_DHCP_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	var re = new RegExp('^[a-zA-Z0-9][a-zA-Z0-9\-\_\.]*[a-zA-Z0-9\-\_]$','gi');
	if((document.form.lan_domain.value != "") && (!re.test(document.form.lan_domain.value))){
		alert("<#JS_validchar#>");
		document.form.lan_domain.focus();
		document.form.lan_domain.select();
		return false;
	}

	if(!validate_ipaddr_final(document.form.dhcp_start, 'dhcp_start') ||
			!validate_ipaddr_final(document.form.dhcp_end, 'dhcp_end') ||
			!validate_ipaddr_final(document.form.dhcp_gateway_x, 'dhcp_gateway_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns1_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns2_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns3_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;
	
	if(!validate_range(document.form.dhcp_lease, 120, 604800))
		return false;
	
	if(intoa(document.form.dhcp_start.value) > intoa(document.form.dhcp_end.value)){
		tmp = document.form.dhcp_start.value;
		document.form.dhcp_start.value = document.form.dhcp_end.value;
		document.form.dhcp_end.value = tmp;
	}
	
	var default_pool = new Array();
	default_pool =get_default_pool(document.form.lan_ipaddr.value, document.form.lan_netmask.value);
	if((inet_network(document.form.dhcp_start.value) < inet_network(default_pool[0])) || (inet_network(document.form.dhcp_end.value) > inet_network(default_pool[1]))){
			if(confirm("<#JS_DHCP3#>")){ //Acceptable DHCP ip pool : "+default_pool[0]+"~"+default_pool[1]+"\n
				document.form.dhcp_start.value=default_pool[0];
				document.form.dhcp_end.value=default_pool[1];
			}else{return false;}
	}
	
	if(!validate_ipaddr(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;

	return true;
}

function setClientMAC(num){
	document.form.dhcp_staticmac_x_0.value = clients_info[num][2];
	document.form.dhcp_staticip_x_0.value = clients_info[num][1];
	document.form.dhcp_staticname_x_0.value = clients_info[num][0];
	hideClients_Block();
}

function showLANIPList(){
	var code = "";
	var show_name = "";
	
	for(var i = 0; i < clients_info.length ; i++){
		if(clients_info[i][0] && clients_info[i][0].length > 20)
			show_name = clients_info[i][0].substring(0, 18) + "..";
		else
			show_name = clients_info[i][0];
		
		if(clients_info[i][2]){
			code += '<a href="javascript:void(0)"><div onclick="setClientMAC('+i+');"><strong>'+clients_info[i][1]+'</strong>';
			code += ' ['+clients_info[i][2]+']';
			if(show_name && show_name.length > 0)
				code += ' ('+show_name+')';
			code += ' </div></a>';
		}
	}
	if (code == "")
		code = '<div style="text-align: center;" onclick="hideClients_Block();"><#Nodata#></div>';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	$("ClientList_Block").innerHTML = code;
}

function hideClients_Block(){
	$j("#chevron").children('i').removeClass('icon-chevron-up').addClass('icon-chevron-down');
	$('ClientList_Block').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){
		$j(obj).children('i').removeClass('icon-chevron-down').addClass('icon-chevron-up');
		$("ClientList_Block").style.display = 'block';
		document.form.dhcp_staticmac_x_0.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function change_dhcp_static_enabled(){
	var a = rcheck(document.form.dhcp_static_x);
	if (a == "0"){
		$("row_static_caption").style.display = "none";
		$("row_static_header").style.display = "none";
		$("row_static_body").style.display = "none";
	} else {
		$("row_static_caption").style.display = "";
		$("row_static_header").style.display = "";
		$("row_static_body").style.display = "";
	}
}

function done_validating(action){
	refreshpage();
}

function markGroupMDHCP(o, c, b) {
	document.form.group_id.value = "ManualDHCPList";
	if(b == " Add "){
		if (document.form.dhcp_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.dhcp_staticmac_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.dhcp_staticmac_x_0.focus();
			document.form.dhcp_staticmac_x_0.select();
			return false;
		}else if(document.form.dhcp_staticip_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.dhcp_staticip_x_0.focus();
			document.form.dhcp_staticip_x_0.select();
			return false;
		}else if (!validate_hwaddr(document.form.dhcp_staticmac_x_0)){
			return false;
		}else if (!validate_ipaddr_final(document.form.dhcp_staticip_x_0, "staticip")){
			return false;
		}else{
			for(i=0; i<m_dhcp.length; i++){
				if(document.form.dhcp_staticmac_x_0.value==m_dhcp[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][0] + ')' );
					document.form.dhcp_staticmac_x_0.focus();
					document.form.dhcp_staticmac_x_0.select();
					return false;
				}
				if(document.form.dhcp_staticip_x_0.value.value==m_dhcp[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][1] + ')' );
					document.form.dhcp_staticip_x_0.focus();
					document.form.dhcp_staticip_x_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function showMDHCPList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table">';

	if(m_dhcp.length == 0)
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
		for(var i = 0; i < m_dhcp.length; i++){
		code +='<tr id="row' + i + '">';
		code +='<td width="25%">' + m_dhcp[i][0] + '</td>';
		code +='<td width="25%">' + m_dhcp[i][1] + '</td>';
		code +='<td width="45%">' + m_dhcp[i][2] + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="ManualDHCPList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
		}
		
		code += '<tr>';
		code += '<td colspan="3">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="return markGroupMDHCP(this, 64, \' Del \');" name="ManualDHCPList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}

	code +='</table>';

	$("MDHCPList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#D9EDF7';
	else
		$("row" + num).style.background='whiteSmoke';
}


function get_default_pool(ip, netmask){
	// --- get lan_ipaddr post set .xxx  By Viz 2011.10
	z=0;
	tmp_ip=0;
	for(i=0;i<document.form.lan_ipaddr.value.length;i++){
		if (document.form.lan_ipaddr.value.charAt(i) == '.')
			z++;
		if (z==3){
			tmp_ip=i+1;
			break;
		}
	}
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	c=0;
	tmp_nm=0;
	for(i=0;i<document.form.lan_netmask.value.length;i++){
		if (document.form.lan_netmask.value.charAt(i) == '.')
			c++;
		if (c==3){
			tmp_nm=i+1;
			break;
		}
	}
	var post_lan_netmask = document.form.lan_netmask.value.substr(tmp_nm,3);
	
	var nm = new Array("0", "128", "192", "224", "240", "248", "252");
	for(i=0;i<nm.length;i++){
		 if(post_lan_netmask==nm[i]){
			gap=256-Number(nm[i]);
			subnet_set = 256/gap;
			for(j=1;j<=subnet_set;j++){
				if(post_lan_ipaddr < j*gap){
					pool_start=(j-1)*gap+1;
					pool_end=j*gap-2;
					break;
				}
			}
			var default_pool_start = subnetPostfix(document.form.dhcp_start.value, pool_start, 3);
			var default_pool_end = subnetPostfix(document.form.dhcp_end.value, pool_end, 3);
			var default_pool = new Array(default_pool_start, default_pool_end);
			return default_pool;
			break;
		}
	}
}

</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
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

    <input type="hidden" name="current_page" value="Advanced_DHCP_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_GWStaticRoute_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="ManualDHCPList">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("", "lan_ipaddr"); %>">
    <input type="hidden" name="lan_netmask" value="<% nvram_get_x("", "lan_netmask"); %>">
    <input type="hidden" name="dhcp_staticnum_x_0" value="<% nvram_get_x("", "dhcp_staticnum_x"); %>" readonly="1" />

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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#LANHostConfig_DHCPServerConfigurable_sectiondesc#></div>
                                    <div id="router_in_pool" class="alert alert-danger" style="display:none; margin: 10px;"><b><#LANHostConfig_DHCPServerConfigurable_sectiondesc2#> <span id="LANIP"></span></b></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 5, 1);"><#LANHostConfig_DHCPServerConfigurable_itemname#></a></th>
                                            <td style="border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="dhcp_enable_x_on_of">
                                                        <input type="checkbox" id="dhcp_enable_x_fake" <% nvram_match_x("", "dhcp_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "dhcp_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="dhcp_enable_x" id="dhcp_enable_x_1" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="dhcp_enable_x" id="dhcp_enable_x_0" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="lan_domain" value="<% nvram_get_x("", "lan_domain"); %>">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,3);"><#LANHostConfig_MinAddress_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_start" value="<% nvram_get_x("", "dhcp_start"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,4);"><#LANHostConfig_MaxAddress_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_end" value="<% nvram_get_x("","dhcp_end"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,5);"><#LANHostConfig_LeaseTime_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="6" size="6" name="dhcp_lease" class="input" value="<% nvram_get_x("", "dhcp_lease"); %>" onKeyPress="return is_number(this)">
                                                &nbsp;<span style="color:#888;">[120..604800]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th style="padding-bottom: 0px;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,6);"><#LANHostConfig_x_LGateway_itemname#></a></th>
                                            <td style="padding-bottom: 0px;">
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_gateway_x" value="<% nvram_get_x("", "dhcp_gateway_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#LANHostConfig_x_LDNSServer1_sectionname#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,7);"><#LANHostConfig_x_LDNSServer1_itemname#> 1:</a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns1_x" value="<% nvram_get_x("", "dhcp_dns1_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,8);"><#LANHostConfig_x_LDNSServer1_itemname#> 2:</a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns2_x" value="<% nvram_get_x("", "dhcp_dns2_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,9);"><#LANHostConfig_x_LDNSServer1_itemname#> 3:</a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns3_x" value="<% nvram_get_x("", "dhcp_dns3_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th style="padding-bottom: 0px;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,10);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
                                            <td style="padding-bottom: 0px;">
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_wins_x" value="<% nvram_get_x("", "dhcp_wins_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
                                            </td>
                                        </tr>
                                    </table>
                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#t2Advanced#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#DHCP_Verbose#></th>
                                            <td>
                                                <select name="dhcp_verbose" class="input">
                                                    <option value="0" <% nvram_match_x("", "dhcp_verbose", "0","selected"); %>><#CTL_Disabled#></option>
                                                    <option value="1" <% nvram_match_x("", "dhcp_verbose", "1","selected"); %>>DHCPv4</option>
                                                    <option value="2" <% nvram_match_x("", "dhcp_verbose", "2","selected"); %>>DHCPv6</option>
                                                    <option value="3" <% nvram_match_x("", "dhcp_verbose", "3","selected"); %>>DHCPv4 + DHCPv6</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <a href="javascript:spoiler_toggle('spoiler_conf')"><span><#CustomConf#> "dnsmasq.conf"</span></a>
                                                <div id="spoiler_conf" style="display:none;">
                                                    <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="dnsmasq.dnsmasq.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("dnsmasq.dnsmasq.conf",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2" style="padding-bottom: 0px;">
                                                <a href="javascript:spoiler_toggle('spoiler_hosts')"><span><#CustomConf#> "hosts"</span></a>
                                                <div id="spoiler_hosts" style="display:none;">
                                                    <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="dnsmasq.hosts" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("dnsmasq.hosts",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>
                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="4" id="GWStatic" style="background-color: #E3E3E3;"><#LANHostConfig_ManualDHCPList_groupitemdesc#></th>
                                        </tr>
                                        <tr>
                                            <th colspan="2" width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,11);"><#LANHostConfig_ManualDHCPEnable_itemname#></a></th>
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="dhcp_static_x_on_of">
                                                        <input type="checkbox" id="dhcp_static_x_fake" <% nvram_match_x("", "dhcp_static_x", "1", "value=1 checked"); %><% nvram_match_x("", "dhcp_static_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="dhcp_static_x" id="dhcp_static_x_1" onclick="change_dhcp_static_enabled()" <% nvram_match_x("", "dhcp_static_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="dhcp_static_x" id="dhcp_static_x_0" onclick="change_dhcp_static_enabled()" <% nvram_match_x("", "dhcp_static_x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_static_caption" style="display:none">
                                            <th width="25%"><#LANHostConfig_ManualMac_itemname#></th>
                                            <th width="25%"><#LANHostConfig_ManualIP_itemname#></th>
                                            <th width="45%"><#LANHostConfig_ManualName_itemname#></th>
                                            <th width="5%">&nbsp;</th>
                                        </tr>
                                        <tr id="row_static_header" style="display:none">
                                            <td width="25%">
                                                <div id="ClientList_Block" class="alert alert-info ddown-list" style="width: 400px;"></div>
                                                <div class="input-append">
                                                    <input type="text" maxlength="12" class="span12" size="12" name="dhcp_staticmac_x_0" onkeypress="return is_hwaddr()" style="float:left; width: 110px"/>
                                                    <button class="btn btn-chevron" id="chevron" type="button" onclick="pullLANIPList(this);" title="Select the MAC of LAN clients."><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="15" class="span12" size="15" name="dhcp_staticip_x_0" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
                                            </td>
                                            <td width="45%">
                                                <input type="text" maxlength="24" class="span12" size="20" name="dhcp_staticname_x_0" onKeyPress="return is_string(this)"/>
                                            </td>
                                            <td width="5%">
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupMDHCP(this, 64, ' Add ');" name="ManualDHCPList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr id="row_static_body" style="display:none">
                                            <td colspan="4" style="border-top: 0 none; padding: 0px;">
                                                <div id="MDHCPList_Block"></div>
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
