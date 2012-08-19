<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title>ASUS Wireless Router <#Web_Title#> - <#menu2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

var apps_dms = '<% nvram_get_x("LANHostConfig", "apps_dms"); %>';

function initial(){
	show_banner(0);
	show_menu(3, -1, 0);
	show_footer();

	$("statusframe").style.display = "block";
	
	if(apps_dms == 1){
		$("applyUPnP").innerHTML = "<#btn_disable#>";
		$("UPnPstatus").innerHTML = "<#btn_Enabled#>";
		$("upnptype1").style.display = "none";
		$("upnptype2").style.display = "none";
	}
	else{
		$("applyUPnP").innerHTML = "<#btn_Enable#>";
		$("UPnPstatus").innerHTML = "<#btn_Disabled#>";
		$("upnptype1").style.display = "none";
		$("upnptype2").style.display = "none";
		$("updateUPnP").style.display = "none";
	}
			
	openHint(14, 1);
}

function submitUPnP(x){
	showLoading();
	
	if(x == 1)
		document.mediaserverForm.apps_dms.value = 0;
	else
		document.mediaserverForm.apps_dms.value = 1;

	document.mediaserverForm.flag.value = "nodetect";
	
	document.mediaserverForm.submit();
}

/*function update_UPnP(){
	showLoading();
	document.mediaserverForm.update_upnp.value = 1;
	document.mediaserverForm.flag.value = "nodetect";
	document.mediaserverForm.submit();
}*/

function update_UPnP(){
	showLoading();
	document.mediaserverForm.action_mode.value = "";
	document.mediaserverForm.action_script.value = "update_UPnP";
	document.mediaserverForm.flag.value = "nodetect";
	document.mediaserverForm.submit();
}
</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form method="post" name="mediaserverForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="action_mode" value=" Apply ">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="sid_list" value="Storage;LANHostConfig;">
<input type="hidden" name="current_page" value="/upnp.asp">
<input type="hidden" name="apps_dms" value="">
<!--input type="hidden" name="update_upnp" value=""-->
<input type="hidden" name="flag" value="">
</form>

<form name="form">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>

<table border="0" align="center" cellpadding="0" cellspacing="0" class="content">
	<tr>
		<td valign="top" width="23">&nbsp;</td>

		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>		

		<td height="430" align="center" valign="top">
			<div id="tabMenu"></div>

<!--=====Beginning of Main Content=====-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
		<td align="left" valign="top" width="550">
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="0">
  			<tr>
					<td class="upnp" width="300">
	  					<img src="images/bullet.gif" width="20" height="20" align="absmiddle"/><#menu2#> : <span id="UPnPstatus"></span>
					</td>
					
					<td height="100" align="left" class="upnp">
	  					<div class="upnp_btn"><a id="applyUPnP" href='javascript:submitUPnP(apps_dms);'></a></div>
					</td>
  			</tr>
  			<tr id="updateUPnP" style="display:none;">
  				<td class="upnp" width="250">
	  					<div width="20" height="20" align="absmiddle"/>
					</td>
  				<td height="100" align="left" class="upnp">
	  					<div class="upnp_btn"><a href='javascript:update_UPnP();'><#CTL_refresh#></a></div>
					</td>
			</tr>
  			<tr id="upnptype1">
					<td class="upnp" width="250">
	  					<img src="images/bullet.gif" width="20" height="20" align="absmiddle"/><#UPnPMediaServerType#> :
					</td>
					<td class="upnp" width="250">
						<input type="radio" name="ushare_comp_mode" class="input" value="1" <% nvram_match_x("LANHostConfig", "ushare_comp_mode", "1", "checked"); %>>UPnP (XBOX 360)
	  				</td>
  			</tr>
  			<tr id="upnptype2">
  					<td width="250">
					</td>
					<td class="upnp" width="250">
						<input type="radio" name="ushare_comp_mode" class="input" value="2" <% nvram_match_x("LANHostConfig", "ushare_comp_mode", "2", "checked"); %>>DLNA (PS3)
	  				</td>
  			</tr>
  			<tr><td class="upnp_bottom" width="250"></td><td class="upnp_bottom" width="250"></td></tr>
			</table>
			
			<p>&nbsp;</p>
			
			<table width="545" border="0" align="center" cellpadding="0" cellspacing="0" class="upnpdesp">
				<tr>
					<td valign="top">
						<table width="450" border="0" align="center" cellpadding="0" cellspacing="0">
							<tr>
								<td width="86" valign="top" class="mapmessage">
									<img src="images/ill-1.gif" width="86" height="87">
									<br>
									<#t2UTStatus#>
								</td>

								<td width="92" valign="top" class="mapmessage">
									<img src="images/ill-2.gif" width="92" height="89">
									<br>
									<#DigitalMediaServer#>
								</td>

								<td width="65" align="center" valign="top">
									<br>
									<img src="images/ill-25.gif" width="53" height="78">
								</td>

								<td width="170" align="center" valign="top">
									<img src="images/ill-6.gif" width="100%" height="87">
									<br>
									<#DigitalMediaPlayer#>
								</td>

								<!--td width="42" align="center" valign="top" class="account">
									<img src="images/ill-4.gif" width="84" height="88">
									<br>
								</td-->

								<td width="42" align="center" valign="middle" class="account">
									<#TV#>
								</td>
							</tr>
						</table>
					</td>
				</tr>

				<tr>
					<td valign="top">
						<table width="450" border="0" cellspacing="0" cellpadding="0">
							<tr>
								<td width="200">&nbsp;</td>
								<td width="84" valign="top" class="mapmessage">
									<img src="images/ill-5.gif" width="141" height="111">
									<br>
									<br>
								</td>

								<td>
									<#DigitalMediaPlayer#>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>

	<!--==============Beginning of hint content=============-->
	<td id="help_td" style="width:15px;" valign="top">
		<form name="hint_form"></form>
		<div id="helpicon"></div>
		<div id="hintofPM" style="display:none;">
			<table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
				<thead>
				<tr>
					<td>
						<div id="helpname" class="AiHintTitle"></div>
					</td>
				</tr>
				</thead>
				
				<tr>
					<td valign="top">
						<div class="hint_body2" id="hint_body"></div>
						<iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
					</td>
				</tr>
			</table>
		</div>
	</td>
	<!--==============Ending of hint content=============-->
				</tr>
			</table>
		</td>

		<td width="20" align="center" valign="top"></td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>

