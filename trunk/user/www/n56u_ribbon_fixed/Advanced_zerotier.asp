<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_32#></title>
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

	init_itoggle('zerotier_enable');
	init_itoggle('zerotier_nat');

});

</script>
<script>

<% login_state_hook(); %>

var m_list = [<% get_nvram_list("ZeroConf", "ZeroList"); %>];
var mlist_ifield = 4;
if(m_list.length > 0){
	var m_list_ifield = m_list[0].length;
	for (var i = 0; i < m_list.length; i++) {
		m_list[i][mlist_ifield] = i;
	}
}
var isMenuopen = 0;
function initial(){
	show_banner(2);
	show_menu(5,17,0);
showmenu();
showMRULESList();
	show_footer();

}
function showmenu(){
showhide_div('allink', found_app_aliddns());
}
function applyRule(){
//	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_zerotier.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
//	}
}

function done_validating(action){
	refreshpage();
}

function markGroupRULES(o, c, b) {
	document.form.group_id.value = "ZeroList";
	if(b == " Add "){
		if (document.form.zero_staticnum_x_0.value >= c){
			alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
			return false;
		}else if (document.form.zero_ip_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.zero_ip_x_0.focus();
			document.form.zero_ip_x_0.select();
			return false;
		}else if(document.form.zero_route_x_0.value==""){
			alert("<#JS_fieldblank#>");
			document.form.zero_route_x_0.focus();
			document.form.zero_route_x_0.select();
			return false;
		}else{
			for(i=0; i<m_list.length; i++){
				if(document.form.zero_ip_x_0.value==m_list[i][0]) {
				if(document.form.zero_route_x_0.value==m_list[i][1]) {
					alert('<#JS_duplicate#>' + ' (' + m_list[i][1] + ')' );
					document.form.zero_ip_x_0.focus();
					document.form.zero_ip_x_0.select();
					return false;
					}
				}
			}
		}
	}
	pageChanged = 0;
	document.form.action_mode.value = b;
	return true;
}

function showMRULESList(){
	var code = '<table width="100%" cellspacing="0" cellpadding="3" class="table table-list">';
	if(m_list.length == 0)
		code +='<tr><td colspan="4" style="text-align: center;"><div class="alert alert-info"><#IPConnection_VSList_Norule#></div></td></tr>';
	else{
	    for(var i = 0; i < m_list.length; i++){
		if(m_list[i][0] == 0)
		zero_enable="已禁用";
		else{
		zero_enable="已启用";
		}
		code +='<tr id="rowrl' + i + '">';
		code +='<td width="20%">&nbsp;' + zero_enable + '</td>';
		code +='<td width="40%">&nbsp;' + m_list[i][1] + '</td>';
		code +='<td width="40%">' + m_list[i][2] + '</td>';
		code +='<td width="50%"></td>';
		code +='<center><td width="20%" style="text-align: center;"><input type="checkbox" name="ZeroList_s" value="' + m_list[i][mlist_ifield] + '" onClick="changeBgColorrl(this,' + i + ');" id="check' + m_list[i][mlist_ifield] + '"></td></center>';
		
		code +='</tr>';
	    }
		code += '<tr>';
		code += '<td colspan="4">&nbsp;</td>'
		code += '<td><button class="btn btn-danger" type="submit" onclick="markGroupRULES(this, 64, \' Del \');" name="ZeroList"><i class="icon icon-minus icon-white"></i></button></td>';
		code += '</tr>'
	}
	code +='</table>';
	$("MRULESList_Block").innerHTML = code;
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

	<input type="hidden" name="current_page" value="Advanced_zerotier.asp">
	<input type="hidden" name="next_page" value="">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="ZeroConf;">
	<input type="hidden" name="group_id" value="ZeroList">
	<input type="hidden" name="action_mode" value="">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="zero_staticnum_x_0" value="<% nvram_get_x("ZeroList", "zero_staticnum_x"); %>" readonly="1" />

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
							<h2 class="box_head round_top"><#menu5_32#> - <#menu5_30#></h2>
							<div class="round_bottom">
							<div>
                            <ul class="nav nav-tabs" style="margin-bottom: 10px;">
								<li id="allink" style="display:none">
                                    <a href="Advanced_aliddns.asp"><#menu5_23_1#></a>
                                </li>
								<li class="active">
                                    <a href="Advanced_zerotier.asp"><#menu5_32_1#></a>
                                </li>
                            </ul>
                        </div>
								<div class="row-fluid">
									<div id="tabMenu" class="submenuBlock"></div>
									<div class="alert alert-info" style="margin: 10px;">
									<p>Zerotier是一个开源，跨平台，而且适合内网穿透互联的傻瓜配置虚拟 VPN LAN<br>
									</p>
									</div>

									<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
										<tr>
											<th width="30%" style="border-top: 0 none;">启用ZeroTier</th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="zerotier_enable_on_of">
														<input type="checkbox" id="zerotier_enable_fake" <% nvram_match_x("", "zerotier_enable", "1", "value=1 checked"); %><% nvram_match_x("", "zerotier_enable", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="zerotier_enable" id="zerotier_enable_1" class="input" value="1" <% nvram_match_x("", "zerotier_enable", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="zerotier_enable" id="zerotier_enable_0" class="input" value="0" <% nvram_match_x("", "zerotier_enable", "0", "checked"); %> /><#checkbox_No#>
												</div>
											</td>

										</tr>
										</tr>
<tr><th>ZeroTier Network ID</th>
				<td>
					<input type="text" class="input" name="zerotier_id" id="zerotier_id" style="width: 200px" value="<% nvram_get_x("","zerotier_id"); %>" />
				</td>
			</tr>
			<tr>
											<th width="30%" style="border-top: 0 none;">自动允许客户端NAT</th>
											<td style="border-top: 0 none;">
													<div class="main_itoggle">
													<div id="zerotier_nat_on_of">
														<input type="checkbox" id="zerotier_nat_fake" <% nvram_match_x("", "zerotier_nat", "1", "value=1 checked"); %><% nvram_match_x("", "zerotier_nat", "0", "value=0"); %>  />
													</div>
												</div>
												<div style="position: absolute; margin-left: -10000px;">
													<input type="radio" value="1" name="zerotier_nat" id="zerotier_nat_1" class="input" value="1" <% nvram_match_x("", "zerotier_nat", "1", "checked"); %> /><#checkbox_Yes#>
													<input type="radio" value="0" name="zerotier_nat" id="zerotier_nat_0" class="input" value="0" <% nvram_match_x("", "zerotier_nat", "0", "checked"); %> /><#checkbox_No#>
												</div>
												 允许Zerotier的拨入客户端访问路由器LAN资源（需要在 Zerotier管理页面设定到LAN网段的路由表）
											</td>

										</tr>
										<tr>
											<th>zerotier官网</th>
											<td>
				<input type="button" class="btn btn-success" value="zerotier官网" onclick="window.open('https://my.zerotier.com/network')" size="0">
				<br>点击跳转到Zerotier官网管理平台，新建或者管理网络，并允许客户端接入访问你私人网路（新接入的节点默认不允许访问）
											</td>
										</tr>
									</table>
<table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
	<tr> <th colspan="4">需要访问其它zerotier的内网LAN网段,IP和网关和zerotier后台对应即可(本机的LAN网段不用填进去)</th></tr>
                                        <tr id="row_rules_caption">
										 
                                            <th width="10%">
                                                启用 <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="20%">
                                                IP <i class="icon-circle-arrow-down"></i>
                                            </th>
											<th width="25%">
                                                网关 <i class="icon-circle-arrow-down"></i>
                                            </th>
                                            <th width="5%">
                                                <center><i class="icon-th-list"></i></center>
                                            </th>
                                        </tr>
                                         <tr>
                                         	<th>
                                          	<select name="zero_enable_x_0" class="input" style="width: 100px">
													<option value="1" <% nvram_match_x("","zero_enable_x_0", "0","selected"); %>>是</option>
													<option value="0" <% nvram_match_x("","zero_enable_x_0", "0","selected"); %>>否</option>
												</select>
                                           </th>
                                         <th><input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="zero_ip_x_0" value="<% nvram_get_x("", "zero_ip_x_0"); %>"/>
                                            </th>
                                         <th>
                                                <input type="text" maxlength="255" class="span12" style="width: 200px" size="200" name="zero_route_x_0" value="<% nvram_get_x("", "zero_route_x_0"); %>" />
                                            </th>
                                         <th>
                                                <button class="btn" style="max-width: 219px" type="submit" onclick="return markGroupRULES(this, 64, ' Add ');" name="markGroupRULES2" value="<#CTL_add#>" size="12"><i class="icon icon-plus"></i></button>
                                            </th>
                                            </td>
                                        </tr>
                                        <tr id="row_rules_body" >
                                            <td colspan="4" style="border-top: 0 none; padding: 0px;">
                                                <div id="MRULESList_Block"></div>
                                            </td>
                                        </tr>
										</table>
										<tr>
											<td colspan="4" style="border-top: 0 none;">
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

