<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<!-- <meta name="viewport" content="width=device-width, initial-scale=1.0"> -->
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
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
    	$j('#pptpd_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#pptpd_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#pptpd_enable_1").attr("checked", "checked");
                $j("#pptpd_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#pptpd_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#pptpd_enable_0").attr("checked", "checked");
                $j("#pptpd_enable_1").removeAttr("checked");
            }
        });
        $j("#pptpd_enable_on_of label.itoggle").css("background-position", $j("input#pptpd_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

    });
</script>
<script>
lan_ipaddr_x = '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>';
dhcp_enable_x = '<% nvram_get_x("LANHostConfig", "dhcp_enable_x"); %>';

<% login_state_hook(); %>

function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();
	
	calc_lan();
	
	change_vpn_srv_proto(0);
	
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
	
	if (dhcp_enable_x != "1")
		$("use_lan_dhcp").style.display = "none";
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/poptop.asp";
		
		document.form.submit();
	}
}

function validForm(){
	var pptpd_clib_ip = parseInt(document.form.pptpd_clib.value);
	var pptpd_clie_ip = parseInt(document.form.pptpd_clie.value);
	
	var pptpd_mtu = parseInt(document.form.pptpd_mtu.value);
	var pptpd_mru = parseInt(document.form.pptpd_mru.value);
	
	if(pptpd_mtu < 512 || pptpd_mtu > 1460){
		alert("MTU value should be between 512 and 1460!");
		document.form.pptpd_mtu.focus();
		return false;
	}
	
	if(pptpd_mru < 512 || pptpd_mru > 1460){
		alert("MRU value should be between 512 and 1460!");
		document.form.pptpd_mru.focus();
		return false;
	}
	
	if(pptpd_clib_ip < 2 || pptpd_clib_ip > 254){
		alert("Start IP value should be between 2 and 254!");
		document.form.pptpd_clib.focus();
		return false;
	}
	
	if(pptpd_clie_ip < 2 || pptpd_clie_ip > 254){
		alert("End IP value should be between 2 and 254!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	if(pptpd_clib_ip > pptpd_clie_ip){
		alert("End IP value should higher or equal than Start IP!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	if((pptpd_clie_ip - pptpd_clib_ip) > 9){
		alert("PPTPD server only allows max 10 clients!");
		document.form.pptpd_clie.focus();
		return false;
	}
	
	return true;
}

function change_vpn_srv_proto(mflag) {
	var mode = document.form.pptpd_type.value;
	if (mode == "1") {
		$("mppe_row").style.display = "none";
	}
	else {
		$("mppe_row").style.display = "";
	}
}

</script>
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
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
    <input type="hidden" name="current_page" value="poptop.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;PPPConnection;">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="flag" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

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
                                            <div id="pptpd_enable_on_of">
                                                <input type="checkbox" id="pptpd_enable_fake" <% nvram_match_x("LANHostConfig", "pptpd_enable", "1", "value=1 checked"); %><% nvram_match_x("LANHostConfig", "pptpd_enable", "0", "value=0"); %>>
                                            </div>
                                        </div>

                                        <div style="position: absolute; margin-left: -10000px;">
                                            <input type="radio" name="pptpd_enable" id="pptpd_enable_1" class="input" value="1" <% nvram_match_x("LANHostConfig", "pptpd_enable", "1", "checked"); %>><#checkbox_Yes#>
                                            <input type="radio" name="pptpd_enable" id="pptpd_enable_0" class="input" value="0" <% nvram_match_x("LANHostConfig", "pptpd_enable", "0", "checked"); %>><#checkbox_No#>
                                        </div>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#PopTopType#></th>
                                    <td>
                                        <select name="pptpd_type" class="input" onchange="change_vpn_srv_proto(1);">
                                            <option value="0" <% nvram_match_x("LANHostConfig", "pptpd_type", "0","selected"); %>>PPTP</option>
                                            <option value="1" <% nvram_match_x("LANHostConfig", "pptpd_type", "1","selected"); %>>L2TP (w/o IPSec)</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#PopTopAuth#></th>
                                    <td>
                                        <select name="pptpd_auth" class="input">
                                            <option value="0" <% nvram_match_x("LANHostConfig", "pptpd_auth", "0","selected"); %>>MS-CHAP v2 only</option>
                                            <option value="1" <% nvram_match_x("LANHostConfig", "pptpd_auth", "1","selected"); %>>MS-CHAP v2/v1</option>
                                            <option value="2" <% nvram_match_x("LANHostConfig", "pptpd_auth", "2","selected"); %>>MS-CHAP v2/v1 and CHAP</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr id="mppe_row">
                                    <th><#PopTopMPPE#></th>
                                    <td>
                                        <select name="pptpd_mppe" class="input">
                                            <option value="0" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "0","selected"); %>>Auto</option>
                                            <option value="1" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "1","selected"); %>>MPPE-128</option>
                                            <option value="2" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "2","selected"); %>>MPPE-56</option>
                                            <option value="3" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "3","selected"); %>>MPPE-40</option>
                                            <option value="4" <% nvram_match_x("LANHostConfig", "pptpd_mppe", "4","selected"); %>>No Encryption (unsafe)</option>
                                        </select>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#PopTopCast#></th>
                                    <td align="left">
                                        <select name="pptpd_cast" class="input">
                                            <option value="0" <% nvram_match_x("LANHostConfig", "pptpd_cast", "0","selected"); %>>Disable</option>
                                            <option value="1" <% nvram_match_x("LANHostConfig", "pptpd_cast", "1","selected"); %>>LAN to VPN</option>
                                            <option value="2" <% nvram_match_x("LANHostConfig", "pptpd_cast", "2","selected"); %>>VPN to LAN</option>
                                            <option value="3" <% nvram_match_x("LANHostConfig", "pptpd_cast", "3","selected"); %>>Both directions</option>
                                        </select>
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
                                <tr>
                                    <th>MTU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="pptpd_mtu" class="input" value="<% nvram_get_x("LANHostConfig", "pptpd_mtu"); %>" onkeypress="return is_number(this)">
                                    </td>
                                </tr>
                                <tr>
                                    <th>MRU:</th>
                                    <td>
                                        <input type="text" maxlength="4" size="5" name="pptpd_mru" class="input" value="<% nvram_get_x("LANHostConfig", "pptpd_mru"); %>" onkeypress="return is_number(this)">
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
                                        <input type="text" maxlength="3" size="2" name="pptpd_clib" value="<% nvram_get_x("LANHostConfig","pptpd_clib"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                        <span>&nbsp;~&nbsp;</span>
                                        <span id="lanip2"></span>
                                        <input type="text" maxlength="3" size="2" name="pptpd_clie" value="<% nvram_get_x("LANHostConfig","pptpd_clie"); %>" style="width: 25px;" onKeyPress="return is_number(this)"/>
                                    </td>
                                </tr>
                                <tr>
                                    <th><#PopTopAcc#></th>
                                    <td>
                                        <span><#PopTopSec#>&nbsp;</span>
                                        <span>/etc/storage/chap-secrets</span>
                                    </td>
                                </tr>
                            </tbody>
                        </table>

                        <center><input name="button" type="button" class="btn btn-primary span3" style="max-width: 219px;" onclick="applyRule();" value="<#CTL_apply#>"/></center>
                        <br>
                    </div>
                </div>
             </div>
        </div>
    </div>
    </form>

    <div style="position: absolute; margin-left: -10000px;">
        <table>
            <!--==============Beginning of hint content=============-->
                <td id="help_td" style="width:15px;" valign="top">
                    <form name="hint_form"></form>
                    <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
                    <div id="hintofPM" style="display:none;">
                    <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
                        <thead>
                        <tr>
                            <td><div id="helpname" class="AiHintTitle"></div><a href="javascript:void(0);" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a></td>
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
                <!--===================================Ending of Main Content===========================================-->
            </td>
                <td width="10" align="center" valign="top">&nbsp;</td>
            </tr>
        </table>
    </div>

    <div id="footer"></div>
</div>

</body>
</html>
