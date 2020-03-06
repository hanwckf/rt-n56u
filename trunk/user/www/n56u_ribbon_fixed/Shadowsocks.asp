<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_16#></title>
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
<script type="text/javascript" src="/dlink.js"></script>
<script>
<% shadowsocks_status(); %>
<% pdnsd_status(); %>
<% rules_count(); %>
var m_rules = [<% get_nvram_list("ShadowsocksConf", "SspList"); %>];
var mrules_ifield = 17;
//var drules_ifield = 17;
if(m_rules.length > 0){
	var m_rules_ifield = m_rules[0].length;
	for (var i = 0; i < m_rules.length; i++) {
		m_rules[i][mrules_ifield] = i;
	}
}
/*
if(d_rules.length > 0){
	var d_rules_ifield = d_rules[0].length;
	for (var i = 0; i < d_rules.length; i++) {
		d_rules[i][drules_ifield] = i;
	}
}*/
var $j = jQuery.noConflict();
$j(document).ready(function(){
	init_itoggle('ss_enable');
	init_itoggle('switch_enable_x_0');
	init_itoggle('v2_mkcp_congestion_x_0');
	init_itoggle('v2_tls_x_0');
	init_itoggle('ss_udp');
	init_itoggle('ss_own');
	init_itoggle('ss_router_proxy',change_ss_watchcat_display);
	init_itoggle('ss_watchcat');
	init_itoggle('ss_update_chnroute');
	init_itoggle('ss_update_gfwlist');
	init_itoggle('ss_turn');
	init_itoggle('socks5_enable');
	init_itoggle('socks5_aenable');
	$j("#tab_ss_cfg, #tab_ss_add, #tab_ss_dlink, #tab_ss_ssl, #tab_ss_cli, #tab_ss_log, #tab_ss_help").click(function(){
		var newHash = $j(this).attr('href').toLowerCase();
		showTab(newHash);
		return false;
	});
});
function initial(){
	show_banner(2);
	show_menu(13,13,0);
	show_footer();
	fill_ss_status(shadowsocks_status());
	fill_ss_check_status(shadowsocks_check_status());
	fill_pd_status(pdnsd_status());
	$("chnroute_count").innerHTML = '<#menu5_17_3#>' + chnroute_count() ;
	$("gfwlist_count").innerHTML = '<#menu5_17_3#>' + gfwlist_count() ;
	switch_ss_type();
	showTab(getHash());
	showMRULESList();
	showDRULESList();
	showssList();
	//shows5List();
	switch_dns();
	var o1 = document.form.global_server;
	var o2 = document.form.lan_con;
	var o3 = document.form.ss_threads;
	var o4 = document.form.china_dns;
	var o5 = document.form.pdnsd_enable;
	var o6 = document.form.socks5_enable;
	var o7 = document.form.tunnel_forward;
	var o8 = document.form.backup_server;
	o1.value = '<% nvram_get_x("","global_server"); %>';
	o2.value = '<% nvram_get_x("","lan_con"); %>';
	o3.value = '<% nvram_get_x("","ss_threads"); %>';
	o4.value = '<% nvram_get_x("","china_dns"); %>';
	o5.value = '<% nvram_get_x("","pdnsd_enable"); %>';
	o6.value = '<% nvram_get_x("","socks5_enable"); %>';
	o7.value = '<% nvram_get_x("","tunnel_forward"); %>';
	o8.value = '<% nvram_get_x("","backup_server"); %>';
	showsdlinkList();
	switch_dns();
}
function textarea_scripts_enabled(v){
//inputCtrl(document.form['scripts.ss.dom.sh'], v);
//inputCtrl(document.form['scripts.ss.ip.sh'], v);
}
function fill_pd_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("pdnsd_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}
function switch_ss_type(){
var b = document.form.ssp_type_x_0.value; //0:ss-orig;1:ssr2:v2ray
if (b=="ss"){
	var v=0;
	showhide_div('row_ss_password', 1);
	showhide_div('row_ss_method', 1);
	showhide_div('row_ss_protocol', v);
	showhide_div('row_ss_protocol_para', v);
	showhide_div('row_ss_obfs', v);
	showhide_div('row_ss_obfs_para', v);
	showhide_div('row_v2_aid', v);
	showhide_div('row_v2_vid', v);
	showhide_div('row_v2_security', v);
	showhide_div('row_v2_net', v);
	showhide_div('row_v2_type', v);
	showhide_div('row_v2_tls', v);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
}
if (b=="ssr"){
	var v=1;
	showhide_div('row_ss_protocol', v);
	showhide_div('row_ss_protocol_para', v);
	showhide_div('row_ss_obfs', v);
	showhide_div('row_ss_obfs_para', v);
	showhide_div('row_ss_password', v);
	showhide_div('row_ss_method', v);
	showhide_div('row_v2_aid', 0);
	showhide_div('row_v2_vid', 0);
	showhide_div('row_v2_security', 0);
	showhide_div('row_v2_net', 0);
	showhide_div('row_v2_type', 0);
	showhide_div('row_v2_tls', 0);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
}
if (b=="trojan"){
	var v=1;
	showhide_div('row_ss_protocol', 0);
	showhide_div('row_ss_protocol_para', 0);
	showhide_div('row_ss_obfs', 0);
	showhide_div('row_ss_obfs_para', 0);
	showhide_div('row_ss_password', v);
	showhide_div('row_ss_method', 0);
	showhide_div('row_v2_aid', 0);
	showhide_div('row_v2_vid', 0);
	showhide_div('row_v2_security', 0);
	showhide_div('row_v2_net', 0);
	showhide_div('row_v2_type', 0);
	showhide_div('row_v2_tls', v);
	showhide_div('row_tj_tls_host', v);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
}
if (b=="v2ray"){
	switch_v2_type();
	var v=1;
	showhide_div('row_ss_protocol', 0);
	showhide_div('row_ss_protocol_para', 0);
	showhide_div('row_ss_obfs', 0);
	showhide_div('row_ss_obfs_para', 0);
	showhide_div('row_ss_password', 0);
	showhide_div('row_ss_method', 0);
	showhide_div('row_v2_aid', v);
	showhide_div('row_v2_vid', v);
	showhide_div('row_v2_security', v);
	showhide_div('row_v2_net', v);
	showhide_div('row_v2_type', v);
	showhide_div('row_v2_tls', v);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
}
if (b=="kumasocks"){
	var v=0;
	var v=1;
	showhide_div('row_ss_protocol', 0);
	showhide_div('row_ss_protocol_para', 0);
	showhide_div('row_ss_obfs', 0);
	showhide_div('row_ss_obfs_para', 0);
	showhide_div('row_ss_password', 0);
	showhide_div('row_ss_method', 0);
	showhide_div('row_v2_aid', 0);
	showhide_div('row_v2_vid', 0);
	showhide_div('row_v2_security', 0);
	showhide_div('row_v2_net', 0);
	showhide_div('row_v2_type', 0);
	showhide_div('row_v2_tls', 0);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
}
if (b=="socks5"){
	var v=0;
	var v=1;
	showhide_div('row_ss_protocol', 0);
	showhide_div('row_ss_protocol_para', 0);
	showhide_div('row_ss_obfs', 0);
	showhide_div('row_ss_obfs_para', 0);
	showhide_div('row_ss_password', 0);
	showhide_div('row_s5_username', 1);
	showhide_div('row_s5_password', 1);
	showhide_div('row_ss_method', 0);
	showhide_div('row_v2_aid', 0);
	showhide_div('row_v2_vid', 0);
	showhide_div('row_v2_security', 0);
	showhide_div('row_v2_net', 0);
	showhide_div('row_v2_type', 0);
	showhide_div('row_v2_tls', 0);
	showhide_div('row_tj_tls_host', 0);
}
}
function switch_v2_type(){
var b = document.form.v2_net_x_0.value; //0:ss-orig;1:ssr
if (b=="tcp"){
	var v=0;
	showhide_div('row_v2_type_tcp', 1);
	showhide_div('row_v2_type', 1);
	showhide_div('row_v2_type_mkcp', v);
	showhide_div('row_v2_mkcp_mtu', v);
	showhide_div('row_v2_mkcp_tti', v);
	showhide_div('row_v2_mkcp_uplink', v);
	showhide_div('row_v2_mkcp_downlink', v);
	showhide_div('row_v2_mkcp_readbu', v);
	showhide_div('row_v2_mkcp_writebu', v);
	showhide_div('row_v2_mkcp_congestion', v);
	showhide_div('row_v2_webs_host', v);
	showhide_div('row_v2_webs_path', v);
	showhide_div('row_v2_http2_host', v);
	showhide_div('row_v2_http2_path', v);
	showhide_div('row_quic_security', v);
	showhide_div('row_quic_key', v);
	showhide_div('row_quic_header', v)
}
if (b=="kcp"){
	var v=0;
	var k=1;
	showhide_div('row_v2_type_tcp', v);
	showhide_div('row_v2_type', k);
	showhide_div('row_v2_type_mkcp', k);
	showhide_div('row_v2_mkcp_mtu', k);
	showhide_div('row_v2_mkcp_tti', k);
	showhide_div('row_v2_mkcp_uplink', k);
	showhide_div('row_v2_mkcp_downlink', k);
	showhide_div('row_v2_mkcp_readbu', k);
	showhide_div('row_v2_mkcp_writebu', k);
	showhide_div('row_v2_mkcp_congestion', k);
	showhide_div('row_v2_webs_host', v);
	showhide_div('row_v2_webs_path', v);
	showhide_div('row_v2_http2_host', v);
	showhide_div('row_v2_http2_path', v);
	showhide_div('row_quic_security', v);
	showhide_div('row_quic_key', v);
	showhide_div('row_quic_header', v)
}
if (b=="ws"){
	var v=0;
	var k=1;
	showhide_div('row_v2_type_tcp', v);
	showhide_div('row_v2_type', v);
	showhide_div('row_v2_mkcp_mtu', v);
	showhide_div('row_v2_mkcp_tti', v);
	showhide_div('row_v2_mkcp_uplink', v);
	showhide_div('row_v2_mkcp_downlink', v);
	showhide_div('row_v2_mkcp_readbu', v);
	showhide_div('row_v2_mkcp_writebu', v);
	showhide_div('row_v2_mkcp_congestion', v);
	showhide_div('row_v2_webs_host', k);
	showhide_div('row_v2_webs_path', k);
	showhide_div('row_v2_http2_host', v);
	showhide_div('row_v2_http2_path', v);
	showhide_div('row_quic_security', v);
	showhide_div('row_quic_key', v);
	showhide_div('row_quic_header', v)
}
if (b=="h2"){
	var v=0;
	var k=1;
	showhide_div('row_v2_type_tcp', v);
	showhide_div('row_v2_type', v);
	showhide_div('row_v2_mkcp_mtu', v);
	showhide_div('row_v2_mkcp_tti', v);
	showhide_div('row_v2_mkcp_uplink', v);
	showhide_div('row_v2_mkcp_downlink', v);
	showhide_div('row_v2_mkcp_readbu', v);
	showhide_div('row_v2_mkcp_writebu', v);
	showhide_div('row_v2_mkcp_congestion', v);
	showhide_div('row_v2_webs_host', v);
	showhide_div('row_v2_webs_path', v);
	showhide_div('row_v2_http2_host', k);
	showhide_div('row_v2_http2_path', k);
	showhide_div('row_quic_security', v);
	showhide_div('row_quic_key', v);
	showhide_div('row_quic_header', v)
}
if (b=="quic"){
	var v=0;
	var k=1;
	showhide_div('row_v2_type_tcp', v);
	showhide_div('row_v2_type', v);
	showhide_div('row_v2_mkcp_mtu', v);
	showhide_div('row_v2_mkcp_tti', v);
	showhide_div('row_v2_mkcp_uplink', v);
	showhide_div('row_v2_mkcp_downlink', v);
	showhide_div('row_v2_mkcp_readbu', v);
	showhide_div('row_v2_mkcp_writebu', v);
	showhide_div('row_v2_mkcp_congestion', v);
	showhide_div('row_v2_webs_host', v);
	showhide_div('row_v2_webs_path', v);
	showhide_div('row_v2_http2_host', v);
	showhide_div('row_v2_http2_path', v);
	showhide_div('row_quic_security', k);
	showhide_div('row_quic_key', k);
	showhide_div('row_quic_header', k);
}
}
function switch_dns(){
var b = document.form.pdnsd_enable.value;
if (b=="0"){
	showhide_div('row_china_dns', 1);
	showhide_div('row_tunnel_forward', 1);
	showhide_div('row_ssp_dns_ip', 0);
	showhide_div('row_ssp_dns_port', 0);
	
}
if (b=="1"){
	showhide_div('row_china_dns', 0);
	showhide_div('row_tunnel_forward', 0);
	showhide_div('row_ssp_dns_ip', 1);
	showhide_div('row_ssp_dns_port', 1);
}
if (b=="2"){
	showhide_div('row_china_dns', 0);
	showhide_div('row_tunnel_forward', 0);
	showhide_div('row_ssp_dns_ip', 0);
	showhide_div('row_ssp_dns_port', 0);
}
}
function applyRule(){
	showLoading();
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "Shadowsocks.asp";
	document.form.next_page.value = "";
	document.form.submit();
}
function submitInternet(v){
	showLoading();
	document.Shadowsocks_action.action = "Shadowsocks_action.asp";
	document.Shadowsocks_action.connect_action.value = v;
	document.Shadowsocks_action.submit();
}
function change_ss_watchcat_display(){
	var v = document.form.ss_router_proxy[0].checked;
	showhide_div('ss_wathcat_option', v);
}
function fill_ss_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("ss_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}
function fill_ss_check_status(status_code){
	var stext = "<#Stopped#>";
	if (status_code == 0)
		stext = "主服务器";
	else if (status_code == 1)
		stext = "备用服务器";
	$("ss_check_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'warning' : 'success') + '">' + stext + '</span>';
}
function fill_ss_tunnel_status(status_code){
	var stext = "Unknown";
	if (status_code == 0)
		stext = "<#Stopped#>";
	else if (status_code == 1)
		stext = "<#Running#>";
	$("ss_tunnel_status").innerHTML = '<span class="label label-' + (status_code != 0 ? 'success' : 'warning') + '">' + stext + '</span>';
}
var arrHashes = ["cfg", "add", "dlink", "ssl", "cli", "log", "help"];
function showTab(curHash){
	var obj = $('tab_ss_'+curHash.slice(1));
	if (obj == null || obj.style.display == 'none')
		curHash = '#cfg';
	for(var i = 0; i < arrHashes.length; i++){
		if(curHash == ('#'+arrHashes[i])){
			$j('#tab_ss_'+arrHashes[i]).parents('li').addClass('active');
			$j('#wnd_ss_'+arrHashes[i]).show();
		}else{
			$j('#wnd_ss_'+arrHashes[i]).hide();
			$j('#tab_ss_'+arrHashes[i]).parents('li').removeClass('active');
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
function markGroupRULES(o, c, b) {
	document.form.group_id.value = "SspList";
	if(b == " Add "){
		if (document.form.ssp_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	document.form.current_page.value = "Shadowsocks.asp#add";
	return true;
	
}
function showMRULESList(){

	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_rules.length == 0)
		code +='<tr><td colspan="5" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
		for(var i = 0; i < m_rules.length; i++){
			if(m_rules[i][0] == "v2ray"){
				s_method=m_rules[i][13]
			}else{
				s_method=m_rules[i][5]
			}
			code +='<tr id="rowrl' + i + '">';
			code +='<td width="10%">&nbsp;' + m_rules[i][0] + '</td>';
			code +='<td width="20%">&nbsp;' + m_rules[i][1] + '</td>';
			code +='<td width="30%"><input type="text" maxlength="6" class="input" size="15" name="ssp_name_x_0" style="width: 145px" disabled="disabled" value="' + m_rules[i][2] + '"/></td>';
			code +='<td width="15%">&nbsp;' + m_rules[i][3] + '</td>';
			code +='<td width="20%"><input type="text" maxlength="6" class="input" size="15" name="ssp_method_x_0" style="width: 80px" disabled="disabled" value="' + s_method + '"/></td>';
			code +='<td width="8%">&nbsp;' + m_rules[i][10] + '</td>';
			code +='<center><td width="5%" style="text-align: center;"><input type="checkbox" name="SspList_s" value="' + m_rules[i][mrules_ifield] + '" onClick="changeBgColorrl(this,' + i + ');" id="check' + m_rules[i][mrules_ifield] + '"></td></center>';
			code +='</tr>';
		}
		code += '<tr>';
		code += '<td colspan="6">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupRULES(this, 64, \' Del \');" name="SspList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MRULESList_Block").innerHTML = code;
}

function showDRULESList(){

	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list" ><tbody id="dlist">';
	if(d_rules.length == 0)
		code +='<tr><td colspan="5" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
		for(var i = 0; i < d_rules.length; i++){
			if(d_rules[i][0] == "v2ray"){
				s_method=d_rules[i][6]
			}else{
				s_method=d_rules[i][5]
			}
			code +='<tr name="d_list" id="rowrl' + i + '">';
			code +='<td width="5%">&nbsp;' + d_rules[i][0] + '</td>';
			code +='<td width="20%">&nbsp;' + d_rules[i][1] + '</td>';
			code +='<td width="20%"><input type="text" maxlength="6" class="input" size="15" name="ssp_name_x_0" style="width: 145px" disabled="disabled" value="' + d_rules[i][2] + '"/></td>';
			code +='<td width="10%">&nbsp;' + d_rules[i][3] + '</td>';
			code +='<td width="15%"><input type="text" maxlength="6" class="input" size="15" name="ssp_method_x_0" style="width: 80px" disabled="disabled" value="' + s_method + '"/></td>';
			code +='<td width="15%">&nbsp;待开发</td>';
			code +='<center><td width="5%" style="text-align: center;"><input type="checkbox" name="del_dlink" value="" onClick="changeBgColorrl(this,' + i + ');" id="check' + d_rules[i][mrules_ifield] + '"></td></center>';
			code +='</tr>';
		}
		code += '<tr>';
		code += '<td colspan="6">&nbsp;</td>'
		code += '<td><button type="button" class="btn btn-danger" id="btn_del" onClick="del_dlinks();"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</tbody></table>';
	$("DRULESList_Block").innerHTML = code;
}

function showssList(){
    var list = document.getElementById('ss_list_2').checked;
	if (list == true){
	l_rules = d_rules;
	}else{
	l_rules = m_rules;
	//showhide_div('row_DList_Block', 0)
	}
	var code2 = '<option value="nil" >停用</option>';
	if(l_rules.length == 0)
		code2 +='<option value="non" >暂无可以用服务器</option>';
	else{
		for(var j = 0; j < l_rules.length; j++){
			code2 +='<option value="'+ j +'" >['+ l_rules[j][0] + ']:'+ l_rules[j][1] + '</option>';
		}
	}
	$("ssList_Block").innerHTML = code2;
	$("ssbList_Block").innerHTML = code2;
}
function shows5List(){
	var code2 = '<option value="nil" >停用</option>';
	if(m_rules.length == 0)
		code2 +='<option value="non" >暂无可以用服务器</option>';
	else{
		for(var j = 0; j < m_rules.length; j++){
			code2 +='<option value="'+ j +'" >['+ m_rules[j][0] + ']:'+ m_rules[j][1] + '</option>';
		}
	}
	$("s5List_Block").innerHTML = code2;
}

function showsdlinkList(){
    var list = document.getElementById('ss_list_2').checked;
	if (list == true){
	var code3 = '<table width="100%" cellpadding="4" cellspacing="0" class="table">';
	var vsid = document.getElementById('ssList_Block').value;
		for(var j = 0; j < d_rules.length; j++){
		if (vsid == j){
		if (d_rules[j][0] == "v2ray"){
		code3 +='<tr style="display:none;"><th width="50%">服务器地址:</th><td><input type="hidden" class="input" name="d_server" style="width: 200px" value="'+  d_rules[j][2] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">服务器端口:</th><td><input type="hidden" class="input" name="d_port" style="width: 200px" value="'+  d_rules[j][3] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">AlterId:</th><td><input type="hidden" class="input" name="d_v2_aid" style="width: 200px" value="'+  d_rules[j][4] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">VmessId (UUID):</th><td><input type="hidden" class="input" name="d_v2_uid" style="width: 200px" value="'+  d_rules[j][5] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">加密:</th><td><input type="hidden" class="input" name="d_v2_security" style="width: 200px" value="'+  d_rules[j][6] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">传输方式:</th><td><input type="hidden" class="input" name="d_v2_net" style="width: 200px" value="'+  d_rules[j][7] + '"></td></tr>';
		if (d_rules[j][7] == "tcp"){
		code3 +='<tr style="display:none;"><th width="50%">伪装类型:</th><td><input type="hidden" class="input" name="d_v2_type" style="width: 200px" value="'+  d_rules[j][8] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">HOST:</th><td><input type="hidden" class="input" name="d_v2_host" style="width: 200px" value="'+  d_rules[j][9] + '"></td></tr>';
		}else if (d_rules[j][7] == "kcp"){
		code3 +='<tr style="display:none;"><th width="50%">伪装类型:</th><td><input type="hidden" class="input" name="d_v2_type" style="width: 200px" value="'+  d_rules[j][8] + '"></td></tr>';
		}else if (d_rules[j][7] == "ws" || d_rules[j][7] == "h2"){
		code3 +='<tr style="display:none;"><th width="50%">HOST:</th><td><input type="hidden" class="input" name="d_v2_host" style="width: 200px" value="'+  d_rules[j][8] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">PATH:</th><td><input type="hidden" class="input" name="d_v2_path" style="width: 200px" value="'+  d_rules[j][9] + '"></td></tr>';
		}
		code3 +='<tr style="display:none;"><th width="50%">TLS:</th><td><input type="hidden" class="input" name="d_v2_tls" style="width: 200px" value="'+  d_rules[j][10] + '"></td></tr>';
		}else if (d_rules[j][0] == "ssr"){
		code3 +='<tr style="display:none;"><th width="50%">服务器地址:</th><td><input type="hidden" class="input" name="d_server" style="width: 200px" value="'+  d_rules[j][2] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">服务器端口:</th><td><input type="hidden" class="input" name="d_port" style="width: 200px" value="'+  d_rules[j][3] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">服务器密码:</th><td><input type="hidden" class="input" name="d_ss_password" style="width: 200px" value="'+  d_rules[j][4] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">加密方式:</th><td><input type="hidden" class="input" name="d_ss_method" style="width: 200px" value="'+  d_rules[j][5] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">协议:</th><td><input type="hidden" class="input" name="d_ss_protocol" style="width: 200px" value="'+  d_rules[j][6] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">传输协议参数:</th><td><input type="hidden" class="input" name="d_ss_protoparam" style="width: 200px" value="'+  d_rules[j][7] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">混淆:</th><td><input type="hidden" class="input" name="d_ss_obfs" style="width: 200px" value="'+  d_rules[j][8] + '"></td></tr>';
		code3 +='<tr style="display:none;"><th width="50%">混淆参数:</th><td><input type="hidden" class="input" name="d_ss_obfsparam" style="width: 200px" value="'+  d_rules[j][9] + '"></td></tr>';
		}
		document.form.d_type.value = d_rules[j][0];
		}
		}
		code3 +='</table>';
	$("DList_Block").innerHTML = code3;
	//showhide_div('row_DList_Block', 1)
	}
}
function changeBgColor(obj, num){
	$("row" + num).style.background=(obj.checked)?'#D9EDF7':'whiteSmoke';
}
</script>
<script type="text/javascript">//<![CDATA[
function padright(str, cnt, pad) {
	return str + Array(cnt + 1).join(pad);
}
function b64EncodeUnicode(str) {
	return btoa(encodeURIComponent(str).replace(/%([0-9A-F]{2})/g, function (match, p1) {
		return String.fromCharCode('0x' + p1);
	}));
}
function b64encutf8safe(str) {
	return b64EncodeUnicode(str).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/g, '');
}
function b64DecodeUnicode(str) {
	return decodeURIComponent(Array.prototype.map.call(atob(str), function (c) {
		return '%' + ('00' + c.charCodeAt(0).toString(16)).slice(-2);
	}).join(''));
}
function b64decutf8safe(str) {
	var l;
	str = str.replace(/-/g, "+").replace(/_/g, "/");
	l = str.length;
	l = (4 - l % 4) % 4;
	if (l)
		str = padright(str, l, "=");
	return b64DecodeUnicode(str);
}
function b64encsafe(str) {
	return btoa(str).replace(/\+/g, "-").replace(/\//g, "_").replace(/=+$/g, '')
}
function b64decsafe(str) {
	var l;
	str = str.replace(/-/g, "+").replace(/_/g, "/");
	l = str.length;
	l = (4 - l % 4) % 4;
	if (l)
		str = padright(str, l, "=");
	return atob(str);
}
function dictvalue(d, key) {
	var v = d[key];
	if (typeof (v) == 'undefined' || v == '')
		return '';
	return b64decsafe(v);
}
function import_ssr_url(btn, urlname, sid) {
	var s = document.getElementById(urlname + '-status');
	if (!s)
		return false;
	var ssrurl = prompt("在这里黏贴配置链接 ssr:// | ss:// | vmess:// | trojan://", "");
	if (ssrurl == null || ssrurl == "") {
		s.innerHTML = "<font color='red'>用户取消</font>";
		return false;
	}
	s.innerHTML = "";
//var ssu = ssrurl.match(/ssr:\/\/([A-Za-z0-9_-]+)/i);
var ssu = ssrurl.split('://');
console.log(ssu.length);
if ((ssu[0] != "ssr" && ssu[0] != "ss" && ssu[0] != "vmess" && ssu[0] != "trojan") || ssu[1] == "") {
	s.innerHTML = "<font color='red'>无效格式</font>";
	return false;
}
var event = document.createEvent("HTMLEvents");
event.initEvent("change", true, true);
if (ssu[0] == "ssr") {
	var sstr = b64decsafe(ssu[1]);
	var ploc = sstr.indexOf("/?");
	document.getElementById('ssp_type_x_0').value = "ssr";
	document.getElementById('ssp_type_x_0').dispatchEvent(event);
	var url0, param = "";
	if (ploc > 0) {
		url0 = sstr.substr(0, ploc);
		param = sstr.substr(ploc + 2);
	}
	var ssm = url0.match(/^(.+):([^:]+):([^:]*):([^:]+):([^:]*):([^:]+)/);
	if (!ssm || ssm.length < 7)
		return false;
	var pdict = {};
	if (param.length > 2)
	{
		var a = param.split('&');
		for (var i = 0; i < a.length; i++) {
			var b = a[i].split('=');
			pdict[decodeURIComponent(b[0])] = decodeURIComponent(b[1] || '');
		}
	}
	document.getElementById('ssp_server_x_0').value = ssm[1];
	document.getElementById('ssp_prot_x_0').value = ssm[2];
	document.getElementById('ss_protocol_x_0').value = ssm[3];
	document.getElementById('ss_method_x_0').value = ssm[4];
	document.getElementById('ss_obfs_x_0').value = ssm[5];
	document.getElementById('ss_key').value = b64decsafe(ssm[6]);
	document.getElementById('ss_obfs_param_x_0').value = dictvalue(pdict, 'obfsparam');
	document.getElementById('ss_proto_param_x_0').value = dictvalue(pdict, 'protoparam');
	var rem = pdict['remarks'];
	if (typeof (rem) != 'undefined' && rem != '' && rem.length > 0)
		document.getElementById('ssp_name_x_0').value = b64decutf8safe(rem);
	s.innerHTML = "<font color='green'>导入ShadowsocksR配置信息成功</font>";
	return false;
} else if (ssu[0] == "ss") {
	var ploc = ssu[1].indexOf("#");
	if (ploc > 0) {
		url0 = ssu[1].substr(0, ploc);
		param = ssu[1].substr(ploc + 1);
	} else {
		url0 = ssu[1]
	}
	var sstr = b64decsafe(url0);
	document.getElementById('ssp_type_x_0').value = "ss";
	document.getElementById('ssp_type_x_0').dispatchEvent(event);
	var team = sstr.split('@');
	console.log(param);
	var part1 = team[0].split(':');
	var part2 = team[1].split(':');
	document.getElementById('ssp_server_x_0').value = part2[0];
	document.getElementById('ssp_prot_x_0').value = part2[1];
	document.getElementById('ss_key').value = part1[1];
	document.getElementById('ss_method_x_0').value = part1[0];
	if (param != undefined) {
		document.getElementById('ssp_name_x_0').value = decodeURI(param);
	}
	s.innerHTML = "<font color='green'>导入Shadowsocks配置信息成功</font>";
	return false;
} else if (ssu[0] == "trojan") {
            var ploc = ssu[1].indexOf("#");
            if (ploc > 0) {
                url0 = ssu[1].substr(0, ploc);
                param = ssu[1].substr(ploc + 1);
            } else {
                url0 = ssu[1]
            }
            var sstr = url0;
            document.getElementById('ssp_type_x_0').value = "trojan";
            document.getElementById('ssp_type_x_0').dispatchEvent(event);
            var team = sstr.split('@');
            var password = team[0]
			var serverPart = team[1].split(':');
			var port = serverPart[1].split('?')[0];
			document.getElementById('ssp_server_x_0').value = serverPart[0];
			document.getElementById('ssp_prot_x_0').value = port;
			document.getElementById('ss_key').value = password;
            if (param != undefined) {
                document.getElementById('ssp_name_x_0').value = decodeURI(param);
            }
            s.innerHTML = "<font color='green'>导入Trojan配置信息成功</font>";
            return false;
}else if (ssu[0] == "vmess") {
	var sstr = b64DecodeUnicode(ssu[1]);
	console.log(sstr);
	var ploc = sstr.indexOf("/?");
	document.getElementById('ssp_type_x_0').value = "v2ray";
	document.getElementById('ssp_type_x_0').dispatchEvent(event);
	var url0, param = "";
	if (ploc > 0) {
		url0 = sstr.substr(0, ploc);
		param = sstr.substr(ploc + 2);
	}
	var ssm = JSON.parse(sstr);
	document.getElementById('ssp_name_x_0').value = ssm.ps;
	document.getElementById('ssp_server_x_0').value = ssm.add;
	document.getElementById('ssp_prot_x_0').value = ssm.port;
	document.getElementById('v2_aid_x_0').value = ssm.aid;
	document.getElementById('v2_vid_x_0').value = ssm.id;
	if (ssm.type == "tcp" ) {
		document.getElementById('row_v2_type_tcp').value = ssm.type;
	}else{
		document.getElementById('row_v2_type_mkcp').value = ssm.type;
	}
	document.getElementById('v2_net_x_0').value = ssm.net;
	document.getElementById('v2_net_x_0').dispatchEvent(event);
	if (ssm.net == "ws" ) {
		document.getElementById('v2_webs_host_x_0').value = ssm.host;
		document.getElementById('v2_webs_path_x_0').value = ssm.path;
	}
	if (ssm.net == "h2" ) {
		document.getElementById('v2_http2_host_x_0').value = ssm.host;
		document.getElementById('v2_http2_path_x_0').value = ssm.path;
	}
	if (ssm.tls == "tls" ) {
		document.getElementById('v2_tls_x_0').value = 1;
		document.getElementById('v2_tls_x_0').checked = true;
	}
	s.innerHTML = "<font color='green'>导入V2ray配置信息成功</font>";
	return false;
}
}

function ctls(){
document.getElementById('v2_tls_x_0').value = 1;
if(document.getElementById('v2_tls_x_0').checked){
document.getElementById('v2_tls_x_0').value = 1;
}else{
document.getElementById('v2_tls_x_0').value = 0;
}

}
//THX 花妆男-->
function del_dlinks() {
	var ListNode = document.getElementById('dlist');
	console.log(ListNode);
	var trListNode = ListNode.getElementsByTagName('tr');
	var checkboxList = document.getElementsByName('del_dlink');
	for (var i = 0; i < checkboxList.length; i++) {
		if (checkboxList[i].checked) {
			ListNode.removeChild(trListNode[i]);
			d_rules.splice(i, 1);
			i--
		}
	}

	document.getElementById("dlll").value = 'var d_rules = ' + ' \n ' + JSON.stringify(d_rules);

};
//<-----
//]]></script>
<style>
.nav-tabs > li > a {
    padding-right: 6px;
    padding-left: 6px;
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
<input type="hidden" name="current_page" value="Shadowsocks.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="ShadowsocksConf;">
<input type="hidden" name="group_id" value="SspList">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="ssp_staticnum_x_0" value="<% nvram_get_x("SspList", "ssp_staticnum_x"); %>" readonly="1" />
<input type="hidden" name="d_type" value="<% nvram_get_x("","d_type"); %>">
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
			<h2 class="box_head round_top"><#menu5_16#> - ShadowSocksR Plus+</h2>
			<div class="round_bottom">
				<div>
					<ul class="nav nav-tabs" style="margin-bottom: 10px;">
						<li class="active">
							<a id="tab_ss_cfg" href="#cfg">客户端</a>
						</li>
						<li>
							<a id="tab_ss_add" href="#add">节点设置</a>
							</li>
							<li>
							<a id="tab_ss_dlink" href="#dlink">订阅节点</a>
							</li>
							<li>
								<a id="tab_ss_ssl" href="#ssl">高级设置</a>
							</li>
							<li>
								<a id="tab_ss_cli" href="#cli">规则管理</a>
							</li>
							<li>
								<a id="tab_ss_log" href="#log">运行日志</a>
							</li>
							<li>
								<a id="tab_ss_help" href="#help">帮助文档</a>
							</li>
						</ul>
					</div>
<div class="row-fluid">
	<div id="tabMenu" class="submenuBlock"></div>
	<div id="wnd_ss_cfg">
	<div class="alert alert-info" style="margin: 10px;">一个兼容Shadowsocks、ShadowsocksR 、Vmess等协议的游戏加速工具。</div>
		<table width="100%" cellpadding="4" cellspacing="0" class="table">
			<tr> <th>当前运行服务器</th>
				<td id="ss_check_status"></td>
			</tr></th> </tr>
			<tr> <th>客户端<#running_status#></th>
				<td id="ss_status"></td>
			</tr></th> </tr>
			<tr id="row_pdnsd_run" style="display:none;"> <th>PDNSD<#running_status#></th>
				<td id="pdnsd_status"></td>
			</tr></th> </tr>
			<tr> <th><#InetControl#></th>
				<td >
					<input type="button" id="btn_connect_1" class="btn btn-info" value=<#Connect#> onclick="submitInternet('Reconnect');">
				</td>
			</tr>
			<tr> <th>总开关</th>
				<td>
					<div class="main_itoggle">
						<div id="ss_enable_on_of">
							<input type="checkbox" id="ss_enable_fake" <% nvram_match_x("", "ss_enable", "1", "value=1 checked"); %><% nvram_match_x("", "ss_enable", "0", "value=0"); %>>
						</div>
					</div>
					<div style="position: absolute; margin-left: -10000px;">
						<input type="radio" value="1" name="ss_enable" id="ss_enable_1" <% nvram_match_x("", "ss_enable", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="ss_enable" id="ss_enable_0" <% nvram_match_x("", "ss_enable", "0", "checked"); %>><#checkbox_No#>
					</div>
				</td>
			</tr>
			<tr> <th width="50%">请选择节点列表:</th>
				<td>
				自定义列表&nbsp;&nbsp;<input type="radio" value="0" name="ss_list" id="ss_list_1" onclick="showssList()" <% nvram_match_x("", "ss_list", "0", "checked"); %>>
				订阅列表&nbsp;&nbsp;<input type="radio" value="1" name="ss_list" id="ss_list_2" onclick="showssList()" <% nvram_match_x("", "ss_list", "1", "checked"); %>>
				</td>
			</tr>
			<tr> <th>主服务器</th>
				<td>
					<select name="global_server" id="ssList_Block" class="input" style="width: 200px;" onchange="showsdlinkList()">
					</select>
				</td>
			</tr>

			<tr id="row_DList_Block" style="display:none;">
		<td colspan="2" style="border-top: 0 none; padding: 0px;">
			<div id="DList_Block"></div>
		</td>
	</tr>
			<tr> <th>故障转移服务器</th>
				<td>
					<select name="backup_server" id="ssbList_Block" class="input" style="width: 200px;">
					</select>
				</td>
			</tr>
			<tr> <th width="50%">多线程并发转发</th>
				<td>
					<select name="ss_threads" class="input" style="width: 200px;">
						<option value="0" >自动（CPU线程数）</option>
						<option value="1" >单线程</option>
						<option value="2" >2 线程</option>
						<option value="4" >4 线程</option>
						<option value="8" >8 线程</option>
						<option value="16" >16 线程</option>
						<option value="24" >24 线程</option>
					</select>
				</td>
			</tr>
			<tr> <th width="50%">运行模式</th>
				<td>
					<select name="ss_run_mode" id="ss_run_mode" class="input" style="width: 200px;" >   
						<option value="gfw" <% nvram_match_x("","ss_run_mode", "gfw","selected"); %> >gfw列表模式</option>
						<option value="router" <% nvram_match_x("","ss_run_mode", "router","selected"); %> >绕过大陆IP模式</option>
						<option value="all" <% nvram_match_x("","ss_run_mode", "all","selected"); %> >全局模式</option>
						<option value="oversea" <% nvram_match_x("","ss_run_mode", "oversea","selected"); %> >海外用户回国模式</option>
					</select>
				</td>
			</tr>
			<tr> <th width="50%">内网控制</th>
				<td>
					<select name="lan_con" class="input" style="width: 200px;">
						<option value="0" >全部走代理</option>
						<option value="1" >指定IP走代理</option>
					</select>
				</td>
			</tr>
			<tr id="row_pdnsd_enable"> <th width="50%">DNS解析方式</th>
				<td>
					<select name="pdnsd_enable" id="pdnsd_enable" class="input" style="width: 200px;" onchange="switch_dns()">
						<option value="0" >使用pdnsd查询</option>
						<option value="1" >使用SmartDNS查询</option>
						<option value="2" >使用其它服务器</option>
					</select>
				</td>
			</tr>
			<tr id="row_china_dns" style="display:none;"> <th width="50%">中国DNS服务器</th>
				<td>
					<select name="china_dns" class="input" style="width: 200px;" >
						<option value="223.5.5.5#53" >阿里DNS (223.5.5.5)</option>
						<option value="114.114.114.114#53" >114 DNS (114.114.114.114)</option>
						<option value="117.50.11.11#53" >OneDNS (117.50.11.11)</option>
						<option value="180.76.76.76#53" >百度DNS (180.76.76.76)</option>
						<option value="119.29.29.29#53" >DNSPOD DNS (119.29.29.29)</option>
						<option value="1.2.4.8#53" >cnnic DNS (1.2.4.8)</option>
					</select>
				</td>
			</tr>
			<tr id="row_tunnel_forward" style="display:none;"> <th width="50%">外国DNS服务器</th>
				<td>
					<select name="tunnel_forward" class="input" style="width: 200px;" >
						<option value="8.8.4.4:53" >Google Public DNS (8.8.4.4)</option>
						<option value="8.8.8.8:53" >Google Public DNS (8.8.8.8)</option>
						<option value="208.67.222.222:53" >OpenDNS (208.67.222.222)</option>
						<option value="208.67.220.220:53" >OpenDNS (208.67.220.220)</option>
						<option value="209.244.0.3:53" >Level 3 Public DNS (209.244.0.3)</option>
						<option value="209.244.0.4:53" >Level 3 Public DNS (209.244.0.4)</option>
						<option value="4.2.2.1:53" >Level 3 Public DNS (4.2.2.1)</option>
						<option value="4.2.2.2:53" >Level 3 Public DNS (4.2.2.2)</option>
						<option value="4.2.2.3:53" >Level 3 Public DNS (4.2.2.3)</option>
						<option value="4.2.2.4:53" >Level 3 Public DNS (4.2.2.4)</option>
						<option value="1.1.1.1:53" >Cloudflare DNS (1.1.1.1)</option>
						<option value="114.114.114.114:53" >Oversea Mode DNS-1 (114.114.114.114)</option>
						<option value="114.114.115.115:53" >Oversea Mode DNS-1 (114.114.115.115)</option>
					</select>
				</td>
			</tr>
			<tr id="row_ssp_dns_ip" style="display:none;"> <th width="50%">SmartDNS加载方式:</th>
				<td>
				自动配置<input type="radio" value="2" name="ssp_dns_ip" id="ssp_dns_ip_2" <% nvram_match_x("", "ssp_dns_ip", "2", "checked"); %>>
				手动配置<input type="radio" value="1" name="ssp_dns_ip" id="ssp_dns_ip_1" <% nvram_match_x("", "ssp_dns_ip", "1", "checked"); %>>
				</td>
			</tr>
				<tr> <th><#menu5_16_17#></th>
				<td>
					<div class="main_itoggle">
						<div id="ss_udp_on_of">
							<input type="checkbox" id="ss_udp_fake" <% nvram_match_x("", "ss_udp", "1", "value=1 checked"); %><% nvram_match_x("", "ss_udp", "0", "value=0"); %>>
						</div>
					</div>
					<div style="position: absolute; margin-left: -10000px;">
						<input type="radio" value="1" name="ss_udp" id="ss_udp_1" <% nvram_match_x("", "ss_udp", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="ss_udp" id="ss_udp_0" <% nvram_match_x("", "ss_udp", "0", "checked"); %>><#checkbox_No#>
					</div>
				</td>
			</tr>
			<tr> <th>路由自身走代理</th>
				<td>
					<div class="main_itoggle">
						<div id="ss_own_on_of">
							<input type="checkbox" id="ss_own_fake" <% nvram_match_x("", "ss_own", "1", "value=1 checked"); %><% nvram_match_x("", "ss_own", "0", "value=0"); %>>
						</div>
					</div>
					<div style="position: absolute; margin-left: -10000px;">
						<input type="radio" value="1" name="ss_own" id="ss_own_1" <% nvram_match_x("", "ss_own", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="ss_own" id="ss_own_0" <% nvram_match_x("", "ss_own", "0", "checked"); %>><#checkbox_No#>
					</div>
				</td>
			</tr>

		</table>
		<table class="table">
			<tr>
				<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 200px" onclick="applyRule();" value="应用设置"/></center></td>
			</tr>
		</table>
	</div>
<div id="wnd_ss_add">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
	<tr> <th colspan="2" style="background-color: #E3E3E3;">添加/删除/编辑节点</th> </tr>
		<tr> <th width="50%">导入节点信息:</th>
			<td>
				<input type="button" class="btn btn-primary" value="点击输入节点URL" onclick="return import_ssr_url(this, '<%=self.option%>', '<%=self.value%>')" />
				<span id="<%=self.option%>-status"></span></td>
			</tr>
			<tr> <th width="50%">服务器节点类型</th>
				<td>
					<select name="ssp_type_x_0" id="ssp_type_x_0" class="input" style="width: 200px;" onchange="switch_ss_type()">
						<option value="ss" <% nvram_match_x("","ssp_type_x_0", "ss","selected"); %> >SS</option>
						<option value="ssr" <% nvram_match_x("","ssp_type_x_0", "ssr","selected"); %> >SSR</option>
						<option value="trojan" <% nvram_match_x("","ssp_type_x_0", "trojan","selected"); %> >Trojan</option>
						<option value="v2ray" <% nvram_match_x("","ssp_type_x_0", "v2ray","selected"); %> >V2ray</option>
						<option value="kumasocks" <% nvram_match_x("","ssp_type_x_0", "kumasocks","selected"); %> >kumasocks</option>
						<option value="socks5" <% nvram_match_x("","ssp_type_x_0", "socks5","selected"); %> >socks5</option>
					</select>
				</td>
			</tr>
			<tr> <th width="50%">别名:（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_name_x_0" id="ssp_name_x_0" style="width: 200px" value="<% nvram_get_x("","ssp_name_x_0"); %>" />
				</td>
			</tr>
			<tr> <th width="50%">服务器IP地址</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_server_x_0" id="ssp_server_x_0" value="<% nvram_get_x("","ssp_server_x_0"); %>" />
				</td>
			</tr>
			<tr> <th width="50%">服务器端口</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_prot_x_0" id="ssp_prot_x_0" style="width: 200px" value="<% nvram_get_x("","ssp_prot_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_ss_password" style="display:none;">  <th width="50%">服务器密码</th>
				<td>
					<input type="password" class="input" size="32" name="ss_key_x_0" id="ss_key" value="<% nvram_get_x("","ss_key_x_0"); %>" />
					<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ss_key')"><i class="icon-eye-close"></i></button>
				</td>
			</tr>	
			<tr id="row_s5_username" style="display:none;">  <th width="50%">socks5用户名</th>
				<td>
					<input type="password" class="input" size="32" name="s5_username_x_0" value="<% nvram_get_x("","s5_username_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_s5_password" style="display:none;">  <th width="50%">socks5密码</th>
				<td>
					<input type="password" class="input" size="32" name="s5_password_x_0" id="s5_key" value="<% nvram_get_x("","s5_password_x_0"); %>" />
					<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('s5_key')"><i class="icon-eye-close"></i></button>
				</td>
			</tr>
			<tr id="row_v2_aid" style="display:none;"> <th width="50%">AlterId</th>
				<td>
					<input type="text" class="input" size="15" name="v2_aid_x_0" id="v2_aid_x_0" style="width: 200px" value="<% nvram_get_x("","v2_aid_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_vid" style="display:none;"> <th width="50%">VmessId (UUID)</th>
				<td>
					<input type="text" class="input" size="15" name="v2_vid_x_0" id="v2_vid_x_0" style="width: 200px" value="<% nvram_get_x("","v2_vid_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_security" style="display:none;"><th width="50%">加密</th>
				<td>
					<select name="v2_security_x_0" id="v2_security_x_0" class="input" style="width: 200px;">   
						<option value="auto" <% nvram_match_x("","v2_security_x_0", "auto","selected"); %> >AUTO</option>
						<option value="none" <% nvram_match_x("","v2_security_x_0", "none","selected"); %> >NONE</option>
						<option value="aes-128-gcm" <% nvram_match_x("","v2_security_x_0", "aes-128-gcm","selected"); %> >AES-128-GCM</option>
						<option value="chacha20-poly1305" <% nvram_match_x("","v2_security_x_0", "chacha20-poly1305","selected"); %> >CHACHA20-POLY1305</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_net" style="display:none;"> <th width="50%">传输方式</th>
				<td>
					<select name="v2_net_x_0" id="v2_net_x_0" class="input" style="width: 200px;" onchange="switch_v2_type()">   
						<option value="tcp" <% nvram_match_x("","v2_net_x_0", "tcp","selected"); %> >TCP</option>
						<option value="kcp" <% nvram_match_x("","v2_net_x_0", "kcp","selected"); %> >mKCP</option>
						<option value="ws" <% nvram_match_x("","v2_net_x_0", "ws","selected"); %> >WebSocket</option>
						<option value="h2" <% nvram_match_x("","v2_net_x_0", "h2","selected"); %> >HTTP/2</option>
						<option value="quic" <% nvram_match_x("","v2_net_x_0", "quic","selected"); %> >QUIC</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_type" style="display:none;"> <th width="50%">伪装类型</th>
				<td>
					<select id="row_v2_type_tcp" name="v2_type_tcp_x_0" class="input" style="width: 200px;display:none;">   
						<option value="none" <% nvram_match_x("","v2_type_tcp_x_0", "none","selected"); %> >未配置</option>
						<option value="http" <% nvram_match_x("","v2_type_tcp_x_0", "http","selected"); %> >HTTP</option>
					</select>
					<select id="row_v2_type_mkcp" name="v2_type_mkcp_x_0" class="input" style="width: 200px;display:none;"> 
						<option value="none" <% nvram_match_x("","v2_type_mkcp_x_0", "none","selected"); %> >未配置</option>												
						<option value="srtp" <% nvram_match_x("","v2_type_mkcp_x_0", "srtp","selected"); %> >VideoCall (SRTP)</option>
						<option value="utp" <% nvram_match_x("","v2_type_mkcp_x_0", "utp","selected"); %> >BitTorrent (uTP)</option>
						<option value="wechat-video" <% nvram_match_x("","v2_type_mkcp_x_0", "wechat-video","selected"); %> >WechatVideo</option>
						<option value="dtls" <% nvram_match_x("","v2_type_mkcp_x_0", "dtls","selected"); %> >DTLS 1.2</option>
						<option value="wireguard" <% nvram_match_x("","v2_type_mkcp_x_0", "wireguard","selected"); %> >WireGuard</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_mkcp_mtu" style="display:none;"> <th width="50%">MTU</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_mtu_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_mtu"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_tti" style="display:none;"> <th width="50%">TTI</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_tti_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_tti"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_uplink" style="display:none;"> <th width="50%">Uplink Capacity</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_uplink_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_uplink"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_downlink" style="display:none;"> <th width="50%">Downlink Capacity</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_downlink_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_downlink"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_readbu" style="display:none;"> <th width="50%">Read Buffer Size</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_readbu_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_readbu"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_writebu" style="display:none;"> <th width="50%">Write Buffer Size</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mkcp_writebu_x_0" style="width: 200px" value="<% nvram_get_x("","v2_mkcp_writebu"); %>" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_congestion" style="display:none;"><th>Congestion</th>
				<td>
					<div class="main_itoggle">
						<div id="v2_mkcp_congestion_x_0_on_of">
							<input type="checkbox" id="v2_mkcp_congestion_x_0_fake" <% nvram_match_x("", "v2_mkcp_congestion_x_0", "1", "value=1 checked"); %><% nvram_match_x("", "v2_mkcp_congestion_x_0", "0", "value=0"); %>>
						</div>
					</div>
					<div style="position: absolute; margin-left: -10000px;">
						<input type="radio" value="1" name="v2_mkcp_congestion_x_0" id="v2_tls_1" <% nvram_match_x("", "v2_mkcp_congestion_x_0", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="v2_mkcp_congestion_x_0" id="v2_tls_0" <% nvram_match_x("", "v2_mkcp_congestion_x_0", "0", "checked"); %>><#checkbox_No#>
					</div>
				</td>
			</tr>
			<tr id="row_v2_webs_host" style="display:none;"> <th width="50%">WebSocket Host</th>
				<td>
					<input type="text" class="input" size="15" name="v2_webs_host_x_0" id="v2_webs_host_x_0" style="width: 200px" value="<% nvram_get_x("","v2_webs_host_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_webs_path" style="display:none;"> <th width="50%">WebSocket Path</th>
				<td>
					<input type="text" class="input" size="15" name="v2_webs_path_x_0" id="v2_webs_path_x_0" style="width: 200px" value="<% nvram_get_x("","v2_webs_path_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_http2_host" style="display:none;"> <th width="50%">HTTP/2 Host</th>
				<td>
					<input type="text" class="input" size="15" name="v2_http2_host_x_0" id="v2_http2_host_x_0" style="width: 200px" value="<% nvram_get_x("","v2_http2_host_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_http2_path" style="display:none;"> <th width="50%">HTTP/2 Path</th>
				<td>
					<input type="text" class="input" size="15" name="v2_http2_path_x_0" id="v2_http2_path_x_0" style="width: 200px" value="<% nvram_get_x("","v2_http2_path_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_quic_security" style="display:none;"> <th width="50%">QUIC Security</th>
				<td>
					<select name="v2_quic_security_x_0" class="input" style="width: 200px;">   
						<option value="none" <% nvram_match_x("","v2_quic_security_x_0", "none","selected"); %> >未配置</option>
						<option value="aes-128-gcm" <% nvram_match_x("","v2_quic_security_x_0", "aes-128-gcm","selected"); %> >aes-128-gcm</option>
						<option value="chacha20-ietf-poly1305" <% nvram_match_x("","v2_quic_security_x_0", "chacha20-ietf-poly1305","selected"); %> >chacha20-ietf-poly1305</option>
					</select>
				</td>
			</tr>
			<tr id="row_quic_key" style="display:none;"> <th width="50%">QUIC Key</th>
				<td>
					<input type="text" class="input" size="15" name="v2_quic_key_x_0" style="width: 200px" value="<% nvram_get_x("","v2_quic_key_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_quic_header" style="display:none;"> <th width="50%">Header</th>
				<td>
					<select name="v2_quic_header_x_0" class="input" style="width: 200px;"> 
						<option value="none" <% nvram_match_x("","v2_quic_header_x_0", "none","selected"); %> >未配置</option>												
						<option value="srtp" <% nvram_match_x("","v2_quic_header_x_0", "srtp","selected"); %> >VideoCall (SRTP)</option>
						<option value="utp" <% nvram_match_x("","v2_quic_header_x_0", "utp","selected"); %> >BitTorrent (uTP)</option>
						<option value="wechat-video" <% nvram_match_x("","v2_quic_header_x_0", "wechat-video","selected"); %> >WechatVideo</option>
						<option value="dtls" <% nvram_match_x("","v2_quic_header_x_0", "dtls","selected"); %> >DTLS 1.2</option>
						<option value="wireguard" <% nvram_match_x("","v2_quic_header_x_0", "wireguard","selected"); %> >WireGuard</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_tls" style="display:none;"><th>TLS</th>
				<td>
				<input type="checkbox" name="v2_tls_x_0" id="v2_tls_x_0" onclick="ctls();" <% nvram_match_x("", "v2_tls_x_0", "1", "value=1 checked"); %><% nvram_match_x("", "v2_tls_x_0", "0", "value=0"); %>>

				</td>
			</tr>
			<tr id="row_tj_tls_host" style="display:none;"><th>TLS Host</th>
				<td>
					<input type="text" class="input" size="15" name="tj_tls_host_x_0" style="width: 200px" value="<% nvram_get_x("", "tj_tls_host_x_0"); %>">
				</td>
			</tr>
			<tr id="row_ss_method" style="display:none;">  <th width="50%">加密方式</th>
				<td>
					<select name="ss_method_x_0" id="ss_method_x_0" class="input" style="width: 200px;">
						<option value="none" <% nvram_match_x("","ss_method_x_0", "none","selected"); %> >none (ssr only)</option>
						<option value="rc4" <% nvram_match_x("","ss_method_x_0", "rc4","selected"); %> >rc4</option>
						<option value="rc4-md5" <% nvram_match_x("","ss_method_x_0", "rc4-md5","selected"); %> >rc4-md5</option>
						<option value="aes-128-cfb" <% nvram_match_x("","ss_method_x_0", "aes-128-cfb","selected"); %> >aes-128-cfb</option>
						<option value="aes-192-cfb" <% nvram_match_x("","ss_method_x_0", "aes-192-cfb","selected"); %> >aes-192-cfb</option>
						<option value="aes-256-cfb" <% nvram_match_x("","ss_method_x_0", "aes-256-cfb","selected"); %> >aes-256-cfb</option>
						<option value="aes-128-ctr" <% nvram_match_x("","ss_method_x_0", "aes-128-ctr","selected"); %> >aes-128-ctr</option>
						<option value="aes-192-ctr" <% nvram_match_x("","ss_method_x_0", "aes-192-ctr","selected"); %> >aes-192-ctr</option>
						<option value="aes-256-ctr" <% nvram_match_x("","ss_method_x_0", "aes-256-ctr","selected"); %> >aes-256-ctr</option>
						<option value="camellia-128-cfb" <% nvram_match_x("","ss_method_x_0", "camellia-128-cfb","selected"); %> >camellia-128-cfb</option>
						<option value="camellia-192-cfb" <% nvram_match_x("","ss_method_x_0", "camellia-192-cfb","selected"); %> >camellia-192-cfb</option>
						<option value="camellia-256-cfb" <% nvram_match_x("","ss_method_x_0", "camellia-256-cfb","selected"); %> >camellia-256-cfb</option>
						<option value="bf-cfb" <% nvram_match_x("","ss_method_x_0", "bf-cfb","selected"); %> >bf-cfb</option>
						<option value="salsa20" <% nvram_match_x("","ss_method_x_0", "salsa20","selected"); %> >salsa20</option>
						<option value="chacha20" <% nvram_match_x("","ss_method_x_0", "chacha20","selected"); %> >chacha20</option>
						<option value="chacha20-ietf" <% nvram_match_x("","ss_method_x_0", "chacha20-ietf","selected"); %> >chacha20-ietf</option>
						<option value="aes-128-gcm" <% nvram_match_x("","ss_method_x_0", "es-128-gcm","selected"); %> >aes-128-gcm (ss only)</option>
						<option value="aes-192-gcm" <% nvram_match_x("","ss_method_x_0", "aes-192-gcm","selected"); %> >aes-192-gcm (ss only)</option>
						<option value="aes-256-gcm" <% nvram_match_x("","ss_method_x_0", "aes-256-gcm","selected"); %> >aes-256-gcm (ss only)</option>
						<option value="chacha20-ietf-poly1305" <% nvram_match_x("","ss_method_x_0", "chacha20-ietf-poly1305","selected"); %> >chacha20-ietf-poly1305 (ss only)</option>
						<option value="xchacha20-ietf-poly1305" <% nvram_match_x("","ss_method_x_0", "xchacha20-ietf-poly1305","selected"); %> >xchacha20-ietf-poly1305 (ss only)</option>
					</select>
				</td>
			</tr>
			<tr> <th width="50%"><#menu5_16_21#></th>
				<td>
					<input type="text" class="input" size="15" name="ss_timeout" style="width: 200px" value="<% nvram_get_x("","ss_timeout"); %>" />
				</td>
			</tr>
			<tr id="row_ss_protocol" style="display:none;"> <th width="50%">协议</th>
				<td>
					<select name="ss_protocol_x_0" id="ss_protocol_x_0" class="input" style="width: 200px;">   
						<option value="origin" <% nvram_match_x("","ss_protocol_x_0", "origin","selected"); %> >origin</option>
						<option value="auth_sha1" <% nvram_match_x("","ss_protocol_x_0", "auth_sha1","selected"); %> >auth_sha1</option>
						<option value="auth_sha1_v2" <% nvram_match_x("","ss_protocol_x_0", "auth_sha1_v2","selected"); %> >auth_sha1_v2</option>
						<option value="auth_sha1_v4" <% nvram_match_x("","ss_protocol_x_0", "auth_sha1_v4","selected"); %> >auth_sha1_v4</option>
						<option value="auth_aes128_md5" <% nvram_match_x("","ss_protocol_x_0", "auth_aes128_md5","selected"); %> >auth_aes128_md5</option>
						<option value="auth_aes128_sha1" <% nvram_match_x("","ss_protocol_x_0", "auth_aes128_sha1","selected"); %> >auth_aes128_sha1</option>
						<option value="auth_chain_a" <% nvram_match_x("","ss_protocol_x_0", "auth_chain_a","selected"); %> >auth_chain_a</option>
						<option value="auth_chain_b" <% nvram_match_x("","ss_protocol_x_0", "auth_chain_b","selected"); %> >auth_chain_b</option>
					</select>
				</td>
			</tr>
			<tr id="row_ss_protocol_para" style="display:none;"> <th width="50%">传输协议参数（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ss_proto_param_x_0" id="ss_proto_param_x_0" value="<% nvram_get_x("","ss_proto_param_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_ss_obfs" style="display:none;"> <th width="50%">混淆</th>
				<td>
					<select name="ss_obfs_x_0" id="ss_obfs_x_0" class="input" style="width: 200px;">   
						<option value="plain" <% nvram_match_x("","ss_obfs_x_0", "plain","selected"); %> >plain</option>
						<option value="http_simple" <% nvram_match_x("","ss_obfs_x_0", "http_simple","selected"); %> >http_simple</option>
						<option value="http_post" <% nvram_match_x("","ss_obfs_x_0", "http_post","selected"); %> >http_post</option>
						<option value="tls1.2_ticket_auth" <% nvram_match_x("","ss_obfs_x_0", "tls1.2_ticket_auth","selected"); %> >tls1.2_ticket_auth</option>
					</select>
				</td>
			</tr>
			<tr id="row_ss_obfs_para" style="display:none;"> <th width="50%">混淆参数（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ss_obfs_param_x_0" id="ss_obfs_param_x_0" style="width: 200px" value="<% nvram_get_x("","ss_obfs_param_x_0"); %>" />
				</td>
			</tr>
			<tr> <th width="50%">本地端口</th>
				<td>
					<input type="text" maxlength="6" class="input" size="15" name="ssp_local_port_x_0" style="width: 200px" value="1080">
				</td>
			</tr>
<!--<tr> <th>自动切换</th>
<td>
<div class="main_itoggle">
<div id="switch_enable_x_0_on_of">
<input type="checkbox" id="switch_enable_x_0_fake" <% nvram_match_x("", "switch_enable_x_0", "1", "value=1 checked"); %><% nvram_match_x("", "switch_enable_x_0", "0", "value=0"); %>>
</div>
</div>
<div style="position: absolute; margin-left: -10000px;">
<input type="radio" value="1" name="switch_enable_x_0" id="switch_enable_x_0_1" <% nvram_match_x("", "switch_enable_x_0", "1", "checked"); %>><#checkbox_Yes#>
<input type="radio" value="0" name="switch_enable_x_0" id="switch_enable_x_0_0" <% nvram_match_x("", "switch_enable_x_0", "0", "checked"); %>><#checkbox_No#>
</div>
</td>
</tr>-->
</table>
<table width="100%" align="center" cellpadding="3" cellspacing="0" class="table">
	<tr id="row_rules_caption">
		<th width="10%">
			类型 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="20%">
			别名 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="24%">
			服务器地址 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="17%">
			服务器端口 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="15%">
			加密方式 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="15%">
			本地端口 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="10%">
			<center><i class="icon-th-list"></i></center>
		</th>
	</tr>
	<tr id="row_rules_header">
	</tr>
	<tr id="row_rules_body">
		<td colspan="7" style="border-top: 0 none; padding: 0px;">
			<div id="MRULESList_Block"></div>
		</td>
	</tr>
</table>
<table class="table">
	<tr>
		<td style="border: 0 none; padding: 0px;"><center><input name="ManualRULESList2" type="submit" class="btn btn-primary" style="width: 219px" onclick="return markGroupRULES(this, 64, ' Add ');" value="保存节点"/></center></td>
		<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 219px" onclick="applyRule();" value="应用设置"/></center></td>
	</tr>
</table>
</div>
<div id="wnd_ss_dlink" style="display:none">
<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr> <th colspan="2" style="background-color: #E3E3E3;">订阅节点:添加完地址请先点击一下保存设置按钮,再点击更新订阅按钮。</th> </tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script19')"><span>订阅地址(一行一个地址):</span></a>
				<div id="script19">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_dlink.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_dlink.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		<tr>
				<td>
					排除节点关键字：<input type="text" maxlength="6" class="input" size="15" name="d_keyword_n" style="width: 200px" placeholder="请以'|'为分隔符" value="<% nvram_get_x("","d_keyword_n"); %>">
					匹配节点关键字：<input type="text" maxlength="6" class="input" size="15" name="d_keyword_y" style="width: 200px" placeholder="请以'|'为分隔符" value="<% nvram_get_x("","d_keyword_y"); %>">
				</td>
			</tr>
	<tr>
		<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 100px" onclick="applyRule();" value="保存设置"/> 
		<input type="button" id="btn_connect_1" class="btn btn-info" value="更新订阅" onclick="submitInternet('Update_dlink');">
		<input type="button" id="btn_connect_1" class="btn btn-info" value="清空订阅" onclick="submitInternet('Reset_dlink');"></center></td>
	</tr>
</table>
		<table width="100%" align="center" cellpadding="3" cellspacing="0" class="table">
	<tr id="row_rules_caption">
		<th width="10%">
			类型 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="20%">
			别名 <i class="icon-circle-arrow-down"></i>
		</th>
		<th width="20%"><center>
			地址 <i class="icon-circle-arrow-down"></i></center>
		</th>
		<th width="15%"><center>
			端口 <i class="icon-circle-arrow-down"></i></center>
		</th>
		<th width="15%"><center>
			加密 <i class="icon-circle-arrow-down"></i></center>
		</th>
		<th width="15%"><center>
			延迟 <i class="icon-circle-arrow-down"></i></center>
		</th>
		<th width="10%">
			<center><i class="icon-th-list"></i></center>
		</th>
	</tr>
	<tr id="row_rules_header">
	</tr>
	<tr id="row_rules_body">
		<td colspan="7" style="border-top: 0 none; padding: 0px;">
			<div id="DRULESList_Block"></div>
		</td>
	</tr>
</table>
</div>
<div id="wnd_ss_ssl" style="display:none">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr> <th colspan="2" style="background-color: #E3E3E3;">节点故障自动切换设置</th> </tr>
		<tr> <th>启用进程自动守护</th>
			<td>
				<div class="main_itoggle">
					<div id="ss_watchcat_on_of">
						<input type="checkbox" id="ss_watchcat_fake" <% nvram_match_x("", "ss_watchcat", "1", "value=1 checked"); %><% nvram_match_x("", "ss_watchcat", "0", "value=0"); %>>
					</div>
				</div>
				<div style="position: absolute; margin-left: -10000px;">
					<input type="radio" value="1" name="ss_watchcat" id="ss_watchcat_1" <% nvram_match_x("", "ss_watchcat", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ss_watchcat" id="ss_watchcat_0" <% nvram_match_x("", "ss_watchcat", "0", "checked"); %>><#checkbox_No#>
				</div>
			</td>
		</tr>
<!--  <tr> <th>启用自动切换</th>
<td>
<div class="main_itoggle">
<div id="ss_turn_on_of">
<input type="checkbox" id="ss_turn_fake" <% nvram_match_x("", "ss_turn", "1", "value=1 checked"); %><% nvram_match_x("", "ss_turn", "0", "value=0"); %>>
</div>
</div>
<div style="position: absolute; margin-left: -10000px;">
<input type="radio" value="1" name="ss_turn" id="ss_turn_1" <% nvram_match_x("", "ss_turn", "1", "checked"); %>><#checkbox_Yes#>
<input type="radio" value="0" name="ss_turn" id="ss_turn_0" <% nvram_match_x("", "ss_turn", "0", "checked"); %>><#checkbox_No#>
</div>
</td>
</tr>
-->
<tr> <th width="50%">自动切换检查周期(秒)</th>
	<td>
		<input type="text" class="input" size="15" name="ss_turn_s" style="width: 200px"  value="<% nvram_get_x("","ss_turn_s"); %>" />
	</td>
</tr>
<tr> <th width="50%">切换检查超时时间(秒)</th>
	<td>
		<input type="text" class="input" size="15" name="ss_turn_ss" style="width: 200px" value="<% nvram_get_x("", "ss_turn_ss"); %>">
	</td>
</tr>
<tr> <th colspan="2" style="background-color: #E3E3E3;">SOCKS5代理</th> </tr>
<tr> <th>启用SOCKS5代理服务</th>
<td>
<div class="main_itoggle">
<div id="socks5_enable_on_of">
<input type="checkbox" id="socks5_enable_fake" <% nvram_match_x("", "socks5_enable", "1", "value=1 checked"); %><% nvram_match_x("", "socks5_enable", "0", "value=0"); %>>
</div>
</div>
<div style="position: absolute; margin-left: -10000px;">
<input type="radio" value="1" name="socks5_enable" id="socks5_enable_1" <% nvram_match_x("", "socks5_enable", "1", "checked"); %>><#checkbox_Yes#>
<input type="radio" value="0" name="socks5_enable" id="socks5_enable_0" <% nvram_match_x("", "socks5_enable", "0", "checked"); %>><#checkbox_No#>
</div>
</td>
</tr>
<tr> <th width="50%">代理端口:</th>
	<td>
		<input type="text" class="input" size="15" name="socks5_port" style="width: 200px" value="<% nvram_get_x("", "socks5_port"); %>">
	</td>
</tr>
<tr id="row_socks5" > <th width="50%">外网访问设置</th>
				<td>
					<select name="socks5_w_enable" class="input" style="width: 200px;" >
						<option value="0" >禁用</option>
						<option value="1" >WAN IPV4</option>
						<option value="2" >WAN IPV6</option>
						<option value="3" >IPV4+IPV6</option>
					</select>
				</td>
			</tr>
<tr> <th>启用 用户名/密码 认证</th>
<td>
<div class="main_itoggle">
<div id="socks5_aenable_on_of">
<input type="checkbox" id="socks5_aenable_fake" <% nvram_match_x("", "socks5_aenable", "1", "value=1 checked"); %><% nvram_match_x("", "socks5_aenable", "0", "value=0"); %>>
</div>
</div>
<div style="position: absolute; margin-left: -10000px;">
<input type="radio" value="1" name="socks5_aenable" id="socks5_aenable_1" <% nvram_match_x("", "socks5_aenable", "1", "checked"); %>><#checkbox_Yes#>
<input type="radio" value="0" name="socks5_aenable" id="socks5_aenable_0" <% nvram_match_x("", "socks5_aenable", "0", "checked"); %>><#checkbox_No#>
</div>
</td>
</tr>
<tr> <th width="50%">用户名:</th>
	<td>
		<input type="text" class="input" size="15" name="socks5_s_username" style="width: 200px" value="<% nvram_get_x("", "socks5_s_username"); %>">
	</td>
</tr>
<tr> <th width="50%">密码:</th>
	<td>
		<input type="text" class="input" size="15" name="socks5_s_password" style="width: 200px" value="<% nvram_get_x("", "socks5_s_password"); %>">
	</td>
</tr>
</table>
<table class="table">
	<tr>
		<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 200px" onclick="applyRule();" value="应用设置"/></center></td>
	</tr>
</table>
</div>
<div id="wnd_ss_cli" style="display:none">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr> <th colspan="2" style="background-color: #E3E3E3;">Chnroute</th> </tr>
		<tr>
			<th width="50%"><#menu5_17_1#>&nbsp;&nbsp;&nbsp;&nbsp;<span class="label label-info" style="padding: 5px 5px 5px 5px;" id="chnroute_count"></span></th>
			<td style="border-top: 0 none;" colspan="2">
				<input type="button" id="btn_connect_3" class="btn btn-info" value=<#menu5_17_2#> onclick="submitInternet('Update_chnroute');">
			</td>
		</tr>
		<tr> <th><#menu5_16_19#></th>
			<td>
				<div class="main_itoggle">
					<div id="ss_update_chnroute_on_of">
						<input type="checkbox" id="ss_update_chnroute_fake" <% nvram_match_x("", "ss_update_chnroute", "1", "value=1 checked"); %><% nvram_match_x("", "ss_update_chnroute", "0", "value=0"); %>>
					</div>
				</div>
				<div style="position: absolute; margin-left: -10000px;">
					<input type="radio" value="1" name="ss_update_chnroute" id="ss_update_chnroute_1" <% nvram_match_x("", "ss_update_chnroute", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ss_update_chnroute" id="ss_update_chnroute_0" <% nvram_match_x("", "ss_update_chnroute", "0", "checked"); %>><#checkbox_No#>
				</div>
			</td>
		</tr>
		<tr> <th colspan="2" style="background-color: #E3E3E3;">gfwlist</th> </tr>
		<tr>
			<th width="50%"><#menu5_17_1#>&nbsp;&nbsp;&nbsp;&nbsp;<span class="label label-info" style="padding: 5px 5px 5px 5px;" id="gfwlist_count"></span></th>
			<td style="border-top: 0 none;" colspan="2">
				<input type="button" id="btn_connect_4" class="btn btn-info" value=<#menu5_17_2#> onclick="submitInternet('Update_gfwlist');">
			</td>
		</tr>
		<tr> <th><#menu5_16_19#></th>
			<td>
				<div class="main_itoggle">
					<div id="ss_update_gfwlist_on_of">
						<input type="checkbox" id="ss_update_gfwlist_fake" <% nvram_match_x("", "ss_update_gfwlist", "1", "value=1 checked"); %><% nvram_match_x("", "ss_update_gfwlist", "0", "value=0"); %>>
					</div>
				</div>
				<div style="position: absolute; margin-left: -10000px;">
					<input type="radio" value="1" name="ss_update_gfwlist" id="ss_update_gfwlist_1" <% nvram_match_x("", "ss_update_gfwlist", "1", "checked"); %>><#checkbox_Yes#>
					<input type="radio" value="0" name="ss_update_gfwlist" id="ss_update_gfwlist_0" <% nvram_match_x("", "ss_update_gfwlist", "0", "checked"); %>><#checkbox_No#>
				</div>
			</td>
		</tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script8')"><span>不走SS代理的LAN IP:</span></a>
				<div id="script8">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_lan_ip.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_lan_ip.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script9')"><span>强制走SS代理的LAN IP:</span></a>
				<div id="script9">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_lan_bip.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_lan_bip.sh",""); %></textarea>
				</div>
			</td>
		</tr>				
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script11')"><span>强制走SS代理的WAN IP:</span></a>
				<div id="script11">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_ip.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_ip.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script12')"><span>不走SS代理的WAN IP:</span></a>
				<div id="script12">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_wan_ip.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_wan_ip.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script10')"><span>强制走SS代理的域名:</span></a>
				<div id="script10">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.ss_dom.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_dom.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		<tr>
			<td colspan="3" >
				<i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script15')"><span>不走SS代理的域名:</span></a>
				<div id="script15">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12" name="scripts.uss_dom.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.uss_dom.sh",""); %></textarea>
				</div>
			</td>
		</tr>
		
		<tr style="display:none">
			<td colspan="3" >
				<div id="script15">
					<textarea rows="8" warp="virtual" spellcheck="false" maxlength="314571" class="span12" id="dlll" name="scripts.dlink.js" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.dlink.js",""); %></textarea>
				</div>
			</td>
		</tr>
		
		<tr>
			<td colspan="2">
				<center><input class="btn btn-primary" style="width: 219px" type="button" value="<#CTL_apply#>" onclick="applyRule()" /></center>
			</td>
		</tr>
	</table>
</div>
<div id="wnd_ss_log" style="display:none">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr>
			<td colspan="3" style="border-top: 0 none; padding-bottom: 0px;">
				<textarea rows="21" class="span12" style="height:377px; font-family:'Courier New', Courier, mono; font-size:13px;" readonly="readonly" wrap="off" id="textarea"><% nvram_dump("ssrplus.log",""); %></textarea>
			</td>
		</tr>
		<tr>
			<td width="15%" style="text-align: left; padding-bottom: 0px;">
				<input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="btn btn-primary" style="width: 200px">
			</td>
		</tr>
	</table>
</div>
<div id="wnd_ss_help" style="display:none">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr> <th colspan="2" style="background-color: #E3E3E3;">PDNSD说明:</th> </tr>
		<tr> <th width="100%">绕过大陆模式启用pdnsd会自动检测并关闭当前正在运行的smartdns,因绕过大陆模式中,pdnsd需要把DNS转发到自身，所以两者不能共存。</th></tr>
		<tr> <th width="100%">绕过大陆模式启用pdnsd会加载CDN域名规则来分流常用网站跑国内DNS，</th></tr>
		<tr> <th colspan="2" style="background-color: #E3E3E3;">SmartDNS说明:</th> </tr>
		<tr> <th width="100%">自动配置:此选项将加载预设的配置文件启动smartdns,不用你手动配置,适合小白,当然配置不一定会很适合你自身.</th></tr>
		<tr> <th width="100%">手动配置:启动SS后需要自行到smartdns页面配置相关上游,端口之类，适合老手</br>
		gfwlist模式下建议:启用第二服务器,端口5353,服务器组gfwlist,上游服务器添加服务器组,并从默认组排除。</br>
		绕过模式下建议:国内DNS加载白名单。
		</th></tr>

		<tr> <th colspan="2" style="background-color: #E3E3E3;">Socks5代理说明:</th> </tr>
		<tr> <th width="100%">WAN IPV4:允许外网IPV4访问socks5</th></tr>
		<tr> <th width="100%">WAN IPV6:允许外网IPV6访问socks5</th></tr>
		<tr> <th width="100%">IPV4+IPV6:运行外网IPV4+IPV6访问socks5</th></tr>
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
<form method="post" name="Shadowsocks_action" action="">
	<input type="hidden" name="connect_action" value="">
</form>
</body>
</html>
