<!DOCTYPE html>
<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script>
var $j = jQuery.noConflict();
var id_timer_mib = 0;
var eth_port_id = 0;

$j(window).bind('hashchange', function(){
	update_page();
	update_tabs();
	set_mib_data();
});

function initial(){
	show_banner(1);
	show_menu(5,9,get_page_id());
	show_footer();
	update_page();
	set_mib_data();
}

function getHashId(){
	var curHash = window.location.hash;
	if (curHash == '')
		curHash = '#0';
	var id = parseInt(curHash.replace('#', '0'));
	if (isNaN(id))
		return 0;
	return id;
}

function get_page_id(){
	var page_id = getHashId() + 1;
    if (support_2g_radio())
        page_id += 1;
	if (support_5g_radio())
		page_id += 1;
	return page_id;
}

function update_page(){
	var port_nm = 'WAN';
	eth_port_id = getHashId();
	if (eth_port_id > 0)
		port_nm = 'LAN' + eth_port_id.toString();
	$("hdr_port").innerHTML = '<#menu5_9#> - ' + port_nm;
	document.title = '<#Web_Title#> - ' + port_nm;
}

function update_tabs(){
	var page_id = get_page_id()-1;
	$j('#tabMenu').children('ul').children('li').removeClass("active");
	$j('#tabMenu').children("ul").children("li").eq(page_id).addClass("active");
}

function set_mib_data(){
	clearTimeout(id_timer_mib);
	$j.ajax({
		type: 'get',
		url: '/status_eth_mib.asp',
		data: {
			port_id: eth_port_id
		},
		dataType: 'html',
		success: function(data){
			$j('#mib_area').text(data);
			id_timer_mib = setTimeout('set_mib_data()', 5000);
		}
	});
}
</script>
<style>
.nav-tabs > li > a {
    padding-right: 6px;
    padding-left: 6px;
}
</style>
</head>

<body onload="initial();" >

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
                            <h2 class="box_head round_top" id="hdr_port"></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table">
                                        <tr>
                                            <td style="border-top: 0 none; padding-bottom: 0px;">
                                                <textarea id="mib_area" rows="23" class="span12" style="height:403px; font-family:'Courier New', Courier, mono; font-size:13px;" readonly="readonly" wrap="off"></textarea>
                                            </td>
                                        </tr>
                                        <tr>
                                            <td style="text-align: right; padding-bottom: 0px;">
                                                <input type="button" onClick="set_mib_data();" value="<#CTL_refresh#>" class="btn btn-primary" style="width: 219px;">
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

    <div id="footer"></div>
</div>
</body>
</html>
