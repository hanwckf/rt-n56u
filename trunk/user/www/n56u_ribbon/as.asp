<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<!-- <meta name="viewport" content="width=device-width, initial-scale=1.0"> -->
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_title#></title>
<!-- <link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css"> -->
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
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
	tabtitle.splice(8,3);
	tablink.splice(8,3);
	
	for(var i=0, j=0; i<tabtitle.length, j<menuL2_title.length;){ //消除Array中的空值
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
		var k = (i/4 < 1)?0:3;  // 1~4項[Wireless,LAN,WAN,USB]由rows[0]遞增，第5項起由rows[3]開始遞增
		$("menu_body").rows[k].cells[i%4].innerHTML = "<b>" + menuL2_title[i] + "</b>"  //填入標題
		$("menu_body").rows[k].cells[i%4].className = "head";         //有填才有樣式(底色);		
		$("menu_body").rows[k+1].cells[i%4].innerHTML = "<div class='alert alert-info'>" + menu_desc[i] + "</div>";   //填入說明
		//$("menu_body").rows[k+1].cells[i%4].className = "desp";         //有填才有樣式(底色);
	}


	for(var l = 0; l < tabtitle.length; l++){

		map_code = '<ul class="nav nav-list">\n';
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

<style>
    table#menu_body tr td.head {text-align: center;}
</style>

</head>

<body onload="initial();" onunload="return unload_body();">

<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no" style="position: absolute;"></iframe>

<form name="form">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_char_to_ascii("WLANConfig11b", "wl_ssid"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
</form>

<div class="container-fluid">
    <div class="row-fluid">
        <div class="span2">
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

        <div class="span10">
            <!--Body content-->
            <div class="row-fluid">
                <div class="span12">
                    <div class="box well grad_colour_dark_blue">
                        <h2 class="box_head round_top"><#menu5_title#></h2>
                        <div class="round_bottom">

                            <div class="row-fluid">
                                <table class="table">
                                	<tr>
                                	    <div id="tabMenu"></div>
                                		<!--=====Beginning of Main Content=====-->
                                		<td style="border-top: 0 none;">
                                			<table id="menu_body" width="100%" border="0" align="center" cellpadding="3" cellspacing="1" bgcolor="#CCCCCC" class="sitemap">
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

<div id="help_td" style="position: absolute; margin-left: -10000px;">
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
</div>

<div id="footer"></div>

</body>
</html>
