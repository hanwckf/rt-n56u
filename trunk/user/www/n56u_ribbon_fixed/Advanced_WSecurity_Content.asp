<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_1_5#></title>
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
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/wireless.js"></script>
<script type="text/javascript" src="/help_wl.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

function initial(){
	show_banner(1);
	show_menu(5,2,5);
	show_footer();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_WSecurity_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(!validate_ipaddr_final(document.form.wl_radius_ipaddr, 'radius_ipaddr'))
		return false;

	if(!validate_range(document.form.wl_radius_port, 0, 65535))
		return false;

	if(!validate_string(document.form.wl_radius_key))
		return false;

	return true;
}

function done_validating(action){
	refreshpage();
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
    <input type="hidden" name="current_page" value="Advanced_WSecurity_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="WLANAuthentication11a;WLANConfig11a;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

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
                            <h2 class="box_head round_top"><#menu5_1#> - <#t2RADIUS#> (5GHz)</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#WLANAuthentication11a_display1_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,2,1);"><#WLANAuthentication11a_ExAuthDBIPAddr_itemname#></a></th>
                                            <td width="50%" style="border-top: 0 none;">
                                                <input type="text" maxlength="15" class="input" size="15" name="wl_radius_ipaddr" value="<% nvram_get_x("","wl_radius_ipaddr"); %>" onKeyPress="return is_ipaddr(this,event);" />
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);"  onmouseover="openTooltip(this,2,2);"><#WLANAuthentication11a_ExAuthDBPortNumber_itemname#></a></th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="5" name="wl_radius_port" value="<% nvram_get_x("","wl_radius_port"); %>" onkeypress="return is_number(this,event);" onblur="return validate_portrange(this, '');"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><a class="help_tooltip" href="javascript:void(0);"  onmouseover="openTooltip(this,2,3);"><#WLANAuthentication11a_ExAuthDBPassword_itemname#></a></th>
                                            <td>
                                                <div class="input-append">
                                                    <input type="password" name="wl_radius_key" id="wl_radius_key" maxlength="64" size="32" style="width: 175px;" value="<% nvram_get_x("","wl_radius_key"); %>"/>
                                                    <button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('wl_radius_key')"><i class="icon-eye-close"></i></button>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="margin-top: 10px; border-top: 0 none;">
                                                <input class="btn btn-info" type="button"  value="<#GO_2G#>" onclick="location.href='Advanced_WSecurity2g_Content.asp';"/>
                                            </td>
                                            <td style="border-top: 0 none;">
                                                <input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" />
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
