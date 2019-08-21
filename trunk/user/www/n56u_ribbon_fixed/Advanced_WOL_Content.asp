<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_2_6#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/jquery.xdomainajax.js"></script>
<script type="text/javascript" src="/bootstrap/js/jquery.maskedinput-1.3.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/client_function.js"></script>
<script>
var $j = jQuery.noConflict();

var ipmonitor = [<% get_static_client(); %>];
var m_dhcp = [<% get_nvram_list("LANHostConfig", "ManualDHCPList"); %>];

var staticClients = get_resolved_clients();

var devices = {};
var allMacs = {};

function initial(){
	var id_menu = 6;
	if(get_ap_mode()){
		id_menu = 5;
		if (lan_proto == '1')
			id_menu--;
	}

	show_banner(1);
	show_menu(5,3,id_menu);
	show_footer();
}

// find oui in localStorage (name = hw_addr)
function findVendorInLocalStorage(mac)
{
    return (allMacs !== null && allMacs[mac] != 'undefined') ? allMacs[mac] : null;
}

function get_resolved_clients()
{
    var clients = new Array();
    for(var i = 0; i < ipmonitor.length; ++i) {
        clients[i] = new Array(3);
        clients[i][0] = ipmonitor[i][0];
        clients[i][1] = ipmonitor[i][1];
        clients[i][2] = ipmonitor[i][2];

        if(clients[i][2] == null || clients[i][2].length == 0)
            clients[i][2] = "*";
    }
    return clients;
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
                url: 'http://www.macvendorlookup.com/api/v2/'+hw_addr+'/json',
                type: 'GET',
                success: function(response){
                    try{
                        var vendorObj = JSON.parse($j(response.responseText).text())[0];
                        $j(value).parents('tr').find('td.vendor').html(vendorObj.company);

                        // add new vendor for saving to localStorage
                        allMacs[hw_addr] = vendorObj.company;

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

                    $j('#wol_mac').val(mac);
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
                }, 1500);
            }
        );
    }
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

    for(i = 0; i < m_dhcp.length; i++)
    {
        var mac  = mac_add_delimiters(m_dhcp[i][0]);
        var name = m_dhcp[i][2];
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
            var btn = '<button class="btn btn-info btn_wakeup"><#WOL_Wake_up#></button><div class="wol_response" class="alert"></div>';

            t_body += '<tr>\n';
            t_body += '  <td class="mac">'+mac+'</td>\n';
            t_body += '  <td>'+name+'</td>\n';
            t_body += '  <td class="vendor">'+vendor+'</td>\n';
            t_body += '  <td>'+btn+'</td>\n';
            t_body += '</tr>\n';
        });
        $j('#wol_table').append(t_body);

        setTimeout("getVendors()", 200);
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

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

    <form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">

    <input type="hidden" name="current_page" value="Advanced_WOL_Content.asp">
    <input type="hidden" name="next_page" value="">
    <input type="hidden" name="next_host" value="">
    <input type="hidden" name="sid_list" value="LANHostConfig;">
    <input type="hidden" name="group_id" value="">
    <input type="hidden" name="action_mode" value="">
    <input type="hidden" name="action_script" value="">

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
                            <h2 class="box_head round_top"><#menu5_2#> - <#menu5_2_6#></h2>
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
                                                <input type="button" id="wol_btn" class="btn btn-primary" value="<#WOL_Wake_up#>" />
                                                <div class="wol_response" class="alert"></div>
                                            </td>
                                        </tr>
                                    </table>

                                    <table width="100%" cellpadding="4" cellspacing="0" class="table" id="wol_table">
                                        <tr>
                                            <th width="25%"><#MAC_Address#></th>
                                            <th width="25%"><#Computer_Name#></th>
                                            <th width="35%"><#WOL_Vendor#></th>
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

    <div id="footer"></div>
</div>
</body>
</html>
