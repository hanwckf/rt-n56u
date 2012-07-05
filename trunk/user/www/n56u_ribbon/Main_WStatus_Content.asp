<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_7_4#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';
function initial(){
	show_banner(1);
	
	if(sw_mode == "1" || sw_mode == "4")
		show_menu(5,8,3);
	else
		show_menu(5,8,2);
	
	show_footer();
	
	enable_auto_hint(0, 11);
	
	//load_body();
}
</script>
</head>

<body onload="initial();" >
<div class="container-fluid" style="padding-right: 0px">
    <div class="row-fluid">
        <div class="span2"><center><div id="logo"></div></center></div>
        <div class="span10" >
            <div id="TopBanner"></div>
        </div>
    </div>
</div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="apply.cgi" >
<input type="hidden" name="current_page" value="Main_WStatus_Content.asp">
<input type="hidden" name="next_page" value="Main_WStatus_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANConfig11a;WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

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
                        <h2 class="box_head round_top"><#menu5_7#> - <#menu5_7_4#> (5GHz)</h2>
                        <div class="round_bottom">
                            <div class="row-fluid">
                                <div id="tabMenu" class="submenuBlock"></div>

                                <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                    <tr>
                                        <td colspan="2" style="border-top: 0 none;">
                                            <textarea rows="20" class="span12" style="font-family:'Courier New', Courier, mono; font-size:13px;" readonly="readonly" wrap=VIRTUAL><% nvram_dump("wlan11b.log","wlan11b.sh"); %></textarea>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td width="40%">
                                            <input type="button" class="btn btn-info" value="<#GO_2G#>" onclick="location.href='Main_WStatus2g_Content.asp';">
                                        </td>
                                        <td style="text-align: right">
                                            <input type="button" onClick="location.href=location.href" value="<#CTL_refresh#>" class="btn btn-primary" style="width: 219px;">
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

<!--==============Beginning of hint content=============-->
 <div id="help_td" style="position: absolute; margin-left: -10000px" valign="top">
     <form name="hint_form"></form>
     <div id="helpicon" onClick="openHint(0,0);"><img src="images/help.gif" /></div>

     <div id="hintofPM" style="display:none;">
         <table width="100%" cellpadding="0" cellspacing="1" class="Help" bgcolor="#999999">
         <thead>
             <tr>
                 <td>
                     <div id="helpname" class="AiHintTitle"></div>
                     <a href="javascript:;" onclick="closeHint()" ><img src="images/button-close.gif" class="closebutton" /></a>
                 </td>
             </tr>
         </thead>

             <tr>
                 <td valign="top" >
                     <div class="hint_body2" id="hint_body"></div>
                     <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
                 </td>
             </tr>
         </table>
     </div>
 </div>
 <!--==============Ending of hint content=============-->

<div id="footer"></div>
</body>
</html>
