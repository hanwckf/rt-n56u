<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_10_3#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,8,3);
	show_footer();
	load_body();

	if(get_ap_mode())
		showhide_div('row_lost_action', 0);

	poll_mode_changed();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_InetDetect_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	var i;
	var obj;
	for (i=0;i<6;i++) {
		obj=document.getElementsByName("di_addr"+i)[0];
		if(!validate_ipaddr_final(obj, ''))
			return false;
		obj=document.getElementsByName("di_port"+i)[0];
		if(obj.value.length > 0 && !validate_range(obj, 1, 65535))
			return false;
	}

	if(!validate_range(document.form.di_time_done, 15, 600))
		return false;
	if(!validate_range(document.form.di_time_fail, 3, 120))
		return false;
	if(!validate_range(document.form.di_timeout, 1, 10))
		return false;

	if (document.form.di_poll_mode.value == "1"){
		if(!validate_range(document.form.di_lost_delay, 0, 600))
			return false;
		
		if (document.form.di_lost_action.value == "2" && !get_ap_mode()){
			if(!validate_range(document.form.di_recon_pause, 0, 600))
				return false;
		}
	}

	return true;
}

function poll_mode_changed(){
	var v = (document.form.di_poll_mode.value == "1") ? 1 : 0;
	if (v)
		lost_action_changed();
	showhide_div('tbl_di_events', v);
}

function lost_action_changed(){
	var v = (document.form.di_lost_action.value == "2" && !get_ap_mode()) ? 1 : 0;
	showhide_div('row_recon_pause', v);
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
    <input type="hidden" name="current_page" value="Advanced_InetDetect_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="General;">
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
                            <h2 class="box_head round_top"><#menu5_10#> - <#menu5_10_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#InetCheck_desc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#InetCheckMode#></th>
                                            <td>
                                                <select name="di_poll_mode" class="input" onchange="poll_mode_changed();">
                                                    <option value="0" <% nvram_match_x("", "di_poll_mode", "0", "selected"); %>><#InetCheckModeItem0#> (*)</option>
                                                    <option value="1" <% nvram_match_x("", "di_poll_mode", "1", "selected"); %>><#InetCheckModeItem1#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#InetCheckHosts#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#InetCheckHostIP4#> 1:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr0" style="width: 145px" value="<% nvram_get_x("","di_addr0"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port0" style="width: 44px;"  value="<% nvram_get_x("","di_port0"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckHostIP4#> 2:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr1" style="width: 145px" value="<% nvram_get_x("","di_addr1"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port1" style="width: 44px;"  value="<% nvram_get_x("","di_port1"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckHostIP4#> 3:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr2" style="width: 145px" value="<% nvram_get_x("","di_addr2"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port2" style="width: 44px;"  value="<% nvram_get_x("","di_port2"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckHostIP4#> 4:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr3" style="width: 145px" value="<% nvram_get_x("","di_addr3"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port3" style="width: 44px;"  value="<% nvram_get_x("","di_port3"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckHostIP4#> 5:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr4" style="width: 145px" value="<% nvram_get_x("","di_addr4"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port4" style="width: 44px;"  value="<% nvram_get_x("","di_port4"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckHostIP4#> 6:</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="di_addr5" style="width: 145px" value="<% nvram_get_x("","di_addr5"); %>" onkeypress="return is_ipaddr(this,event);"/>&nbsp;:
                                                <input type="text" maxlength="5" class="input" size="10" name="di_port5" style="width: 44px;"  value="<% nvram_get_x("","di_port5"); %>" onkeypress="return is_number(this,event);"/>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#InetCheckPoll#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#InetCheckPeriod#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" style="width: 94px;" name="di_time_done" value="<% nvram_get_x("", "di_time_done"); %>" onkeypress="return is_number(this,event);"/>&nbsp;/
                                                <input type="text" maxlength="3" class="input" size="15" style="width: 94px;" name="di_time_fail" value="<% nvram_get_x("", "di_time_fail"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[ 55 / 5 ]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#InetCheckTimeout#></th>
                                            <td>
                                                <input type="text" maxlength="2" class="input" size="15" name="di_timeout" value="<% nvram_get_x("", "di_timeout"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[1..10]</span>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="tbl_di_events">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;"><#InetCheckEvents#></th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#InetCheckLostDelay#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="di_lost_delay" value="<% nvram_get_x("", "di_lost_delay"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[0..600]</span>
                                            </td>
                                        </tr>
                                        <tr id="row_lost_action">
                                            <th><#InetCheckLostAction#></th>
                                            <td>
                                                <select name="di_lost_action" class="input" style="width: 320px;" onchange="lost_action_changed();">
                                                    <option value="0" <% nvram_match_x("", "di_lost_action", "0", "selected"); %>><#InetCheckLostItem0#></option>
                                                    <option value="1" <% nvram_match_x("", "di_lost_action", "1", "selected"); %>><#InetCheckLostItem1#></option>
                                                    <option value="2" <% nvram_match_x("", "di_lost_action", "2", "selected"); %>><#InetCheckLostItem2#></option>
                                                    <option value="3" <% nvram_match_x("", "di_lost_action", "3", "selected"); %>><#InetCheckLostItem3#></option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_recon_pause" style="display:none;">
                                            <th><#InetCheckReconPause#></th>
                                            <td>
                                                <input type="text" maxlength="3" class="input" size="15" name="di_recon_pause" value="<% nvram_get_x("", "di_recon_pause"); %>" onkeypress="return is_number(this,event);"/>
                                                &nbsp;<span style="color:#888;">[0..600]</span>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2" style="padding-bottom: 0px;">
                                                <a href="javascript:spoiler_toggle('script3')"><span><#RunInetState#></span></a>
                                                <div id="script3" style="display:none;">
                                                    <textarea rows="16" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.inet_state_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.inet_state_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table class="table">
                                        <tr>
                                            <td style="border: 0 none;">
                                                <center><input type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="<#CTL_apply#>"/></center>
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
