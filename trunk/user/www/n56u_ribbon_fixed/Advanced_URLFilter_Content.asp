<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_2#></title>
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
        $j('#url_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#url_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#url_enable_x_11").attr("checked", "checked");
                $j("#url_enable_x_00").removeAttr("checked");
                enable_url();
            },
            onClickOff: function(){
                $j("#url_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#url_enable_x_00").attr("checked", "checked");
                $j("#url_enable_x_11").removeAttr("checked");
                enable_url();
            }
        });
        $j("#url_enable_x_on_of label.itoggle").css("background-position", $j("input#url_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#url_enable_x_1_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#url_enable_x_1_fake").attr("checked", "checked").attr("value", 1);
                $j("#url_enable_x_1_1").attr("checked", "checked");
                $j("#url_enable_x_1_0").removeAttr("checked");
                enable_url_1();
            },
            onClickOff: function(){
                $j("#url_enable_x_1_fake").removeAttr("checked").attr("value", 0);
                $j("#url_enable_x_1_0").attr("checked", "checked");
                $j("#url_enable_x_1_1").removeAttr("checked");
                enable_url_1();
            }
        });
        $j("#url_enable_x_1_on_of label.itoggle").css("background-position", $j("input#url_enable_x_1_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script>

function initial(){
	show_banner(1);
	show_menu(5,5,3);
	show_footer();
	
	enable_url();
	enable_url_1();
	load_body();
}

function applyRule(){
	if(validForm()){
		updateDateTime(document.form.current_page.value);
		
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_URLFilter_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	if(((document.form.url_enable_x[0].checked ==true) || (document.form.url_enable_x_1[0].checked ==true))
		&& (document.form.url_date_x_Sun.checked ==false)
		&& (document.form.url_date_x_Mon.checked ==false)
		&& (document.form.url_date_x_Tue.checked ==false)
		&& (document.form.url_date_x_Wed.checked ==false)
		&& (document.form.url_date_x_Thu.checked ==false)
		&& (document.form.url_date_x_Fri.checked ==false)
		&& (document.form.url_date_x_Sat.checked ==false)){
			alert("<#FirewallConfig_KeywordActiveDate_itemname#><#JS_fieldblank#>");
			document.form.url_enable_x[0].checked=false;
			document.form.url_enable_x[1].checked=true;
			document.form.url_enable_x_1[0].checked=false;
			document.form.url_enable_x_1[1].checked=true;
			return false;
	}

	if(document.form.url_enable_x[0].checked == 1){
		if(!validate_timerange(document.form.url_time_x_starthour, 0)
			|| !validate_timerange(document.form.url_time_x_startmin, 1)
			|| !validate_timerange(document.form.url_time_x_endhour, 2)
			|| !validate_timerange(document.form.url_time_x_endmin, 3)
			){return false;}

		var starttime = eval(document.form.url_time_x_starthour.value + document.form.url_time_x_startmin.value);
		var endtime = eval(document.form.url_time_x_endhour.value + document.form.url_time_x_endmin.value);

		if(starttime > endtime){
			alert("<#FirewallConfig_URLActiveTime_itemhint#>");
			document.form.url_time_x_starthour.focus();
			document.form.url_time_x_starthour.select();
			return false;
		}
		if(starttime == endtime){
			alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.url_time_x_starthour.focus();
			document.form.url_time_x_starthour.select();
			return false;
		}
	}

	if(document.form.url_enable_x_1[0].checked == 1){
		if(!validate_timerange(document.form.url_time_x_starthour_1, 0)
			|| !validate_timerange(document.form.url_time_x_startmin_1, 1)
			|| !validate_timerange(document.form.url_time_x_endhour_1, 2)
			|| !validate_timerange(document.form.url_time_x_endmin_1, 3)
			){return false;}

		var starttime_1 = eval(document.form.url_time_x_starthour_1.value + document.form.url_time_x_startmin_1.value);
		var endtime_1 = eval(document.form.url_time_x_endhour_1.value + document.form.url_time_x_endmin_1.value);
		
		if(starttime_1 > endtime_1){
			alert("<#FirewallConfig_URLActiveTime_itemhint#>");
			document.form.url_time_x_starthour_1.focus();
			document.form.url_time_x_starthour_1.select();
			return false;
		}
		if(starttime_1 == endtime_1){
			alert("<#FirewallConfig_URLActiveTime_itemhint2#>");
			document.form.url_time_x_starthour_1.focus();
			document.form.url_time_x_starthour_1.select();
			return false;
		}
	}

	if(document.form.url_enable_x[0].checked == 1 && document.form.url_enable_x_1[0].checked == 1){
		if(starttime < starttime_1){
			if(!(endtime < starttime_1)){
				alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
				return false; 
			}
		}
		if(starttime_1 < starttime){
			if(!(endtime_1 < starttime)){
				alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
				return false; 
			}
		}
		if(starttime == starttime_1){
			alert("<#FirewallConfig_URLActiveTime_itemhint4#>");
			return false;
		}
	}
	return true;
}

function enable_url(){
	if(document.form.url_enable_x[1].checked == 1)
		$("url_time").style.display = "none";
	else 
		$("url_time").style.display = "";	
	return change_common_radio(this, 'FirewallConfig', 'url_enable_x', '1')
}

function enable_url_1(){
	if(document.form.url_enable_x_1[1].checked == 1)
		$("url_time_1").style.display = "none";
	else 
		$("url_time_1").style.display = "";
	return change_common_radio(this, 'FirewallConfig', 'url_enable_x_1', '1')
}

function done_validating(action){
	refreshpage();
}
</script>
<style>
    .nav-tabs > li > a {
          padding-right: 6px;
          padding-left: 6px;
    }
    .radio.inline + .radio.inline,
    .checkbox.inline + .checkbox.inline {
      margin-left: 3px;
    }
</style>
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
    <input type="hidden" name="current_page" value="Advanced_URLFilter_Content.asp">
    <input type="hidden" name="next_page" value="Advanced_URLFilter_Content.asp">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="FirewallConfig;">
    <input type="hidden" name="group_id" value="UrlList">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">

    <input type="hidden" name="url_date_x" value="<% nvram_get_x("FirewallConfig","url_date_x"); %>">
    <input type="hidden" name="url_time_x" value="<% nvram_get_x("FirewallConfig","url_time_x"); %>">
    <input type="hidden" name="url_time_x_1" value="<% nvram_get_x("FirewallConfig","url_time_x_1"); %>">
    <input type="hidden" name="url_num_x_0" value="<% nvram_get_x("FirewallConfig", "url_num_x"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#menu5_5#> - <#menu5_5_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#FirewallConfig_UrlFilterEnable_sectiondesc#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#FirewallConfig_UrlFilterEnable_itemname#> 1?</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="url_enable_x_on_of">
                                                        <input type="checkbox" id="url_enable_x_fake" <% nvram_match_x("FirewallConfig", "url_enable_x", "1", "value=1 checked"); %><% nvram_match_x("FirewallConfig", "url_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="url_enable_x" id="url_enable_x_11" onClick="enable_url();" <% nvram_match_x("FirewallConfig","url_enable_x", "1", "checked"); %>><#CTL_Enabled#>
                                                    <input type="radio" value="0" name="url_enable_x" id="url_enable_x_00" onClick="enable_url();" <% nvram_match_x("FirewallConfig","url_enable_x", "0", "checked"); %>><#CTL_Disabled#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="url_time">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,9,2);"><#FirewallConfig_KeywordActiveTime_itemname#> 1:</a></th>
                                            <td>
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_starthour" onKeyPress="return is_number(this)">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_startmin" onKeyPress="return is_number(this)">-
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endhour" onKeyPress="return is_number(this)">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endmin" onKeyPress="return is_number(this)">
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#FirewallConfig_UrlFilterEnable_itemname#> 2?</th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="url_enable_x_1_on_of">
                                                        <input type="checkbox" id="url_enable_x_1_fake" <% nvram_match_x("FirewallConfig", "url_enable_x_1", "1", "value=1 checked"); %><% nvram_match_x("FirewallConfig", "url_enable_x_1", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="url_enable_x_1" id="url_enable_x_1_1" onClick="enable_url_1();" <% nvram_match_x("FirewallConfig","url_enable_x_1", "1", "checked"); %>><#CTL_Enabled#>
                                                    <input type="radio" value="0" name="url_enable_x_1" id="url_enable_x_1_0" onClick="enable_url_1();" <% nvram_match_x("FirewallConfig","url_enable_x_1", "0", "checked"); %>><#CTL_Disabled#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="url_time_1">
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,9,2);"><#FirewallConfig_KeywordActiveTime_itemname#> 2:</a></th>
                                            <td>
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_starthour_1" onKeyPress="return is_number(this)">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_startmin_1" onKeyPress="return is_number(this)">-
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endhour_1" onKeyPress="return is_number(this)">:
                                                <input type="text" maxlength="2" class="input" style="width: 25px;" size="2" name="url_time_x_endmin_1" onKeyPress="return is_number(this)">
                                            </td>
                                        </tr>

                                        <tr>
                                            <th><a class="help_tooltip"  href="javascript:void(0);" onmouseover="openTooltip(this,9,1);"><#FirewallConfig_KeywordActiveDate_itemname#></a></th>
                                            <td>
                                                <div class="controls">
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Mon" class="input" onChange="return changeDate();"><#DAY_Mon#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Tue" class="input" onChange="return changeDate();"><#DAY_Tue#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Wed" class="input" onChange="return changeDate();"><#DAY_Wed#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Thu" class="input" onChange="return changeDate();"><#DAY_Thu#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Fri" class="input" onChange="return changeDate();"><#DAY_Fri#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Sat" class="input" onChange="return changeDate();"><#DAY_Sat#></label>
                                                    <label class="checkbox inline"><input type="checkbox" name="url_date_x_Sun" class="input" onChange="return changeDate();"><#DAY_Sun#></label>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th id="UrlList"><#FirewallConfig_UrlList_groupitemdesc#></th>
                                            <td style="padding: 8px;">
                                                <input type="text" maxlength="32" size="36" name="url_keyword_x_0" style="float: left; max-width: 450px; min-width: 260px;" onKeyPress="return is_string(this)">
                                                <button class="btn" style="margin-left: 5px;" type="submit" onClick="if(validForm()){return markGroup(this, 'UrlList', 128, ' Add ');}" name="UrlList" size="12"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>&nbsp;</th>
                                            <td style="padding: 8px;">
                                                <select size="8" name="UrlList_s" multiple="multiple" style="max-width: 460px; min-width: 270px; vertical-align:middle; float: left;">
                                                    <% nvram_get_table_x("FirewallConfig","UrlList"); %>
                                                </select>
                                                <button class="btn btn-danger" style="margin-left: 5px;" type="submit" onClick="return markGroup(this, 'UrlList', 128, ' Del ');" name="UrlList" size="12"><i class="icon icon-minus icon-white"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <br />
                                                <center><input class="btn btn-primary" style="width: 219px" onclick="applyRule();" type="button" value="<#CTL_apply#>" /></center>
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
