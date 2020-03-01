<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_21#></title>
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
<% smartdns_status(); %>

$j(document).ready(function(){
	init_itoggle('sdns_enable');
	init_itoggle('sdns_tcp_server');
	init_itoggle('sdns_ipv6_server');
	init_itoggle('snds_ip_change');
	init_itoggle('sdns_www');
	init_itoggle('sdns_ipv6');
	init_itoggle('sdns_exp');
	init_itoggle('sdnse_enable');
	init_itoggle('sdnse_address');
	init_itoggle('sdnse_tcp');
	init_itoggle('sdnse_as');
	init_itoggle('sdnse_speed');
	init_itoggle('sdnse_ns');
	init_itoggle('sdnse_ipc');
	init_itoggle('sdnse_ipset');
	init_itoggle('sdnse_cache');
	init_itoggle('sdnse_coredump');
	init_itoggle('ss_black');
	init_itoggle('ss_white');
	init_itoggle('sdnss_enable_x_0');
		$j("#tab_sm_cfg, #tab_sm_sec, #tab_sm_dns, #tab_sm_cou").click(function(){
		var newHash = $j(this).attr('href').toLowerCase();
		showTab(newHash);
		return false;
	});
});

var m_list = [<% get_nvram_list("SmartdnsConf", "SdnsList"); %>];
var mlist_ifield = 6;
if(m_list.length > 0){
	var m_list_ifield = m_list[0].length;
	for (var i = 0; i < m_list.length; i++) {
		m_list[i][mlist_ifield] = i;
	}
}
function initial(){
	show_banner(2);
	show_menu(5,16);
	show_footer();
	showTab(getHash());
	showMRULESList();
	showmenu();
	fill_status(smartdns_status());
}

function applyRule(){
	//if(validForm()){
		showLoading();
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "Advanced_smartdns.asp";
		document.form.next_page.value = "";
		document.form.submit();
	//}
}
var arrHashes = ["cfg", "sec", "dns", "cou"];
function showTab(curHash){
	var obj = $('tab_sm_'+curHash.slice(1));
	if (obj == null || obj.style.display == 'none')
		curHash = '#cfg';
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i])){
			$j('#tab_sm_'+arrHashes[i]).parents('li').addClass('active');
			$j('#wnd_sm_'+arrHashes[i]).show();
		}else{
			$j('#wnd_sm_'+arrHashes[i]).hide();
			$j('#tab_sm_'+arrHashes[i]).parents('li').removeClass('active');
		}
	}
	window.location.hash = curHash;
}

function getHash(){
	var curHash = window.location.hash.toLowerCase();
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i]))
			return curHash;
	}
	return ('#'+arrHashes[0]);
}

function fill_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("smartdns_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}
function markGroupRULES(o, c, b) {
	document.form.group_id.value = "SdnsList";
	if(b == " Add "){
		if (document.form.sdnss_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.sdnss_ip_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.sdnss_ip_x_0.focus();
			document.form.sdnss_ip_x_0.select();
			return false;
		}else if(document.form.sdnss_name_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.sdnss_name_0.focus();
			document.form.sdnss_name_0.select();
			return false;
		}else{
			for(i=0; i<m_list.length; i++){
				if(document.form.sdnss_ip_x_0.value==m_list[i][2]) {
				if(document.form.sdnss_type_x_0.value==m_list[i][4]) {
					alert('<#JS_duplicate#>' + ' (' + m_list[i][2] + ')' );
					document.form.sdnss_ip_x_0.focus();
					document.form.sdnss_ip_x_0.select();
					return false;
					}
				}
				if(document.form.sdnss_name_x_0.value.value==m_list[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_list[i][1] + ')' );
					document.form.sdnss_name_0.focus();
					document.form.sdnss_name_0.select();
					return false;
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	document.form.current_page.value = "Advanced_smartdns.asp#dns";
	return true;
}
function showmenu(){
showhide_div('adglink', found_app_adguardhome());
}
function showMRULESList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_list.length == 0)
		code +='<tr><td colspan="3" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < m_list.length; i++){
		if(m_list[i][0] == 0)
		adbybyrulesroad="已禁用";
		else{
		adbybyrulesroad="已启用";
		}
		if(m_list[i][5] == 0)
		ipc="禁用";
		else if(m_list[i][5] == "whitelist"){
		ipc="白名单";
		}else{
		ipc="黑名单";
		}
		code +='<tr id="rowrl' + i + '">';
		code +='<td width="10%">&nbsp;' + adbybyrulesroad + '</td>';
		code +='<td width="20%">&nbsp;' + m_list[i][1] + '</td>';
		code +='<td width="25%" class="spanb">' + m_list[i][2] + '</td>';
		code +='<td width="10%">&nbsp;' + m_list[i][3] + '</td>';
		code +='<td width="10%">&nbsp;' + m_list[i][4] + '</td>';
		code +='<td width="15%">&nbsp;' + ipc + '</td>';
		code +='<center><td width="5%" style="text-align: center;"><input type="checkbox" name="SdnsList_s" value="' + m_list[i][mlist_ifield] + '" onClick="changeBgColorrl(this,' + i + ');" id="check' + m_list[i][mlist_ifield] + '"></td></center>';
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="6">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupRULES(this, 64, \' Del \');" name="SdnsList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MRULESList_Block").innerHTML = code;
}
</script>

<style>
.nav-tabs > li > a {
    padding-right: 6px;
    padding-left: 6px;
}
.spanb{
    overflow:hidden;
　　text-overflow:ellipsis;
　　white-space:nowrap;
}
</style>
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
	
    <input type="hidden" name="current_page" value="Advanced_smartdns.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="SmartdnsConf;">
    <input type="hidden" name="group_id" value="SdnsList">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
	<input type="hidden" name="sdnss_staticnum_x_0" value="<% nvram_get_x("SdnsList", "sdnss_staticnum_x"); %>" readonly="1" />

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
                            <h2 class="box_head round_top"><#menu5_24#> - <#menu5_29#></h2>
                            <div class="round_bottom">
							<div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
                                <li class="active">
                                    <a href="Advanced_smartdns.asp"><#menu5_24#></a>
                                </li>
								 <li id="adglink" style="display:none">
                                    <a href="Advanced_adguardhome.asp"><#menu5_28#></a>
                                </li>
                            </ul>
                        </div>
						<div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
                                <li class="active">
                                    <a id="tab_sm_cfg" href="#cfg">基本设置</a>
                                </li>
								 <li>
                                    <a id="tab_sm_sec" href="#sec">第二服务器</a>
                                <li>
								<li>
                                    <a id="tab_sm_dns" href="#dns">上游服务器</a>
                                <li>
                                    <a id="tab_sm_cou" href="#cou">其他设置</a>
                                </li>
                            </ul>
                        </div>
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">SmartDNS是一个本地高性能DNS服务器，支持避免域名污染，支持返回最快IP，支持广告过滤。</br>
									SmartDNS官方网站:<a href="https://pymumu.github.io/smartdns/">https://pymumu.github.io/smartdns/</a>
</div>
</div>
<div id="wnd_sm_cfg">
                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr> <th width="50%"><#running_status#></th>
                                            <td id="smartdns_status" colspan="2"></td>
                                        </tr>

                                        <tr> <th><#menu5_21_1#></th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_enable_on_of">
                                                    <input type="checkbox" id="sdns_enable_fake" <% nvram_match_x("", "sdns_enable", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_enable", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_enable" id="sdns_enable_1" <% nvram_match_x("", "sdns_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_enable" id="sdns_enable_0" <% nvram_match_x("", "sdns_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr> <th>服务器名称</th>
                                            <td>
                                                <input type="text" maxlength="15" class="input" size="15" name="snds_name" style="width: 200px" value="<% nvram_get_x("","snds_name"); %>" />
                                            </td>
                                        </tr>

                                        <tr> <th>本地端口</th>
                                            <td>
                                                <input type="text" maxlength="5" class="input" size="15" name="sdns_port" style="width: 200px" value="<% nvram_get_x("", "sdns_port"); %>">
                                            </td>
                                        </tr>
										 <tr> <th>TCP服务器</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_tcp_server_on_of">
                                                    <input type="checkbox" id="sdns_tcp_server_fake" <% nvram_match_x("", "sdns_tcp_server", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_tcp_server", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_tcp_server" id="sdns_tcp_server_1" <% nvram_match_x("", "sdns_tcp_server", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_tcp_server" id="sdns_tcp_server_0" <% nvram_match_x("", "sdns_tcp_server", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>IPV6服务器</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_ipv6_server_on_of">
                                                    <input type="checkbox" id="sdns_ipv6_server_fake" <% nvram_match_x("", "sdns_ipv6_server", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_ipv6_server", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_ipv6_server" id="sdns_ipv6_server_1" <% nvram_match_x("", "sdns_ipv6_server", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_ipv6_server" id="sdns_ipv6_server_0" <% nvram_match_x("", "sdns_ipv6_server", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>双栈IP优选</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="snds_ip_change_on_of">
                                                    <input type="checkbox" id="snds_ip_change_fake" <% nvram_match_x("", "snds_ip_change", "1", "value=1 checked"); %><% nvram_match_x("", "snds_ip_change", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="snds_ip_change" id="snds_ip_change_1" <% nvram_match_x("", "snds_ip_change", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="snds_ip_change" id="snds_ip_change_0" <% nvram_match_x("", "snds_ip_change", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th width="50%">双栈IP优选阈值</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="64" name="snds_ip_change_time" style="width: 50px" value="<% nvram_get_x("", "snds_ip_change_time"); %>"> 毫秒（0-100）
                                            </td>
                                        </tr>
										<tr> <th>禁用IPV6解析</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_ipv6_on_of">
                                                    <input type="checkbox" id="sdns_ipv6_fake" <% nvram_match_x("", "sdns_ipv6", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_ipv6", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_ipv6" id="sdns_ipv6_1" <% nvram_match_x("", "sdns_ipv6", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_ipv6" id="sdns_ipv6_0" <% nvram_match_x("", "sdns_ipv6", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>域名预加载</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_www_on_of">
                                                    <input type="checkbox" id="sdns_www_fake" <% nvram_match_x("", "sdns_www", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_www", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_www" id="sdns_www_1" <% nvram_match_x("", "sdns_www", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_www" id="sdns_www_0" <% nvram_match_x("", "sdns_www", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>过期缓存服务</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdns_exp_on_of">
                                                    <input type="checkbox" id="sdns_exp_fake" <% nvram_match_x("", "sdns_exp", "1", "value=1 checked"); %><% nvram_match_x("", "sdns_exp", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdns_exp" id="sdns_exp_1" <% nvram_match_x("", "sdns_exp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdns_exp" id="sdns_exp_0" <% nvram_match_x("", "sdns_exp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>加载ChnrouteIP为白名单</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="ss_white_on_of">
                                                    <input type="checkbox" id="ss_white_fake" <% nvram_match_x("", "ss_white", "1", "value=1 checked"); %><% nvram_match_x("", "ss_white", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_white" id="ss_white_1" <% nvram_match_x("", "ss_white", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_white" id="ss_white_0" <% nvram_match_x("", "ss_white", "0", "checked"); %>><#checkbox_No#>
                                                </div>
												<div><span style="color:#888;">此项可配合科学上网来实现大陆IP才走国内DNS</span></div>
                                            </td>
                                        </tr>
										<tr> <th>加载ChnrouteIP为黑名单</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="ss_black_on_of">
                                                    <input type="checkbox" id="ss_black_fake" <% nvram_match_x("", "ss_black", "1", "value=1 checked"); %><% nvram_match_x("", "ss_black", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="ss_black" id="ss_black_1" <% nvram_match_x("", "ss_black", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="ss_black" id="ss_black_0" <% nvram_match_x("", "ss_black", "0", "checked"); %>><#checkbox_No#>
                                                </div>
												<div><span style="color:#888;">此项可配合科学上网来实现大陆IP禁止走国外DNS</span></div>
                                            </td>
                                        </tr>
<tr>
											<th>重定向</th>
											<td>
												<select name="snds_redirect" class="input" style="width: 200px">
													<option value="0" <% nvram_match_x("","snds_redirect", "0","selected"); %>>无</option>
													<option value="1" <% nvram_match_x("","snds_redirect", "1","selected"); %>>作为dnsmasq的上游服务器</option>
													<option value="2" <% nvram_match_x("","snds_redirect", "2","selected"); %>>重定向53端口到SmartDns</option>
												</select>
											</td>
										</tr>
                                        <tr> <th>缓存大小</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="15" name="snds_cache" style="width: 200px" value="<% nvram_get_x("", "snds_cache"); %>">
												<div><span style="color:#888;">缓存DNS的结果，缓存大小，配置零则不缓存</span></div>
                                            </td>
                                        </tr>
										<tr> <th>域名TTL</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="15" name="sdns_ttl" style="width: 200px" value="<% nvram_get_x("", "sdns_ttl"); %>">
												<div><span style="color:#888;">设置所有域名的TTL</span></div>
                                            </td>
                                        </tr>
										<tr> <th>域名TTL最小值</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="15" name="sdns_ttl_min" style="width: 200px" value="<% nvram_get_x("", "sdns_ttl_min"); %>">
												<div><span style="color:#888;">设置所有域名的TTL最小值</span></div>
                                            </td>
                                        </tr>
										<tr> <th>域名TTL最大值</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="15" name="sdns_ttl_max" style="width: 200px" value="<% nvram_get_x("", "sdns_ttl_max"); %>">
												<div><span style="color:#888;">设置所有域名的TTL最大值</span></div>
                                            </td>
                                        </tr>
										
										</table>
										</div>
										<div id="wnd_sm_sec">
										<table width="100%" cellpadding="2" cellspacing="0" class="table">
										<tr> <th width="50%">启用</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_enable_on_of">
                                                    <input type="checkbox" id="sdnse_enable_fake" <% nvram_match_x("", "sdnse_enable", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_enable", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_enable" id="sdnse_enable_1" <% nvram_match_x("", "sdnse_enable", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_enable" id="sdnse_enable_0" <% nvram_match_x("", "sdnse_enable", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>本地端口</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="64" name="sdnse_port" style="width: 200px" value="<% nvram_get_x("", "sdnse_port"); %>">
										
                                            </td>
                                        </tr>
										<tr> <th>TCP服务器</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_tcp_on_of">
                                                    <input type="checkbox" id="sdnse_tcp_fake" <% nvram_match_x("", "sdnse_tcp", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_tcp", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_tcp" id="sdnse_tcp_1" <% nvram_match_x("", "sdnse_tcp", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_tcp" id="sdnse_tcp_0" <% nvram_match_x("", "sdnse_tcp", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										
										<tr> <th>跳过测速</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_speed_on_of">
                                                    <input type="checkbox" id="sdnse_speed_fake" <% nvram_match_x("", "sdnse_speed", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_speed", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_speed" id="sdnse_speed_1" <% nvram_match_x("", "sdnse_speed", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_speed" id="sdnse_speed_0" <% nvram_match_x("", "sdnse_speed", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th width="50%">服务器组</th>
                                            <td>
                                                <input type="text" maxlength="64" class="input" size="64" name="sdnse_name" placeholder="default" style="width: 200px" value="<% nvram_get_x("", "sdnse_name"); %>">
												<div><span style="color:#888;">使用指定服务器组查询，比如office, home</span></div>
                                            </td>
                                        </tr>
										<tr> <th>跳过address规则</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_address_on_of">
                                                    <input type="checkbox" id="sdnse_address_fake" <% nvram_match_x("", "sdnse_address", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_address", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_address" id="sdnse_address_1" <% nvram_match_x("", "sdnse_address", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_address" id="sdnse_address_0" <% nvram_match_x("", "sdnse_address", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>跳过Nameserver规则</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_ns_on_of">
                                                    <input type="checkbox" id="sdnse_ns_fake" <% nvram_match_x("", "sdnse_ns", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_ns", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_ns" id="sdnse_ns_1" <% nvram_match_x("", "sdnse_ns", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_ns" id="sdnse_ns_0" <% nvram_match_x("", "sdnse_ns", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>跳过ipset规则</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_ipset_on_of">
                                                    <input type="checkbox" id="sdnse_ipset_fake" <% nvram_match_x("", "sdnse_ipset", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_ipset", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_ipset" id="sdnse_ipset_1" <% nvram_match_x("", "sdnse_ipset", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_ipset" id="sdnse_ipset_0" <% nvram_match_x("", "sdnse_ipset", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>跳过address SOA(#)规则</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_as_on_of">
                                                    <input type="checkbox" id="sdnse_as_fake" <% nvram_match_x("", "sdnse_as", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_as", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_as" id="sdnse_as_1" <% nvram_match_x("", "sdnse_as", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_as" id="sdnse_as_0" <% nvram_match_x("", "sdnse_as", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>跳过双栈优选</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_ipc_on_of">
                                                    <input type="checkbox" id="sdnse_ipc_fake" <% nvram_match_x("", "sdnse_ipc", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_ipc", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_ipc" id="sdnse_ipc_1" <% nvram_match_x("", "sdnse_ipc", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_ipc" id="sdnse_ipc_0" <% nvram_match_x("", "sdnse_ipc", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										<tr> <th>跳过cache</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_cache_on_of">
                                                    <input type="checkbox" id="sdnse_cache_fake" <% nvram_match_x("", "sdnse_cache", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_cache", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_cache" id="sdnse_cache_1" <% nvram_match_x("", "sdnse_cache", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_cache" id="sdnse_cache_0" <% nvram_match_x("", "sdnse_cache", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
										
										</table>
										</div>
										<div id="wnd_sm_dns">
										<table width="100%" cellpadding="4" cellspacing="0" class="table">
										<tbody>
                                        <tr>
                                         <th width="50%">启用:</th>
										 <td>
                                                <div class="main_itoggle">
                                                <div id="sdnss_enable_x_0_on_of">
                                                    <input type="checkbox" id="sdnss_enable_x_0_fake" <% nvram_match_x("", "sdnss_enable_x_0", "1", "value=1 checked"); %><% nvram_match_x("", "sdnss_enable_x_0", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnss_enable_x_0" id="sdnss_enable_x_0_1" <% nvram_match_x("", "sdnss_enable_x_0", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnss_enable_x_0" id="sdnss_enable_x_0_0" <% nvram_match_x("", "sdnss_enable_x_0", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
											</tr>
                                        <tr>
                                         <th>上游名称:</th>
										 <td>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="sdnss_name_x_0" value="<% nvram_get_x("", "sdnss_name_x_0"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
											</tr>
                                        <tr>
                                         <th>上游地址:</th>
										 <td>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="sdnss_ip_x_0" value="<% nvram_get_x("", "sdnss_ip_x_0"); %>" onKeyPress="return is_string(this,event);"/>
                                            </td>
											</tr>
                                        <tr>
                                         <th>上游服务器端口:</th>
										 <td>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="sdnss_port_x_0" value="default" onKeyPress="return is_string(this,event);"/>
											</td>
											 </tr>
                                        <tr>
                                         <th>上游类型</th>
										 <td>
                                          	<select name="sdnss_type_x_0" class="input" style="width: 200px">
													<option value="tcp" <% nvram_match_x("","sdnss_type_x_0", "0","selected"); %>>tcp</option>
													<option value="udp" <% nvram_match_x("","sdnss_type_x_0", "udp","selected"); %>>udp</option>
													<option value="tls" <% nvram_match_x("","sdnss_type_x_0", "tls","selected"); %>>tls</option>
													<option value="https" <% nvram_match_x("","sdnss_type_x_0", "https","selected"); %>>https</option>
												</select>
                                            </td>
											</tr>
                                        <tr>
                                         <th>IP过滤</th>
										 <td>
                                          	<select name="sdnss_ipc_x_0" class="input" style="width: 200px">
													<option value="0" <% nvram_match_x("","sdnss_ipc_x_0", "0","selected"); %>>禁用</option>
													<option value="whitelist" <% nvram_match_x("","sdnss_ipc_x_0", "whitelist","selected"); %>>白名单</option>
													<option value="blacklist" <% nvram_match_x("","sdnss_ipc_x_0", "blacklist","selected"); %>>黑名单</option>
												</select>
                                            </td>
                                            </tr>
											<tr><th colspan="2" style="background-color: #E3E3E3;">指定服务器组可用于单独解析gfwlist,如果不需要配合SS解析gfwlist,可以不填</th></tr>
											 <tr>
											 <th>服务器组(留空为不指定):</th>
										 <td>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="sdnss_named_x_0" value="<% nvram_get_x("", "sdnss_named_x_0"); %>" />
											</td>
											 </tr>
											  <tr>
											 <th>加入ipset(解析gfwlist要用):</th>
										 <td>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="sdnss_ipset_x_0" value="<% nvram_get_x("", "sdnss_ipset_x_0"); %>" />注意IP直接填,如果是域名:例如https://ndns.233py.com/dns-query 只填写ndns.233py.com就可以了.
											</td>
											 </tr>
											 <tr>
											 <th>将服务器从默认组中排除</th>
										 <td>
                                          	<select name="sdnss_non_x_0" class="input" style="width: 200px">
													<option value="0" <% nvram_match_x("","sdnss_non_x_0", "0","selected"); %>>否</option>
													<option value="1" <% nvram_match_x("","sdnss_non_x_0", "1","selected"); %>>是</option>
												</select>
                                            </td>
                                            </tr>
											</tbody>
											</table>
											<table width="100%" align="center" cellpadding="0" cellspacing="0" class="table">
                                        <tr>
                                            <td><center><input name="ManualRULESList2" type="submit" class="btn btn-primary" style="width: 100px" onclick="return markGroupRULES(this, 64, ' Add ');" value="保存上游"/></center></td>										
                                        </tr>
										
										</table>
<table width="100%" align="center" cellpadding="3" cellspacing="0" class="table">
                                        <tr id="row_rules_caption">
										 
                                            <th width="10%">
                                                启用 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="20%">
                                                名称 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="25%">
                                                地址 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="10%">
                                                端口 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="10%">
                                                协议 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="15%">
                                                过滤 <i class="icon-circle-arrow-down"></i>
                                            </th>
                                            <th width="5%">
                                                <center><i class="icon-th-list"></i></center>
                                            </th>
                                        </tr>
                                        <tr id="row_rules_body" >
                                            <td colspan="7" style="border-top: 0 none; padding: 0px;">
                                                <div id="MRULESList_Block"></div>
                                            </td>
                                        </tr>
										</table>
										</div>
										<div id="wnd_sm_cou">
										<table width="100%" cellpadding="2" cellspacing="0" class="table">
										<tr>
											<td colspan="7" >
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script9')"><span>域名地址:</span></a>
												<div id="script9">
													<textarea rows="8" wrap="off" spellcheck="false" class="span12" name="scripts.smartdns_address.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.smartdns_address.conf",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="6" >
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script10')"><span>IP黑名单:</span></a>
												<div id="script10">
													<textarea rows="8" wrap="off" spellcheck="false" class="span12" name="scripts.smartdns_blacklist-ip.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.smartdns_blacklist-ip.conf",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="6" >
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script12')"><span>IP白名单:</span></a>
												<div id="script12">
													<textarea rows="8" wrap="off" spellcheck="false" class="span12" name="scripts.smartdns_whitelist-ip.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.smartdns_whitelist-ip.conf",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr>
											<td colspan="6" >
												<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script11')"><span>自定义设置:</span></a>
												<div id="script11">
													<textarea rows="8" wrap="off" spellcheck="false" class="span12" name="scripts.smartdns_custom.conf" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.smartdns_custom.conf",""); %></textarea>
												</div>
											</td>
										</tr>
										<tr> <th>生成coredump
										</th>
                                            <td>
                                                <div class="main_itoggle">
                                                <div id="sdnse_coredump_on_of">
                                                    <input type="checkbox" id="sdnse_coredump_fake" <% nvram_match_x("", "sdnse_coredump", "1", "value=1 checked"); %><% nvram_match_x("", "sdnse_coredump", "0", "value=0"); %>>
                                                </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="sdnse_coredump" id="sdnse_coredump_1" <% nvram_match_x("", "sdnse_coredump", "1", "checked"); %>><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="sdnse_coredump" id="sdnse_coredump_0" <% nvram_match_x("", "sdnse_coredump", "0", "checked"); %>><#checkbox_No#>
                                                </div>
                                            </td>
											
                                        </tr>
										</table>
										</div>										
                                   <table class="table">									
                                        <tr>
                                            <td colspan="6">
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
