<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<!-- <meta name="viewport" content="width=device-width, initial-scale=1.0"> -->
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu2#></title>

<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
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
    	$j('#vpns_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#vpns_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#vpns_enable_1").attr("checked", "checked");
                $j("#vpns_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#vpns_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#vpns_enable_0").attr("checked", "checked");
                $j("#vpns_enable_1").removeAttr("checked");
            }
        });
        $j("#vpns_enable_on_of label.itoggle").css("background-position", $j("input#vpns_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#vpn_settings').click(function(){
            $j(this).parents('ul').find('li').removeClass('active');
            $j(this).parent().addClass('active');

            $j('#vpn_settings_window').show();
            $j('#vpn_clients_window').hide();

            return false;
        });

        $j('#vpn_clients').click(function(){
            $j.get('/vpn_clients.asp', function(response){
                vpn_clients = eval(response);
                createBodyTable();
            });

            $j(this).parents('ul').find('li').removeClass('active');
            $j(this).parent().addClass('active');

            $j('#vpn_clients_window').show();
            $j('#vpn_settings_window').hide();

            return false;
        });
    });
</script>
<script>
lan_ipaddr_x = '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>';
dhcp_enable_x = '<% nvram_get_x("LANHostConfig", "dhcp_enable_x"); %>';
var ACLList = [<% get_nvram_list("LANHostConfig", "VPNSACLList", "vpns_pass_x"); %>];

<% login_state_hook(); %>

<% openvpn_srv_cert_hook(); %>

var vpn_clients = [];

function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();
	
	if (!found_app_ovpn()){
		document.form.vpns_type.remove(2);
	}
	
	calc_lan();
	
	change_vpn_srv_proto(0);
	showACLList();
	
	load_body();
}

function calc_lan(){
	var lan_part = lan_ipaddr_x;
	var lastdot = lan_ipaddr_x.lastIndexOf(".");
	if (lastdot > 3)
		lan_part = lan_part.slice(0, lastdot+1);
	
	$("lanip0").innerHTML = lan_ipaddr_x;
	$("lanip1").innerHTML = lan_part;
	$("lanip2").innerHTML = lan_part;
	$("lanip3").innerHTML = lan_part;
	
	if (dhcp_enable_x != "1")
		$("use_lan_dhcp").style.display = "none";
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/vpnsrv.asp";
		
		document.form.submit();
	}
}

function validForm(){
	var mode = document.form.vpns_type.value;
	var vpns_cli0_ip = parseInt(document.form.vpns_cli0.value);
	var vpns_cli1_ip = parseInt(document.form.vpns_cli1.value);
	
	if (mode == "2") {
		if(!validate_range(document.form.vpns_ov_port, 1, 65535))
			return false;
	}
	else {
		if(!validate_range(document.form.vpns_mtu, 512, 1460))
			return false;
		if(!validate_range(document.form.vpns_mru, 512, 1460))
			return false;
	}
	
	if(vpns_cli0_ip < 2 || vpns_cli0_ip > 254){
		alert("Start IP value should be between 2 and 254!");
		document.form.vpns_cli0.focus();
		return false;
	}
	
	if(vpns_cli1_ip < 2 || vpns_cli1_ip > 254){
		alert("End IP value should be between 2 and 254!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	if(vpns_cli0_ip > vpns_cli1_ip){
		alert("End IP value should higher or equal than Start IP!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	if((vpns_cli1_ip - vpns_cli0_ip) > 9){
		alert("PPTPD server only allows max 10 clients!");
		document.form.vpns_cli1.focus();
		return false;
	}
	
	return true;
}

function done_validating(action){
	if (action == " Del ")
		refreshpage();
}

function change_vpn_srv_proto(mflag) {
	var mode = document.form.vpns_type.value;
	if (mode == "1" || mode == "2")
		$("row_vpns_mppe").style.display = "none";
	else
		$("row_vpns_mppe").style.display = "";
	
	if (mode == "2" && !openvpn_srv_cert_found())
		$("sert_info").style.display = "";
	else
		$("sert_info").style.display = "none";
	
	if (mode == "2") {
		$("row_vpns_auth").style.display = "none";
		$("row_vpns_cast").style.display = "none";
		$("row_vpns_mtu").style.display = "none";
		$("row_vpns_mru").style.display = "none";
		$("ACLList_Block").style.display = "none";
		
		$("row_vpns_ov_prot").style.display = "";
		$("row_vpns_ov_port").style.display = "";
		$("row_vpns_ov_atls").style.display = "";
		$("row_vpns_ov_auth").style.display = "";
		$("row_vpns_ov_ciph").style.display = "";
		$("row_vpns_ov_comp").style.display = "";
	}
	else {
		$("row_vpns_ov_prot").style.display = "none";
		$("row_vpns_ov_port").style.display = "none";
		$("row_vpns_ov_atls").style.display = "none";
		$("row_vpns_ov_auth").style.display = "none";
		$("row_vpns_ov_ciph").style.display = "none";
		$("row_vpns_ov_comp").style.display = "none";
		
		$("row_vpns_auth").style.display = "";
		$("row_vpns_cast").style.display = "";
		$("row_vpns_mtu").style.display = "";
		$("row_vpns_mru").style.display = "";
		$("ACLList_Block").style.display = "";
	}
}

function markGroupACL(o, c, b) {
	var acl_addr;
	document.form.group_id.value = "VPNSACLList";
	if(b == " Add "){
		acl_addr = parseInt(document.form.vpns_addr_x_0.value);
		if (document.form.vpns_num_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.vpns_user_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.vpns_user_x_0.focus();
			document.form.vpns_user_x_0.select();
			return false;
		}else if(document.form.vpns_pass_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.vpns_pass_x_0.focus();
			document.form.vpns_pass_x_0.select();
			return false;
		}else if((document.form.vpns_addr_x_0.value!="") && (acl_addr<2 || acl_addr>254)){
			alert("IP octet value should be between 2 and 254!");
			document.form.vpns_addr_x_0.focus();
			document.form.vpns_addr_x_0.select();
			return false;
		}else{
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
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function showACLList(){
	var code = "";
	var acl_addr;
	var lan_part = lan_ipaddr_x;
	var lastdot = lan_ipaddr_x.lastIndexOf(".");
	if (lastdot > 3)
		lan_part = lan_part.slice(0, lastdot+1);

	if(ACLList.length == 0)
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
		for(var i = 0; i < ACLList.length; i++){
		if (ACLList[i][2] == "")
			acl_addr = "*";
		else
			acl_addr = lan_part + ACLList[i][2];
		code +='<tr id="row' + i + '">';
		code +='<td width="35%">' + ACLList[i][0] + '</td>';
		code +='<td width="35%">*****</td>';
		code +='<td width="25%">' + acl_addr + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="VPNSACLList_s" value="' + i + '" onClick="changeBgColor(this,' + i + ');" id="check' + i + '"></td>';
		code +='</tr>';
		}

		code += '<tr>';
		code += '<td colspan="3">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupACL(this, 10, \' Del \');" name="VPNSACLList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}

	$j('#ACLList_Block').append(code);
}

function changeBgColor(obj, num){
	if(obj.checked)
		$("row" + num).style.background='#D9EDF7';
	else
		$("row" + num).style.background='whiteSmoke';
}

function createBodyTable()
{
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
    $j('#vpn_clients_window table tr.client').remove();
    $j('#vpn_clients_window table:first').append(t_body);
}

</script>

<style>
    #vpn_clients_window {
        display: none;
    }
</style>
</head>

<body onload="initial();" onunload="unload_body();">

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
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;PPPConnection;">
    <input type="hidden" name="group_id" value="VPNSACLList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="flag" value="">
    <input type="hidden" name="vpns_num_x_0" value="<% nvram_get_x("LANHostConfig", "vpns_num_x"); %>" readonly="1" />

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
                                    <a id="vpn_settings" href="#"><#Settings#></a>
                                </li>
                                <li>
                                    <a id="vpn_clients" href="#"><#ConClients#></a>
                                </li>
                            </ul>
                        </div>

                        <div id="vpn_settings_window">
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;"><#PopTopBase#></th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <tr>
                                        <th width="50%"><#PopTopEnable#></th>
                                        <td>
                                            <div class="main_itoggle">
                                                <div id="vpns_enable_on_of">
                                                    <input type="checkbox" id="vpns_enable_fake" <% nvram_match_x("LANHostConfig", "vpns_enable", "1", "value=1 checked"); %><% nvram_match_x("LANHostConfig", "vpns_enable", "0", "value=0"); %>>
                                                </div>
                                            </div>

                                            <div style="position: absolute; margin-left: -10000px;">
                                                <input type="radio" name="vpns_enable" id="vpns_enable_1" class="input" value="1" <% nvram_match_x("LANHostConfig", "vpns_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                <input type="radio" name="vpns_enable" id="vpns_enable_0" class="input" value="0" <% nvram_match_x("LANHostConfig", "vpns_enable", "0", "checked"); %>><#checkbox_No#>
                                            </div>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><#PopTopType#></th>
                                        <td>
                                            <select name="vpns_type" class="input" onchange="change_vpn_srv_proto(1);">
                                                <option value="0" <% nvram_match_x("LANHostConfig", "vpns_type", "0","selected"); %>>PPTP</option>
                                                <option value="1" <% nvram_match_x("LANHostConfig", "vpns_type", "1","selected"); %>>L2TP (w/o IPSec)</option>
                                                <option value="2" <% nvram_match_x("LANHostConfig", "vpns_type", "2","selected"); %>>OpenVPN</option>
                                            </select>
                                            <span id="sert_info" style="display:none" class="label label-warning"><#OVPN_Cert#></span>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_auth">
                                        <th><#PopTopAuth#></th>
                                        <td>
                                            <select name="vpns_auth" class="input">
                                                <option value="0" <% nvram_match_x("LANHostConfig", "vpns_auth", "0","selected"); %>>MS-CHAP v2 only</option>
                                                <option value="1" <% nvram_match_x("LANHostConfig", "vpns_auth", "1","selected"); %>>MS-CHAP v2/v1</option>
                                                <option value="2" <% nvram_match_x("LANHostConfig", "vpns_auth", "2","selected"); %>>MS-CHAP v2/v1 and CHAP</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_mppe">
                                        <th><#PopTopMPPE#></th>
                                        <td>
                                            <select name="vpns_mppe" class="input">
                                                <option value="0" <% nvram_match_x("LANHostConfig", "vpns_mppe", "0","selected"); %>>Auto</option>
                                                <option value="1" <% nvram_match_x("LANHostConfig", "vpns_mppe", "1","selected"); %>>MPPE-128</option>
                                                <option value="2" <% nvram_match_x("LANHostConfig", "vpns_mppe", "2","selected"); %>>MPPE-40</option>
                                                <option value="3" <% nvram_match_x("LANHostConfig", "vpns_mppe", "3","selected"); %>>No encryption</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_cast">
                                        <th><#PopTopCast#></th>
                                        <td align="left">
                                            <select name="vpns_cast" class="input">
                                                <option value="0" <% nvram_match_x("LANHostConfig", "vpns_cast", "0","selected"); %>>Disable</option>
                                                <option value="1" <% nvram_match_x("LANHostConfig", "vpns_cast", "1","selected"); %>>LAN to VPN</option>
                                                <option value="2" <% nvram_match_x("LANHostConfig", "vpns_cast", "2","selected"); %>>VPN to LAN</option>
                                                <option value="3" <% nvram_match_x("LANHostConfig", "vpns_cast", "3","selected"); %>>Both directions</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_mtu">
                                        <th>MTU:</th>
                                        <td>
                                            <input type="text" maxlength="4" size="5" name="vpns_mtu" class="input" value="<% nvram_get_x("LANHostConfig", "vpns_mtu"); %>" onkeypress="return is_number(this)">
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_mru">
                                        <th>MRU:</th>
                                        <td>
                                            <input type="text" maxlength="4" size="5" name="vpns_mru" class="input" value="<% nvram_get_x("LANHostConfig", "vpns_mru"); %>" onkeypress="return is_number(this)">
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
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_ov_atls" style="display:none">
                                        <th><#OVPN_HMAC#></th>
                                        <td>
                                            <select name="vpns_ov_atls" class="input">
                                                <option value="0" <% nvram_match_x("", "vpns_ov_atls", "0","selected"); %>><#checkbox_No#></option>
                                                <option value="1" <% nvram_match_x("", "vpns_ov_atls", "1","selected"); %>><#checkbox_Yes#></option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_ov_auth" style="display:none">
                                        <th><#OVPN_Auth#></th>
                                        <td>
                                            <select name="vpns_ov_auth" class="input">
                                                <option value="0" <% nvram_match_x("", "vpns_ov_auth", "0","selected"); %>>SHA1 (*)</option>
                                                <option value="1" <% nvram_match_x("", "vpns_ov_auth", "1","selected"); %>>SHA256</option>
                                                <option value="2" <% nvram_match_x("", "vpns_ov_auth", "2","selected"); %>>SHA512</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_ov_ciph" style="display:none">
                                        <th><#OVPN_Ciph#></th>
                                        <td>
                                            <select name="vpns_ov_ciph" class="input">
                                                <option value="0" <% nvram_match_x("", "vpns_ov_ciph", "0","selected"); %>>No encryption</option>
                                                <option value="1" <% nvram_match_x("", "vpns_ov_ciph", "1","selected"); %>>Blowfish 128bit (*)</option>
                                                <option value="2" <% nvram_match_x("", "vpns_ov_ciph", "2","selected"); %>>AES 128bit</option>
                                                <option value="3" <% nvram_match_x("", "vpns_ov_ciph", "3","selected"); %>>Triple-DES 192bit</option>
                                                <option value="4" <% nvram_match_x("", "vpns_ov_ciph", "4","selected"); %>>AES 256bit</option>
                                            </select>
                                        </td>
                                    </tr>
                                    <tr id="row_vpns_ov_comp" style="display:none">
                                        <th><#OVPN_Comp#></th>
                                        <td>
                                            <select name="vpns_ov_comp" class="input">
                                                <option value="0" <% nvram_match_x("", "vpns_ov_comp", "0","selected"); %>><#checkbox_No#></option>
                                                <option value="1" <% nvram_match_x("", "vpns_ov_comp", "1","selected"); %>><#checkbox_Yes#> (LZO)</option>
                                            </select>
                                        </td>
                                    </tr>
                                </tbody>
                            </table>
                            <table class="table">
                                <thead>
                                    <tr>
                                        <th colspan="2" style="background-color: #E3E3E3;"><#PopTopCli#></th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <tr>
                                        <th width="50%"><#PopTopSrvIP#></th>
                                        <td>
                                            <span id="lanip0"></span>
                                        </td>
                                    </tr>
                                    <tr id="use_lan_dhcp">
                                        <th><#PopTopCliDHCP#></th>
                                        <td>
                                            <span><% nvram_get_x("LANHostConfig","dhcp_start"); %></span>
                                            <span>&nbsp;~&nbsp;</span>
                                            <span><% nvram_get_x("LANHostConfig","dhcp_end"); %></span>
                                        </td>
                                    </tr>
                                    <tr>
                                        <th><#PopTopCliPool#></th>
                                        <td>
                                            <span id="lanip1"></span>
                                            <input type="text" maxlength="3" size="2" name="vpns_cli0" value="<% nvram_get_x("LANHostConfig","vpns_cli0"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                            <span>&nbsp;~&nbsp;</span>
                                            <span id="lanip2"></span>
                                            <input type="text" maxlength="3" size="2" name="vpns_cli1" value="<% nvram_get_x("LANHostConfig","vpns_cli1"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                        </td>
                                    </tr>
                                </tbody>
                            </table>
                            <table width="100%" cellpadding="4" cellspacing="0" class="table" id="ACLList_Block">
                                    <tr>
                                        <th colspan="4" style="background-color: #E3E3E3;"><#PopTopAcc#></th>
                                    </tr>
                                    <tr>
                                        <th width="35%"><#ISP_Authentication_user#></th>
                                        <th width="35%"><#ISP_Authentication_pass#></th>
                                        <th width="25%"><#PopTopResIP#></th>
                                        <th width="5%">&nbsp;</th>
                                    </tr>
                                    <tr>
                                        <td>
                                            <input type="text" size="14" class="span12" autocomplete="off" maxlength="32" name="vpns_user_x_0" onkeypress="return is_string(this)" />
                                        </td>
                                        <td>
                                            <input type="text" size="14" class="span12" autocomplete="off" maxlength="32" name="vpns_pass_x_0" onkeypress="return is_string(this)" />
                                        </td>
                                        <td>
                                            <span id="lanip3"></span>
                                            <input type="text" size="2" maxlength="3" autocomplete="off" style="width: 25px;" name="vpns_addr_x_0" onkeypress="return is_number(this)" />
                                        </td>
                                        <td>
                                            <button class="btn" type="submit" onclick="return markGroupACL(this, 10, ' Add ');" name="VPNSACLList2"><i class="icon icon-plus"></i></button>
                                        </td>
                                    </tr>
                            </table>
                            <table class="table">
                                <tr>
                                    <td style="border: 0 none;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center></td>
                                </tr>
                            </table>
                            <br>
                        </div>

                        <div id="vpn_clients_window">
                            <table class="table" style="width: 100%">
                                <tr>
                                    <th width="25%" style="border-top: 0 none;"><#IPLocal#></th>
                                    <th width="25%" style="border-top: 0 none;"><#IPRemote#></th>
                                    <th width="25%" style="border-top: 0 none;"><#Login#></th>
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
