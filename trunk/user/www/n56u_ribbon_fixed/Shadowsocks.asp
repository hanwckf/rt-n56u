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
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap-table.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">
<script type="text/javascript" src="/jquery.js?random=<% uptime(); %>"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap-table.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap-table-zh-CN.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/itoggle.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/validator.js"></script>
<script>

var node_global_max = 0;
<% shadowsocks_status(); %>
<% pdnsd_status(); %>
<% rules_count(); %>
node_global_max = 0;

	var $j = jQuery.noConflict();
	$j(document).ready(function () {
		init_itoggle('ss_enable');
		init_itoggle('switch_enable_x_0');
		init_itoggle('ss_chdns');
		init_itoggle('ss_router_proxy', change_ss_watchcat_display);
		init_itoggle('ss_watchcat');
		init_itoggle('ss_update_chnroute');
		init_itoggle('ss_update_gfwlist');
		init_itoggle('ss_turn');
		init_itoggle('socks5_enable');
		init_itoggle('socks5_aenable');
		init_itoggle('ss_schedule_enable', change_on);
		$j("#tab_ss_cfg, #tab_ss_add, #tab_ss_dlink, #tab_ss_ssl, #tab_ss_cli, #tab_ss_log, #tab_ss_help").click(function () {
			var newHash = $j(this).attr('href').toLowerCase();
			showTab(newHash);
			return false;
		});
		$j("#close_add").click(function () {
			$j("#vpnc_settings").fadeOut(200);
		});
		$j("#btn_add_link").click(function () {
			$j("#vpnc_settings").fadeIn(200);
		});
		$j("#btn_del_link").click(function () {
			del_dlink();
		});
		$j("#btn_ping_link").click(function () {
			ping_dlink();
		});
		$j("#btn_aping_link").click(function () {
			aping_dlink();
		});
	});

function initial(){
	show_banner(2);
	show_menu(13,13,0);
	show_footer();
	fill_ss_status(shadowsocks_status());
	$("chnroute_count").innerHTML = '<#menu5_17_3#>' + chnroute_count() ;
	$("gfwlist_count").innerHTML = '<#menu5_17_3#>' + gfwlist_count() ;
	switch_ss_type();
	showTab(getHash());
	showMRULESList();
	switch_dns();
	var o2 = document.form.lan_con;
	var o3 = document.form.ss_threads;
	var o4 = document.form.china_dns;
	var o5 = document.form.pdnsd_enable;
	var o6 = document.form.socks5_enable;
	var o7 = document.form.tunnel_forward;

	o2.value = '<% nvram_get_x("","lan_con"); %>';
	o3.value = '<% nvram_get_x("","ss_threads"); %>';
	o4.value = '<% nvram_get_x("","china_dns"); %>';
	o5.value = '<% nvram_get_x("","pdnsd_enable"); %>';
	o6.value = '<% nvram_get_x("","socks5_enable"); %>';
	o7.value = '<% nvram_get_x("","tunnel_forward"); %>';
	switch_dns();
	if(ss_schedule_support){
		document.form.ss_date_x_Sun.checked = getDateCheck(document.form.ss_schedule.value, 0);
		document.form.ss_date_x_Mon.checked = getDateCheck(document.form.ss_schedule.value, 1);
		document.form.ss_date_x_Tue.checked = getDateCheck(document.form.ss_schedule.value, 2);
		document.form.ss_date_x_Wed.checked = getDateCheck(document.form.ss_schedule.value, 3);
		document.form.ss_date_x_Thu.checked = getDateCheck(document.form.ss_schedule.value, 4);
		document.form.ss_date_x_Fri.checked = getDateCheck(document.form.ss_schedule.value, 5);
		document.form.ss_date_x_Sat.checked = getDateCheck(document.form.ss_schedule.value, 6);
		document.form.ss_time_x_hour.value = getrebootTimeRange(document.form.ss_schedule.value, 0);
		document.form.ss_time_x_min.value = getrebootTimeRange(document.form.ss_schedule.value, 1);
		document.getElementById('ss_schedule_enable_tr').style.display = "";
		change_on();
		
	}
	else{
		document.getElementById('ss_schedule_enable_tr').style.display = "none";
		document.getElementById('ss_schedule_date_tr').style.display = "none";
		document.getElementById('ss_schedule_time_tr').style.display = "none";
	}
}
function textarea_scripts_enabled(v){
//inputCtrl(document.form['scripts.ss.dom.sh'], v);
//inputCtrl(document.form['scripts.ss.ip.sh'], v);
}

function change_on(){
var v = document.form.ss_schedule_enable_x.value;
        showhide_div('ss_schedule_date_tr', v);
		showhide_div('ss_schedule_time_tr', v);
		if ( v == 1 )
		check_Timefield_checkbox();

}

function validForm(){
	
	if(ss_schedule_support){
		if(!document.form.ss_date_x_Sun.checked && !document.form.ss_date_x_Mon.checked &&
		!document.form.ss_date_x_Tue.checked && !document.form.ss_date_x_Wed.checked &&
		!document.form.ss_date_x_Thu.checked && !document.form.ss_date_x_Fri.checked &&
		!document.form.ss_date_x_Sat.checked && document.form.ss_schedule_enable_x[0].checked)
		{
			alert(Untranslated.filter_lw_date_valid);
			document.form.ss_date_x_Sun.focus();
			return false;
		}
	}

	return true;
}

function switch_ss_type(){
var b = document.form.ssp_type.value;
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
	showhide_div('row_v2_mux', 0);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
	showhide_div('row_ss_plugin', 1);
	showhide_div('row_ss_plugin_opts', 1);
	showhide_div('row_ssp_insecure', 0);
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
	showhide_div('row_v2_mux', 0);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
	showhide_div('row_ss_plugin', 0);
	showhide_div('row_ss_plugin_opts', 0);
	showhide_div('row_ssp_insecure', 0);
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
	showhide_div('row_v2_mux', 0);
	showhide_div('row_tj_tls_host', v);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
	showhide_div('row_ss_plugin', 0);
	showhide_div('row_ss_plugin_opts', 0);
	showhide_div('row_ssp_insecure', 1);
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
	showhide_div('row_v2_mux', v);
	showhide_div('row_tj_tls_host', v);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
	showhide_div('row_ss_plugin', 0);
	showhide_div('row_ss_plugin_opts', 0);
	showhide_div('row_ssp_insecure', 1);
}
if (b=="socks5"){
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
	showhide_div('row_v2_mux', 0);
	showhide_div('row_tj_tls_host', 0);
	showhide_div('row_s5_username', 0);
	showhide_div('row_s5_password', 0);
	showhide_div('row_ss_plugin', 0);
	showhide_div('row_ss_plugin_opts', 0);
	showhide_div('row_ssp_insecure', 0);
	showhide_div('row_v2_http_host', v);
	showhide_div('row_v2_http_path', v);
}
}
function switch_v2_type(){
var b = document.form.v2_transport.value;
if (b=="tcp"){
	var v=0;
	showhide_div('v2_tcp_guise', 1);
	showhide_div('row_v2_type', 1);
	showhide_div('row_v2_http_host', 1);
	showhide_div('row_v2_http_path', 1);
	showhide_div('v2_kcp_guise', v);
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
	showhide_div('v2_tcp_guise', v);
	showhide_div('row_v2_type', k);
	showhide_div('v2_kcp_guise', k);
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
	showhide_div('row_v2_http_host', v);
	showhide_div('row_v2_http_path', v);
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
	showhide_div('row_v2_http_host', v);
	showhide_div('row_v2_http_path', v);
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
	showhide_div('row_v2_http_host', v);
	showhide_div('row_v2_http_path', v);
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
	showhide_div('row_v2_http_host', v);
	showhide_div('row_v2_http_path', v);
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
if(validForm()){
	if(ss_schedule_support){
			updateDateTime();
		}
		}
	showLoading();
	document.form.action_mode.value = " Restart ";
	document.form.current_page.value = "Shadowsocks.asp";
	document.form.next_page.value = "";
	document.form.submit();
}
function submitInternet(v){
	$j.ajax({
		type: "POST",
		url: "/Shadowsocks_action.asp",
		data: {
			connect_action : v,
		},
		dataType: "json",
		success: function(response) {
		
		}
	});
	alert("脚本执行成功...")
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
$j.ajax({
url:'/dbconf?p=ss&v=<% uptime(); %>',
type:'get',
success:function(res){
//显示节点下拉列表 by 花妆男
// 渲染父节点  obj 需要渲染的数据 keyStr key需要去除的字符串
var keyStr="ssconf_basic_json_";
var nodeList = document.getElementById("nodeList"); // 获取节点
var unodeList = document.getElementById("u_nodeList"); // 获取节点
for(var key  in  db_ss){ // 遍历对象
var optionObj = JSON.parse(db_ss[key] ); // 字符串转为对象
//if(optionObj.ping != "failed"){   //过滤ping不通的节点
var text = '[ '+ (optionObj.type ? optionObj.type:"类型获取失败") +' ] ' + (optionObj.alias?optionObj.alias:"名字获取失败"); // 判断下怕获取失败 ，括号是运算的问题
 // 添加 
nodeList.options.add(new Option(text, key.replace(keyStr,''))); // 通过 replacce把不要的字符去掉
unodeList.options.add(new Option(text, key.replace(keyStr,''))); // 通过 replacce把不要的字符去掉

$j('#nodeList>option').sort(function (a, b) {
	var aText = $j(a).val() * 1;
	var bText = $j(b).val() * 1;
	if (aText > bText) return -1;
	if (aText < bText) return 1;
	return 0;
}).appendTo('#nodeList');
$j('#nodeList>option').eq(0).attr("selected", "selected");
//udp列表
$j('#u_nodeList>option').sort(function (a, b) {
	var aText = $j(a).val() * 1;
	var bText = $j(b).val() * 1;
	if (aText > bText) return -1;
	if (aText < bText) return 1;
	return 0;
}).appendTo('#u_nodeList');
$j('#u_nodeList>option').eq(0).attr("selected", "selected");
//$j('#nodeList').selectpicker('val', '<% nvram_get_x("","global_server"); %>'); //主服务器列表默认
//$j('#u_nodeList').selectpicker('val', '<% nvram_get_x("","udp_relay_server"); %>'); //UDP服务器列表默认
document.form.global_server.value = '<% nvram_get_x("","global_server"); %>';
document.form.udp_relay_server.value = '<% nvram_get_x("","udp_relay_server"); %>';
//}
} 

//订阅节点表格
var myss = new Array();
var i = 0;
for(var key  in  db_ss){ // 遍历对象
var dbss = JSON.parse(db_ss[key] )
dbss.ids = key.replace("ssconf_basic_json_",'');
myss[i] = dbss;
i = i+1;
if (myss != null) {
	var node_i = parseInt(key.replace("ssconf_basic_json_",''));
	if (node_i > node_global_max) {
		node_global_max = node_i;
	}
}
}
	$j('#table99').bootstrapTable({
    data: myss, 
    striped: true,
	pageNumber: 1,
	pagination: true,
	sortable: true, 
	sortName : 'ids',
	sortOrder: "desc",
	sidePagination: 'client',
	pageSize: 15,
	pageList: [15, 25, 35, 50],	// 分页显示记录数
	uniqueId: "ids",
    columns: [{
        field: 'delete',
        title: '删除',
		checkbox: true,
		width:'30px'
    },{
        field: 'ids',
        title: '序号',
		width:'30px',
		align: 'center',
		valign: 'middle',
		sortable: true      
    },{
        field: 'type',
        title: '类型',
		align: 'center',
		valign: 'middle',
		width:'10px'
    }, {
        field: 'alias',
		cellStyle: formatTableUnit,
        formatter: paramsMatter,
        title: '别名',
		align: 'center',
		valign: 'middle',
		width:'230px'
    }, {
        field: 'server',
		cellStyle: formatTableUnit,
        formatter: paramsMatter,
        title: '服务器地址',
		align: 'center',
		valign: 'middle',
		width:'200px'
    },{
        field: 'ping',
        title: 'ping',
		align: 'center',
		valign: 'middle',
		width:'50px',
		cellStyle:cellStylesales,
		formatter: actionFormatter2,
		sortable: true
    },{
        field: 'lost',
        title: '丢包',
		align: 'center',
		valign: 'middle',
		width:'50px'
    },{
		field: 'operate',
		title: '操作',
		width: '100px',
		align: 'center',
		valign: 'middle',
		formatter: actionFormatter
	 }]
});

}
})
}

function cellStylesales(value, row, index) {
	var ping = row.ping
	if (typeof (ping) == "undefined") {
		return ""
	} else if (ping < 100) {
		return { css: { background: '#04B404', color: '#000' } };
	} else if (ping < 300) {
		return { css: { background: '#ffeb3b', color: '#000' } };
	} else {
		return { css: { background: '#f44336', color: '#000' } };
	}
}

function actionFormatter2(value, row, index) {
	var ping = row.ping
	var result = "";

	if (typeof (ping) == "undefined") {
		result += "-";
	} else if (ping != "failed") {
		result += ping + "ms";
	} else {
		result += ping
	}
	return result;
}


function actionFormatter(value, row, index) {
	var id = row.ids
	//console.log(row.ids)
	var result = "";
	result += "<a href='javascript:;' onclick=\"EditViewById('" + id + "')\" title='编辑'>编辑 | </a>";
	result += "<a href='javascript:;' onclick=\"del('" + id + "')\" title='删除'>删除</a>";

	return result;
}

//编辑节点
function EditViewById(id){
	alert("此功能待完善...TODO...")
}
//单项删除		
 function del(id){
	var p = "ssconf_basic";
	var ns = {};
	ns[p + "_json_" + id] = "deleting";	
	$j.ajax({
		url: "/applydb.cgi?userm1=del&p=ss",
		type: 'POST',
		contentType: "application/x-www-form-urlencoded",
		dataType: 'text',
		data: $j.param(ns),
		error: function(xhr) {
			console.log("error in posting config of table");
		},
		success: function(response) {
			location.reload();
		}
	});
}
//批量删除
 function del_dlink(){
	var row=$j("#table99").bootstrapTable('getSelections');
	var p = "ssconf_basic";
	var ns = {};
		for(var key  in  row){
	
		ns[p + "_json_" + row[key].ids] = "deleting";
	}
	//console.log(ns)
	
	$j.ajax({
		url: "/applydb.cgi?userm1=del&p=ss",
		type: 'POST',
		contentType: "application/x-www-form-urlencoded",
		dataType: 'text',
		data: $j.param(ns),
		error: function(xhr) {
			console.log("error in posting config of table");
		},
		success: function(response) {
			location.reload();
		}
	});
}

//ping节点
 function ping_dlink(){
	var row=$j("#table99").bootstrapTable('getSelections');
	var p = "ssconf_basic";
	var ns = {};
		for(var key  in  row){
	
		ns[row[key].ids] = "ping";
	}
	$j.ajax({
		url: "/applydb.cgi?useping=1&p=ss",
		type: 'POST',
		contentType: "application/x-www-form-urlencoded",
		dataType: 'text',
		data: $j.param(ns),
		error: function(xhr) {
			console.log("error in posting config of table");
		},
		success: function(response) {
			//location.reload();
			alert("执行脚本成功")
		}
	});
}

//ping全部节点
 function aping_dlink(){
	var ns = {};
	ns[1] = "allping";
	$j.ajax({
		url: "/applydb.cgi?useping=1&p=ss",
		type: 'POST',
		contentType: "application/x-www-form-urlencoded",
		dataType: 'text',
		data: $j.param(ns),
		error: function(xhr) {
			console.log("error in posting config of table");
		},
		success: function(response) {
			//location.reload();
			alert("执行脚本成功")
		}
	});
}
				
function paramsMatter(value, row, index) {
                var span = document.createElement("span");
                span.setAttribute("title", value);
                span.innerHTML = value;
                return span.outerHTML;
            }
//td宽度以及内容超过宽度隐藏
function formatTableUnit(value, row, index) {
                return {
                    css: {
                        "white-space": "nowrap",
                        "text-overflow": "ellipsis",
                        "overflow": "hidden",
                        "max-width": "60px"
                    }
                }
            }   			

//-----------导入链接开始
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
//console.log(ssu.length);
if ((ssu[0] != "ssr" && ssu[0] != "ss" && ssu[0] != "vmess" && ssu[0] != "trojan") || ssu[1] == "") {
	s.innerHTML = "<font color='red'>无效格式</font>";
	return false;
}
var event = document.createEvent("HTMLEvents");
event.initEvent("change", true, true);
if (ssu[0] == "ssr") {
	var sstr = b64decsafe(ssu[1]);
	var ploc = sstr.indexOf("/?");
	document.getElementById('ssp_type').value = "ssr";
	document.getElementById('ssp_type').dispatchEvent(event);
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
	document.getElementById('ssp_server').value = ssm[1];
	document.getElementById('ssp_prot').value = ssm[2];
	document.getElementById('ss_protocol').value = ssm[3];
	document.getElementById('ss_method').value = ssm[4];
	document.getElementById('ss_obfs').value = ssm[5];
	document.getElementById('ss_password').value = b64decsafe(ssm[6]);
	document.getElementById('ss_obfs_param').value = dictvalue(pdict, 'obfsparam');
	document.getElementById('ss_protocol_param').value = dictvalue(pdict, 'protoparam');
	var rem = pdict['remarks'];
	if (typeof (rem) != 'undefined' && rem != '' && rem.length > 0)
		document.getElementById('ssp_name').value = b64decutf8safe(rem);
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
	document.getElementById('ssp_type').value = "ss";
	document.getElementById('ssp_type').dispatchEvent(event);
	var team = sstr.split('@');
	console.log(param);
	var part1 = team[0].split(':');
	var part2 = team[1].split(':');
	document.getElementById('ssp_server').value = part2[0];
	document.getElementById('ssp_prot').value = part2[1];
	document.getElementById('ss_password').value = part1[1];
	document.getElementById('ss_method').value = part1[0];
	if (param != undefined) {
		document.getElementById('ssp_name').value = decodeURI(param);
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
            document.getElementById('ssp_type').value = "trojan";
            document.getElementById('ssp_type').dispatchEvent(event);
            var team = sstr.split('@');
            var password = team[0]
			var serverPart = team[1].split(':');
			var port = serverPart[1].split('?')[0];
			document.getElementById('ssp_server').value = serverPart[0];
			document.getElementById('ssp_prot').value = port;
			document.getElementById('ss_password').value = password;
            if (param != undefined) {
                document.getElementById('ssp_name').value = decodeURI(param);
            }
            s.innerHTML = "<font color='green'>导入Trojan配置信息成功</font>";
            return false;
}else if (ssu[0] == "vmess") {
	var sstr = b64DecodeUnicode(ssu[1]);
	console.log(sstr);
	var ploc = sstr.indexOf("/?");
	document.getElementById('ssp_type').value = "v2ray";
	document.getElementById('ssp_type').dispatchEvent(event);
	var url0, param = "";
	if (ploc > 0) {
		url0 = sstr.substr(0, ploc);
		param = sstr.substr(ploc + 2);
	}
	var ssm = JSON.parse(sstr);
	document.getElementById('ssp_name').value = ssm.ps;
	document.getElementById('ssp_server').value = ssm.add;
	document.getElementById('ssp_prot').value = ssm.port;
	document.getElementById('v2_alter_id').value = ssm.aid;
	document.getElementById('v2_vmess_id').value = ssm.id;
	if (ssm.net == "tcp" ) {
		document.getElementById('v2_tcp_guise').value = ssm.type;
		document.getElementById('v2_http_host').value = ssm.host;
		document.getElementById('v2_http_path').value = ssm.path;
	}else{
		document.getElementById('v2_kcp_guise').value = ssm.type;
	}
	document.getElementById('v2_transport').value = ssm.net;
	document.getElementById('v2_transport').dispatchEvent(event);
	if (ssm.net == "ws" ) {
		document.getElementById('v2_ws_host').value = ssm.host;
		document.getElementById('v2_ws_path').value = ssm.path;
	}
	if (ssm.net == "h2" ) {
		document.getElementById('v2_h2_host').value = ssm.host;
		document.getElementById('v2_h2_path').value = ssm.path;
	}
	if (ssm.tls == "tls" ) {
		document.getElementById('v2_tls').value = 1;
		document.getElementById('v2_tls').checked = true;
		document.getElementById('ssp_insecure').value = 1;
		document.getElementById('ssp_insecure').checked = true;
		document.getElementById('ssp_tls_host').value = ssm.host;
	}
	s.innerHTML = "<font color='green'>导入V2ray配置信息成功</font>";
	return false;
}
}
//-----------导入链接结束
//-----------TLS开关
function ctls(){
document.getElementById('v2_tls').value = 1;
if(document.getElementById('v2_tls').checked){
document.getElementById('v2_tls').value = 1;
}else{
document.getElementById('v2_tls').value = 0;
}
}

function cmux(){
document.getElementById('v2_mux').value = 1;
if(document.getElementById('v2_mux').checked){
document.getElementById('v2_mux').value = 1;
}else{
document.getElementById('v2_mux').value = 0;
}
}

function cais(){
document.getElementById('ssp_insecure').value = 1;
if(document.getElementById('ssp_insecure').checked){
document.getElementById('ssp_insecure').value = 1;
}else{
document.getElementById('ssp_insecure').value = 0;
}
}
//-----------TLS开关

function check_Timefield_checkbox(){
	if( document.form.ss_date_x_Sun.checked == true 
		|| document.form.ss_date_x_Mon.checked == true 
		|| document.form.ss_date_x_Tue.checked == true
		|| document.form.ss_date_x_Wed.checked == true
		|| document.form.ss_date_x_Thu.checked == true
		|| document.form.ss_date_x_Fri.checked == true
		|| document.form.ss_date_x_Sat.checked == true	){
			inputCtrl(document.form.ss_time_x_hour,1);
			inputCtrl(document.form.ss_time_x_min,1);
			document.form.ss_schedule.disabled = false;
	}
	else{
			inputCtrl(document.form.ss_time_x_hour,0);
			inputCtrl(document.form.ss_time_x_min,0);
			document.form.ss_schedule.disabled = true;
			document.getElementById('ss_schedule_time_tr').style.display ="";
	}
}

function getrebootTimeRange(str, pos)
{
	if (pos == 0)
		return str.substring(7,9);
	else if (pos == 1)
		return str.substring(9,11);
}

function setrebootTimeRange(rd, rh, rm)
{
	return(rd.value+rh.value+rm.value);
}

function updateDateTime()
{
	if(document.form.ss_schedule_enable_x[0].checked){
		document.form.ss_schedule_enable.value = "1";
		document.form.ss_schedule.disabled = false;
		document.form.ss_schedule.value = setDateCheck(
		document.form.ss_date_x_Sun,
		document.form.ss_date_x_Mon,
		document.form.ss_date_x_Tue,
		document.form.ss_date_x_Wed,
		document.form.ss_date_x_Thu,
		document.form.ss_date_x_Fri,
		document.form.ss_date_x_Sat);
		document.form.ss_schedule.value = setrebootTimeRange(
		document.form.ss_schedule,
		document.form.ss_time_x_hour,
		document.form.ss_time_x_min);
	}
	else
		document.form.ss_schedule_enable.value = "0";
}
//点击保存节点按钮
function showNodeData(idName,obj){
      var nodeData = document.getElementById(idName);
	  //console.log(nodeData);
      for (var key in obj) {
          var tr = document.createElement("tr");
          var td = document.createElement("td");
          var td2 = document.createElement("td");
          var input = document.createElement("input");
          td.innerText = obj[key];
          tr.appendChild(td);
          input.name = key;
          td2.appendChild(input);
          tr.appendChild(td2);
          nodeData.appendChild(tr);
      }
    }
	 
 function add_ss(){
 var type = document.getElementById("ssp_type").value;
 if (type == "ss"){
 var DataObj = {
      type: document.getElementById("ssp_type").value, 
      alias: document.getElementById("ssp_name").value,
      server: document.getElementById("ssp_server").value,
	  server_port: document.getElementById("ssp_prot").value,
	  password: document.getElementById("ss_password").value,
	  encrypt_method_ss: document.getElementById("ss_method").value,
	  plugin: document.getElementById("ss_plugin").value,
	  plugin_opts: document.getElementById("ss_plugin_opts").value,
	  coustom: "1", 
	  }
	}else if (type == "ssr"){
 var DataObj = {
      type: document.getElementById("ssp_type").value, 
      alias: document.getElementById("ssp_name").value,
      server: document.getElementById("ssp_server").value,
	  server_port: document.getElementById("ssp_prot").value,
	  protocol: document.getElementById("ss_protocol").value,
	  protocol_param: document.getElementById("ss_protocol_param").value,
	  encrypt_method: document.getElementById("ss_method").value,
	  obfs: document.getElementById("ss_obfs").value,
	  obfs_param: document.getElementById("ss_obfs_param").value,
	  password: document.getElementById("ss_password").value,
	  coustom: "1", 
    }
	} else if (type == "v2ray"){
 var DataObj = {
      type: document.getElementById("ssp_type").value, 
      alias: document.getElementById("ssp_name").value,
      server: document.getElementById("ssp_server").value,
	  server_port: document.getElementById("ssp_prot").value,
	  insecure: document.getElementById("ssp_insecure").value,
	  mux: document.getElementById("v2_mux").value,
	  security: document.getElementById("v2_security").value,
	  vmess_id: document.getElementById("v2_vmess_id").value,
	  alter_id: document.getElementById("v2_alter_id").value,
	  transport: document.getElementById("v2_transport").value,
	  tcp_guise: document.getElementById("v2_tcp_guise").value,
	  http_host: document.getElementById("v2_http_host").value,
	  http_path: document.getElementById("v2_http_path").value,
	  tls: document.getElementById("v2_tls").value,
	  tls_host: document.getElementById("ssp_tls_host").value, 
	  coustom: "1", 
 }
 if (document.getElementById("v2_transport").value == "kcp"){
	DataObj.kcp_guise = document.getElementById("v2_kcp_guise").value;
	DataObj.mtu = document.getElementById("v2_mtu").value;
	DataObj.tti = document.getElementById("v2_tti").value;
	DataObj.uplink_capacity = document.getElementById("v2_uplink_capacity").value;
	DataObj.downlink_capacity = document.getElementById("v2_downlink_capacity").value;
	DataObj.read_buffer_size = document.getElementById("v2_read_buffer_size").value;
	DataObj.write_buffer_size = document.getElementById("v2_write_buffer_size").value;
 }else if (document.getElementById("v2_transport").value == "ws"){
	DataObj.ws_host = document.getElementById("v2_ws_host").value;
	DataObj.ws_path = document.getElementById("v2_ws_path").value;
 }else if (document.getElementById("v2_transport").value == "h2"){
	DataObj.h2_host = document.getElementById("v2_h2_host").value;
	DataObj.h2_path = document.getElementById("v2_h2_path").value;
 }else if (document.getElementById("v2_transport").value == "quic"){
	DataObj.quic_guise = document.getElementById("v2_quic_guise").value;
	DataObj.quic_key = document.getElementById("v2_quic_key").value;
	DataObj.quic_security = document.getElementById("v2_quic_security").value;
 } 
	}else if (type == "trojan"){
 var DataObj = {
      type: document.getElementById("ssp_type").value, 
      alias: document.getElementById("ssp_name").value,
      server: document.getElementById("ssp_server").value,
	  server_port: document.getElementById("ssp_prot").value,
	  password: document.getElementById("ss_password").value,
	  insecure: document.getElementById("ssp_insecure").value,
	  tls: document.getElementById("v2_tls").value,
	  tls_host: document.getElementById("ssp_tls_host").value,
	  coustom: "1", 
    }
	}else if (type == "socks5"){
 var DataObj = {
      type: document.getElementById("ssp_type").value, 
      alias: document.getElementById("ssp_name").value,
      server: document.getElementById("ssp_server").value,
	  server_port: document.getElementById("ssp_prot").value,
	  coustom: "1", 
    }
	}
	var post_dbus = JSON.stringify(DataObj)
	node_global_max += 1;
	var p = "ssconf_basic";
	var ns = {};
	ns[p + "_json_" + node_global_max] = post_dbus;
	push_data(ns);
	console.log(DataObj)
}	 
//post数据到后台处理
function push_data(obj) {
	$j.ajax({
		type: "POST",
		url: '/applydb.cgi?p=ss',
		contentType: "application/x-www-form-urlencoded",
		dataType: 'text',
		data: $j.param(obj),
		success: function(response) {
			$j("#vpnc_settings").fadeOut(200);
			location.reload();
		}
	});
}

function showsdlinkList() {
	var key = "ssconf_basic_json_" + document.getElementById("nodeList").value;
	var result = JSON.parse(db_ss[key]);
	document.getElementById("d_type").value = result.type;
}

function showsudlinkList() {
	var key = "ssconf_basic_json_" + document.getElementById("u_nodeList").value;
	var result = JSON.parse(db_ss[key]);
	document.getElementById("ud_type").value = result.type;
}
</script>
<style>
.nav-tabs > li > a {
    padding-right: 6px;
    padding-left: 6px;
}

.contentM_qis {
	position: absolute;
	-webkit-border-radius: 5px;
	-moz-border-radius: 5px;
	border-radius: 5px;
	z-index: 200;
	background-color:#ffffff;
	margin-left: 15px;
	top: 80px;
	width:650px;
	return height:auto;
	box-shadow: 3px 3px 10px #000;
	display:none;
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
<input type="hidden" id="d_type" name="d_type" value="<% nvram_get_x("","d_type"); %>">
<input type="hidden" id="ud_type" name="ud_type" value="<% nvram_get_x("","ud_type"); %>">
<input type="hidden" name="ss_schedule" value="<% nvram_get_x("", "ss_schedule"); %>" disabled>
<input type="hidden" name="ss_schedule_enable" value="<% nvram_get_x("", "ss_schedule_enable"); %>">
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
							<a id="tab_ss_add" href="#add">节点管理</a>
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
	<div class="alert alert-info" style="margin: 10px;">一个兼容Shadowsocks、ShadowsocksR 、Vmess等协议的游戏加速工具。
	<div><span style="color:#E53333;">注意:</span></div>
	<div><span style="color:#E53333;">1.chinadns-ng仅当绕过大陆模式有域名污染时才建议打开来分流防止污染！当然会占用一部分内存。</span></div>
	<div><span style="color:#E53333;">2.服务器确定连上后,网页还是打不开,可尝试切换国外DNS</span></div>
</div>
		<table width="100%" cellpadding="4" cellspacing="0" class="table">
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
			<tr> <th>主服务器:
				</th>
				<td>
					<select name="global_server" id="nodeList" style="width: 200px;" onchange="showsdlinkList()">
					<option value="nil" >停用</option>         
					</select>
				</td>
			</tr>
			<tr> <th>游戏UDP中继服务器:
			</th>
				<td>
					<select name="udp_relay_server" id="u_nodeList" style="width: 200px;" onchange="showsudlinkList()">
						<option value="nil" >停用</option>
						<option value="same" >与主服务相同</option>
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
			<tr> <th width="50%">需要代理的端口</th>
				<td>
					<select name="s_dports" class="input" style="width: 200px;">
						<option value="0" <% nvram_match_x("","s_dports", "0","selected"); %>>所有端口（默认）</option>
						<option value="1" <% nvram_match_x("","s_dports", "1","selected"); %>>仅常用端口(不走P2P流量到代理)</option>
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
			<tr id="row_pdnsd_enable"> <th width="50%">DNS解析方式(仅GFW模式生效)</th>
				<td>
					<select name="pdnsd_enable" id="pdnsd_enable" class="input" style="width: 200px;" onchange="switch_dns()">
						<option value="0" >使用dns2tcp查询</option>
						<option value="1" >使用其它服务器查询</option>
					</select>
				</td>
			</tr>
			<tr> <th>加载chinadns-ng(仅绕过模式生效)</th>
				<td>
					<div class="main_itoggle">
						<div id="ss_chdns_on_of">
							<input type="checkbox" id="ss_chdns_fake" <% nvram_match_x("", "ss_chdns", "1", "value=1 checked"); %><% nvram_match_x("", "ss_chdns", "0", "value=0"); %>>
						</div>
					</div>
					<div style="position: absolute; margin-left: -10000px;">
						<input type="radio" value="1" name="ss_chdns" id="ss_chdns_1" <% nvram_match_x("", "ss_chdns", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="ss_chdns" id="ss_chdns_0" <% nvram_match_x("", "ss_chdns", "0", "checked"); %>><#checkbox_No#>
					</div>
				</td>
			</tr>
			<tr id="row_china_dns" style="display:none;"> <th width="50%">国内DNS(仅chinadns-ng生效)</th>
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
			<tr id="row_tunnel_forward" style="display:none;"> <th width="50%">国外DNS</th>
				<td>
					<select name="tunnel_forward" class="input" style="width: 200px;" >
						<option value="8.8.4.4#53" >Google Public DNS (8.8.4.4)</option>
						<option value="8.8.8.8#53" >Google Public DNS (8.8.8.8)</option>
						<option value="208.67.222.222#53" >OpenDNS (208.67.222.222)</option>
						<option value="208.67.220.220#53" >OpenDNS (208.67.220.220)</option>
						<option value="209.244.0.3#53" >Level 3 Public DNS (209.244.0.3)</option>
						<option value="209.244.0.4#53" >Level 3 Public DNS (209.244.0.4)</option>
						<option value="4.2.2.1#53" >Level 3 Public DNS (4.2.2.1)</option>
						<option value="4.2.2.2#53" >Level 3 Public DNS (4.2.2.2)</option>
						<option value="4.2.2.3#53" >Level 3 Public DNS (4.2.2.3)</option>
						<option value="4.2.2.4#53" >Level 3 Public DNS (4.2.2.4)</option>
						<option value="1.1.1.1#53" >Cloudflare DNS (1.1.1.1)</option>
						<option value="114.114.114.114#53" >Oversea Mode DNS-1 (114.114.114.114)</option>
						<option value="114.114.115.115#53" >Oversea Mode DNS-1 (114.114.115.115)</option>
					</select>
				</td>
			</tr><!--
			<tr id="row_ssp_dns_ip" style="display:none;"> <th width="50%">SmartDNS加载方式:</th>
				<td>
				自动配置<input type="radio" value="2" name="ssp_dns_ip" id="ssp_dns_ip_2" <% nvram_match_x("", "ssp_dns_ip", "2", "checked"); %>>
				手动配置<input type="radio" value="1" name="ssp_dns_ip" id="ssp_dns_ip_1" <% nvram_match_x("", "ssp_dns_ip", "1", "checked"); %>>
				</td>
			</tr>-->

		</table>
		<table class="table">
			<tr>
				<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" style="width: 200px" onclick="applyRule();" value="应用设置"/></center></td>
			</tr>
		</table>
	</div>
<!--节点列表-->
<div id="wnd_ss_add">
<table width="100%" cellpadding="4" cellspacing="0" class="table">
<tr> <th colspan="2" style="background-color: #E3E3E3;">
<select name="ss_list_mode" style="display: none" id="ss_list_mode" class="input" style="width: 100px;" >   
						<option value="a" >全部节点</option>
						<option value="d" >自定义节点</option>
						<option value="c" >订阅节点</option>
					</select>
					<input type="button" id="btn_add_link" class="btn btn-info" value="添加/导入节点">
					<input type="button" id="btn_ping_link" class="btn btn-info" value="ping节点">
					<input type="button" id="btn_aping_link" class="btn btn-info" value="ping全部节点">
					<input type="button" id="btn_del_link" class="btn btn-danger" value="批量删除节点">
					<input type="button" id="btn_connect_1" class="btn btn-danger" value="清空所有节点" onclick="submitInternet('Reset_dlink');">
</th> </tr>

</table>
<table id="table99"></table>

<div id="vpnc_settings"  class="contentM_qis" style="z-index:9999; box-shadow: 3px 3px 10px #000;margin-top: 0px;">
	<table width="100%" cellpadding="4" cellspacing="0" class="table" id="sslist">
	<tr> <th colspan="2" style="background-color: #E3E3E3;">添加/删除/编辑节点</th> </tr>
		<tr> <th width="50%">导入节点信息:</th>
			<td>
				<input type="button" class="btn btn-primary" value="点击输入节点URL" onclick="return import_ssr_url(this, '<%=self.option%>', '<%=self.value%>')" />
				<span id="<%=self.option%>-status"></span></td>
				
			</tr>
			<tr> <th width="50%">服务器节点类型</th>
				<td>
					<select name="ssp_type" id="ssp_type" class="input" style="width: 200px;" onchange="switch_ss_type()">
						<option value="ss" >SS</option>
						<option value="ssr" >SSR</option>
						<option value="trojan" >Trojan</option>
						<option value="v2ray" >V2ray</option>
						<option value="socks5" >SOCKS5</option>
					</select>
				</td>
			</tr>
			
			<tr> <th width="50%">别名:（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_name" id="ssp_name" style="width: 200px" value="" />
				</td>
			</tr>
			<tr> <th width="50%">服务器IP地址</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_server" id="ssp_server" value="" />
				</td>
			</tr>
			<tr> <th width="50%">服务器端口</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_prot" id="ssp_prot" style="width: 200px" value="" />
				</td>
			</tr>
			<tr id="row_ss_password" style="display:none;">  <th width="50%">服务器密码</th>
				<td>
					<input type="password" class="input" size="32" name="ss_password" id="ss_password" value="" />
					<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('ss_password')"><i class="icon-eye-close"></i></button>
				</td>
			</tr>
            <tr id="row_ss_method" style="display:none;">  <th width="50%">加密方式</th>
				<td>
					<select name="ss_method" id="ss_method" class="input" style="width: 200px;">
						<option value="none" >none (ssr only)</option>
						<option value="rc4" >rc4</option>
						<option value="rc4-md5" >rc4-md5</option>
						<option value="aes-128-cfb" >aes-128-cfb</option>
						<option value="aes-192-cfb" >aes-192-cfb</option>
						<option value="aes-256-cfb" >aes-256-cfb</option>
						<option value="aes-128-ctr" >aes-128-ctr</option>
						<option value="aes-192-ctr" >aes-192-ctr</option>
						<option value="aes-256-ctr" >aes-256-ctr</option>
						<option value="camellia-128-cfb" >camellia-128-cfb</option>
						<option value="camellia-192-cfb" >camellia-192-cfb</option>
						<option value="camellia-256-cfb" >camellia-256-cfb</option>
						<option value="bf-cfb" >bf-cfb</option>
						<option value="salsa20" >salsa20</option>
						<option value="chacha20" >chacha20</option>
						<option value="chacha20-ietf" >chacha20-ietf</option>
						<option value="aes-128-gcm" >aes-128-gcm (ss only)</option>
						<option value="aes-192-gcm" >aes-192-gcm (ss only)</option>
						<option value="aes-256-gcm" >aes-256-gcm (ss only)</option>
						<option value="chacha20-ietf-poly1305" >chacha20-ietf-poly1305 (ss only)</option>
						<option value="xchacha20-ietf-poly1305" >xchacha20-ietf-poly1305 (ss only)</option>
					</select>
				</td>
			</tr>
			<tr id="row_ss_protocol" style="display:none;"> <th width="50%">协议</th>
				<td>
					<select name="ss_protocol" id="ss_protocol" class="input" style="width: 200px;">   
						<option value="origin" >origin</option>
						<option value="auth_sha1" >auth_sha1</option>
						<option value="auth_sha1_v2" >auth_sha1_v2</option>
						<option value="auth_sha1_v4" >auth_sha1_v4</option>
						<option value="auth_aes128_md5" >auth_aes128_md5</option>
						<option value="auth_aes128_sha1" >auth_aes128_sha1</option>
						<option value="auth_chain_a" >auth_chain_a</option>
						<option value="auth_chain_b" >auth_chain_b</option>
					</select>
				</td>
			</tr>
			<tr id="row_ss_plugin" style="display:none;"> <th width="50%">插件</th>
				<td>
					<input type="text" class="input" size="15" name="ss_plugin" id="ss_plugin" value="" />
				</td>
			</tr>
			<tr id="row_ss_plugin_opts" style="display:none;"> <th width="50%">插件参数</th>
				<td>
					<input type="text" class="input" size="15" name="ss_plugin_opts" id="ss_plugin_opts" value="" />
				</td>
			</tr>
            <!--SS参数结束--SSR参数开始-->
			<tr id="row_ss_protocol_para" style="display:none;"> <th width="50%">传输协议参数（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ss_protocol_param" id="ss_protocol_param" value="" />
				</td>
			</tr>
			<tr id="row_ss_obfs" style="display:none;"> <th width="50%">混淆</th>
				<td>
					<select name="ss_obfs" id="ss_obfs" class="input" style="width: 200px;">   
						<option value="plain" >plain</option>
						<option value="http_simple" >http_simple</option>
						<option value="http_post" >http_post</option>
						<option value="tls1.2_ticket_auth" >tls1.2_ticket_auth</option>
						<option value="tls1.tls1.2_ticket_fastauth" >tls1.tls1.2_ticket_fastauth</option>
					</select>
				</td>
			</tr>
			<tr id="row_ss_obfs_para" style="display:none;"> <th width="50%">混淆参数（可选）</th>
				<td>
					<input type="text" class="input" size="15" name="ss_obfs_param" id="ss_obfs_param" style="width: 200px" value="" />
				</td>
			</tr>
			<!--SSR参数结束-->			
			</tbody>
			<tr id="row_s5_username" style="display:none;">  <th width="50%">socks5用户名</th>
				<td>
					<input type="password" class="input" size="32" name="s5_username_x_0" value="" />
				</td>
			</tr>
			<tr id="row_s5_password" style="display:none;">  <th width="50%">socks5密码</th>
				<td>
					<input type="password" class="input" size="32" name="s5_password_x_0" id="s5_key" value="" />
					<button style="margin-left: -5px;" class="btn" type="button" onclick="passwordShowHide('s5_key')"><i class="icon-eye-close"></i></button>
				</td>
			</tr>
			<!--V2RAY-->	
			<tr id="row_v2_aid" style="display:none;"> <th width="50%">AlterId</th>
				<td>
					<input type="text" class="input" size="15" name="v2_alter_id" id="v2_alter_id" style="width: 200px" value="" />
				</td>
			</tr>
			<tr id="row_v2_vid" style="display:none;"> <th width="50%">VmessId (UUID)</th>
				<td>
					<input type="text" class="input" size="15" name="v2_vmess_id" id="v2_vmess_id" style="width: 200px" value="<% nvram_get_x("","v2_vid_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_security" style="display:none;"><th width="50%">加密</th>
				<td>
					<select name="v2_security" id="v2_security" class="input" style="width: 200px;">   
						<option value="auto" >AUTO</option>
						<option value="none" >NONE</option>
						<option value="aes-128-gcm" >AES-128-GCM</option>
						<option value="chacha20-poly1305" >CHACHA20-POLY1305</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_net" style="display:none;"> <th width="50%">传输方式</th>
				<td>
					<select name="v2_transport" id="v2_transport" class="input" style="width: 200px;" onchange="switch_v2_type()">   
						<option value="tcp" >TCP</option>
						<option value="kcp" >mKCP</option>
						<option value="ws" >WebSocket</option>
						<option value="h2" >HTTP/2</option>
						<option value="quic" >QUIC</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_type" style="display:none;"> <th width="50%">伪装类型</th>
				<td>
					<select id="v2_tcp_guise" name="v2_tcp_guise" class="input" style="width: 200px;display:none;">   
						<option value="none" >未配置</option>
						<option value="http" >HTTP</option>
					</select>
					<select id="v2_kcp_guise" name="v2_kcp_guise" class="input" style="width: 200px;display:none;"> 
						<option value="none" >未配置</option>												
						<option value="srtp" >VideoCall (SRTP)</option>
						<option value="utp"  >BitTorrent (uTP)</option>
						<option value="wechat-video" >WechatVideo</option>
						<option value="dtls" >DTLS 1.2</option>
						<option value="wireguard" >WireGuard</option>
					</select>
				</td>
			</tr>
			<tr id="row_v2_http_host" style="display:none;"> <th width="50%">HTTP Host</th>
				<td>
					<input type="text" class="input" size="15" name="v2_http_host" id="v2_http_host" style="width: 200px" value="" />
				</td>
			</tr>
			<tr id="row_v2_http_path" style="display:none;"> <th width="50%">HTTP Path</th>
				<td>
					<input type="text" class="input" size="15" name="v2_http_path" id="v2_http_path" style="width: 200px" value="" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_mtu" style="display:none;"> <th width="50%">MTU</th>
				<td>
					<input type="text" class="input" size="15" name="v2_mtu" id="v2_mtu" style="width: 200px" value="1350" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_tti" style="display:none;"> <th width="50%">TTI</th>
				<td>
					<input type="text" class="input" size="15" name="v2_tti" id="v2_tti" style="width: 200px" value="50" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_uplink" style="display:none;"> <th width="50%">Uplink Capacity</th>
				<td>
					<input type="text" class="input" size="15" name="v2_uplink_capacity" id="v2_uplink_capacity" style="width: 200px" value="5" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_downlink" style="display:none;"> <th width="50%">Downlink Capacity</th>
				<td>
					<input type="text" class="input" size="15" name="v2_downlink_capacity" id="v2_downlink_capacity" style="width: 200px" value="20" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_readbu" style="display:none;"> <th width="50%">Read Buffer Size</th>
				<td>
					<input type="text" class="input" size="15" name="v2_read_buffer_size" id="v2_read_buffer_size" style="width: 200px" value="2" />
				</td>
			</tr>
			<tr id="row_v2_mkcp_writebu" style="display:none;"> <th width="50%">Write Buffer Size</th>
				<td>
					<input type="text" class="input" size="15" name="v2_write_buffer_size" id="v2_write_buffer_size" style="width: 200px" value="2" />
				</td>
			</tr>
			<tr id="row_v2_webs_host" style="display:none;"> <th width="50%">WebSocket Host</th>
				<td>
					<input type="text" class="input" size="15" name="v2_ws_host" id="v2_ws_host" style="width: 200px" value="<% nvram_get_x("","v2_webs_host_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_webs_path" style="display:none;"> <th width="50%">WebSocket Path</th>
				<td>
					<input type="text" class="input" size="15" name="v2_ws_path" id="v2_ws_path" style="width: 200px" value="<% nvram_get_x("","v2_webs_path_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_http2_host" style="display:none;"> <th width="50%">HTTP/2 Host</th>
				<td>
					<input type="text" class="input" size="15" name="v2_h2_host" id="v2_h2_host" style="width: 200px" value="<% nvram_get_x("","v2_http2_host_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_v2_http2_path" style="display:none;"> <th width="50%">HTTP/2 Path</th>
				<td>
					<input type="text" class="input" size="15" name="v2_h2_path" id="v2_h2_path" style="width: 200px" value="<% nvram_get_x("","v2_http2_path_x_0"); %>" />
				</td>
			</tr>
			<tr id="row_quic_security" style="display:none;"> <th width="50%">QUIC Security</th>
				<td>
					<select name="v2_quic_security" id="v2_quic_security" class="input" style="width: 200px;">   
						<option value="none" >未配置</option>
						<option value="aes-128-gcm" >aes-128-gcm</option>
						<option value="chacha20-ietf-poly1305" >chacha20-ietf-poly1305</option>
					</select>
				</td>
			</tr>
			<tr id="row_quic_key" style="display:none;"> <th width="50%">QUIC Key</th>
				<td>
					<input type="text" class="input" size="15" name="v2_quic_key" id="v2_quic_key" style="width: 200px" value="" />
				</td>
			</tr>
			<tr id="row_quic_header" style="display:none;"> <th width="50%">QUIC Header</th>
				<td>
					<select name="v2_quic_guise" id="v2_quic_guise" class="input" style="width: 200px;"> 
						<option value="none" >未配置</option>												
						<option value="srtp" >VideoCall (SRTP)</option>
						<option value="utp" >BitTorrent (uTP)</option>
						<option value="wechat-video" >WechatVideo</option>
						<option value="dtls" >DTLS 1.2</option>
						<option value="wireguard" >WireGuard</option>
					</select>
				</td>
			</tr>
			<tr id="row_ssp_insecure" style="display:none;"><th>allowInsecure</th>
				<td>
				<input type="checkbox" name="ssp_insecure" id="ssp_insecure" onclick="cais();" >

				</td>
			</tr>
			<tr id="row_v2_tls" style="display:none;"><th>TLS</th>
				<td>
				<input type="checkbox" name="v2_tls" id="v2_tls" onclick="ctls();" >

				</td>
			</tr>
			<tr id="row_tj_tls_host" style="display:none;"><th>TLS Host</th>
				<td>
					<input type="text" class="input" size="15" name="ssp_tls_host" id="ssp_tls_host" style="width: 200px" value="">
				</td>
			</tr>
			<tr id="row_v2_mux" style="display:none;"><th>MUX</th>
				<td>
				<input type="checkbox" name="v2_mux" id="v2_mux" onclick="cmux();" >
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
	<tr>
		<td style="border: 0 none; padding: 0px;"><center><input name="ManualRULESList2" id="ManualRULESList2" type="button" class="btn btn-primary" onclick="add_ss();" style="width: 219px" value="保存节点"/></center></td>
<td style="border: 0 none; padding: 0px;"><center><input name="button" type="button" class="btn btn-primary" id="close_add" style="width: 219px" value="取消"/></center></td>
	</tr>
</table>
</div>
</div>
<!--添加订阅节点-->
<div id="wnd_ss_dlink" style="display:none">
	<table width="100%" cellpadding="4" cellspacing="0" class="table">
		<tr>
			<th colspan="2" style="background-color: #E3E3E3;">订阅节点:添加完地址请先点击一下保存设置按钮,再点击更新订阅按钮。</th>
		</tr>
		<tr>
			<td colspan="3">
				<i class="icon-hand-right"></i> <a
					href="javascript:spoiler_toggle('script19')"><span>订阅地址(一行一个地址):</span></a>
				<div id="script19">
					<textarea rows="8" wrap="off" spellcheck="false" maxlength="314571" class="span12"
						name="scripts.ss_dlink.sh"
						style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.ss_dlink.sh",""); %></textarea>
				</div>
			</td>
		</tr>
	</table>
	<table width="100%" cellpadding="4" cellspacing="0" class="table">

		<tr id="ss_schedule_enable_tr" width="50%">

			<th width="50%">启用定时更新订阅</th>
			<td>
				<div class="main_itoggle">
					<div id="ss_schedule_enable_on_of">
						<input type="checkbox" id="ss_schedule_enable_fake"
							<% nvram_match_x("", "ss_schedule_enable", "1", "value=1 checked"); %><% nvram_match_x("", "ss_schedule_enable", "0", "value=0"); %>>
					</div>
				</div>

				<div style="position: absolute; margin-left: -10000px;">
					<input type="radio" name="ss_schedule_enable_x" id="ss_schedule_enable_1" class="input" value="1"
						<% nvram_match_x("", "ss_schedule_enable", "1", "checked"); %> />
					<#checkbox_Yes#>
						<input type="radio" name="ss_schedule_enable_x" id="ss_schedule_enable_0" class="input"
							value="0" <% nvram_match_x("", "ss_schedule_enable", "0", "checked"); %> />
						<#checkbox_No#>
				</div>
			</td>
		</tr>
		<tr id="ss_schedule_date_tr">
			<th>自动更新星期</th>
			<td>
				<input type="checkbox" name="ss_date_x_Sun" class="input" onclick="check_Timefield_checkbox();">日
				<input type="checkbox" name="ss_date_x_Mon" class="input" onclick="check_Timefield_checkbox();">一
				<input type="checkbox" name="ss_date_x_Tue" class="input" onclick="check_Timefield_checkbox();">二
				<input type="checkbox" name="ss_date_x_Wed" class="input" onclick="check_Timefield_checkbox();">三
				<input type="checkbox" name="ss_date_x_Thu" class="input" onclick="check_Timefield_checkbox();">四
				<input type="checkbox" name="ss_date_x_Fri" class="input" onclick="check_Timefield_checkbox();">五
				<input type="checkbox" name="ss_date_x_Sat" class="input" onclick="check_Timefield_checkbox();">六
			</td>
		</tr>
		<tr id="ss_schedule_time_tr">
			<th>自动更新时间</th>
			<td>
				<input type="text" maxlength="2" class="input_3_table" style="width: 30px" name="ss_time_x_hour"
					onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 0);"
					autocorrect="off" autocapitalize="off">时:
				<input type="text" maxlength="2" class="input_3_table" style="width: 30px" name="ss_time_x_min"
					onKeyPress="return validator.isNumber(this,event);" onblur="validator.timeRange(this, 1);"
					autocorrect="off" autocapitalize="off">分
			</td>
		</tr>
	</table>
			 <table width="100%" cellpadding="4" cellspacing="0" class="table">
	<tr>
		<td style="border: 0 none; padding: 0px;"><center>
		<input name="button" type="button" class="btn btn-primary" style="width: 100px" onclick="applyRule();" value="保存设置"/> 
		<input type="button" id="btn_connect_2" class="btn btn-info" value="更新订阅" onclick="submitInternet('Update_dlink');">
		</center></td>
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
<!--
<tr> <th width="50%">自定义国内IP更新地址:</th>
	<td>
		<input type="text" class="input" size="15" name="ss_chnroute_url" style="width: 200px"  value="<% nvram_get_x("","ss_chnroute_url"); %>" />
	</td>
</tr>
<tr> <th width="50%">广告过滤地址:</th>
	<td>
		<input type="text" class="input" size="15" name="ss_adblock_url" style="width: 200px"  value="<% nvram_get_x("","ss_adblock_url"); %>" />
	</td>
</tr>-->

<tr> 
	<th colspan="2" style="background-color: #E3E3E3;">SOCKS5代理</th> 
</tr>
<tr> 
	<th>启用SOCKS5代理服务</th>
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
					<select name="socks5_wenable" class="input" style="width: 200px;" >
						<option value="0" <% nvram_match_x("","socks5_wenable", "0","selected"); %>>禁用</option>
						<option value="1" <% nvram_match_x("","socks5_wenable", "1","selected"); %>>WAN IPV4</option>
						<option value="2" <% nvram_match_x("","socks5_wenable", "2","selected"); %>>WAN IPV6</option>
						<option value="3" <% nvram_match_x("","socks5_wenable", "3","selected"); %>>IPV4+IPV6</option>
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
		<tr> <th width="50%">自定义国内IP更新地址:</th>
	<td>
		<input type="text" class="input" size="15" name="ss_chnroute_url" style="width: 200px"  value="<% nvram_get_x("","ss_chnroute_url"); %>" />
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
		<tr> <th colspan="2" style="background-color: #E3E3E3;">chinadns-ng说明:</th> </tr>
		<tr> <th width="100%">绕过大陆模式启用chinadns会加载CDN域名规则来分流常用网站跑国内DNS,加载gfwlist列表来分流到国外DNS</th></tr>
		<tr> <th width="100%">此模式会占用一部分内存资源,内存少的机器请谨慎开启。</th></tr>
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
