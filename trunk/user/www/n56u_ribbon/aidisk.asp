<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<!-- <meta name="viewport" content="width=device-width, initial-scale=1.0"> -->
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu3#></title>

<!-- <link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css"> -->
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

<% disk_pool_mapping_info(); %>
<% available_disk_names_and_sizes(); %>
<% get_AiDisk_status(); %>

var account_num;
var accounts;

var pools = [];
var folderlist = [];

var page = parseInt('<% get_parameter("page"); %>'-'0');

function initial(){
	$("statusframe").style.display = "block";
	
	if(page == 2)
		show_iframe("aidisk/Aidisk-2.asp");
	else if(page == 3)
		show_iframe("aidisk/Aidisk-3.asp");
	else if(page == 4)
		show_iframe("aidisk/Aidisk-4.asp");
	else
		show_iframe("aidisk/Aidisk-1.asp");
	
	if($("dummyShareway").value == "")
		$("dummyShareway").value = 0;
		
	show_banner(0);
	show_menu(2, -1, 0);
	show_footer();
}

function show_iframe(src){
	$("sub_frame").src = "";
	
	setTimeout('$("sub_frame").src = \"'+src+'\";', 1);
}

function show_iframe_page(iframe_id){
	if(iframe_id)
		if($(iframe_id))
			return $(iframe_id).src;
	
	return "";
}

function show_help_iframe(page_num){
	var page_img = "";
	var page_title = "";
	var page_src = "";

	$("hint_body").style.display = "none";
	$("statusframe").style.display = "block";
	if(page_num == 2){
		page_img = "/images/iframe-iconRouter.gif";
		page_title = "Account Management";
		page_src = "/aidisk/Aidisk-2_help.asp";
	}
	else if(page_num == 3){
		page_img = "/images/iframe-iconRouter.gif";
		page_title = "DDNS";
		page_src = "/aidisk/Aidisk-3_help.asp";
	}
	else if(page_num == 4){
		page_img = "/images/iframe-iconRouter.gif";
		page_title = "Advanced Setting";
		page_src = "/aidisk/Aidisk-4_help.asp";
	}
	else if(page_num == 5){
		big_help_td();
		page_title = "ASUS DDNS <#DDNS_termofservice_Title#>";
		page_src = "/aidisk/ASUS_DDNS_TOS.asp";
	}
	else{
		page_img = "/images/iframe-iconRouter.gif";
		page_title = "<#menu3#>";
		page_src = "/aidisk/Aidisk-1_help.asp";
	}
	
	showtext($("helpname"), page_title);
	setTimeout('$("statusframe").src = \"'+page_src+'\";', 1);
}

function big_help_td(){
	//$("help_td").style.width = "300px";
	//$("statusframe").style.width = "300px";
}

function restore_help_td(){
	//$("hint_body").style.display = "block"; //avoid this block to be hide when open ASUS TOS in step2
	//$("help_td").style.width = "170px";
	//$("statusframe").style.width = "200px";
}

function get_account_parameter(){
	account_num = $("accountNum").value;
	
	accounts = new Array(account_num);
	
	for(var i = 0; i < account_num; ++i){
		accounts[i] = new Array(3);
		
		accounts[i][0] = $("account"+i).value;
		accounts[i][1] = $("passwd"+i).value;
		accounts[i][2] = $("permission"+i).value;
	}
}

function initialAccount(){
	document.applyForm.action = "/aidisk/initial_account.asp";
	document.applyForm.submit();
}

function resultOfInitialAccount(){
	createAccount();
}

function createAccount(){
	if(accounts[0]){
		document.applyForm.protocol.value = "";
		document.applyForm.pool.value = "";
		document.applyForm.folder.value = "";
		document.applyForm.permission.value = "";
		
		document.applyForm.action = "/aidisk/create_account.asp";
		document.applyForm.account.value = accounts[0][0];
		document.applyForm.password.value = accounts[0][1];
		
		document.applyForm.submit();
	}
	else
		alert("Wrong! No account!");	// No translate
}

function resultOfCreateAccount(){
	pools = pool_names();
	if(pools && pools.length > 0)
		folderlist = get_sharedfolder_in_pool(pools[0]);
	
	submitChangePermission("ftp");
}

function submitChangePermission(protocol){
	if(pools && pools.length > 0){
		if(folderlist && folderlist.length > 0){
			document.applyForm.password.value = "";
			
			document.applyForm.action = "/aidisk/set_account_permission.asp";
			document.applyForm.account.value = accounts[0][0];
			document.applyForm.pool.value = pools[0];
			document.applyForm.folder.value = folderlist[0];
			document.applyForm.protocol.value = protocol;
			document.applyForm.permission.value = accounts[0][2];
			document.applyForm.flag.value = "aidisk_wizard";
			
			folderlist.shift();
			document.applyForm.submit();
			return;
		}
		else{
			pools.shift();
			
			if(pools && pools.length > 0){
				folderlist = get_sharedfolder_in_pool(pools[0]);
			
				submitChangePermission(protocol);
				return;
			}
		}
	}
	
	accounts.shift();
	
	if(accounts.length > 0)
		createAccount();
	else
		switchShareMode("ftp", "account");
}

function switchShareMode(protocol, mode){

	document.applyForm.account.value = "";
	document.applyForm.pool.value = "";
	document.applyForm.folder.value = "";
	document.applyForm.permission.value = "";
	
	document.applyForm.action = "/aidisk/switch_share_mode.asp";
	document.applyForm.protocol.value = protocol;
	document.applyForm.mode.value = mode;

	document.applyForm.submit();
}

function resultOfSwitchShareMode(){
	switchAppStatus("ftp", "on");
}

function switchAppStatus(protocol, flag){
	document.applyForm.mode.value = "";
	
	document.applyForm.action = "/aidisk/switch_AiDisk_app.asp";
	document.applyForm.protocol.value = protocol;
	document.applyForm.flag.value = flag;

	document.applyForm.submit();
}

function resultOfSwitchAppStatus(error_msg){
	finish_dummyway_setting();
}

function finish_dummyway_setting(){
	switchDDNS();
}

function switchDDNS(){
	document.ddnsForm.ddns_enable_x.value = getASUSDDNS_enable();
	document.ddnsForm.current_page.value = document.parameterForm.next_page.value;
	
	document.ddnsForm.submit();
}

var ddns_enable_x = '<% nvram_get_x("LANHostConfig", "ddns_enable_x"); %>';

function setASUSDDNS_enable(flag){
		this.ddns_enable_x = flag;
}

function getASUSDDNS_enable(){
	return this.ddns_enable_x;
}
</script>
</head>

<body onload="initial();" onunload="return unload_body();">
<!-- top bar -->
<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no" style="position: absolute;"></iframe>

<form method="post" name="applyForm" id="applyForm" action="" target="hidden_frame">
<input type="hidden" name="dummyShareway" id="dummyShareway" value="<% nvram_get_x("", "dummyShareway"); %>">
<input type="hidden" name="account" id="account" value="">
<input type="hidden" name="password" id="password" value="">
<input type="hidden" name="protocol" id="protocol" value="">
<input type="hidden" name="mode" id="mode" value="">
<input type="hidden" name="pool" id="pool" value="">
<input type="hidden" name="folder" id="folder" value="">
<input type="hidden" name="permission" id="permission" value="">
<input type="hidden" name="flag" id="flag" value="on">
</form>

<form method="post" name="ddnsForm" id="ddnsForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="ddns_enable_x" value="">
</form>

<form method="post" name="parameterForm" id="parameterForm" action="" target="">
<input type="hidden" name="next_page" id="next_page" value="">
<input type="hidden" name="accountNum" id="accountNum" value="0">
<input type="hidden" name="account0" id="account0" value="">
<input type="hidden" name="passwd0" id="passwd0" value="">                                
<input type="hidden" name="permission0" id="permission0" value="">
<input type="hidden" name="account1" id="account1" value="">
<input type="hidden" name="passwd1" id="passwd1" value="">
<input type="hidden" name="permission1" id="permission1" value="">
</form>

<form name="form">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span2">
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

         <div class="span10">
            <!--Body content-->

            <div class="row-fluid">
                <div class="span12">
                    <div class="box well grad_colour_dark_blue">
                        <h2 class="box_head round_top">AiDisk</h2>
                        <div class="round_bottom">

                            <div class="row-fluid">
                                <div class="span9">
                                    <div id="tabMenu"></div>
                                    <iframe id="sub_frame" src="" width="100%" height="455" frameborder="0" scrolling="no"></iframe>
                                    <form name="hint_form"></form>
                                    <div id="helpicon" style="display: none;"></div>
                                </div>
                                <div class="span3" style="margin-top: 22px;">
                                    <div id="hintofPM" style="display:none;">
                                        <table width="100%" cellpadding="0" cellspacing="1">
                                            <thead>
                                            <tr>
                                                <td>
                                                    <h4 id="helpname" class="AiHintTitle"></h4>
                                                </td>
                                            </tr>
                                            </thead>
                                            <tr>
                                                <td valign="top">
                                                    <div class="alert alert-info" id="hint_body"></div>
                                                    <iframe id="statusframe" name="statusframe" class="statusframe" src="Aidisk-1_help.asp" style="width: 97%; height: 300px;" frameborder="0" /></iframe>
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
    </div>
</div>

<div id="footer"></div>
</body>
</html>
