<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - adbyby</title>
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
	init_itoggle('adbyby_enable');
	init_itoggle('hosts_ad');
	init_itoggle('anti_ad');
	init_itoggle('block_ios');
	init_itoggle('block_douyin');
	init_itoggle('tv_hosts');
	init_itoggle('adbyby_adb_update');
	init_itoggle('adbyby_ip_x', change_adbyby_ip_enabled);
	init_itoggle('adbyby_rules_x', change_adbyby_rules_enabled);
	var i=0;
    var z=0;
    var adbyby_update_hour = '<% nvram_get_x("", "adbyby_update_hour"); %>';
    var adbyby_update_min = '<% nvram_get_x("", "adbyby_update_min"); %>';
    while (i<24){
	i=i>9?i:"0"+i;
$j("#adbyby_update_hour").append("<option value='"+i+"'>"+i+"</option>");
i++;
}
$j("#adbyby_update_hour").val(adbyby_update_hour);
while (z<60)
{
z=z>9?z:"0"+z;
$j("#adbyby_update_min").append("<option value='"+z+"'>"+z+"</option>");
z++;
}
$j("#adbyby_update_min").val(adbyby_update_min);
});

</script>
<script>
<% adbyby_status(); %>
<% login_state_hook(); %>

var ipmonitor = [<% get_static_client(); %>];
var wireless = [];
var m_dhcp = [<% get_nvram_list("AdbybyConf", "AdIPList"); %>];
var m_rules = [<% get_nvram_list("AdbybyConf", "AdRULESList"); %>];
var mdhcp_ifield = 4;
if(m_dhcp.length > 0){
	var m_dhcp_ifield = m_dhcp[0].length;
	for (var i = 0; i < m_dhcp.length; i++) {
		m_dhcp[i][mdhcp_ifield] = i;
	}
}

var mrules_ifield = 2;
if(m_rules.length > 0){
	var m_rules_ifield = m_rules[0].length;
	for (var i = 0; i < m_rules.length; i++) {
		m_rules[i][mrules_ifield] = i;
	}
}

var clients_info = getclients(1,0);

var isMenuopen = 0;

function initial(){
	show_banner(2);
	show_menu(5,15);
	showmenu();
	show_footer();
	fill_adbyby_status(adbyby_status());
	//change_adbyby_enable();
	showMDHCPList();
	showLANIPList();
	showMRULESList();
	change_adbyby_ip_enabled();
	change_adbyby_rules_enabled();
	if (!login_safe())
		textarea_scripts_enabled(0);
		//load_body();
}

function showmenu(){
showhide_div('adlink', found_app_koolproxy());
}

function textarea_scripts_enabled(v){
	inputCtrl(document.form['scripts.adbyby_rules.sh'], v);
	inputCtrl(document.form['scripts.adbyby_blockip.sh'], v);
	inputCtrl(document.form['scripts.adbyby_adblack.sh'], v);
	inputCtrl(document.form['scripts.adbyby_adesc.sh'], v);
	inputCtrl(document.form['scripts.adbyby_adhost.sh'], v);
	inputCtrl(document.form['scripts.adbyby_host.sh'], v);
}

function applyRule(){
	//if(validForm()){
		showLoading();
		document.form.bkye.name = "group_id2";
	if (document.form.adbyby_ip_x[0].checked||document.form.adbyby_rules_x[0].checked)
		document.form.action_mode.value = " Restart ";
	else
		document.form.action_mode.value = " Apply ";
		document.form.current_page.value = "/Advanced_adbyby.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}


function submitInternet(v){
	showLoading();
	document.adbyby_action.action = "Ad_action.asp";
	document.adbyby_action.connect_action.value = v;
	document.adbyby_action.submit();
}

//function change_adbyby_enable(){
//	var m = document.form.adbyby_enable[0].checked;
	//showhide_div('kp_update_b', m);
//}


function fill_adbyby_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("adbyby_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
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
	document.form.adbybyip_mac_x_0.value = clients_info[num][2];
	document.form.adbybyip_ip_x_0.value = clients_info[num][1];
	document.form.adbybyip_name_x_0.value = clients_info[num][0];
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

function pullLANIPList(obj){
	if(isMenuopen == 0){
		$j(obj).children('i').removeClass('icon-chevron-down').addClass('icon-chevron-up');
		$("ClientList_Block").style.display = 'block';
		document.form.adbybyip_mac_x_0.focus();
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function change_adbyby_ip_enabled(){
	var v = document.form.adbyby_ip_x[0].checked;
	showhide_div('row_static_arp', v);
	showhide_div('row_static_caption', v);
	showhide_div('row_static_header', v);
	showhide_div('row_static_body', v);
}

function change_adbyby_rules_enabled(){
	var v = document.form.adbyby_rules_x[0].checked;
	showhide_div('row_rules_caption', v);
	showhide_div('row_rules_header', v);
	showhide_div('row_rules_body', v);
}

function done_validating(action){
	refreshpage();
}

function markGroupMDHCP(o, c, b) {
	document.form.group_id.value = "AdIPList";
	if(b == " Add "){
		if (document.form.adbybyip_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.adbybyip_mac_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.adbybyip_mac_x_0.focus();
			document.form.adbybyip_mac_x_0.select();
			return false;
		}else if(document.form.adbybyip_ip_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.adbybyip_ip_x_0.focus();
			document.form.adbybyip_ip_x_0.select();
			return false;
		}else if (!validate_hwaddr(document.form.adbybyip_mac_x_0)){
			return false;
		}else if (!validate_ipaddr_final(document.form.adbybyip_ip_x_0, 'staticip')){
			return false;
		}else{
			for(i=0; i<m_dhcp.length; i++){
				if(document.form.adbybyip_mac_x_0.value==m_dhcp[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][0] + ')' );
					document.form.adbybyip_mac_x_0.focus();
					document.form.adbybyip_mac_x_0.select();
					return false;
				}
				if(document.form.adbybyip_ip_x_0.value.value==m_dhcp[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_dhcp[i][1] + ')' );
					document.form.adbybyip_ip_x_0.focus();
					document.form.adbybyip_ip_x_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function markGroupRULES(o, c, b) {
	document.form.group_id.value = "AdRULESList";
	if(b == " Add "){
		if (document.form.adbybyrules_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.adbybyrules_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.adbybyrules_x_0.focus();
			document.form.adbybyrules_x_0.select();
			return false;
		}else if(document.form.adbybyrules_road_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.adbybyrules_road_0.focus();
			document.form.adbybyrules_road_0.select();
			return false;
		}else{
			for(i=0; i<m_rules.length; i++){
				if(document.form.adbybyrules_x_0.value==m_rules[i][0]) {
					alert('<#JS_duplicate#>' + ' (' + m_rules[i][0] + ')' );
					document.form.adbybyrules_x_0.focus();
					document.form.adbybyrules_x_0.select();
					return false;
				}
				if(document.form.adbybyrules_road_x_0.value.value==m_rules[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_rules[i][1] + ')' );
					document.form.adbybyrules_road_0.focus();
					document.form.adbybyrules_road_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function showMDHCPList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_dhcp.length == 0)
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < m_dhcp.length; i++){
		if(m_dhcp[i][3] == 0)
		adbybyiproad="直连模式";
		else if(m_dhcp[i][3] == 1){
		adbybyiproad="全局模式";
		}else if(m_dhcp[i][3] == 2){
		adbybyiproad="Plus+模式";
		}
		code +='<tr id="row' + i + '">';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][0] + '</td>';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][1] + '</td>';
		code +='<td width="25%">&nbsp;' + m_dhcp[i][2] + '</td>';
		code +='<td width="20%">&nbsp;' + adbybyiproad + '</td>';
		code +='<td width="5%" style="text-align: center;"><input type="checkbox" name="AdIPList_s" value="' + m_dhcp[i][mdhcp_ifield] + '" onClick="changeBgColor(this,' + i + ');" id="check' + m_dhcp[i][mdhcp_ifield] + '"></td>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="4">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupMDHCP(this, 64, \' Del \');" name="AdIPList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MDHCPList_Block").innerHTML = code;
}

function showMRULESList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_rules.length == 0)
		code +='<tr><td colspan="3" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < m_rules.length; i++){
		if(m_rules[i][1] == 0)
		adbybyrulesroad="已禁用";
		else{
		adbybyrulesroad="已启用";
		}
		code +='<tr id="rowrl' + i + '">';
		code +='<td width="75%">&nbsp;' + m_rules[i][0] + '</td>';
		code +='<td width="8%">&nbsp;</td>';
		code +='<td width="12%">&nbsp;' + adbybyrulesroad + '</td>';
		code +='<center><td width="5%" style="text-align: center;"><input type="checkbox" name="AdRULESList_s" value="' + m_rules[i][mrules_ifield] + '" onClick="changeBgColorrl(this,' + i + ');" id="check' + m_rules[i][mrules_ifield] + '"></td></center>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="3">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupRULES(this, 64, \' Del \');" name="AdRULESList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MRULESList_Block").innerHTML = code;
}

function changeBgColor(obj, num){
	$("row" + num).style.background=(obj.checked)?'#D9EDF7':'whiteSmoke';
}
function changeBgColorrl(obj, num){
	$("rowrl" + num).style.background=(obj.checked)?'#D9EDF7':'whiteSmoke';
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

	<input type="hidden" name="current_page" value="Advanced_adbyby.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="AdbybyConf;">
	<input type="hidden" name="group_id" value="AdIPList">
	<input type="hidden" name="bkye" value="AdRULESList">
	<input type="hidden" name="action_mode" value="">
	<input type="hidden" name="action_script" value="">
    <input type="hidden" name="adbybyip_staticnum_x_0" value="<% nvram_get_x("AdIPList", "adbybyip_staticnum_x"); %>" readonly="1" />
	<input type="hidden" name="adbybyrules_staticnum_x_0" value="<% nvram_get_x("AdRULESList", "adbybyrules_staticnum_x"); %>" readonly="1" />
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
							<h2 class="box_head round_top"><#menu5_20_1#> - <#menu5_20#></h2>
							<div class="round_bottom">
							<div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
								
								<li class="active">
                                    <a href="Advanced_adbyby.asp"><#menu5_20_1#></a>
                                </li>
								 <li id="adlink" style="display:none">
                                    <a href="Advanced_koolproxy.asp"><#menu5_26_1#></a>
                                </li>
                            </ul>
                        </div>
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">广告屏蔽大师 Plus + 可以全面过滤各种横幅、弹窗、视频广告，同时阻止跟踪、隐私窃取及各种恶意网站<br />
									<div>Plus + 版本可以和 Hosts 结合方式运行，过滤广告不损失带宽</div>
									<div>anti-AD项目地址:<a href="https://github.com/privacy-protection-tools/anti-AD">https://github.com/privacy-protection-tools/anti-AD</a></div>
									<div>静态规则：【<% nvram_get_x("", "adbyby_ltime"); %>】 | 视频规则：【<% nvram_get_x("", "adbyby_vtime"); %>】</div>
									<div>anti-AD规则：【<% nvram_get_x("", "anti_ad_count"); %>】条 | Hosts AD：【<% nvram_get_x("", "adbyby_hostsad"); %>】条</div>
									<div>第三方规则：【<% nvram_get_x("", "adbyby_user"); %>】条</div>
									<div> </div>
									</div>
									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
									<tr> <th>运行状态:</th>
                                            <td id="adbyby_status" colspan="3"></td>
                                        </tr>
										<tr >
											<th width="50%">启用 Adbyby 功能 &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;</th>
											<td>
													<div class="main_itoggle">
													<div id="adbyby_enable_on_of">
														<input type="checkbox" id="adbyby_enable_fake" <% nvram_match_x("", "adbyby_enable", "1", "value=1 checked"); %><% nvram_match_x("", "adbyby_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="adbyby_enable" id="adbyby_enable_1" class="input" <% nvram_match_x("", "adbyby_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="adbyby_enable" id="adbyby_enable_0" class="input" <% nvram_match_x("", "adbyby_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
											<th width="50%">
											<a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 0, 1);">过滤方案选择:</a></th>
											<td>
												<select name="adbyby_set" class="input">
													<option value="0" <% nvram_match_x("","adbyby_set", "0","selected"); %>>全局模式（推荐），全部IP走adbyby过滤</option>
													<option value="1" <% nvram_match_x("","adbyby_set", "1","selected"); %>>Plus + 模式(只过滤列表内域名结合ABP名单)</option>
													<option value="2" <% nvram_match_x("","adbyby_set", "2","selected"); %>>内网IP列表控制模式</option>
												</select>
											</td>
										</tr>
										<tr id="adbyby_update_tr">
											<th>规则自动更新:</th>
											<td>
												<select name="adbyby_update" class="input" style="width: 60px;">
													<option value="0" <% nvram_match_x("","adbyby_update", "0","selected"); %>>每天</option>
													<option value="1" <% nvram_match_x("","adbyby_update", "1","selected"); %>>每隔</option>
													<option value="2" <% nvram_match_x("","adbyby_update", "2","selected"); %>>关闭</option>
												</select>
												 <select name="adbyby_update_hour" id="adbyby_update_hour" class="input" style="width: 50px">

                                                </select>时
												<select name="adbyby_update_min" id="adbyby_update_min" class="input" style="width: 50px">

                                                </select>分
											</td>
										</tr>
										<tr>
											<th width="50%">
											拦截 Apple iOS 的OTA更新:</th>
											<td>
													<div class="main_itoggle">
													<div id="block_ios_on_of">
														<input type="checkbox" id="block_ios_fake" <% nvram_match_x("", "block_ios", "1", "value=1 checked"); %><% nvram_match_x("", "block_ios", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="block_ios" id="block_ios_1" class="input" value="1" <% nvram_match_x("", "block_ios", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="block_ios" id="block_ios_0" class="input" value="0" <% nvram_match_x("", "block_ios", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
											<th width="50%">
											拦截 抖音 APP 和网站:</th>
											<td>
													<div class="main_itoggle">
													<div id="block_douyin_on_of">
														<input type="checkbox" id="block_douyin_fake" <% nvram_match_x("", "block_douyin", "1", "value=1 checked"); %><% nvram_match_x("", "block_douyin", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="block_douyin" id="block_douyin_1" class="input" value="1" <% nvram_match_x("", "block_douyin", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="block_douyin" id="block_douyin_0" class="input" value="0" <% nvram_match_x("", "block_douyin", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
											<th width="50%">
											<a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 2, 1);">加载anti-AD项目规则:</a></th>
											<td>
													<div class="main_itoggle">
													<div id="anti_ad_on_of">
														<input type="checkbox" id="anti_ad_fake" <% nvram_match_x("", "anti_ad", "1", "value=1 checked"); %><% nvram_match_x("", "anti_ad", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="anti_ad" id="anti_ad_1" class="input" value="1" <% nvram_match_x("", "anti_ad", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="anti_ad" id="anti_ad_0" class="input" value="0" <% nvram_match_x("", "anti_ad", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
                                            <th width="50%">加载anti-AD下载地址:</th>
                                            <td>
                                                <input type="text"  class="input" size="60" name="anti_ad_link" value="<% nvram_get_x("","anti_ad_link"); %>" />
                                            </td>
                                        </tr>
										<tr>
											<th width="50%">加载hosts规则</th>
											<td>
													<div class="main_itoggle">
													<div id="hosts_ad_on_of">
														<input type="checkbox" id="hosts_ad_fake" <% nvram_match_x("", "hosts_ad", "1", "value=1 checked"); %><% nvram_match_x("", "hosts_ad", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="hosts_ad" id="hosts_ad_1" class="input" value="1" <% nvram_match_x("", "hosts_ad", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="hosts_ad" id="hosts_ad_0" class="input" value="0" <% nvram_match_x("", "hosts_ad", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="3">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script15')"><span>hosts规则下载列表(一行一个地址):</span></a>
												<div id="script15">
													<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_host.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_host.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										</table>
										<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="5" id="GWStatic" style="background-color: #E3E3E3;">自定义IP过滤设置</th>
                                        </tr>
                                        <tr>
                                            <th colspan="2" width="50%">启用内网过滤控制</th>
                                            <td colspan="2" width="50%">
                                                <div class="main_itoggle">
                                                    <div id="adbyby_ip_x_on_of">
                                                        <input type="checkbox" id="adbyby_ip_x_fake" <% nvram_match_x("", "adbyby_ip_x", "1", "value=1 checked"); %><% nvram_match_x("", "adbyby_ip_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="adbyby_ip_x" id="adbyby_ip_x_1" onclick="change_adbyby_ip_enabled()" <% nvram_match_x("", "adbyby_ip_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="adbyby_ip_x" id="adbyby_ip_x_0" onclick="change_adbyby_ip_enabled()" <% nvram_match_x("", "adbyby_ip_x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										</table>
										<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr id="row_static_caption" style="display:none">
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
                                        <tr id="row_static_header" style="display:none">
                                            <td width="25%">
                                                <div id="ClientList_Block" class="alert alert-info ddown-list" style="width: 400px;"></div>
                                                <div class="input-append">
                                                    <input type="text" maxlength="12" class="span12" size="12" name="adbybyip_mac_x_0" value="<% nvram_get_x("", "adbybyip_mac_x_0"); %>" onkeypress="return is_hwaddr(event);" style="float:left; width: 110px"/>
                                                    <button class="btn btn-chevron" id="chevron" type="button" onclick="pullLANIPList(this);" title="Select the MAC of LAN clients."><i class="icon icon-chevron-down"></i></button>
                                                </div>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="15" class="span12" size="15" name="adbybyip_ip_x_0" value="<% nvram_get_x("", "adbybyip_ip_x_0"); %>" onkeypress="return is_ipaddr(this,event);"/>
                                            </td>
                                            <td width="25%">
                                                <input type="text" maxlength="24" class="span12" size="20" name="adbybyip_name_x_0" value="<% nvram_get_x("", "adbybyip_name_x_0"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
											 <td width="20%">
                                          	<select name="adbybyip_ip_road_x_0" class="input" style="width: 110px">
													<option value="0" <% nvram_match_x("","adbybyip_ip_road_x_0", "0","selected"); %>>直连模式</option>
													<option value="1" <% nvram_match_x("","adbybyip_ip_road_x_0", "1","selected"); %>>全局模式</option>
													<option value="2" <% nvram_match_x("","adbybyip_ip_road_x_0", "2","selected"); %>>plus+模式</option>
												</select>
                                            </td>
                                            <td width="5%">
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupMDHCP(this, 64, ' Add ');" name="ManualDHCPList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
                                            </td>
                                        </tr>
                                        <tr id="row_static_body" style="display:none">
                                            <td colspan="5" style="border-top: 0 none; padding: 0px;">
                                                <div id="MDHCPList_Block"></div>
                                            </td>
                                        </tr>
                                    </table>
									 <table width="100%" align="center" cellpadding="5" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="5" id="GWStatic" style="background-color: #E3E3E3;">第三方过滤规则</th>
                                        </tr>
                                        <tr>
                                            <th width="50%">启用第三方过滤规则</th>
                                            <td width="50%">
                                                <div class="main_itoggle">
                                                    <div id="adbyby_rules_x_on_of">
                                                        <input type="checkbox" id="adbyby_rules_x_fake" <% nvram_match_x("", "adbyby_rules_x", "1", "value=1 checked"); %><% nvram_match_x("", "adbyby_rules_x", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="adbyby_rules_x" id="adbyby_rules_x_1" onclick="change_adbyby_rules_enabled()" <% nvram_match_x("", "adbyby_rules_x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="adbyby_rules_x" id="adbyby_rules_x_0" onclick="change_adbyby_rules_enabled()" <% nvram_match_x("", "adbyby_rules_x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										</table>
										<table width="100%" align="center" cellpadding="3" cellspacing="0" class="table">
                                        <tr id="row_rules_caption" style="display:none">
										 
                                            <th width="85%">
                                                规则地址 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="10%">
                                                状态 <i class="icon-circle-arrow-down"></i>
                                            </th>
                                            <th width="5%">
                                                <center><i class="icon-th-list"></i></center>
                                            </th>
                                        </tr>
                                        <tr id="row_rules_header" style="display:none">
                                            <td width="85%">
                                                <input type="text" maxlength="255" class="span12" size="200" name="adbybyrules_x_0" value="<% nvram_get_x("", "adbybyrules_x_0"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
											 <td width="10%">
                                          	<select name="adbybyrules_road_x_0" class="input" style="width: 65px">
													<option value="0" <% nvram_match_x("","adbybyrules_road_x_0", "0","selected"); %>>禁用</option>
													<option value="1" <% nvram_match_x("","adbybyrules_road_x_0", "1","selected"); %>>启用</option>
												</select>
                                            </td>
                                            <td width="5%">
                                               <center> <button class="btn" style="max-width: 219px " type="submit" onclick="return markGroupRULES(this, 64, ' Add ');" name="ManualRULESList2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button></center>
                                            </td>
                                        </tr>
                                        <tr id="row_rules_body" style="display:none">
                                            <td colspan="3" style="border-top: 0 none; padding: 0px;">
                                                <div id="MRULESList_Block"></div>
                                            </td>
                                        </tr>
                                    </table>
									<table class="table">
										<tr>
											<td colspan="3" >
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script9')"><span>Plus+ 模式过滤的域名:</span></a>
												<div id="script9">
													<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_adhost.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_adhost.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="3" style="border-top: 0 none;">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script10')"><span>域名白名单:</span></a>
												<div id="script10" style="display:none;">
													<textarea rows="24" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_adesc.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_adesc.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="3" style="border-top: 0 none;">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script11')"><span>IP黑名单:</span></a>
												<div id="script11" style="display:none;">
													<textarea rows="24" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_blockip.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_blockip.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="3" style="border-top: 0 none;">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script12')"><span>域名黑名单:</span></a>
												<div id="script12" style="display:none;">
													<textarea rows="24" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_adblack.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_adblack.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="3" style="border-top: 0 none;">
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script13')"><span>用户自定义规则:</span></a>
												<div id="script13" style="display:none;">
													<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.adbyby_rules.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.adbyby_rules.sh",""); %></textarea>
												</div>
											</td>
										</tr>
										
										
									
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
<form method="post" name="adbyby_action" action="">
    <input type="hidden" name="connect_action" value="">
</form>
</body>
</html>
