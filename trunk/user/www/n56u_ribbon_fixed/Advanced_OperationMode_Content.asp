<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_1_title#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

var auth_mode = '<% nvram_get_x("", "wl_auth_mode"); %>';
var wep_x = '<% nvram_get_x("", "wl_wep_x"); %>';
var auth_mode2 = '<% nvram_get_x("", "rt_auth_mode"); %>';
var wep_x2 = '<% nvram_get_x("", "rt_wep_x"); %>';
var ssid_2g = '<% nvram_char_to_ascii("WLANConfig11b", "rt_ssid"); %>';
var ssid_5g = '<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>';
var sw_mode = '<% nvram_get_x("", "sw_mode"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

function initial(){
	show_banner(2);
	
	show_menu(5,7,1);
	
	show_footer();

	if(sw_mode=="1" || sw_mode=="3"){
		$('wl_rt').style.display="";
		$('rt_wo_nat').style.display="none";
	}else if(sw_mode=="4"){
		$('wl_rt').style.display="none";
		$('rt_wo_nat').style.display="";
	}
	
	setScenerion(sw_mode);
}

function saveMode(){
	
	if(sw_mode == "1"){ 
		if(document.form.sw_mode[0].checked == true){
			alert("<#Web_Title#> <#op_already_configured#>");
			return false;
		}
	}else	if(sw_mode == "4"){ 
		if(document.form.sw_mode[1].checked == true){
			alert("<#Web_Title#> <#op_already_configured#>");
			return false;
		}
	}else if(sw_mode == "3"){
		if(document.form.sw_mode[2].checked == true){
			alert("<#Web_Title#> <#op_already_configured#>");
			return false;
		}
	}
	
	if(document.form.sw_mode[0].checked == true || document.form.sw_mode[1].checked == true){
		document.form.action="/start_apply.htm";
		document.form.target="hidden_frame";
		document.form.current_page.value = "Advanced_OperationMode_Content.asp";
		document.form.action_mode.value = " Apply ";
	}else if(document.form.sw_mode[2].checked == true){
		if(ssid_2g == "ASUS" && ssid_5g == "ASUS_5G" && auth_mode == "open" && wep_x == "0" && auth_mode2 == "open" && wep_x2 == "0")
			document.form.flag.value = 'adv_ap_mode';
		else{
			document.form.flag.value = 'ap_mode_AOC';
			document.form.action="/start_apply2.htm";
			document.form.target="hidden_frame";
			document.form.current_page.value = "Advanced_OperationMode_Content.asp";
			document.form.action_mode.value = " Apply ";
		}
	}
	
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

var id_WANunplungHint;

function setScenerion(mode){
	if(mode == '1' || mode == '4'){
		//$j("#Senario").css("background","url(/images/gw.gif) no-repeat");
		//ea $j("#Senario").css("background","url(/images/gw.jpg) no-repeat");
		$j("#radio2").hide();
		$j("#Internet_span").css("display", "");
		$j("#ap-line").css("display", "none");
		//$j("#ap-line").animate({width:"133px"}, 2000);
		$j(".AP").hide();
		$j("#mode_desc").html("<#OP_GW_desc1#><#OP_GW_desc2#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
	}	
	/*else if(mode == '2'){
		$j("#Senario").css("background", "url(/images/rt.gif) no-repeat");
		$j("#radio2").css("display", "block");
		$j("#Internet_span").css("display", "block");
		$j("#AP").html("<#Device_type_03_AP#>");
		$j("#mode_desc").html("<#OP_GW_desc1#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
		clearTimeout(id_WANunplungHint);
		$j("#Unplug-hint").css("display", "none");
		$j("#ap-line").css("display", "none");
	}*/
	else if(mode == '3'){
		//$j("#Senario").css("background", "url(/images/ap.gif) no-repeat");
		//ea $j("#Senario").css("background", "url(/images/ap.jpg) no-repeat");
		$j("#radio2").css("display", "none");
		$j("#Internet_span").css("display", "");
		$j("#ap-line").css("display", "none");
		//ea$j("#AP").css("display", "");
		//ea $j("#AP").html("<#Device_type_02_RT#>");
		$j(".AP").show();
		$j("#mode_desc").html("<#OP_AP_desc1#><#OP_AP_desc2#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
		clearTimeout(id_WANunplungHint);
		$j("#Unplug-hint").css("display", "none");
	}/*
	else if(mode == '4'){
		$j("#Senario").css("background","url(/images/rt.gif) no-repeat");
		$j("#radio2").hide();
		$j("#Internet_span").hide();
		$j("#ap-line").css("display", "none");
		//$j("#ap-line").animate({width:"133px"}, 2000);
		$j("#AP").html("<#Internet#>");
		$j("#mode_desc").html("<#OP_RT_desc1#><#OP_RT_desc2#>");
		$j("#nextButton").attr("value","<#CTL_next#>");
	}*/
}

</script>

<style>
table {width: 100%;}
table td {text-align: center; }
</style>

</head>

<body onload="initial();" onunLoad="disable_auto_hint(11, 3);return unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="hiddenMask" class="popup_bg" style="position: absolute; margin-left: -10000px;">
        <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
            <tr>
            <td>
                <div class="drword" id="drword" style="height:110px;"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
                    <br/>
                    <br/>
            </div>
              <div class="drImg"><img src="images/DrsurfImg.gif"></div>
                <div style="height:70px;"></div>
            </td>
            </tr>
        </table>
    <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
    <form method="post" name="form" id="ruleForm" action="/QIS_wizard.htm">
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">
    <input type="hidden" name="sid_list" value="WLANConfig11b;IPConnection;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="prev_page" value="/Advanced_OperationMode_Content.asp">
    <input type="hidden" name="current_page" value="">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b",  "wl_ssid"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
    <input type="hidden" name="flag" value="">
    <input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("", "lan_ipaddr"); %>">

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
                            <h2 class="box_head round_top"><#t1SYS#> - <#t2OP#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                        <div style="margin-left: 10px; margin-top: 10px;">
                                            <div class="controls">
                                                <label class="radio inline"><div id="wl_rt" style="display:none;"><input type="radio" name="sw_mode" class="input" value="1" onclick="setScenerion(1);" <% nvram_match_x("IPConnection", "sw_mode", "1", "checked"); %>><#OP_GW_item#></div></label>
                                                <label class="radio inline"><div id="rt_wo_nat" style="display:none;"><input type="radio" name="sw_mode" class="input" value="4" onclick="setScenerion(4);" <% nvram_match_x("IPConnection", "sw_mode", "4", "checked"); %>><#OP_GW_item#></div></label>
                                                <!--<label class="radio inline"> <input type="radio" name="sw_mode" class="input" value="4" onclick="setScenerion(4);" <% nvram_match_x("IPConnection", "sw_mode", "4", "checked"); %>> <#OP_RT_item#>  </label>-->
                                                <label class="radio inline"><div id="wl_ap"><input type="radio" name="sw_mode" class="input" value="3" onclick="setScenerion(3);" <% nvram_match_x("IPConnection", "sw_mode", "3", "checked"); %>><#OP_AP_item#></div></label>
                                            </div>

                                            <div id="mode_desc" style="margin-left: 0px; margin-top: 10px; margin-right: 10px;" class="alert alert-info"><#OP_GW_desc1#></div>
                                        </div>

                                        <div style="margin-top: 10px; margin-right: 10px; margin-left: 10px;">
                                            <center><div id="Senario" class="span12" style="height: 170px;">
                                                <div class="row-fluid">
                                                    <table style="width: 100%">
                                                        <tr>
                                                            <td><span class="label"><#Wireless_Clients#></span></td>
                                                            <td>&nbsp;</td>
                                                            <td><span class="label label-info"><#Web_Title#></span></td>
                                                            <td class="AP">&nbsp;</td>
                                                            <td class="AP"><span class="label"><#Device_type_02_RT#></span></td>
                                                            <td>&nbsp;</td>
                                                            <td id="Internet_span"><span class="label"><#Internet#></span></td>
                                                        </tr>
                                                        <tr>
                                                            <td><img width="70%" src="bootstrap/img/wl_device/clients.gif"></td>
                                                            <td><img width="70%" src="bootstrap/img/wl_device/arrow-left.gif"></td>
                                                            <td><img width="70%" src="bootstrap/img/wl_device/n56u.gif"></td>
                                                            <td class="AP"><img width="70%" src="bootstrap/img/wl_device/arrow-left.gif"></td>
                                                            <td class="AP"><img width="70%" src="bootstrap/img/wl_device/server.gif"></td>
                                                            <td><img width="70%" src="bootstrap/img/wl_device/arrow-left.gif"></td>
                                                            <td><img width="70%" src="bootstrap/img/wl_device/globe.gif"></td>
                                                        </tr>
                                                    </table>
                                                    <!--<span class=""><img style="margin-right: 5px;" src="bootstrap/img/wl_device/clients.gif"><#Wireless_Clients#></span> -->
                                                    <!--<span style="margin-left: 10px;"><img style="margin-right: 5px;" src="bootstrap/img/wl_device/n56u.gif"><#Web_Title#></span>-->
                                                    <!--<span id="AP" style="margin-left: 10px;"><img style="margin-right: 5px;" src="bootstrap/img/wl_device/arrow-left.gif"><img style="margin-right: 5px;" src="bootstrap/img/wl_device/server.gif"><span class="label label-info"><#Device_type_03_AP#></span></span>-->
                                                    <!--<span id="Internet_span" style="margin-left: 10px;"><img style="margin-right: 5px;" src="bootstrap/img/wl_device/arrow-left.gif"><img style="margin-right: 5px;" src="bootstrap/img/wl_device/globe.gif"><#Internet#></span>-->
                                                </div>

                                                <div id="ap-line" style="display: none;position: absolute; margin-left: -10000px"></div>
                                                <div id="Unplug-hint" style="border:2px solid red; background-color:#FFF; padding:3px;margin:0px 0px 0px 150px;width:250px; position:absolute; display:none;"><#web_redirect_suggestion1#></div>
                                            </div></center>
                                        </div>

                                        <center><input name="button" type="button" class="btn btn-primary" style="width: 219px;" onClick="saveMode();" value="<#CTL_onlysave#>"></center>
                                        <br/>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    </form>

    <div id="help_td" style="position: absolute; margin-left:-10000px; display:none;" valign="top">
        <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>"><img src="images/help.gif" /></div>
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

    <form name="hint_form"></form>
    <div id="footer"></div>
</div>
</body>
</html>
