<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_title#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">

<script type="text/javascript" src="state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
var map_code = "";

function initial(){
	show_banner(0);
	show_menu(5,0,0);
	show_footer();
	show_sitemap();
}

var menu_desc = ["<#menu5_1_desc#>", "<#menu5_2_desc#>", "<#menu5_3_desc#>", "<#menu5_4_desc#>", 
                 "<#menu5_5_desc#>", "<#menu5_6_desc#>", "<#menu5_7_desc#>","<#menu5_7_desc#>"];
//0:Wireless 1:LAN 2:WAN 3:USB 4:Firewall 5:Admin 6:Log

function adjust_menu_desc(){ // In different cases, Ex. firewall is disabled. The menu is changed and adjust in this function.
	
	if(sw_mode == "3"){
		menu_desc.splice(2,1);
		menu_desc.splice(3,1);
		menu_desc.splice(4,1);
	}
}

function show_sitemap(){

	adjust_menu_desc();
	
	for(var i=0, j=0; i<tabtitle.length, j<menuL2_title.length;){
		if(tabtitle[i] == ""){
			tabtitle.splice(i,1);
			tablink.splice(i,1);
		}
		else
			i++;		
		if(menuL2_title[j] == "")
			menuL2_title.splice(j,1);
		else
			j++;
	}
	
	for(var i=0; i<menuL2_title.length; i++){
		var k = (i/4 < 1)?0:3;
		$("menu_body").rows[k].cells[i%4].innerHTML = menuL2_title[i];
		$("menu_body").rows[k].cells[i%4].className = "head";
		$("menu_body").rows[k+1].cells[i%4].innerHTML = menu_desc[i];
		$("menu_body").rows[k+1].cells[i%4].className = "desp";
	}
		
	for(var l = 0; l < tabtitle.length; l++){

		map_code = '<ul>\n';
		for(var m = 1; m < tabtitle[l].length; m++){
			if(tablink[l][m] == "")
				continue;
			
			map_code += '    <li>\n';
			map_code += '        <a href="'+tablink[l][m]+'">'
			map_code += tabtitle[l][m];
			map_code += '</a>\n    </li>\n';
		}
		map_code += '</ul>\n';
		
		var n = (l/4 < 1)?0:3;
		$("menu_body").rows[n+2].cells[l%4].innerHTML = map_code;
	}	
}
</script>
</head>

<body onload="initial();" onunload="return unload_body();">

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

<form name="form">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>

<table width="983" border="0" align="center" cellpadding="0" cellspacing="0" background="images/body-bg1.gif">
	<tr>
		<td width="23">&nbsp;</td>
		
		<td valign="top" width="202">
			<div id="mainMenu"></div>	
			<div id="subMenu"></div>
		</td>				
		<div id="tabMenu"></div>
		<!--=====Beginning of Main Content=====-->
		<td height="400" valign="top">
			<table id="menu_body" width="98%" border="0" align="center" cellpadding="3" cellspacing="1" bgcolor="#CCCCCC" class="sitemap">
				<tr>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
				</tr>
				
				<tr>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
				</tr>
				
				<tr valign="top">
					<td height="120"></td>
					<td height="120"></td>
					<td height="120"></td>
					<td height="120"></td>
				</tr>
				
				<tr>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
					<td>&nbsp;</td>
				</tr>
				
				<tr>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
					<td width="25%">&nbsp;</td>
				</tr>
				
				<tr valign="top">
					<td height="120"></td>
					<td height="120"></td>
					<td height="120"></td>
					<td height="120"></td>
				</tr>
			</table>
		</td>
		<td id="help_td" style="width:15px;" align="center" valign="top">
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
		<td width="20" align="center">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>

</body>
</html>
