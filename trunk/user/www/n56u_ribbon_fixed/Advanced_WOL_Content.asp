<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - WoL></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/bootstrap/js/jquery.xdomainajax.js"></script>
<script type="text/javascript" src="/bootstrap/js/jquery.maskedinput-1.3.min.js"></script>

<script>
var $j = jQuery.noConflict();

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>

// [[IP, MAC, DeviceName, Type, http, printer, iTune], ...]
var staticClients   = [<% get_static_client(); %>];

// [[MAC, IP, Name], ...]
var manualDhcpClients = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

var devices = {};
var allMacs = {};

function initial(){
	show_banner(1);
	show_menu(5,3,5);
	show_footer();
}

// find oui in localStorage (name = hw_addr)
function findVendorInLocalStorage(mac)
{
    return (allMacs !== null && allMacs[mac] != 'undefined') ? allMacs[mac] : null;
}

// get mac vendors by api from site http://www.macvendorlookup.com/
function getVendors()
{
    var $macs = $j('#wol_table .mac');
    $macs.each(function(index, value)
    {
        var hw_addr = $j(value).text();
            // need only first six symbols without ':'
            hw_addr = hw_addr.replace(new RegExp(":",'g'),'').substring(0,6);

        // try to find vendor from localStorage
        var company = findVendorInLocalStorage(hw_addr);

        if(company == null)
        {
            // this ajax request with hack from xdomainajax.js
            $j.ajax({
                url: 'http://www.macvendorlookup.com/api/BASKEUS/'+hw_addr,
                type: 'GET',
                success: function(response){
                    try{
                        var vendorObj = JSON.parse($j(response.responseText).text())[0];
                        $j(value).parents('tr').find('td.vendor').html(vendorObj.company);

                        // add new vendor for saving to localStorage
                        allMacs[vendorObj.oui] = vendorObj.company;

                        // save vendor to localStorage
                        setToLocalStorage('hw_addr', JSON.stringify(allMacs));
                    }
                    catch(err){
                        // not found hw vendor ((
                    }

                }
            });
        }
        else
        {
            $j(value).parents('tr').find('td.vendor').html(company);
        }
    });

}

function sendWakeUp(mac, $button)
{
    var $respClass = $button.parents('td').find('div.wol_response');
    if(mac != '')
    {
        $j.getJSON('/wol_action.asp', {dstmac: mac},
            function(response){
                var respMac = (response != null && typeof response === 'object' && "wol_mac" in response)
                              ? response.wol_mac.toUpperCase()
                              : null;

                $button.hide();

                if(respMac == mac)
                {
                    $respClass.removeClass('alert-error')
                              .addClass('alert-success')
                              .html('Success')
                              .show();
                }
                else
                {
                    $respClass.removeClass('alert-success')
                              .addClass('alert-error')
                              .html('Error')
                              .show();
                }

                var idTimeOut = setTimeout(function(){
                    clearTimeout(idTimeOut);
                    $respClass.hide();
                    $button.show();
                }, 2000);
            }
        );
    }
}

function addSeparators(rawMac) {
    var ret="";
    for (var i=0; i < rawMac.length; i++) {
        ret += rawMac.charAt(i);
        if (i % 2 == 1 && i < rawMac.length -1) {
            ret += ":";
        }
    }
    return ret.toUpperCase();
}

function getDevices()
{
    var mergedDevices = {};
    for(var i = 0; i < staticClients.length; i++)
    {
        var mac  = staticClients[i][1];
        var name = staticClients[i][2];
        mergedDevices[mac] = name ? name : '';
    }

    for(i = 0; i < manualDhcpClients.length; i++)
    {
        var mac  = addSeparators(manualDhcpClients[i][0]);
        var name = manualDhcpClients[i][2];
        mergedDevices[mac] = name ? name : '';
    }

    return mergedDevices;
}

$j(document).ready(function() {
    // wol masked input mac
    $j.mask.definitions['@']='[A-Fa-f0-9]';
    $j('#wol_mac').mask("@@:@@:@@:@@:@@:@@");

    try{
        // try load all mac addresses from localStorage
        allMacs = getFromLocalStorage('hw_addr');
        allMacs = JSON.parse(allMacs);
        if($j(allMacs).size() == 0)
        {
            throw new Error('empty object');
        }
    }
    catch(err){
        allMacs = {};
    }

    devices = getDevices();

    // create table of devices
    var countDevices = Object.keys(devices).length;
    if(countDevices > 0 || countDevices === 'undefined')
    {
        var t_body = '';

        $j.each(devices, function(mac, name){
            var vendor = '';
            var btn = '<button class="btn btn-info btn_wakeup">Wake up</button><div class="wol_response" class="alert"></div>';

            t_body += '<tr>\n';
            t_body += '  <td class="mac">'+mac+'</td>\n';
            t_body += '  <td>'+name+'</td>\n';
            t_body += '  <td class="vendor">'+vendor+'</td>\n';
            t_body += '  <td>'+btn+'</td>\n';
            t_body += '</tr>\n';
        });
        $j('#wol_table').append(t_body);

        getVendors();
    }

    // event click "Wake up"
    $j('#wol_btn, .btn_wakeup').click(function(){
        var $button = $j(this);
        var mac;

        mac = ($button.prop('id') == 'wol_btn')
              ? $button.parents('tr').find('.mac').val()
              : mac = $button.parents('tr').find('.mac').text();

        sendWakeUp(mac.toUpperCase(), $button);

        return false;
    });
});
</script>
<style>
    #wol_table{
        margin-top: 20px;
    }

    #wol_table td:first-child {
        font-family: monospace;
        font-weight: bold;
    }

    .wol_response
    {
        display: none;
        float: left;
        padding: 5px 11px;
        width: 52px;
        height: 18px;
        text-align: center;
        border-radius: 5px;
    }
</style>
</head>

<body onload="initial();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="hiddenMask" class="popup_bg" style="position: absolute; margin-left: -10000px;">
        <table cellpadding="5" cellspacing="0" id="dr_sweet_advise" class="dr_sweet_advise" align="center">
            <tr>
            <td>
                <div class="drword" id="drword"><#Main_alert_proceeding_desc4#> <#Main_alert_proceeding_desc1#>...
                    <br/>
                    <br/>
                </div>
              <div class="drImg"><img src="images/DrsurfImg.gif"></div>
                <div style="height:70px; "></div>
            </td>
            </tr>
        </table>
    <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
    <input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

    <input type="hidden" name="current_page" value="Advanced_LAN_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="modified" value="0">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">
    <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">

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
                            <h2 class="box_head round_top"><#menu5_2#> - Wake-on-LAN</h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" style="margin-top: 10px;">
                                        <tr>
                                            <th width="25%" style="border-top: 0 none; "><#MAC_Address#></th>
                                            <td width="395px" style="border-top: 0 none; ">
                                                <input style="float: left; margin-right: 5px; font-family: monospace" id="wol_mac" type="text" maxlength="17" class="span12 mac" size="15" name="wol_mac" value="<% nvram_get_x("","wol_mac_last"); %>"/>
                                            </td>
                                            <td style="border-top: 0 none; ">
                                                <input type="button" id="wol_btn" class="btn btn-primary" value="Wake-up" />
                                                <div class="wol_response" class="alert"></div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="wol_table">
                                        <tr>
                                            <th width="25%"><#MAC_Address#></th>
                                            <th width="25%"><#Computer_Name#></th>
                                            <th width="35%">Vendor</th>
                                            <th width="15%">&nbsp;</th>
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
</div>
</body>
</html>
