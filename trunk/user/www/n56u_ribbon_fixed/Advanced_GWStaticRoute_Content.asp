<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_3#></title>
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
        $j('#dr_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                change_common_radio(this, '', 'dr_enable_x', '1');
                $j("#dr_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#dr_enable_x_1").attr("checked", "checked");
                $j("#dr_enable_x_0").removeAttr("checked");
            },
            onClickOff: function(){
                change_common_radio(this, '', 'dr_enable_x', '0');
                $j("#dr_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#dr_enable_x_0").attr("checked", "checked");
                $j("#dr_enable_x_1").removeAttr("checked");
            }
        });
        $j("#dr_enable_x_on_of label.itoggle").css("background-position", $j("input#dr_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#sr_enable_x_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#sr_enable_x_fake").attr("checked", "checked").attr("value", 1);
                $j("#sr_enable_x_1").attr("checked", "checked");
                $j("#sr_enable_x_0").removeAttr("checked");
                change_sr_enabled();
            },
            onClickOff: function(){
                $j("#sr_enable_x_fake").removeAttr("checked").attr("value", 0);
                $j("#sr_enable_x_0").attr("checked", "checked");
                $j("#sr_enable_x_1").removeAttr("checked");
                change_sr_enabled();
            }
        });
        $j("#sr_enable_x_on_of label.itoggle").css("background-position", $j("input#sr_enable_x_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    })
</script>

<script>

var GWStaticList = [<% get_nvram_list("RouterConfig", "GWStatic"); %>];

function initial(){
	show_banner(1);
	show_menu(5,3,3);
	show_footer();
	
	change_sr_enabled();
	
	showGWStaticList();
}

function applyRule(){
	showLoading();
	
	if (rcheck(document.form.sr_enable_x) == "0")
		document.form.action_mode.value = " Apply ";
	else
		document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "/Advanced_GWStaticRoute_Content.asp";
	document.form.next_page.value = "";
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function change_sr_enabled(){
	var a = rcheck(document.form.sr_enable_x);
	if (a == "0"){
		$("tbl_sroutes").style.display = "none";
	} else {
		$("tbl_sroutes").style.display = "";
	}
}

function GWStatic_markGroup(o, s, c, b) {
	document.form.group_id.value = s;
	
	if(b == " Add "){
		if (document.form.sr_num_x_0.value > c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
		else if (!validate_ipaddr_final(document.form.sr_ipaddr_x_0, '') ||
				 !validate_ipaddr_final(document.form.sr_netmask_x_0, '') ||
				 !validate_ipaddr_final(document.form.sr_gateway_x_0, '')){
				 return false;
		}
		else if (document.form.sr_ipaddr_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_ipaddr_x_0.focus();
				 return false;
		}
		else if (document.form.sr_netmask_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_netmask_x_0.focus();
				 return false;
		}
		else if (document.form.sr_gateway_x_0.value == ""){
				 alert("<#JS_fieldblank#>");
				 document.form.sr_gateway_x_0.focus();
				 return false;
		}
		else if (GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_ipaddr_x_0.value, 16, 0) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_netmask_x_0.value, 16, 1) &&
				 GWStatic_validate_duplicate_noalert(GWStaticList, document.form.sr_gateway_x_0.value, 16, 2) &&
				 GWStatic_validate_duplicate(GWStaticList, document.form.sr_if_x_0.value, 2, 4)
				) return false;  //Check the IP, Submask, gateway and Interface is duplicate or not.
	}
	
	pageChanged = 0;
	
	document.form.action_mode.value = b;
	return true;
}

function GWStatic_validate_duplicate_noalert(o, v, l, off){
	for (var i=0; i < o.length; i++)
	{
		if (entry_cmp(o[i][off], v, l)==0){ 
			return true;
		}
	}
	return false;
}

function GWStatic_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++){
		if(entry_cmp(o[i][off].toLowerCase(), v.toLowerCase(), l) == 0){
			alert('<#JS_duplicate#>');
			return true;
		}
	}
	return false;
}

function showGWStaticList(){
	var code = "";
	code +='<table width="100%" cellspacing="0" cellpadding="3" class="table">';
	if(GWStaticList.length == 0)
		code +='<tr><td colspan="6" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < GWStaticList.length; i++){
		code +='<tr id="row' + i + '">';
		code +='<td width="28%">' + GWStaticList[i][0] + '</td>';	//IP
		code +='<td width="22%">' + GWStaticList[i][1] + '</td>';	//Mask
		code +='<td width="22%">' + GWStaticList[i][2] + '</td>';	//Gateway
		code +='<td width="10%">' + GWStaticList[i][3] + '</td>';	//Metric
		code +='<td width="13%">' + GWStaticList[i][4] + '</td>';	//Interface
		code +='<td width="5%"><input type="checkbox" name="GWStatic_s" value="' + i + '" id="check' + i + '"></td>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="5">&nbsp;</td>'
		code += '<td style="padding-left: 0px; margin-right: 0px;" ><button class="btn btn-danger" type="submit" onclick="markGroup(this, \'GWStatic\', 64,\' Del \');"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	
	$("GWStaticList_Block").innerHTML = code;
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
    <input type="hidden" name="current_page" value="Advanced_GWStaticRoute_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="RouterConfig;">
    <input type="hidden" name="group_id" value="GWStatic">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="first_time" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="sr_num_x_0" value="<% nvram_get_x("RouterConfig", "sr_num_x"); %>" readonly="1">

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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#RouterConfig_GWStaticEnable_sectiondesc#></div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="2" style="background-color: #E3E3E3;">
                                                <#menu5_2_3#>
                                            </th>
                                        </tr>
                                        <tr>
                                            <th width="50%"><#RouterConfig_GWDHCPEnable_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="dr_enable_x_on_of">
                                                        <input type="checkbox" id="dr_enable_x_fake" <% nvram_match_x("", "dr_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "dr_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="dr_enable_x" id="dr_enable_x_1" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '1')" <% nvram_match_x("RouterConfig", "dr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="dr_enable_x" id="dr_enable_x_0" class="input" onClick="return change_common_radio(this, 'RouterConfig', 'dr_enable_x', '0')" <% nvram_match_x("RouterConfig", "dr_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th><#RouterConfig_GWStaticEnable_itemname#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="sr_enable_x_on_of">
                                                        <input type="checkbox" id="sr_enable_x_fake" <% nvram_match_x("", "sr_enable_x", "1", "value=1 checked"); %><% nvram_match_x("", "sr_enable_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sr_enable_x" id="sr_enable_x_1" class="input" onclick="change_sr_enabled();" <% nvram_match_x("RouterConfig", "sr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sr_enable_x" id="sr_enable_x_0" class="input" onclick="change_sr_enabled();" <% nvram_match_x("RouterConfig", "sr_enable_x", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table" id="tbl_sroutes">
                                        <tr>
                                            <th colspan="6" style="background-color: #E3E3E3;"><#RouterConfig_GWStatic_groupitemdesc#></th>
                                        </tr>
                                        <tr>
                                            <th width="28%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6,1);"><#RouterConfig_GWStaticIP_itemname#></a></th>
                                            <th width="22%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6,2);"><#RouterConfig_GWStaticMask_itemname#></a></th>
                                            <th width="22%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6,3);"><#RouterConfig_GWStaticGW_itemname#></a></th>
                                            <th width="10%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6,4);"><#RouterConfig_GWStaticMT_itemname#></a></th>
                                            <th width="13%"><a class="help_tooltip" href="javascript:void(0);" onmouseover="openTooltip(this,6,5);"><#RouterConfig_GWStaticIF_itemname#></a></th>
                                            <th width="5%">&nbsp;</th>
                                        </tr>
                                        <tr>
                                            <td><input type="text" maxlength="15" class="span12" size="12" name="sr_ipaddr_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
                                            <td><input type="text" maxlength="15" class="span12" size="12" name="sr_netmask_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
                                            <td><input type="text" maxlength="15" class="span12" size="12" name="sr_gateway_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"/></td>
                                            <td><input type="text" maxlength="3"  class="span12" size="1" name="sr_matric_x_0"  onkeypress="return is_number(this)"></td>
                                            <td>
                                                <select name="sr_if_x_0" class="span12">
                                                    <option value="LAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "LAN","selected", 0); %>>LAN</option>
                                                    <option value="MAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "MAN","selected", 0); %>>MAN</option>
                                                    <option value="WAN" <% nvram_match_list_x("RouterConfig","sr_if_x", "WAN","selected", 0); %>>WAN</option>
                                                </select>
                                            </td>
                                            <td>
                                                <button class="btn" type="submit" onClick="return GWStatic_markGroup(this, 'GWStatic', 64, ' Add ');" name="GWStatic"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="6" style="border-top: 0 none; padding: 0px;">
                                                <div id="GWStaticList_Block"></div>
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
