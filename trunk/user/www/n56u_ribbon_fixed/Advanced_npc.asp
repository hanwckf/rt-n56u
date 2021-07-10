<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_17_2#></title>
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
<script type="text/javascript" src="/help.js"></script>
<script>
var $j = jQuery.noConflict();
<% npc_status(); %>
$j(document).ready(function() {

	init_itoggle('npc_enable',change_npc_enable_bridge);

});

</script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(2);
	show_menu(5,15,2);
	show_footer();
	showmenu();

	change_npc_enable_bridge(1);
	fill_status(npc_status());

	if (!login_safe())
		textarea_scripts_enabled(0);
}

function showmenu(){
showhide_div('frplink', found_app_frp());
}

function textarea_scripts_enabled(v){
	inputCtrl(document.form['scripts.npc_script.sh'], v);
}
function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("npc_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}

function applyRule(){
//	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_npc.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}


function done_validating(action){
	refreshpage();
}

function change_npc_enable_bridge(mflag){
	var m = document.form.npc_enable[0].checked;
	showhide_div("npc_protocol_tr", m);
	showhide_div("npc_vkey_tr", m);
	showhide_div("npc_server_addr_tr", m);
	showhide_div("npc_server_port_tr", m);
	showhide_div("npc_compress_tr", m);
	showhide_div("npc_crypt_tr", m);
	showhide_div("npc_log_level_tr", m);
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

	<input type="hidden" name="current_page" value="Advanced_npc.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="NpcConf;">
	<input type="hidden" name="group_id" value="">
	<input type="hidden" name="action_mode" value="">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("", "wan0_ipaddr"); %>" readonly="1">
	<input type="hidden" name="wan_netmask" value="<% nvram_get_x("", "wan0_netmask"); %>" readonly="1">
	<input type="hidden" name="dhcp_start" value="<% nvram_get_x("", "dhcp_start"); %>">
	<input type="hidden" name="dhcp_end" value="<% nvram_get_x("", "dhcp_end"); %>">

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
							<h2 class="box_head round_top"><#menu5_17#> - <#menu5_17_2#></h2>
							<div class="round_bottom">
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">nps 是一款轻量级、高性能、功能强大的内网穿透代理服务器。
									</div>

									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr> 
											<th>npc<#running_status#></th>
											<td id="npc_status" colspan="2"></td>
										</tr>
										<tr>
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">启用 npc 内网穿透</a></th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="npc_enable_on_of">
														<input type="checkbox" id="npc_enable_fake" <% nvram_match_x("", "npc_enable", "1", "value=1 checked"); %><% nvram_match_x("", "npc_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="npc_enable" id="npc_enable_1" class="input" value="1" onClick="change_npc_enable_bridge(1);" <% nvram_match_x("", "npc_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="npc_enable" id="npc_enable_0" class="input" value="0" onClick="change_npc_enable_bridge(1);" <% nvram_match_x("", "npc_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="npc_server_addr_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">服务器地址:</a></th>
											<td style="border-top: 0 none;">
												<input type="text" maxlength="50" class="input" size="50" id="npc_server_addr" name="npc_server_addr" placeholder="127.0.0.1" value="<% nvram_get_x("","npc_server_addr"); %>"  onkeypress="return is_string(this,event);" />
											</td>
										</tr>
										<tr id="npc_server_port_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">端口:</a></th>
											<td style="border-top: 0 none;">
												<input type="text" maxlength="5" class="input" size="15" id="npc_server_port" name="npc_server_port" placeholder="8024" value="<% nvram_get_x("","npc_server_port"); %>"  onkeypress="return is_number(this,event);" />
											</td>
										</tr>
										<tr id="npc_protocol_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">协议类型:</a></th>
											<td style="border-top: 0 none;">
												<select name="npc_protocol" id="npc_protocol" class="input" style="width: 200px">
													<option value="tcp" <% nvram_match_x("","npc_protocol", "tcp","selected"); %>>TCP</option>
													<option value="kcp" <% nvram_match_x("","npc_protocol", "kcp","selected"); %>>KCP</option>
												</select>
											</td>
										</tr>
										<tr id="npc_vkey_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">密钥(vkey):</a></th>
											<td style="border-top: 0 none;">
											<div class="input-append">
												<input type="password" maxlength="512" class="input" size="16" name="npc_vkey" id="npc_vkey" style="width: 175px;" value="<% nvram_get_x("","npc_vkey"); %>" onKeyPress="return is_string(this,event);"/>
												<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('npc_vkey')"><i class="icon-eye-close"></i></button>
											</div>
											</td>
										</tr>
										<tr id="npc_compress_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">使用压缩传输:</a></th>
											<td style="border-top: 0 none;">
												<div class="main_itoggle">
													<div id="npc_compress_on_of">
														<input type="checkbox" id="npc_compress_fake" <% nvram_match_x("", "npc_compress", "1", "value=1 checked"); %><% nvram_match_x("", "npc_compress", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="npc_compress" id="npc_compress_1" class="input" value="1" <% nvram_match_x("", "npc_compress", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="npc_compress" id="npc_compress_0" class="input" value="0" <% nvram_match_x("", "npc_compress", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="npc_crypt_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">使用加密传输:</a></th>
											<td style="border-top: 0 none;">
												<div class="main_itoggle">
													<div id="npc_crypt_on_of">
														<input type="checkbox" id="npc_crypt_fake" <% nvram_match_x("", "npc_crypt", "1", "value=1 checked"); %><% nvram_match_x("", "npc_crypt", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="npc_crypt" id="npc_crypt_1" class="input" value="1" <% nvram_match_x("", "npc_crypt", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="npc_crypt" id="npc_crypt_0" class="input" value="0" <% nvram_match_x("", "npc_crypt", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="npc_log_level_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">日志级别:</a></th>
											<td style="border-top: 0 none;">
												<select name="npc_log_level" id="npc_log_level" class="input" style="width: 200px">
													<option value="0" <% nvram_match_x("","npc_log_level", "0","selected"); %>>Emergency</option>
													<option value="2" <% nvram_match_x("","npc_log_level", "2","selected"); %>>Critical</option>
													<option value="3" <% nvram_match_x("","npc_log_level", "3","selected"); %>>Error</option>
													<option value="4" <% nvram_match_x("","npc_log_level", "4","selected"); %>>Warning</option>
													<option value="7" <% nvram_match_x("","npc_log_level", "7","selected"); %>>Debug</option>
												</select>
											</td>
										</tr>
										<tr id="row_post_wan_script">
											<td colspan="2">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script2')"><span>npc启动脚本-不懂请不要乱改！！！</span></a>
												<div id="script2">
													<textarea rows="18" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.npc_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.npc_script.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										
										<tr>
											<td colspan="2" style="border-top: 0 none;">
												<br />
												<center><input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" /></center>
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

