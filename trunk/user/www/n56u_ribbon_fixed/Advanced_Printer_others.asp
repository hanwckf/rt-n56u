<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<title>Wireless Router <#Web_Title#> - <#menu5_4_4#></title>
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#u2ec_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#u2ec_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#u2ec_enable_1").attr("checked", "checked");
                $j("#u2ec_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#u2ec_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#u2ec_enable_0").attr("checked", "checked");
                $j("#u2ec_enable_1").removeAttr("checked");
            }
        });
        $j("#u2ec_enable_on_of label.itoggle").css("background-position", $j("input#u2ec_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');

        $j('#lprd_enable_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#lprd_enable_fake").attr("checked", "checked").attr("value", 1);
                $j("#lprd_enable_1").attr("checked", "checked");
                $j("#lprd_enable_0").removeAttr("checked");
            },
            onClickOff: function(){
                $j("#lprd_enable_fake").removeAttr("checked").attr("value", 0);
                $j("#lprd_enable_0").attr("checked", "checked");
                $j("#lprd_enable_1").removeAttr("checked");
            }
        });
        $j("#lprd_enable_on_of label.itoggle").css("background-position", $j("input#lprd_enable_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });

</script>

<script>

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	if(sw_mode == "3")
		show_menu(5,7,4);
	else
		show_menu(5,7,5);
	show_footer();
	
//	enable_auto_hint(21, 7);
}

function applyRule(){
	showLoading(); 
	document.form.action_mode.value = " Apply ";
	document.form.current_page.value = "/Advanced_Printer_others.asp";
	document.form.next_page.value = "";
	document.form.submit();
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
</style>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(21, 7);return unload_body();">

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
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log", "productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_Printer_others.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="General;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
    <input type="hidden" name="firmver" value="<% nvram_get_x("", "firmver"); %>">

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
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;">
                                                <#PrinterPortU2E#>
                                            </th>
                                            <td style="border-top: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="u2ec_enable_on_of">
                                                        <input type="checkbox" id="u2ec_enable_fake" <% nvram_match_x("General", "u2ec_enable", "1", "value=1 checked"); %><% nvram_match_x("General", "u2ec_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="u2ec_enable" id="u2ec_enable_1" class="input" <% nvram_match_x("General", "u2ec_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="u2ec_enable" id="u2ec_enable_0" class="input" <% nvram_match_x("General", "u2ec_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#PrinterPortLPR#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="lprd_enable_on_of">
                                                        <input type="checkbox" id="lprd_enable_fake" <% nvram_match_x("General", "lprd_enable", "1", "value=1 checked"); %><% nvram_match_x("General", "lprd_enable", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="lprd_enable" id="lprd_enable_1" class="input" <% nvram_match_x("General", "lprd_enable", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="lprd_enable" id="lprd_enable_0" class="input" <% nvram_match_x("General", "lprd_enable", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <th>
                                                <#PrinterPortRAW#>
                                            </th>
                                            <td>
                                                <select name="rawd_enable" class="input">
                                                    <option value="0" <% nvram_match_x("General", "rawd_enable", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("General", "rawd_enable", "1","selected"); %>><#checkbox_Yes#></option>
                                                    <option value="2" <% nvram_match_x("General", "rawd_enable", "2","selected"); %>><#checkbox_Yes#> (bidirectional)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
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
