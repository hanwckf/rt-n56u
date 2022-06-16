<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_10_2#></title>
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
	show_menu(5,8,2);
	show_footer();

	if (!login_safe())
		textarea_scripts_enabled(0);

	if (get_ap_mode()){
		showhide_div('row_post_wan_script', 0);
		showhide_div('row_post_iptables_script', 0);
	}
}

function textarea_scripts_enabled(v){
	inputCtrl(document.form['scripts.start_script.sh'], v);
	inputCtrl(document.form['scripts.started_script.sh'], v);
	inputCtrl(document.form['scripts.shutdown_script.sh'], v);
	inputCtrl(document.form['scripts.post_wan_script.sh'], v);
	inputCtrl(document.form['scripts.post_iptables_script.sh'], v);
	inputCtrl(document.form['scripts.ez_buttons_script.sh'], v);
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_Scripts_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
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

    <input type="hidden" name="current_page" value="Advanced_Scripts_Content.asp">
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
                            <h2 class="box_head round_top"><#menu5_10#> - <#menu5_10_2#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;"><#Scripts_desc#></div>

                                    <table  width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th style="background-color: #E3E3E3;"><#UserScripts#></th>
                                        </tr>
                                        <tr>
                                            <td>
                                                <a href="javascript:spoiler_toggle('script0')"><span><#RunPreStart#></span></a>
                                                <div id="script0" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="4096" class="span12" name="scripts.start_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.start_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>
                                                <a href="javascript:spoiler_toggle('script1')"><span><#RunPostStart#></span></a>
                                                <div id="script1" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.started_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.started_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td>
                                                <a href="javascript:spoiler_toggle('script5')"><span><#RunShutdown#></span></a>
                                                <div id="script5" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="4096" class="span12" name="scripts.shutdown_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.shutdown_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_post_wan_script">
                                            <td>
                                                <a href="javascript:spoiler_toggle('script2')"><span><#RunPostWAN#></span></a>
                                                <div id="script2" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.post_wan_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.post_wan_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr id="row_post_iptables_script">
                                            <td>
                                                <a href="javascript:spoiler_toggle('script3')"><span><#RunPostFWL#></span></a>
                                                <div id="script3" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="8192" class="span12" name="scripts.post_iptables_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.post_iptables_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td style="padding-bottom: 0px;">
                                                <a href="javascript:spoiler_toggle('script4')"><span><#RunEzBtns#></span></a>
                                                <div id="script4" style="display:none;">
                                                    <textarea rows="24" wrap="off" spellcheck="false" maxlength="4096" class="span12" name="scripts.ez_buttons_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ez_buttons_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
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
