<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - 网易云解锁</title>
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
<script type="text/javascript" src="/help_b.js"></script>
<script>
var $j = jQuery.noConflict();

$j(document).ready(function() {

	init_itoggle('wyy_enable');
	init_itoggle('wyy_flac');

});

</script>
<script>

<% login_state_hook(); %>
var ipmonitor = [<% get_static_client(); %>];
var wireless = [];
var m_dhcp = [<% get_nvram_list("WyyConf", "WIPList"); %>];
var mdhcp_ifield = 4;
if(m_dhcp.length > 0){
	var m_dhcp_ifield = m_dhcp[0].length;
	for (var i = 0; i < m_dhcp.length; i++) {
		m_dhcp[i][mdhcp_ifield] = i;
	}
}
var clients_info = getclients(1,0);
var isMenuopen = 0;
function initial(){
	show_banner(2);
	show_menu(5,20,0);

	show_footer();
	showMDHCPList();
	showLANIPList();
	var o1 = document.form.wyy_apptype;
	var o2 = document.form.wyy_cloudserver;
	//var o3 = document.form.wyy_musicapptype;
	var o4 = document.form.wyy_coustom_server;
	//var o5 = document.form.wyy_coustom_music;

	o1.value = '<% nvram_get_x("","wyy_apptype"); %>';
	o2.value = '<% nvram_get_x("","wyy_cloudserver"); %>';
	//o3.value = '<% nvram_get_x("","wyy_musicapptype"); %>';
	o4.value = '<% nvram_get_x("","wyy_coustom_server"); %>';
	//o5.value = '<% nvram_get_x("","wyy_coustom_music"); %>';
	switch_wyy_type();

}

function applyRule(){
//	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_wyy.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}

function switch_wyy_type(){
var b = document.form.wyy_apptype.value;
var c = document.form.wyy_cloudserver.value;
if (b=="go"){
	showhide_div('row_wyy_cloudserver', 0);
	showhide_div('row_wyy_musicapptype', 1);
	showhide_div('row_wyy_coustom_music', 0);
	showhide_div('row_wyy_coustom_server', 0);
//	var c = document.form.wyy_musicapptype.value;
//if (c=="coustom"){
//	showhide_div('row_wyy_coustom_music', 1);
//	showhide_div('row_wyy_coustom_server', 0);
//}
}
if (b=="cloud"){
	showhide_div('row_wyy_cloudserver', 1);
	showhide_div('row_wyy_musicapptype', 0);
	//showhide_div('row_wyy_coustom_music', 0);
	showhide_div('row_wyy_coustom_server', 0);
if (c=="coustom"){
	showhide_div('row_wyy_coustom_music', 0);
	showhide_div('row_wyy_coustom_server', 1);
}
}
}
function switch_wyy_cous(){
var b = document.form.wyy_cloudserver.value;
if (b=="coustom"){
	showhide_div('row_wyy_coustom_server', 1);
	showhide_div('row_wyy_coustom_music', 0);
}else{
	showhide_div('row_wyy_coustom_server', 0);
	showhide_div('row_wyy_coustom_music', 0);
}
}
/*
function switch_wyy_mus(){
var b = document.form.wyy_musicapptype.value;
if (b=="coustom"){
	showhide_div('row_wyy_coustom_music', 1);
	showhide_div('row_wyy_coustom_server', 0);
}else{
	showhide_div('row_wyy_coustom_server', 0);
	showhide_div('row_wyy_coustom_music', 0);
}
}
*/
function done_validating(action){
	refreshpage();
}

function showMDHCPList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_dhcp.length == 0)
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < m_dhcp.length; i++){
		if(m_dhcp[i][3] == "http")
		wyyroad="不代理http";
		else if(m_dhcp[i][3] == "https"){
		wyyroad="不代理https";
		}else if(m_dhcp[i][3] == "disable"){
		wyyroad="忽略代理";
		}
		code +='<tr id="row' + i + '">';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][0] + '</td>';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][1] + '</td>';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][2] + '</td>';
		code +='<td width="20%">&nbsp;' + wyyroad + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="WIPList_s" value="' + m_dhcp[i][mdhcp_ifield] + '" onClick="changeBgColor(this,' + i + ');" id="check' + m_dhcp[i][mdhcp_ifield] + '"></td>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="4">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupMDHCP(this, 64, \' Del \');" name="WIPList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MDHCPList_Block").innerHTML = code;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){
		$j(obj).children('i').removeClass('icon-chevron-down').addClass('icon-chevron-up');
		$("ClientList_Block").style.display = 'block';
		document.form.wyy_mac_x_0.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function sortbyIP(){
	m_dhcp.sort(function(a,b){
		var aa = a[1].split(".");
		var bb = b[1].split(".");
		var resulta = aa[0]*0x1000000 + aa[1]*0x10000 + aa[2]*0x100 + aa[3]*1;
		var resultb = bb[0]*0x1000000 + bb[1]*0x10000 + bb[2]*0x100 + bb[3]*1;
		return resulta-resultb;
	});
	showMDHCPList();
}

function sortbyMAC(){
	m_dhcp.sort(function(a,b){
		return parseInt(a[0])-parseInt(b[0]);
	});
	showMDHCPList();
}

function sortbyName(){
	m_dhcp.sort(function(a,b){
		var aa = a[2].toLowerCase();
		var bb = b[2].toLowerCase();
		if (aa < bb) return -1;
		if (aa > bb) return 1;
		return 0;
	});
	showMDHCPList();
}

function sortbyId(){
	m_dhcp.sort(function(a,b){
		return a[mdhcp_ifield] - b[mdhcp_ifield];
	});
	showMDHCPList();
}

function setClientMAC(num){
	document.form.wyy_mac_x_0.value = clients_info[num][2];
	document.form.wyy_ip_x_0.value = clients_info[num][1];
	document.form.wyy_name_x_0.value = clients_info[num][0];
	hideClients_Block();
}

function showLANIPList(){
	var code = "";
	var show_name = "";
	
	for(var i = 0; i < clients_info.length ; i++){
		if(clients_info[i][0] && clients_info[i][0].length > 20)
			show_name = clients_info[i][0].substring(0, 18) + "..";
		else
			show_name = clients_info[i][0];
		
		if(clients_info[i][2]){
			code += '<a href="javascript:void(0)"><div onclick="setClientMAC('+i+');"><strong>'+clients_info[i][1]+'</strong>';
			code += ' ['+clients_info[i][2]+']';
			if(show_name && show_name.length > 0)
				code += ' ('+show_name+')';
			code += ' </div></a>';
		}
	}
	if (code == "")
		code = '<div style="text-align: center;" onclick="hideClients_Block();"><#Nodata#></div>';
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	$("ClientList_Block").innerHTML = code;
}

function hideClients_Block(){
	$j("#chevron").children('i').removeClass('icon-chevron-up').addClass('icon-chevron-down');
	$('ClientList_Block').style.display='none';
	isMenuopen = 0;
}

function markGroupMDHCP(o, c, b) {
	document.form.group_id.value = "WIPList";
	if(b == " Add "){
		if (document.form.wyy_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.wyy_mac_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.wyy_mac_x_0.focus();
			document.form.wyy_mac_x_0.select();
			return false;
		}else if(document.form.wyy_ip_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.wyy_ip_x_0.focus();
			document.form.wyy_ip_x_0.select();
			return false;
		}else if (!validate_hwaddr(document.form.wyy_mac_x_0)){
			return false;
		}else if (!validate_ipaddr_final(document.form.wyy_ip_x_0, 'staticip')){
			return false;
		}else{
			for(i=0; i<m_dhcp.length; i++){
				if(document.form.wyy_mac_x_0.value==m_dhcp[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][0] + ')' );
					document.form.wyy_mac_x_0.focus();
					document.form.wyy_mac_x_0.select();
					return false;
				}
				if(document.form.wyy_ip_x_0.value.value==m_dhcp[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][1] + ')' );
					document.form.wyy_ip_x_0.focus();
					document.form.wyy_ip_x_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
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

	<input type="hidden" name="current_page" value="Advanced_wyy.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="WyyConf;">
	<input type="hidden" name="group_id" value="WIPList">
	<input type="hidden" name="action_mode" value="">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("", "wan0_ipaddr"); %>" readonly="1">
	<input type="hidden" name="wan_netmask" value="<% nvram_get_x("", "wan0_netmask"); %>" readonly="1">
	<input type="hidden" name="dhcp_start" value="<% nvram_get_x("", "dhcp_start"); %>">
	<input type="hidden" name="dhcp_end" value="<% nvram_get_x("", "dhcp_end"); %>">
	<input type="hidden" name="wyy_staticnum_x_0" value="<% nvram_get_x("WIPList", "wyy_staticnum_x"); %>" readonly="1" />

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
							<h2 class="box_head round_top">音乐解锁 - 解锁网易云灰色歌曲</h2>
							<div class="round_bottom">
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">
									<p>采用 [QQ/虾米/百度/酷狗/酷我/咕咪/JOOX]等音源，替换网易云变灰歌曲链接
									</p>
									</div>

									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr>
											<th width="30%" style="border-top: 0 none;">启用解锁</th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="wyy_enable_on_of">
														<input type="checkbox" id="wyy_enable_fake" <% nvram_match_x("", "wyy_enable", "1", "value=1 checked"); %><% nvram_match_x("", "wyy_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="wyy_enable" id="wyy_enable_1" class="input" value="1" <% nvram_match_x("", "wyy_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="wyy_enable" id="wyy_enable_0" class="input" value="0" <% nvram_match_x("", "wyy_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
启用后，路由器自动分流解锁，大部分设备无需设置代理
											</td>

										</tr>
										</tr>
                                         <tr>
											<th>解锁程序选择</th>
											<td>
												<select name="wyy_apptype" class="input" style="width: 250px" onchange="switch_wyy_type()">
													<option value="go" <% nvram_match_x("","wyy_apptype", "go","selected"); %>>Golang 版本</option>
													<option value="cloud" <% nvram_match_x("","wyy_apptype", "cloud","selected"); %>>云解锁（ [CTCGFW] 云服务器）</option>
												</select>
											</td>
										</tr>
										 <tr id="row_wyy_cloudserver" style="display:none;">
											<th>服务器位置</th>
											<td>
												<select name="wyy_cloudserver" class="input" style="width: 250px" onchange="switch_wyy_cous()">
													<option value="cdn-shanghai.service.project-openwrt.eu.org:30000:30001">[CTCGFW] 腾讯云上海（高音质）</option>
													<option value="hyird.xyz:30000:30001">[hyird] 阿里云北京（高音质）</option>
													<option value="39.96.56.58:30000:30000">[Sunsky] 阿里云北京（高音质）</option>
													<option value="cdn-henan.service.project-openwrt.eu.org:33221:33222">[CTCGFW] 移动河南（无损音质）</option>
													<option value="coustom">-- 自定义 --</option>
												</select>
												<br>
													自定义服务器格式为 IP[域名]:HTTP端口:HTTPS端口
如果服务器为LAN内网IP，需要将这个服务器IP放入例外客户端 (不代理HTTP和HTTPS)
											</td>

										</tr>
										<!--<tr id="row_wyy_musicapptype" style="display:none;">
											<th>音源选择</th>
											<td>
											<select name="wyy_musicapptype" class="input" style="width: 250px" onchange="switch_wyy_mus()">
											<option value="default">默认</option>
											<option value="netease">网易云音乐</option>
											<option value="qq">QQ音乐</option>
											<option value="xiami">虾米音乐</option>
											<option value="baidu">百度音乐</option>
											<option value="kugou">酷狗音乐</option>
											<option value="kuwo">酷我音乐(高音质/FLACの解锁可能性)</option>
											<option value="migu">咕咪音乐</option>
											<option value="joox">JOOX音乐</option>
											<option value="coustom">-- 自定义 --</option>
										</select>
											</td>
										</tr>-->
<tr id="row_wyy_coustom_server" style="display:none;"><th>自定义服务器</th>
				<td>
					<input type="text" class="input" name="wyy_coustom_server" id="wyy_coustom_server" style="width: 200px" value="" />
				</td>
			</tr>
			<!--
<tr id="row_wyy_coustom_music" style="display:none;"> <th>自定义音源</th>
				<td>
					<input type="text" class="input" size="15" name="wyy_coustom_music" id="wyy_coustom_music" style="width: 200px" value="" />
				</td>
			</tr>-->
			<tr>
											<th width="30%" style="border-top: 0 none;">启用无损音质</th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="wyy_flac_on_of">
														<input type="checkbox" id="wyy_flac_fake" <% nvram_match_x("", "wyy_flac", "1", "value=1 checked"); %><% nvram_match_x("", "wyy_flac", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="wyy_flac" id="wyy_flac_1" class="input" value="1" <% nvram_match_x("", "wyy_flac", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="wyy_flac" id="wyy_flac_0" class="input" value="0" <% nvram_match_x("", "wyy_flac", "0", "checked"); %> /><#checkbox_No#>
												</div>
目前仅支持酷我、QQ、咪咕
											</td>

										</tr>
										<tr>
											<th>HTTPS 证书</th>
											<td>
				<input type="button" class="btn btn-success" value="下载CA根证书" onclick="window.open('https://raw.githubusercontent.com/nondanee/UnblockNeteaseMusic/master/ca.crt')" size="0"><br>Mac/iOS客户端需要安装 CA根证书并信任<br>iOS系统需要在“设置 -&gt; 通用 -&gt; 关于本机 -&gt; 证书信任设置”中，信任 UnblockNeteaseMusic Root CA <br>Linux 设备请在启用时加入 --ignore-certificate-errors 参数
											</td>
										</tr>
										<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr>
                                            <th colspan="5" id="GWStatic" style="background-color: #E3E3E3;">例外客户端规则（可以为局域网客户端分别设置不同的例外模式，默认无需设置）</th>
                                        </tr>
                                        <tr>
                                            <th width="25%">
                                                <#LANHostConfig_ManualMac_itemname#> <a href="javascript:sortbyMAC();" style="outline:0;"><i class="icon-circle-arrow-down"></i></a>
                                            </th>
                                            <th width="25%">
                                                <#LANHostConfig_ManualIP_itemname#> <a href="javascript:sortbyIP();" style="outline:0;"><i class="icon-circle-arrow-down"></i></a>
                                            </th>
                                            <th width="25%">
                                                <#LANHostConfig_ManualName_itemname#> <a href="javascript:sortbyName();" style="outline:0;"><i class="icon-circle-arrow-down"></i></a>
                                            </th>
											<th width="20%">
                                                过滤模式 <a href="javascript:sortbyName();" style="outline:0;"><i class="icon-circle-arrow-down"></i></a>
                                            </th>
                                            <th width="5%">
                                                <center><a href="javascript:sortbyId();" style="outline:0;"><i class="icon-th-list"></i></a></center>
                                            </th>
                                        </tr>
                                        <tr>
                                            <td width="25%">
                                                <div id="ClientList_Block" class="alert alert-info ddown-list" style="width: 400px;"></div>
                                                <div class="input-append">
                                                    <input type="text" maxlength="12" class="span12" size="12" name="wyy_mac_x_0" value="" onkeypress="return is_hwaddr(event);" style="float:left; width: 110px"/>
                                                    <button class="btn btn-chevron" id="chevron" type="button" onclick="pullLANIPList(this);" title="Select the MAC of LAN clients."><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="15" class="span12" size="15" name="wyy_ip_x_0" value="" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="24" class="span12" size="20" name="wyy_name_x_0" value="" onKeyPress="return is_string(this,event);"/>
                                            </td>
											 <td width="20%">
                                          	<select name="wyy_ip_road_x_0" class="input" style="width: 110px">
													<option value="disable" >不代理HTTP和HTTPS</option>
													<option value="http" >不代理HTTP</option>
													<option value="https" >不代理HTTPS</option>
												</select>
                                            </td>
                                            <td width="5%">
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupMDHCP(this, 64, ' Add ');" name="ManualDHCPList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td colspan="5" style="border-top: 0 none; padding: 0px;">
                                                <div id="MDHCPList_Block"></div>
                                            </td>
                                        </tr>
										<tr>
											<td colspan="5" style="border-top: 0 none;">
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

