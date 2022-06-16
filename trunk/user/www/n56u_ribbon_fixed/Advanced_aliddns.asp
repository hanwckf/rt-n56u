<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - Aliddns 域名解析</title>
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

$j(document).ready(function() {

	init_itoggle('aliddns_enable',change_aliddns_enable_bridge);

});

</script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(2);
	show_menu(5,17,0);
	show_footer();
	showmenu();

	change_aliddns_enable_bridge(1);

	if (!login_safe())
		textarea_scripts_enabled(0);
}

function showmenu(){
showhide_div('zelink', found_app_zerotier());
}

function textarea_scripts_enabled(v){
	inputCtrl(document.form['scripts.ddns_script.sh'], v);
}

function applyRule(){
//	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_aliddns.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}


function done_validating(action){
	refreshpage();
}

function change_aliddns_enable_bridge(mflag){
	var m = document.form.aliddns_enable[0].checked;
	showhide_div("aliddns_ak_tr", m);
	showhide_div("aliddns_sk_tr", m);
	showhide_div("aliddns_interval_tr", m);
	showhide_div("aliddns_ttl_tr", m);
	showhide_div("aliddns_domain_tr", m);
	showhide_div("aliddns_domain2_tr", m);
	showhide_div("aliddns_domain6_tr", m);
	showhide_div("row_post_wan_script", m);
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

	<input type="hidden" name="current_page" value="Advanced_aliddns.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="LANHostConfig;General;">
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
							<h2 class="box_head round_top"><#menu5_23#> - <#menu5_30#></h2>
							<div class="round_bottom">
							<div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
								<li class="active">
                                    <a href="Advanced_aliddns.asp"><#menu5_23_1#></a>
                                </li>
								<li id="zelink" style="display:none">
                                    <a href="Advanced_zerotier.asp"><#menu5_32_1#></a>
                                </li>
                            </ul>
                        </div>
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">使用 Aliddns 实现顶级个人域名的 ddns 服务。 <a href="https://www.aliyun.com" target="blank"><i><u>Aliddns 主页</u></i></a>
												<ul style="padding-top:5px;margin-top:10px;float: left;">
												<li><a href="https://github.com/kyriosli/koolshare-aliddns" target="blank"><i><u>Aliddns 项目地址：https://github.com/kyriosli/koolshare-aliddns</u></i></a></li>
												<li><a href="http://koolshare.cn/thread-64703-1-1.html" target="blank"><i><u>Aliddns 使用帮助：http://koolshare.cn/thread-64703-1-1.html</u></i></a></li>
												<li>使用前需要将域名添加到 aliyun 中，并添加一条A记录，使用之后将自动更新ip</li>
												<li>点 <a href="https://help.aliyun.com/knowledge_detail/39844.html" target="blank"><i><u>这里</u></i></a> 查看不同域名注册商修改 DNS 方法。</li>
												<li>点 <a href="https://help.aliyun.com/knowledge_detail/38738.html" target="blank"><i><u>这里</u></i></a> 查看如何获取 Aliddns 的 Access Key ID 和 Access Key Secret。</li>
												</ul>
									</div>

									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr >
											<th width="30%" style="border-top: 0 none;">上次运行:</th>
											<td  colspan="3"style="border-top: 0 none;">
											   <div >【<% nvram_get_x("","aliddns_last_act"); %>】</div>
											</td>
										</tr>
										<tr>
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">启用 Aliddns 域名解析</a></th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="aliddns_enable_on_of">
														<input type="checkbox" id="aliddns_enable_fake" <% nvram_match_x("", "aliddns_enable", "1", "value=1 checked"); %><% nvram_match_x("", "aliddns_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="aliddns_enable" id="aliddns_enable_1" class="input" value="1" onClick="change_aliddns_enable_bridge(1);" <% nvram_match_x("", "aliddns_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="aliddns_enable" id="aliddns_enable_0" class="input" value="0" onClick="change_aliddns_enable_bridge(1);" <% nvram_match_x("", "aliddns_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr id="aliddns_interval_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">检查周期(秒) :</a></th>
											<td style="border-top: 0 none;">
												<input type="text" maxlength="5" class="input" size="15" id="aliddns_interval" name="aliddns_interval" placeholder="600" value="<% nvram_get_x("","aliddns_interval"); %>"  onkeypress="return is_number(this,event);" />
											</td>
										</tr>
										<tr id="aliddns_ttl_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">解析TTL(秒) :</a></th>
											<td style="border-top: 0 none;">
												<input type="text" maxlength="5" class="input" size="15" id="aliddns_ttl" name="aliddns_ttl" placeholder="600" value="<% nvram_get_x("","aliddns_ttl"); %>"  onkeypress="return is_number(this,event);" />
												<div>&nbsp;<span style="color:#888;">[1-86400]默认10分钟，免费版的范围是600-86400</span></div>
											</td>
										</tr>
										<tr id="aliddns_ak_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">Access Key ID :</a></th>
											<td style="border-top: 0 none;">
											<div class="input-append">
												<input type="password" maxlength="512" class="input" size="15" name="aliddns_ak" id="aliddns_ak" style="width: 175px;" value="<% nvram_get_x("","aliddns_ak"); %>" onKeyPress="return is_string(this,event);"/>
												<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('aliddns_ak')"><i class="icon-eye-close"></i></button>
											</div>
											</td>
										</tr>
										<tr id="aliddns_sk_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">Access Key Secret :</a></th>
											<td style="border-top: 0 none;">
											<div class="input-append">
												<input type="password" maxlength="512" class="input" size="15" name="aliddns_sk" id="aliddns_sk" style="width: 175px;" value="<% nvram_get_x("","aliddns_sk"); %>" onKeyPress="return is_string(this,event);"/>
												<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('aliddns_sk')"><i class="icon-eye-close"></i></button>
											</div>
											</td>
										</tr>
										<tr id="aliddns_domain_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">顶级域名1</a></th>
											<td style="border-top: 0 none;">
												<input style="width: 80px;" type="text" maxlength="255" class="input" size="15" id="aliddns_name" name="aliddns_name" placeholder="www" value="<% nvram_get_x("","aliddns_name"); %>" onKeyPress="return is_string(this,event);" /> . <input style="width: 110px;" type="text" maxlength="255" class="input" size="15" id="aliddns_domain" name="aliddns_domain" placeholder="google.com" value="<% nvram_get_x("","aliddns_domain"); %>" onKeyPress="return is_string(this,event);" />
											</td>
										</tr>
										<tr id="aliddns_domain2_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">顶级域名2</a></th>
											<td style="border-top: 0 none;">
												<input style="width: 80px;" type="text" maxlength="255" class="input" size="15" id="aliddns_name2" name="aliddns_name2" placeholder="www" value="<% nvram_get_x("","aliddns_name2"); %>" onKeyPress="return is_string(this,event);" /> . <input style="width: 110px;" type="text" maxlength="255" class="input" size="15" id="aliddns_domain2" name="aliddns_domain2" placeholder="google.com" value="<% nvram_get_x("","aliddns_domain2"); %>" onKeyPress="return is_string(this,event);" />
											</td>
										</tr>
										<tr id="aliddns_domain6_tr" style="display:none;">
											<th width="30%" style="border-top: 0 none;"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">顶级域名3[IPv6]</a></th>
											<td style="border-top: 0 none;">
												<input style="width: 80px;" type="text" maxlength="255" class="input" size="15" id="aliddns_name6" name="aliddns_name6" placeholder="www" value="<% nvram_get_x("","aliddns_name6"); %>" onKeyPress="return is_string(this,event);" /> . <input style="width: 110px;" type="text" maxlength="255" class="input" size="15" id="aliddns_domain6" name="aliddns_domain6" placeholder="google.com" value="<% nvram_get_x("","aliddns_domain6"); %>" onKeyPress="return is_string(this,event);" />
											</td>
										</tr>
										<tr id="row_post_wan_script" style="display:none;">
											<td colspan="2" style="border-top: 0 none;">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script2')"><span>Aliddns 脚本-基于 Aliddns 用户 API 实现的纯 Shell 动态域名客户端</span></a>
												<div id="script2" style="display:none;">
													<textarea rows="18" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ddns_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ddns_script.sh",""); %></textarea>
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

