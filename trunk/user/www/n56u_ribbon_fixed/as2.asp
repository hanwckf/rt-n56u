<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_20#></title>
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

$j(document).ready(function() {
    init_itoggle('www_L8x');
    init_itoggle('www_L9x');
});
</script>
<script>
function initial(){
    show_banner(1);
    show_menu(8,0,0);
    show_footer();
    show_menu_title(12,5,0);
}

function applyRule(){
//    if(validForm()){
        showLoading(3);
        
        document.form.action_mode.value = " Apply ";
        document.form.current_page.value = "/as2.asp";
        document.form.next_page.value = "";
        
        document.form.submit();
//    }
}


function done_validating(action){
    refreshpage();
}

function button_remenu0(){
    var str = "这是一个没用的按钮？";
    if(confirm(str)){
	showLoading(5);
    var $j = jQuery.noConflict();
    $j.post('/apply.cgi',
    {
        'action_mode': ' button_remenu0 ',
    });
    setTimeout("refreshpage();", 5000);
    }
}

function button_remenu1(){
    var str = "这是两个个没用的按钮！";
    if(confirm(str)){
	showLoading(5);
    var $j = jQuery.noConflict();
    $j.post('/apply.cgi',
    {
        'action_mode': ' button_remenu1 ',
    });
    setTimeout("refreshpage();", 5000);
    }
}

function www_upver(){
    var str = "脚本还在开发啦，阿Sir！";
    $j.post('/apply.cgi',
    {
        'action_mode': ' www_upver ',
    });
    setTimeout("refreshpage();", 2000);
}


function delval(id){
    var obj = $j('#'+id);
    var obj_icon = $j('#'+id+'_icon');
    var changeTo = (obj.attr('value') == '') ? '0' : '1';
    if (changeTo==1){
    	obj.attr('value' , '');
    	obj_icon.attr('class' , 'icon-share-alt');
    }else{
    	obj.attr('value' , obj.attr('refresh'));
    	obj_icon.attr('class' , 'icon-trash');
    }
}

function get_menu_title(i, ii){
menux_titlex=eval("menu"+i+"_title"+ii);
}

function show_menu_title(Li1, Li2, Li3){
    var i, ii;
    var menu_title0_code="";
    var menu_title_code="";
    for(i = 1; i <= 1; i++){
        menu_title0_code +='<tr>\n'
        for(ii = 1; ii <= Li2; ii++){
        get_menu_title(0, ii);
            menu_title0_code +='<td style="border: 0 none;">\n'
                menu_title0_code +='<input style="width: 75px;" name="menu0_title' +ii+ '" id="menu0_title' +ii+ '" type="text" maxlength="64" class="input" size="15" value="' +menux_titlex+ '" refresh="' +menux_titlex+ '" onKeyPress="return is_string(this,event);" />\n'
                menu_title0_code +='<button style="margin-left: -5px;" class="btn" type="button" onclick="delval(\'menu0_title' +ii+ '\')"><i name="menu0_title' +ii+ '_icon" id="menu0_title' +ii+ '_icon" class="icon-trash"></i></button>\n'
            menu_title0_code +='</td>\n'
        }
        menu_title0_code +='</tr>\n'
    }
$j('#menu_title0').append(menu_title0_code);

    for(i = 1; i <= Li1; i++){
        menu_title_code +='<tr>\n'
        for(ii = 1; ii <= Li2; ii++){
        get_menu_title(i, ii);
            menu_title_code +='<td style="border: 0 none;">\n'
                menu_title_code +='<input style="width: 75px;" name="menu' +i+ '_title' +ii+ '" id="menu' +i+ '_title' +ii+ '" type="text" maxlength="64" class="input" size="15" value="' +menux_titlex+ '" refresh="' +menux_titlex+ '" onKeyPress="return is_string(this,event);" />\n'
                menu_title_code +='<button style="margin-left: -5px;" class="btn" type="button" onclick="delval(\'menu' +i+ '_title' +ii+ '\')"><i name="menu' +i+ '_title' +ii+ '_icon" id="menu' +i+ '_title' +ii+ '_icon" class="icon-trash"></i></button>\n'
            menu_title_code +='</td>\n'
        }
        menu_title_code +='</tr>\n'
    }
$j('#menu_title').append(menu_title_code);
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

    <input type="hidden" name="current_page" value="as2.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;General;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="wan_ipaddr" value="<% nvram_get_x("", "wan0_ipaddr"); %>" readonly="1">
    <input type="hidden" name="wan_netmask" value="<% nvram_get_x("", "wan0_netmask"); %>" readonly="1">
    <input type="hidden" name="dhcp_start" value="<% nvram_get_x("", "dhcp_start"); %>">
    <input type="hidden" name="dhcp_end" value="<% nvram_get_x("", "dhcp_end"); %>">

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
                            <h2 class="box_head round_top">扩展功能</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <div class="alert alert-info" style="margin: 10px;">这个页面是用于安置无处安放的插件
                                    <div>主要存放的插件： </div>
                                    <div>1.内网穿透插件：包含nps,aliddns,frp,zerotier </div>
                                    <div>2.校园网专用插件：锐捷、赛尔认证 </div>
                                    <div>3.处于学习目的的科学插件 </div>
                                    </div>

                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="4" style="background-color: rgba(255,255,255,0.2);">开关</th>
                                        </tr>
                                        <tr>
                                            <th width="30%"><a class="help_tooltip" href="javascript: void(0)" onmouseover="openTooltip(this, 26, 9);">启用【高级设置】自动缩进</a></th>
                                            <td style="border: 0 none;">
                                                <div class="main_itoggle">
                                                    <div id="www_L8x_on_of">
                                                        <input type="checkbox" id="www_L8x_fake" <% nvram_match_x("", "www_L8x", "1", "value=1 checked"); %><% nvram_match_x("", "www_L8x", "0", "value=0"); %>  />
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="www_L8x" id="www_L8x_1" class="input" value="1" <% nvram_match_x("", "www_L8x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="www_L8x" id="www_L8x_0" class="input" value="0" <% nvram_match_x("", "www_L8x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                            <th>
                                            <input class="btn btn-success" style="" type="button" value="重置脚本" onclick="button_remenu0()" />
                                            </th>
                                            <th>
                                            <input class="btn btn-primary" style="" type="button" value="检查更新" onclick="www_upver()" />
                                            </th>
                                        </tr>
                                        <tr>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="www_L9x_on_of">
                                                        <input type="checkbox" id="www_L9x_fake" <% nvram_match_x("", "www_L9x", "1", "value=1 checked"); %><% nvram_match_x("", "www_L9x", "0", "value=0"); %>  />
                                                    </div>
                                                </div>
                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" value="1" name="www_L9x" id="www_L9x_1" class="input" value="1" <% nvram_match_x("", "www_L9x", "1", "checked"); %> /><#checkbox_Yes#>
                                                    <input type="radio" value="0" name="www_L9x" id="www_L9x_0" class="input" value="0" <% nvram_match_x("", "www_L9x", "0", "checked"); %> /><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>
                                            <th colspan="2">
                                                <input class="btn btn-success" style="" type="button" value="默认菜单" onclick="button_remenu1()" />
                                            </th>
                                            <th>
                                            <input class="btn btn-primary" style="" type="button" value="恢复默认" onclick="www_upver()" />
                                        </th>
                                    </table>
                                    <table width="100%" align="center" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <th colspan="4" style="background-color: rgba(255,255,255,0.2);">默认菜单标题设置脚本</th>
                                        </tr>
                                        <tr>
                                            <td colspan="3" style="border-top: 0 none;">
                                                <i class="icon-hand-right"></i> <a href="javascript:spoiler_toggle('script10')"><span>点这里自定义默认菜单标题</span></a>
                                                <div id="script10" style="display:none;">
                                                    <textarea rows="10" wrap="off" spellcheck="false" maxlength="2097152" class="span12" name="scripts.menu_title_script.sh" style="font-family:'Courier New'; font-size:12px;"><% nvram_dump("scripts.menu_title_script.sh",""); %></textarea>
                                                </div>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td style="border: 0 none;" colspan="2">
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

