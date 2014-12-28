<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_4_4#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
	init_itoggle('u2ec_enable');
	init_itoggle('lprd_enable');
});

</script>
<script>

function initial(){
	var id_menu = 5;

	if(!found_app_smbd() && !found_app_ftpd())
		id_menu = 3;
	else if(!found_app_smbd() || !found_app_ftpd())
		id_menu = 4;

	if(get_ap_mode())
		id_menu -= 1;

	show_banner(1);
	show_menu(5,6,id_menu);
	show_footer();

	if(!found_srv_u2ec())
		$("row_u2ec").style.display = "none";

	if(!found_srv_lprd())
		$("row_lprd").style.display = "none";
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

    <input type="hidden" name="current_page" value="Advanced_Printer_others.asp">
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
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_5#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%" style="border-top: 0 none;">
                                                <#PrinterPortRAW#>
                                            </th>
                                            <td style="border-top: 0 none;">
                                                <select name="rawd_enable" class="input">
                                                    <option value="0" <% nvram_match_x("General", "rawd_enable", "0","selected"); %>><#checkbox_No#></option>
                                                    <option value="1" <% nvram_match_x("General", "rawd_enable", "1","selected"); %>><#checkbox_Yes#></option>
                                                    <option value="2" <% nvram_match_x("General", "rawd_enable", "2","selected"); %>><#checkbox_Yes#> (bidirectional)</option>
                                                </select>
                                            </td>
                                        </tr>
                                        <tr id="row_lprd">
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
                                        <tr id="row_u2ec">
                                            <th>
                                                <#PrinterPortU2E#>
                                            </th>
                                            <td>
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

    <div id="footer"></div>
</div>
</body>
</html>
