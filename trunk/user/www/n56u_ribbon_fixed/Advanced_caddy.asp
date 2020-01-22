<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - 文件管理</title>
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
<% caddy_status(); %>
<% disk_pool_mapping_info(); %>


$j(document).ready(function() {
    init_itoggle('caddy_enable');
	init_itoggle('caddy_wan');
	init_itoggle('caddy_wip6');
	init_itoggle('caddy_file');

});

</script>
<script>

<% login_state_hook(); %>

function initial(){
	show_banner(2);
	show_menu(5,19);
	show_footer();
show_caddy_stroage();
show_caddy_dir();
switch_caddy_type();
fill_status(caddy_status());
	var o1 = document.form.caddy_storage;
	var o2 = document.form.caddy_dir;
	o1.value = '<% nvram_get_x("","caddy_storage"); %>';
	o2.value = '<% nvram_get_x("","caddy_dir"); %>';
	if (!login_safe())
		textarea_scripts_enabled(0);
}

function textarea_scripts_enabled(v){
	inputCtrl(document.form['scripts.caddy_script.sh'], v);
}

function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("caddy_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}

function applyRule(){
//	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_caddy.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}


function done_validating(action){
	refreshpage();
}

function show_caddy_stroage(){

	var code = '<option value="-1" >请选择存储设备</option>';
	code +='<option value="/media/" >/media/</option>';
	if(pool_names().length == 0)
		code +='<option value="non" >未发现存储设备</option>';
	else{
	
		for(var i = 0; i < pool_names().length; ++i){
			code +='<option value="/media/'+ pool_names()[i] +'" >/media/'+ pool_names()[i] + '/</option>';
		}
	}
	$("caddy_storage").innerHTML = code;
}

function show_caddy_dir(){

	var code = '<option value="/tmp" >/tmp/caddy</option>';
	if(pool_names().length == 0)
		code +='<option value="non" >未插入存储设备</option>';
	else{
	
		for(var i = 0; i < pool_names().length; ++i){
			code +='<option value="/media/'+ pool_names()[i] +'" >/media/'+ pool_names()[i] + '/caddy</option>';
		}
	}
	$("caddy_dir").innerHTML = code;
}

function switch_caddy_type(){
var b = document.form.caddy_file.value;
if (b=="0"){
	var v=0;
	showhide_div('row_wname', 0);
	showhide_div('row_wpassword', 0);
}
if (b=="1"){
	var v=1;
	showhide_div('row_wname', 1);
	showhide_div('row_wpassword', 1);
}
if (b=="2"){
	var v=1;
	showhide_div('row_wname', 1);
	showhide_div('row_wpassword', 1);
}
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

	<input type="hidden" name="current_page" value="Advanced_caddy.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="CaddyConf;">
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
							<h2 class="box_head round_top">在线文件管理</h2>
							<div class="round_bottom">
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">File Browser是一个基于GO的轻量级文件管理系统支持登录系统 角色系统、在线PDF、图片、视频浏览、上传下载、打包下载等功能。
									WEBDAV 是一种文件服务，类似的服务有 SMB、NFS、FTP 等，特点是基于 HTTP/HTTPS 协议。
									
									</div>

									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
									<tr width="50%"> <th><#running_status#></th>
                                            <td id="caddy_status" colspan="2"></td>
                                        </tr>
										<tr>
											<th width="50%">总开关</th>
											<td>
													<div class="main_itoggle">
													<div id="caddy_enable_on_of">
														<input type="checkbox" id="caddy_enable_fake" <% nvram_match_x("", "caddy_enable", "1", "value=1 checked"); %><% nvram_match_x("", "caddy_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="caddy_enable" id="caddy_enable_1" class="input" value="1" <% nvram_match_x("", "caddy_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="caddy_enable" id="caddy_enable_0" class="input" value="0" <% nvram_match_x("", "caddy_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										</tr>
										<tr>
											<th width="30%">WAN IPV4访问开关</th>
											<td>
													<div class="main_itoggle">
													<div id="caddy_wan_on_of">
														<input type="checkbox" id="caddy_wan_fake" <% nvram_match_x("", "caddy_wan", "1", "value=1 checked"); %><% nvram_match_x("", "caddy_wan", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="caddy_wan" id="caddy_wan_1" class="input" value="1" <% nvram_match_x("", "caddy_wan", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="caddy_wan" id="caddy_wan_0" class="input" value="0" <% nvram_match_x("", "caddy_wan", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
											<th width="30%">WAN IPV6访问开关</th>
											<td>
													<div class="main_itoggle">
													<div id="caddy_wip6_on_of">
														<input type="checkbox" id="caddy_wip6_fake" <% nvram_match_x("", "caddy_wip6", "1", "value=1 checked"); %><% nvram_match_x("", "caddy_wip6", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="caddy_wip6" id="caddy_wip6_1" class="input" value="1" <% nvram_match_x("", "caddy_wip6", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="caddy_wip6" id="caddy_wip6_0" class="input" value="0" <% nvram_match_x("", "caddy_wip6", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
											<tr>
                                            <th width="30%">启动模式:</th>
                                            <td colspan="2">
                                                <select id="caddy_file" name="caddy_file" class="input" onchange="switch_caddy_type()">
                                                    <option value="0" <% nvram_match_x("","caddy_file", "0","selected"); %>>File Browser</option>
                                                    <option value="1" <% nvram_match_x("","caddy_file", "1","selected"); %>>WebDAV</option>
													<option value="2" <% nvram_match_x("","caddy_file", "2","selected"); %>>File Browser+WebDAV</option>
                                                </select>
                                            </td>
                                        </tr>
										<tr>
                                            <th width="30%">Caddy存放目录:</th>
                                            <td colspan="2">
                                                <select name="caddy_dir" id="caddy_dir" class="input">
                                                </select>
                                            </td>
                                        </tr>
										<tr>
                                            <th width="30%">主目录:</th>
                                            <td colspan="2">
                                                <select name="caddy_storage" id="caddy_storage" class="input">

                                                </select>
                                            </td>
                                        </tr>
										<tr>
                                            <th>File Browser端口:</th>
                                            <td>
                                                <input type="text" name="caddyf_wan_port" maxlength="8"  class="input" size="60" value="<% nvram_get_x("","caddyf_wan_port"); %>" /><a href="http://<% nvram_get_x("", "lan_ipaddr_t"); %>:<% nvram_get_x("","caddyf_wan_port"); %>" target="_blank">打开管理页面</a>
                                            </td>
                                        </tr>
										<tr>
                                            <th>webdav端口:</th>
                                            <td>
                                                <input type="text" name="caddyw_wan_port" maxlength="8"  class="input" size="60" value="<% nvram_get_x("","caddyw_wan_port"); %>" /><a href="http://<% nvram_get_x("", "lan_ipaddr_t"); %>:<% nvram_get_x("","caddyw_wan_port"); %>" target="_blank">打开管理页面</a>
                                            </td>
                                        </tr>
										<tr id="row_wname" style="display:none;">
                                            <th>webdav用户名:</th>
                                            <td>
                                                <input type="text" name="caddy_wname" maxlength="8"  class="input" size="60" value="<% nvram_get_x("","caddy_wname"); %>" />
                                            </td>
                                        </tr>
										<tr id="row_wpassword" style="display:none;">  <th width="50%">webdav密码:</th>
				<td>
					<input type="password" class="input" size="32" name="caddy_wpassword" id="w_key" value="<% nvram_get_x("","caddy_wpassword"); %>" />
					<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('w_key')"><i class="icon-eye-close"></i></button>
				</td>
			</tr>
										<tr id="row_post_wan_script">
											<td colspan="2">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script2')"><span>caddy脚本-不懂请不要乱改！！！</span></a>
												<div id="script2">
													<textarea rows="18" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.caddy_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.caddy_script.sh",""); %></textarea>
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
