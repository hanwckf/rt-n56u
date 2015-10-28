<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_6_3#></title>
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
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();

<% login_state_hook(); %>

function initial(){
	show_banner(1);
	show_menu(5,7,4);
	show_footer();

	if (!login_safe()){
		inputCtrl(document.form.file, 0);
		inputCtrl(document.form.button, 0);
	}
}

function fwUpload(){
	if (!document.form.file.value) {
		alert("<#JS_Shareblanktest#>");
		document.form.file.focus();
		return false;
	}
	disableCheckChangedStatus();
	document.form.submit();
}

$j.fn.fileName = function(){
	var $this = $j(this),
	$val = $this.val(),
	valArray = $val.split('\\'),
	newVal = valArray[valArray.length-1],
	$button = $this.siblings('.button');
	if(newVal !== '') {
		newVal = newVal.substring(0,26);
		$button.text(newVal);
	}
};

</script>
<style>
.file {
	display: inline-block;
	width:218px;
	position: relative;
	-moz-border-radius: 4px;
	-webkit-border-radius:4px;
	border-radius: 4px;
	margin-bottom:0px;
	text-align: center;

	background-color: #f5f5f5;
	*background-color: #e6e6e6;
	background-image: -ms-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -webkit-gradient(linear, 0 0, 0 100%, from(#ffffff), to(#e6e6e6));
	background-image: -webkit-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -o-linear-gradient(top, #ffffff, #e6e6e6);
	background-image: linear-gradient(top, #ffffff, #e6e6e6);
	background-image: -moz-linear-gradient(top, #ffffff, #e6e6e6);
	border: 1px solid #ddd;
}
/* style text of the upload field and add an attachment icon */
.file .button {
	font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
	font-size:11px;
	color:#555;
	height:27px;
	line-height:26px;
	display: block;
}
/* hide the real file upload input field */
.file input {
	cursor: pointer;
	height: 100%;
	position: absolute;
	right: 0;
	top: 0;
	filter: alpha(opacity=1);
	-moz-opacity: 0.01;
	font-size: 100px;
}
</style>
</head>

<body onload="initial();">

<div class="wrapper">
    <div id="LoadingBar" class="popup_bg">
        <center>
        <div class="container-fluid" style="margin-top: 150px;">
            <div class="well" style="background-color: #212121; width: 60%;">
                <div class="progress" style="max-width: 450px; text-align: left;">
                    <div class="bar" id="proceeding_img"><span id="proceeding_img_text"></span></div>
                </div>
                <div class="alert alert-danger" style="max-width: 400px;"><#FIRM_ok_desc#></div>
            </div>
        </div>
        </center>

        <table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" style="position: absolute; margin-left: -10000px;" align="center">
            <tr>
                <td height="80">
                    <div class="progress">
                    </div>
                    <div class="alert alert-info"><#FIRM_ok_desc#></div>
                </td>
            </tr>
        </table>

    </div>
    <div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->

    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" action="upgrade.cgi" name="form" target="hidden_frame" enctype="multipart/form-data">
    <input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="action_mode" value="">

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
                            <h2 class="box_head round_top"><#menu5_6#> - <#menu5_6_3#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;">
                                        <#FW_desc1#>
                                        <ol>
                                            <li><#FW_desc2#></li>
                                            <li><#FW_desc3#></li>
                                            <li><#FW_desc4#></li>
                                            <li><#FW_desc5#></li>
                                            <li><#FW_desc6#></li>
                                        </ol>
                                    </div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th width="50%"><#FW_item1#></th>
                                            <td><input type="text" class="input" value="<% nvram_get_x("", "productid"); %>" readonly="1"></td>
                                        </tr>
                                        <tr>
                                            <th><#FW_item2#></th>
                                            <td><input type="text" name="firmver" class="input" value="<% nvram_get_x("",  "firmver_sub"); %>" readonly="1"></td>
                                        </tr>
                                        <tr>
                                            <th><#FW_item5#></th>
                                            <td>
                                                <input type="file" name="file" size="36" />
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="2">
                                                <center><input type="button" name="button" class="btn btn-primary" style="width: 219px;" onclick="fwUpload();" value="<#CTL_upload#>" /></center>
                                            </td>
                                        </tr>
                                    </table>

                                    <div class="alert alert-info" style="margin-left: 10px; margin-right: 10px;">
                                        <strong><#FW_note#></strong>
                                        <ol>
                                            <li><#FW_n1#></li>
                                            <li><#FW_n2#></li>
                                        </ol>
                                    </div>

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
