<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_6_6#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,7,6);
	show_footer();

	if (!login_safe()){
		$j('#btn_exec').attr('disabled', 'disabled');
		$j('#SystemCmd').attr('disabled', 'disabled');
	}else
		document.form.SystemCmd.focus();
}

function getResponse(){
	$j.get('/console_response.asp', function(data){
		var response = ($j.browser.msie && !is_ie11p) ? data.nl2br() : data;
		$j("#console_area").text(response);
		$j('#btn_exec').removeAttr('disabled');
	});
}

function startPost(){
	if (!login_safe())
		return false;
	$j('#btn_exec').attr('disabled', 'disabled');
	$j.post('/apply.cgi',
	{
		'action_mode': ' SystemCmd ',
		'current_page': 'console_response.asp',
		'next_page': 'console_response.asp',
		'SystemCmd': $j('#SystemCmd').val()
	},
	function(response){
		getResponse();
	});
}

function clearOut(){
	$j('#console_area').html('');
	$j('#SystemCmd').val('');
}

function checkEnter(e){
	e = e || event;
	return (e.keyCode || event.which || event.charCode || 0) === 13;
}
</script>
</head>

<body onLoad="initial();" >

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

    <form method="post" name="form" action="apply.cgi" onkeypress="return !checkEnter(event)">
    <input type="hidden" name="current_page" value="">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="">
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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_6#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-danger" style="margin: 10px;"><#Console_warn#></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <td width="80%" style="border-top: 0 none"><input type="text" id="SystemCmd" class="span12" name="SystemCmd" maxlength="127" onkeypress="if (checkEnter(event)) startPost();" value=""></td>
                                            <td style="border-top: 0 none"><input class="btn btn-primary span12" id="btn_exec" onClick="startPost()" type="button" value="<#CTL_refresh#>" name="action"></td>
                                            <td style="border-top: 0 none"><button class="btn span12" onClick="clearOut();" type="button" value="<#CTL_refresh#>" name="action" style="outline: 0"><i class="icon icon-remove"></i></button></td>
                                        </tr>
                                        <tr>
                                            <td colspan="3" style="border-top: 0 none">
                                                <textarea class="span12" id="console_area" style="font-family: 'Courier New', Courier, mono; font-size:13px;" rows="23" wrap="off" readonly="1"><% nvram_dump("syscmd.log","syscmd.sh"); %></textarea>
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
