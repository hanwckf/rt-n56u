<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_2#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>

<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#dhcp_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                change_common_radio(this, 'WLANConfig11b', 'dhcp_enable_x', '1');
                $j("#dhcp_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#dhcp_enable_x_1").attr("checked", "checked");
                $j("#dhcp_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, 'WLANConfig11b', 'dhcp_enable_x', '0');
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
                change_common_radio(this, 'WLANConfig11b', 'dhcp_static_x', '1');
                $j("#dhcp_static_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#dhcp_static_x_1").attr("checked", "checked");
                $j("#dhcp_static_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, 'WLANConfig11b', 'dhcp_static_x', '0');
                $j("#dhcp_static_x_fake").removeAttr("checked").attr("value", 0);
                $j("#dhcp_static_x_0").attr("checked", "checked");
                $j("#dhcp_static_x_1").removeAttr("checked");
            }
        });
        $j("#dhcp_static_x_on_of label.itoggle").css("background-position", $j("input#dhcp_static_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';
var MDHCPList = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

<% login_state_hook(); %>

var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]


function initial(){
	show_banner(1);
	show_menu(5,3,2);
	show_footer();
	showtext($("LANIP"), '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>');
	
	if((inet_network(document.form.lan_ipaddr.value)>=inet_network(document.form.dhcp_start.value))&&(inet_network(document.form.lan_ipaddr.value)<=inet_network(document.form.dhcp_end.value))){
			$('router_in_pool').style.display="";
	}
	
	showMDHCPList();
	
	enable_auto_hint(5, 7);

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
	if(!re.test(document.form.lan_domain.value) && document.form.lan_domain.value != ""){
		alert("<#JS_validchar#>");
		document.form.lan_domain.focus();
		document.form.lan_domain.select();
		return false;
	}
	
	if(!validate_string(document.form.lan_domain))
		return false;
	
	if(!validate_ipaddr_final(document.form.dhcp_start, 'dhcp_start') ||
			!validate_ipaddr_final(document.form.dhcp_end, 'dhcp_end') ||
			!validate_ipaddr_final(document.form.dhcp_gateway_x, 'dhcp_gateway_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns1_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns2_x, 'dhcp_dns2_x') ||
			!validate_ipaddr_final(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;
	
	if(!validate_range(document.form.dhcp_lease, 120, 604800))
		return false;
	
	if(intoa(document.form.dhcp_start.value) > intoa(document.form.dhcp_end.value)){	//exchange start < end
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
			for(i=0; i<MDHCPList.length; i++){
				if(document.form.dhcp_staticmac_x_0.value==MDHCPList[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + MDHCPList[i][0] + ')' );
					document.dhcp_staticmac_x_0.focus();
					document.dhcp_staticmac_x_0.select();
					return false;
				}
				if(document.form.dhcp_staticip_x_0.value.value==MDHCPList[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + MDHCPList[i][1] + ')' );
					document.form.dhcp_staticip_x_0.focus();
					document.form.dhcp_staticip_x_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	pageChangedCount = 0;
	document.form.action_mode.value = b;
	return true;
}

function showMDHCPList(){
	var code = "";

	if(MDHCPList.length == 0)
		code +='<tr><td colspan="4"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 0; i < MDHCPList.length; i++){
		code +='<tr id="row' + i + '">';
		code +='<td width="25%">' + MDHCPList[i][0] + '</td>';
		code +='<td width="25%">' + MDHCPList[i][1] + '</td>';
		code +='<td width="45%">' + MDHCPList[i][2] + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="ManualDHCPList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
		}
		
		if(MDHCPList.length > 0)
		{
		    code += '<tr>';
		    code += '<td colspan="3">&nbsp;</td>'
		    code += '<td><button class="btn btn-danger" type="submit" onclick="return markGroupMDHCP(this, 32, \' Del \');" name="ManualDHCPList"><i class="icon icon-minus icon-white"></i></button></td>';
		    code += '</tr>'
		}
	}

	$j('#MDHCPList_Block').append(code);
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#D9EDF7';
	else
		$("row" + num).style.background='whiteSmoke';
}


// Viz add 2011.10 default DHCP pool range{
function get_default_pool(ip, netmask){
	// --- get lan_ipaddr post set .xxx  By Viz 2011.10
	z=0;
	tmp_ip=0;
	for(i=0;i<document.form.lan_ipaddr.value.length;i++){
			if (document.form.lan_ipaddr.value.charAt(i) == '.')	z++;
			if (z==3){ tmp_ip=i+1; break;}
	}		
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	// --- get lan_netmask post set .xxx	By Viz 2011.10
	c=0;
	tmp_nm=0;
	for(i=0;i<document.form.lan_netmask.value.length;i++){
			if (document.form.lan_netmask.value.charAt(i) == '.')	c++;
			if (c==3){ tmp_nm=i+1; break;}
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
	//alert(document.form.dhcp_start.value+" , "+document.form.dhcp_end.value);//Viz
}
// } Viz add 2011.10 default DHCP pool range	

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(5, 7);return unload_body();">
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
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_DHCP_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_GWStaticRoute_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="ManualDHCPList">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
    <input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>">
    <input type="hidden" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>">
    <input type="hidden" name="dhcp_staticnum_x_0" value="<% nvram_get_x("LANHostConfig", "dhcp_staticnum_x"); %>" readonly="1" />

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

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">
                                                <div id="router_in_pool" style="display:none;"><b><#LANHostConfig_DHCPServerConfigurable_sectiondesc2#> <span id="LANIP"></span></b></div>
                                            </th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this, 5, 1);"><#LANHostConfig_DHCPServerConfigurable_itemname#></a></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="dhcp_enable_x_on_of">
                                                        <input type="checkbox" id="dhcp_enable_x_fake" <% nvram_match_x("WLANConfig11b", "dhcp_enable_x", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "dhcp_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="dhcp_enable_x" id="dhcp_enable_x_1" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '1')" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="dhcp_enable_x" id="dhcp_enable_x_0" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '0')" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="32" class="input" size="32" name="lan_domain" value="<% nvram_get_x("LANHostConfig", "lan_domain"); %>" onblur="is_string(this);">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,3);"><#LANHostConfig_MinAddress_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_start" value="<% nvram_get_x("LANHostConfig","dhcp_start"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,4);"><#LANHostConfig_MaxAddress_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_end" value="<% nvram_get_x("LANHostConfig","dhcp_end"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,5);"><#LANHostConfig_LeaseTime_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="6" size="6" name="dhcp_lease" class="input" value="<% nvram_get_x("LANHostConfig", "dhcp_lease"); %>" onKeyPress="return is_number(this)"> sec
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,6);"><#LANHostConfig_x_LGateway_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_gateway_x" value="<% nvram_get_x("LANHostConfig","dhcp_gateway_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#LANHostConfig_x_LDNSServer1_sectionname#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,7);"><#LANHostConfig_x_LDNSServer1_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns1_x" value="<% nvram_get_x("LANHostConfig","dhcp_dns1_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,7);"><#LANHostConfig_x_LDNSServer2_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns2_x" value="<% nvram_get_x("LANHostConfig","dhcp_dns2_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,8);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="dhcp_wins_x" value="<% nvram_get_x("LANHostConfig","dhcp_wins_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="MDHCPList_Block">
                                        <tr>
                                            <th colspan="4" id="GWStatic" style="background-color: #E3E3E3;"><#LANHostConfig_ManualDHCPList_groupitemdesc#></th>
                                        </tr>
                                        <tr>
                                            <th colspan="2" width="50%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,5,9);"><#LANHostConfig_ManualDHCPEnable_itemname#></a></th>
                                            <td colspan="2">
                                                <div class="main_itoggle">
                                                    <div id="dhcp_static_x_on_of">
                                                        <input type="checkbox" id="dhcp_static_x_fake" <% nvram_match_x("WLANConfig11b", "dhcp_static_x", "1", "value=1 checked"); %><% nvram_match_x("WLANConfig11b", "dhcp_static_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="dhcp_static_x" id="dhcp_static_x_1" onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '1')" <% nvram_match_x("LANHostConfig","dhcp_static_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="dhcp_static_x" id="dhcp_static_x_0" onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '0')" <% nvram_match_x("LANHostConfig","dhcp_static_x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th width="25%"><#LANHostConfig_ManualMac_itemname#></th>
                                            <th width="25%"><#LANHostConfig_ManualIP_itemname#></th>
                                            <th width="45%"><#LANHostConfig_ManualName_itemname#></th>
                                            <th width="5%">&nbsp;</th>
                                        </tr>
                                        <tr>
                                            <td width="25%">
                                                <input type="text" maxlength="12" class="span12" size="15" name="dhcp_staticmac_x_0" onkeypress="return is_hwaddr()" />
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="15" class="span12" size="15" name="dhcp_staticip_x_0" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
                                            </td>
                                            <td width="45%">
                                                <input type="text" maxlength="24" class="span12" size="20" name="dhcp_staticname_x_0" onKeyPress="return is_string(this)"/>
                                            </td>
                                            <td width="5%">
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupMDHCP(this, 32, ' Add ');" name="ManualDHCPList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
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

    <!--==============Beginning of hint content=============-->
    <div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
        <form name="hint_form"></form>
        <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

        <div id="hintofPM" style="display:none;">
            <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
            <thead>
                <tr>
                    <td>
                        <div id="helpname" class="AiHintTitle"></div>
                        <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
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
    </div>
    <!--==============Ending of hint content=============-->

    <div id="footer"></div>
</div>
</body>
</html>
