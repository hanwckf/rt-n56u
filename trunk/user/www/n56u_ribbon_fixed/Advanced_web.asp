<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - 菜单设置</title>
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
<script type="text/javascript" src="/client_function.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {
    init_itoggle('w_ai');
	init_itoggle('w_vpn_s');
	init_itoggle('w_vpn_c');
	init_itoggle('w_wnet');
	init_itoggle('w_sys');
	init_itoggle('w_usb');
	init_itoggle('w_net');
	init_itoggle('w_log');
	init_itoggle('w_scu');
	init_itoggle('w_dnsf');
	init_itoggle('w_ss');
	init_itoggle('w_men');
	init_itoggle('w_adbyby');
	init_itoggle('w_pdnsd');

});
</script>
<script>
<% login_state_hook(); %>
function initial(){
	show_banner(2);
	show_menu(5,8,4);
	show_footer();
	if (!login_safe())
		textarea_scripts_enabled(0);
		//load_body();
	if (found_app_shadowsocks()){
	showhide_div('row_wss', true);
}
if (found_app_scutclient()){
	showhide_div('row_wscu', true);
}
if (found_app_dnsforwarder()){
	showhide_div('row_wdnsf', true);
}
if (found_app_mentohust()){
	showhide_div('row_wmen', true);
}
if (found_app_adbyby() || found_app_koolproxy()){
	showhide_div('row_wadbyby', true);
}
if (found_app_smartdns() || found_app_adguardhome()){
	showhide_div('row_wpdnsd', true);
}
}


function applyRule(){
	//if(validForm()){
		showLoading();
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "Advanced_web.asp";
		document.form.next_page.value = "";
		document.form.submit();
	//}
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

	<input type="hidden" name="current_page" value="Advanced_web.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="DwebConf;">
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
							<h2 class="box_head round_top">自定义菜单选项</h2>
							<div class="round_bottom">
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">把你不想在网页上显示的菜单选项关闭，适用于重度强迫症......<br />
									<div>此选项只能屏蔽页面的显示，并不会删除程序。</div>
									</div>
									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr >
											<th width="50%">VPN服务器</th>
											<td>
													<div class="main_itoggle">
													<div id="w_vpn_s_on_of">
														<input type="checkbox" id="w_vpn_s_fake" <% nvram_match_x("", "w_vpn_s", "1", "value=1 checked"); %><% nvram_match_x("", "w_vpn_s", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_vpn_s" id="w_vpn_s_1" class="input" <% nvram_match_x("", "w_vpn_s", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_vpn_s" id="w_vpn_s_0" class="input" <% nvram_match_x("", "w_vpn_s", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
																				<tr >
											<th width="50%">VPN客户端</th>
											<td>
													<div class="main_itoggle">
													<div id="w_vpn_c_on_of">
														<input type="checkbox" id="w_vpn_c_fake" <% nvram_match_x("", "w_vpn_c", "1", "value=1 checked"); %><% nvram_match_x("", "w_vpn_c", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_vpn_c" id="w_vpn_c_1" class="input" <% nvram_match_x("", "w_vpn_c", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_vpn_c" id="w_vpn_c_0" class="input" <% nvram_match_x("", "w_vpn_c", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr >
											<th width="50%">网络流量</th>
											<td>
													<div class="main_itoggle">
													<div id="w_wnet_on_of">
														<input type="checkbox" id="w_wnet_fake" <% nvram_match_x("", "w_wnet", "1", "value=1 checked"); %><% nvram_match_x("", "w_wnet", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_wnet" id="w_wnet_1" class="input" <% nvram_match_x("", "w_wnet", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_wnet" id="w_wnet_0" class="input" <% nvram_match_x("", "w_wnet", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr >
											<th width="50%">系统信息</th>
											<td>
													<div class="main_itoggle">
													<div id="w_sys_on_of">
														<input type="checkbox" id="w_sys_fake" <% nvram_match_x("", "w_sys", "1", "value=1 checked"); %><% nvram_match_x("", "w_sys", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_sys" id="w_sys_1" class="input" <% nvram_match_x("", "w_sys", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_sys" id="w_sys_0" class="input" <% nvram_match_x("", "w_sys", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr >
											<th width="50%">USB应用</th>
											<td>
													<div class="main_itoggle">
													<div id="w_usb_on_of">
														<input type="checkbox" id="w_usb_fake" <% nvram_match_x("", "w_usb", "1", "value=1 checked"); %><% nvram_match_x("", "w_usb", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_usb" id="w_usb_1" class="input" <% nvram_match_x("", "w_usb", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_usb" id="w_usb_0" class="input" <% nvram_match_x("", "w_usb", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr >
											<th width="50%">网络信息</th>
											<td>
													<div class="main_itoggle">
													<div id="w_net_on_of">
														<input type="checkbox" id="w_net_fake" <% nvram_match_x("", "w_net", "1", "value=1 checked"); %><% nvram_match_x("", "w_net", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_net" id="w_net_1" class="input" <% nvram_match_x("", "w_net", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_net" id="w_net_0" class="input" <% nvram_match_x("", "w_net", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr >
											<th width="50%">系统日志</th>
											<td>
													<div class="main_itoggle">
													<div id="w_log_on_of">
														<input type="checkbox" id="w_log_fake" <% nvram_match_x("", "w_log", "1", "value=1 checked"); %><% nvram_match_x("", "w_log", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_log" id="w_log_1" class="input" <% nvram_match_x("", "w_log", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_log" id="w_log_0" class="input" <% nvram_match_x("", "w_log", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="row_wscu" style="display:none">
											<th width="50%">Scutclient</th>
											<td>
													<div class="main_itoggle">
													<div id="w_scu_on_of">
														<input type="checkbox" id="w_scu_fake" <% nvram_match_x("", "w_scu", "1", "value=1 checked"); %><% nvram_match_x("", "w_scu", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_scu" id="w_scu_1" class="input" <% nvram_match_x("", "w_scu", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_scu" id="w_scu_0" class="input" <% nvram_match_x("", "w_scu", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										
										<tr id="row_wdnsf" style="display:none">
											<th width="50%" >DNS-forwarder</th>
											<td>
													<div class="main_itoggle">
													<div id="w_dnsf_on_of">
														<input type="checkbox" id="w_dnsf_fake" <% nvram_match_x("", "w_dnsf", "1", "value=1 checked"); %><% nvram_match_x("", "w_dnsf", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_dnsf" id="w_dnsf_1" class="input" <% nvram_match_x("", "w_dnsf", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_dnsf" id="w_dnsf_0" class="input" <% nvram_match_x("", "w_dnsf", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="row_wss" style="display:none">
											<th width="50%">科学上网</th>
											<td>
													<div class="main_itoggle">
													<div id="w_ss_on_of">
														<input type="checkbox" id="w_ss_fake" <% nvram_match_x("", "w_ss", "1", "value=1 checked"); %><% nvram_match_x("", "w_ss", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_ss" id="w_ss_1" class="input" <% nvram_match_x("", "w_ss", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_ss" id="w_ss_0" class="input" <% nvram_match_x("", "w_ss", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="row_wmen" style="display:none">
											<th width="50%" >Mentohust</th>
											<td>
													<div class="main_itoggle">
													<div id="w_men_on_of">
														<input type="checkbox" id="w_men_fake" <% nvram_match_x("", "w_men", "1", "value=1 checked"); %><% nvram_match_x("", "w_men", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_men" id="w_men_1" class="input" <% nvram_match_x("", "w_men", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_men" id="w_men_0" class="input" <% nvram_match_x("", "w_men", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="row_wadbyby" style="display:none">
											<th width="50%">广告管理</th>
											<td>
													<div class="main_itoggle">
													<div id="w_adbyby_on_of">
														<input type="checkbox" id="w_adbyby_fake" <% nvram_match_x("", "w_adbyby", "1", "value=1 checked"); %><% nvram_match_x("", "w_adbyby", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_adbyby" id="w_adbyby_1" class="input" <% nvram_match_x("", "w_adbyby", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_adbyby" id="w_adbyby_0" class="input" <% nvram_match_x("", "w_adbyby", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="row_wpdnsd" style="display:none">
											<th width="50%" >DNS加速</th>
											<td>
													<div class="main_itoggle">
													<div id="w_pdnsd_on_of">
														<input type="checkbox" id="w_pdnsd_fake" <% nvram_match_x("", "w_pdnsd", "1", "value=1 checked"); %><% nvram_match_x("", "w_pdnsd", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="w_pdnsd" id="w_pdnsd_1" class="input" <% nvram_match_x("", "w_pdnsd", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="w_pdnsd" id="w_pdnsd_0" class="input" <% nvram_match_x("", "w_pdnsd", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										
									
											<td colspan="2">
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
<form method="post" name="adbyby_action" action="">
    <input type="hidden" name="connect_action" value="">
</form>
</body>
</html>
