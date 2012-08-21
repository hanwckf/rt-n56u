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

<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]


function initial(){
	show_banner(1);
	show_menu(5,2,2);
	show_footer();
	showtext($("LANIP"), '<% nvram_get_x("LANHostConfig", "lan_ipaddr"); %>');
	
	if((inet_network(document.form.lan_ipaddr.value)>=inet_network(document.form.dhcp_start.value))&&(inet_network(document.form.lan_ipaddr.value)<=inet_network(document.form.dhcp_end.value))){
			$('router_in_pool').style.display="";
	}
	
	enable_auto_hint(5, 7);

	load_body();
}

function applyRule(){
	if(validForm()){
		showLoading();
		
		document.form.action_mode.value = " Restart ";
		document.form.current_page.value = "/Advanced_DHCP_Content.asp";
		document.form.next_page.value = "";
		
		document.form.submit();
	}
}

function validForm(){
	var re = new RegExp('^[a-zA-Z0-9][a-zA-Z0-9\-\_\.]*[a-zA-Z0-9\-\_]$','gi');
	if(!re.test(document.form.lan_domain.value) && document.form.lan_domain.value != ""){
		alert("<#JS_validchar#>");
		document.form.lan_domain.focus();
		document.form.lan_domain.select();
		return false;
	}
	
	if(!validate_string(document.form.lan_domain))
		return false;
	
	if(!validate_ipaddr_final(document.form.dhcp_start, 'dhcp_start') ||
			!validate_ipaddr_final(document.form.dhcp_end, 'dhcp_end') ||
			!validate_ipaddr_final(document.form.dhcp_gateway_x, 'dhcp_gateway_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns1_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns2_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_dns3_x, 'dhcp_dns1_x') ||
			!validate_ipaddr_final(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;
	
	if(!validate_range(document.form.dhcp_lease, 120, 604800))
		return false;
	
	if(intoa(document.form.dhcp_start.value) > intoa(document.form.dhcp_end.value)){	//exchange start < end
		tmp = document.form.dhcp_start.value;
		document.form.dhcp_start.value = document.form.dhcp_end.value;
		document.form.dhcp_end.value = tmp;
	}
	
	var default_pool = new Array();
	default_pool =get_default_pool(document.form.lan_ipaddr.value, document.form.lan_netmask.value);
	if((inet_network(document.form.dhcp_start.value) < inet_network(default_pool[0])) || (inet_network(document.form.dhcp_end.value) > inet_network(default_pool[1]))){
			if(confirm("<#JS_DHCP3#>")){ //Acceptable DHCP ip pool : "+default_pool[0]+"~"+default_pool[1]+"\n
				document.form.dhcp_start.value=default_pool[0];
				document.form.dhcp_end.value=default_pool[1];
			}else{return false;}
	}
	
	if(!validate_ipaddr(document.form.dhcp_wins_x, 'dhcp_wins_x'))
		return false;

	return true;
}

function done_validating(action){
	refreshpage();
}

// Viz add 2011.10 default DHCP pool range{
function get_default_pool(ip, netmask){
	// --- get lan_ipaddr post set .xxx  By Viz 2011.10
	z=0;
	tmp_ip=0;
	for(i=0;i<document.form.lan_ipaddr.value.length;i++){
			if (document.form.lan_ipaddr.value.charAt(i) == '.')	z++;
			if (z==3){ tmp_ip=i+1; break;}
	}		
	post_lan_ipaddr = document.form.lan_ipaddr.value.substr(tmp_ip,3);
	// --- get lan_netmask post set .xxx	By Viz 2011.10
	c=0;
	tmp_nm=0;
	for(i=0;i<document.form.lan_netmask.value.length;i++){
			if (document.form.lan_netmask.value.charAt(i) == '.')	c++;
			if (c==3){ tmp_nm=i+1; break;}
	}		
	var post_lan_netmask = document.form.lan_netmask.value.substr(tmp_nm,3);
	
var nm = new Array("0", "128", "192", "224", "240", "248", "252");
	for(i=0;i<nm.length;i++){
				 if(post_lan_netmask==nm[i]){
							gap=256-Number(nm[i]);
							subnet_set = 256/gap;
							for(j=1;j<=subnet_set;j++){
									if(post_lan_ipaddr < j*gap){
												pool_start=(j-1)*gap+1;
												pool_end=j*gap-2;
												break;						
									}
							}					
																	
							var default_pool_start = subnetPostfix(document.form.dhcp_start.value, pool_start, 3);
							var default_pool_end = subnetPostfix(document.form.dhcp_end.value, pool_end, 3);							
							var default_pool = new Array(default_pool_start, default_pool_end);
							return default_pool;
							break;
				 }
	}	
	//alert(document.form.dhcp_start.value+" , "+document.form.dhcp_end.value);//Viz
}
// } Viz add 2011.10 default DHCP pool range	

</script>
</head>

<body onload="initial();" onunLoad="disable_auto_hint(5, 7);return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get_f("general.log","productid"); %>">

<input type="hidden" name="current_page" value="Advanced_DHCP_Content.asp">
<input type="hidden" name="next_page" value="Advanced_GWStaticRoute_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="ManualDHCPList">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">

<input type="hidden" name="lan_ipaddr" value="<% nvram_get_x("LANHostConfig","lan_ipaddr"); %>">
<input type="hidden" name="lan_netmask" value="<% nvram_get_x("LANHostConfig","lan_netmask"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div><br />

<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td align="left" valign="top">
	  <table width="98%" border="0" align="center" cellpadding="4" cellspacing="0" class="FormTitle" table>
		<thead>
		  <tr>
			<td><#menu5_2#> - <#menu5_2_2#></td>
		  </tr>
		</thead>
		
		<tr>
		  <td bgcolor="#FFFFFF">
		  	<#LANHostConfig_DHCPServerConfigurable_sectiondesc#><br>
		  	<div id="router_in_pool" style="color:#FF3300;display:none;"><b><#LANHostConfig_DHCPServerConfigurable_sectiondesc2#> <span id="LANIP"></span></b></div>
		  </td>
		</tr>
		
		<tbody>
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			  <tr>
				<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,1);"><#LANHostConfig_DHCPServerConfigurable_itemname#></a></th>
				<td>
				  <input type="radio" value="1" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '1')" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "1", "checked"); %>><#checkbox_Yes#>
				  <input type="radio" value="0" name="dhcp_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'dhcp_enable_x', '0')" <% nvram_match_x("LANHostConfig","dhcp_enable_x", "0", "checked"); %>><#checkbox_No#>
				</td>
			  </tr>
			  
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,2);"><#LANHostConfig_DomainName_itemname#></a></th>
				<td>
				  <input type="text" maxlength="32" class="input" size="32" name="lan_domain" value="<% nvram_get_x("LANHostConfig", "lan_domain"); %>" onblur="is_string(this);">
				</td>
			  </tr>
			  
			  <tr>
			  <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,3);"><#LANHostConfig_MinAddress_itemname#></a></th>
			  <td>
				<input type="text" maxlength="15" class="input" size="15" name="dhcp_start" value="<% nvram_get_x("LANHostConfig","dhcp_start"); %>" onKeyPress="return is_ipaddr(this);" onKeyUp="change_ipaddr(this);">
			  </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,4);"><#LANHostConfig_MaxAddress_itemname#></a></th>
            <td>
              <input type="text" maxlength="15" class="input" size="15" name="dhcp_end" value="<% nvram_get_x("LANHostConfig","dhcp_end"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
            </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,5);"><#LANHostConfig_LeaseTime_itemname#></a></th>
            <td>
              <input type="text" maxlength="6" size="6" name="dhcp_lease" class="input" value="<% nvram_get_x("LANHostConfig", "dhcp_lease"); %>" onKeyPress="return is_number(this)">
            </td>
			  </tr>
			  
			  <tr>
            <th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,6);"><#LANHostConfig_x_LGateway_itemname#></a></th>
            <td>
              <input type="text" maxlength="15" class="input" size="15" name="dhcp_gateway_x" value="<% nvram_get_x("LANHostConfig","dhcp_gateway_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
            </td>
			  </tr>
			</table>
		  </td>
		</tr>
		
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			  <thead>
			  <tr>
				<td colspan="2"><#LANHostConfig_x_LDNSServer1_sectionname#></td>
			  </tr>
			  </thead>
			  <tr>
				<th width="40%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,7);"><#LANHostConfig_x_LDNSServer1_itemname#></a></th>
				<td>
				  <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns1_x" value="<% nvram_get_x("LANHostConfig","dhcp_dns1_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
				</td>
			  </tr>
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,8);"><#LANHostConfig_x_LDNSServer2_itemname#></a></th>
				<td>
				  <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns2_x" value="<% nvram_get_x("LANHostConfig","dhcp_dns2_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
				</td>
			  </tr>
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,9);"><#LANHostConfig_x_LDNSServer3_itemname#></a></th>
				<td>
				  <input type="text" maxlength="15" class="input" size="15" name="dhcp_dns3_x" value="<% nvram_get_x("LANHostConfig","dhcp_dns3_x"); %>" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)">
				</td>
			  </tr>
			  <tr>
				<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#LANHostConfig_x_WINSServer_itemname#></a></th>
				<td>
				  <input type="text" maxlength="15" class="input" size="15" name="dhcp_wins_x" value="<% nvram_get_x("LANHostConfig","dhcp_wins_x"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
				</td>
			  </tr>
			</table>
		  </td>
		</tr>
		
		<!-- manually assigned the DHCP List -->
		<tr>
		  <td bgcolor="#FFFFFF">
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
			  <thead>
			  <tr>
				<td colspan="4" id="GWStatic"><#LANHostConfig_ManualDHCPList_groupitemdesc#></td>
			  </tr>
			  </thead>
			  <tr>
				<th colspan="2"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,11);"><#LANHostConfig_ManualDHCPEnable_itemname#></a></th>
				<td colspan="2">
					<input type="radio" value="1" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '1')" <% nvram_match_x("LANHostConfig","dhcp_static_x", "1", "checked"); %> /><#checkbox_Yes#>
					<input type="radio" value="0" name="dhcp_static_x"  onclick="return change_common_radio(this, 'LANHostConfig', 'dhcp_static_x', '0')" <% nvram_match_x("LANHostConfig","dhcp_static_x", "0", "checked"); %> /><#checkbox_No#>
				</td>
			  </tr>
			  <tr bgcolor="#FFFFFF">
				<th style="text-align:center;"><#LANHostConfig_ManualMac_itemname#></th>
				<th style="text-align:center;"><#LANHostConfig_ManualIP_itemname#></th>
				<th style="text-align:center;"><#LANHostConfig_ManualName_itemname#></th>
				<th width="40"></th>
			  </tr>
			  <tr bgcolor="#FFFFFF">
			    <td align="center">
				<input type="hidden" name="dhcp_staticnum_x_0" value="<% nvram_get_x("LANHostConfig", "dhcp_staticnum_x"); %>" readonly="1" />
				<input type="text" maxlength="12" class="input" size="15" name="dhcp_staticmac_x_0" onkeypress="return is_hwaddr()" />
			    </td>
			    <td align="center">
				<input type="text" maxlength="15" class="input" size="15" name="dhcp_staticip_x_0" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)" />
			    </td>
			    <td align="center">
				<input type="text" maxlength="24" class="input" size="20" name="dhcp_staticname_x_0" onKeyPress="return is_string(this)"/>
			    </td>
			    <td width="40">
				<input class="button" type="submit" onclick="return markGroup(this, 'ManualDHCPList', 64, ' Add ');" name="ManualDHCPList2" value="<#CTL_add#>" size="12" />
			    </td>
			  </tr>
			  <tr bgcolor="#FFFFFF">
			    <td colspan="3" align="center">
				<select class="input" size="5" name="ManualDHCPList_s" multiple="multiple" style="width:100%; font-size:12px; font-family:'fixedsys', Courier, mono;">
					<% nvram_get_table_x("LANHostConfig","ManualDHCPList"); %>
				</select>
			    </td>
			    <td>
				<input class="button" type="submit" onclick="return markGroup(this, 'ManualDHCPList', 64, ' Del ');" name="ManualDHCPList" value="<#CTL_del#>" size="12" />
			    </td>
			  </tr>
			  
			  <tr align="right">
            <td colspan="4">
              <input type="button" name="button" class="button" onclick="applyRule();" value="<#CTL_apply#>"/>
            </td>
          	  </tr>
        	</table>
      	  </td>
		</tr>
		</tbody>
	  </table>		
	</td>
</form>

	<!-- help block -->
	<td id="help_td" style="width:15px;" valign="top">
<form name="hint_form"></form>
	  <div id="helpicon" onClick="openHint(0,0);" title="<#Help_button_default_hint#>">
	  	<img src="images/help.gif" />
	  </div>
      
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
	</td>
	<!--==============Ending of hint content=============-->
  </tr>
</table>
		<!--===================================Ending of Main Content===========================================-->		
	</td>
	
    <td width="10" align="center" valign="top">&nbsp;</td>
  </tr>
</table>

<div id="footer"></div>
</body>
</html>
