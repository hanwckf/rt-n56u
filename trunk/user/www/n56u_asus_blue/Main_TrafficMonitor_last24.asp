<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<meta name="svg.render.forceflash" content="false" />	

<title>ASUS Wireless Router <#Web_Title#> - <#menu4_2#> : <#menu4_2_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="tmmenu.css">
<link rel="stylesheet" type="text/css" href="menu_style.css">  <!--  Viz 2010.09 -->

<script src='svg.js' data-path="/src/" data-debug="true"></script>	
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmmenu.js"></script>
<script language="JavaScript" type="text/javascript" src="/tmcal.js"></script>

<script type='text/javascript'>

wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';
qos_enabled = '<% nvram_get_x("",  "qos_enable"); %>';
preferred_lang = '<% nvram_get_x("",  "preferred_lang"); %>';
chk_hwnat = '<% check_hwnat(); %>';

<% nvram("wan0_ifname,lan_ifname,wl_ifname,wan_proto,web_svg,rstats_enable,rstats_colors"); %>

var cprefix = 'bw_24';
var updateInt = 120;
var updateDiv = updateInt;
var updateMaxL = 720;
var updateReTotal = 1;
var hours = 24;
var lastHours = 0;
var debugTime = 0;

function showHours()
{
	if (hours == lastHours) return;
	showSelectedOption('hr', lastHours, hours);
	lastHours = hours;
}

function switchHours(h)
{
	if ((!svgReady) || (updating)) return;

	hours = h;
	updateMaxL = (720 / 24) * hours;
	showHours();
	loadData();
	cookie.set(cprefix + 'hrs', hours);
}

var ref = new TomatoRefresh('update.cgi', 'output=bandwidth&arg0=speed');

ref.refresh = function(text)
{
	++updating;
	try {
		this.refreshTime = 1500;
		speed_history = {};
		try {
			eval(text);
			if (rstats_busy) {
				E('rbusy').style.display = 'none';
				rstats_busy = 0;
			}
			this.refreshTime = (fixInt(speed_history._next, 1, 120, 60) + 2) * 1000;
		}
		catch (ex) {
			speed_history = {};
		}
		if (debugTime) E('dtime').innerHTML = (new Date()) + ' ' + (this.refreshTime / 1000);
		loadData();
	}
	catch (ex) {
//		alert('ex=' + ex);
	}
	--updating;
}

ref.showState = function()
{
//	E('refresh-button').value = this.running ? 'Stop' : 'Start';
}

ref.toggleX = function()
{
	this.toggle();
	this.showState();
	cookie.set(cprefix + 'refresh', this.running ? 1 : 0);
}

ref.initX = function()
{
	var a;

	a = fixInt(cookie.get(cprefix + 'refresh'), 0, 1, 1);
	if (a) {
		ref.refreshTime = 100;
		ref.toggleX();
	}
}

function init()
{
	if(qos_enabled=="0" && preferred_lang=="JP"){
		$('QoS_disabledesc').style.display="";
	}else{
		$('QoS_disabledesc').style.display="none";
	}	

	if(chk_hwnat=="1" && preferred_lang=="JP"){
		$('HWNAT_disabledesc').style.display="";
	}else{
		$('HWNAT_disabledesc').style.display="none";
	}

	if (nvram.rstats_enable != '1') return;

	try {
	//	<% bandwidth("speed"); %>
	}
	catch (ex) {
	//	speed_history = {};
	}
	rstats_busy = 0;
	if (typeof(speed_history) == 'undefined') {
		speed_history = {};
		rstats_busy = 1;
//		E('rbusy').style.display = '';
	}

	hours = fixInt(cookie.get(cprefix + 'hrs'), 1, 24, 24);
	updateMaxL = (720 / 24) * hours;
	showHours();
	initCommon(1, 0, 0, 1);	   //Viz 2010.09
	ref.initX();
}

function switchPage(page){
	if(page == "1")
		location.href = "/Main_TrafficMonitor_realtime.asp";
	else if(page == "2")
		return false;
	else
		location.href = "/Main_TrafficMonitor_daily.asp";
}

function Zoom(func){
	if (func == "in")
		document.form.zoom.value = parseInt(document.form.zoom.value) - 1;
	else
		document.form.zoom.value = parseInt(document.form.zoom.value) + 1;;
		
	if(document.form.zoom.value == 1)
		switchHours("4");
	else if(document.form.zoom.value == 2)
		switchHours("6");
	else if(document.form.zoom.value == 3)
		switchHours("12");
	else if(document.form.zoom.value == 4)
		switchHours("18");
	else if(document.form.zoom.value == 5)
		switchHours("24");
	else if(document.form.zoom.value > 5)
		document.form.zoom.value = 5;
	else if(document.form.zoom.value < 1)
		document.form.zoom.value = 1;
	else
		return false;
}
</script>
</head>

<body onload="show_banner(0); show_menu(4, -1, 0); show_footer(); init();" >

<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" action="../apply.cgi" >
<input type="hidden" name="current_page" value="Main_TrafficMonitor_last24.asp">
<input type="hidden" name="next_page" value="Main_TrafficMonitor_last24.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="WLANConfig11b;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
<input type="hidden" name="wl_ssid2" value="<% nvram_get_x("WLANConfig11b",  "wl_ssid2"); %>">
<input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
<input type="hidden" name="zoom" value="3">

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="23">&nbsp;</td>

<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
		
    <td valign="top">
	<div id="tabMenu"></div>
<!--===================================Beginning of Main Content===========================================-->
      <table width="98%" border="0" align="center" cellpadding="0" cellspacing="0">
        <div id='rstats'>		
        <tr>
          <td valign="top" >   
			<!--br-->        
            <table width="500" border="0" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTitle">
	      <tr>
		<td bgcolor="#FFFFFF">
		  <table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
        <tr>
        <td>
        <div>
        	<div id="QoS_disabledesc" align="left" style="color:#FF3300;">
        		<#TM_Note1#>
        	</div>
        	<div id="HWNAT_disabledesc" align="left" style="color:#FF3300;">
        		<#TM_Note2#>
        	</div>        	
          <div align="right">
              <select onchange="switchPage(this.options[this.selectedIndex].value)" class="top-input">
								<option><#switchpage#></option>
								<option value="1"><#menu4_2_1#></option>
								<option value="2" selected><#menu4_2_2#></option>
								<option value="3"><#menu4_2_3#></option>
							</select>	
              <input type="button" value="+" onclick="Zoom('in');" class="button" name="button">
              <input type="button" value="-" onclick="Zoom('out');" class="button" name="button">
					</div>
				</div>
				
					<div id="tab-area"></div>
			<!--========= svg =========-->
<!--[if IE]>
	<div id="svg-table" align="left">
	<object id="graph" src="tm.svg" classid="image/svg+xml" width="680" height="300">
	</div>
<![endif]-->
<!--[if !IE]>-->
	<object id="graph" data="tm.svg" type="image/svg+xml" width="680" height="300">
<!--<![endif]-->
</object>
      			<!--========= svg =========-->
      			
        </td>
       </tr>
        	    
  		     <tr>
							<td colspan="2" >
				    	 <table width="100%"   border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
						  	<tr>
						  		<th width='8%' align='center' valign='top' style="text-align:left"><#Network#></th>
						  		<th width='8%' align='center' valign='top'><#Color#></th>
						  		<th width='8%' align='center' valign='top'><#Current#></th>
						  		<th width='8%' align='center' valign='top'><#Average#></th>
						  		<th width='8%' align='center' valign='top'><#Maximum#></th>
						  		<th width='8%' align='center' valign='top'><#Total#></th>
						  	</tr>
						  	<tr>
						  		<td width='8%' align='center' valign='top' style="text-align:left"><#Downlink#></b></td>
						  		<td width='10%' align='center' valign='top'>
						  			<!--  Viz 2010.09
						  			<div align="right">
						  				<b style='border-bottom: 4px solid; display: none;' id='rx-name'>　</b>
										<select id='rx-sel' class="bwm-input" style="background:#FF9000; width: 95%;"  onchange="switchColorRX(this.options[this.selectedIndex].value);">						  					
						  					<option value="0" style="background-color:#FF9000;"></option>
						  					<option value="1" style="background-color:#003EBA;"></option>	
						  					<option value="2" style="background-color:#000000;"></option>
						  					<option value="3" style="background-color:#dd0000;"></option>
						  					<option value="4" style="background-color:#999999;"></option>
						  					<option value="5" style="background-color:#118811;"></option>
              				                       </select> 
              			                         </div>						  								  			
              			                                 -->


<div id='rx-sel'>
<ul id="navigation-1"><b style='border-bottom: 4px solid; display: none;' id='rx-name'></b>
   <li><a  title="Color" style="color:#B7E1F7;"><img src="images/arrow-top.gif" width="12" height="12"/ ></a>
      <ul class="navigation-2">
         <li><a title="Orange" style="background-color:#FF9000;" onclick="switchColorRX(0)"></a></li>
         <li><a title="Blue" style="background-color:#003EBA;" onclick="switchColorRX(1)"></a></li>
         <li><a title="Black" style="background-color:#000000;" onclick="switchColorRX(2)"></a></li>
         <li><a title="Red" style="background-color:#dd0000;" onclick="switchColorRX(3)"></a></li>
         <li><a title="Gray" style="background-color:#999999;" onclick="switchColorRX(4)"></a></li>
         <li><a title="Green" style="background-color:#118811;"onclick="switchColorRX(5)"></a></li>
      </ul>
   </li>
</ul>
</div>
                       

      
              			                         
						  		</td>
						  		<td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='rx-current'></span></td>
						  		<td width='15%' align='center' valign='top' style="text-align:right" id='rx-avg'></td>
						  		<td width='15%' align='center' valign='top' style="text-align:right" id='rx-max'></td>
						  		<td width='15%' align='center' valign='top' style="text-align:right" id='rx-total'></td>
						    </tr>
						    <tr>
						    	<td width='8%' align='center' valign='top' style="text-align:left"><#Uplink#></b></td>
						    	<td width='10%' align='center' valign='top'>
						  	<!--  Viz 2010.09
						  			<div align="right">
						  			<b style='border-bottom: 4px solid; display:none;' id='tx-name'>　			</b>
						  				<select id='tx-sel' class="bwm-input"  style="background:#003EBA; width: 95%;" onchange="switchColorTX(this.options[this.selectedIndex].value)">
						  					<option value="1" style="background-color:#003EBA;"></option>
						  					<option value="2" style="background-color:#000000;"></option>
						  					<option value="3" style="background-color:#dd0000;"></option>
						  					<option value="4" style="background-color:#999999;"></option>
						  					<option value="5" style="background-color:#118811;"></option>
						  					<option value="0" style="background-color:#FF9000;"></option>
              			                            </select>
              			                        </div>						    	
              			      -->                  
<div id='tx-sel'>
<ul id="navigation-1"><b style='border-bottom: 4px solid; display: none;' id='tx-name'></b>
   <li><a  title="Color" style="color:#B7E1F7;"><img src="images/arrow-top.gif" width="12" height="12"/ ></a>
      <ul class="navigation-2">
         <li><a title="Orange" style="background-color:#FF9000;" onclick="switchColorTX(0)"></a></li>
         <li><a title="Blue" style="background-color:#003EBA;" onclick="switchColorTX(1)"></a></li>
         <li><a title="Black" style="background-color:#000000;" onclick="switchColorTX(2)"></a></li>
         <li><a title="Red" style="background-color:#dd0000;" onclick="switchColorTX(3)"></a></li>
         <li><a title="Gray" style="background-color:#999999;" onclick="switchColorTX(4)"></a></li>
         <li><a title="Green" style="background-color:#118811;"onclick="switchColorTX(5)"></a></li>
      </ul>
   </li>
</ul>
</div>              			                            

            		    			
						    	</td>
						  		<td width='15%' align='center' valign='top' style="text-align:right;font-weight: bold;"><span id='tx-current'></span></td>
									<td width='15%' align='center' valign='top' style="text-align:right" id='tx-avg'></td>
									<td width='15%' align='center' valign='top' style="text-align:right" id='tx-max'></td>
									<td width='15%' align='center' valign='top' style="text-align:right" id='tx-total'></td>
								</tr>
							 </table>
							</td>
					</tr>
									</table>
					</td>
				</tr>
			
		<tr style="display:none">
		<td bgcolor="#FFFFFF">
		  <table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
		    <thead>
			<tr>
			  <td colspan="5" id="TriggerList">Display Options</td>
			</tr>
		    </thead>

			<div id='bwm-controls'>
			<tr>
			<th width='50%'><#Traffic_Hours#>:&nbsp;</th>
			<td>
					<a href='javascript:switchHours(4);' id='hr4'>4</a>,
					<a href='javascript:switchHours(6);' id='hr6'>6</a>,
					<a href='javascript:switchHours(12);' id='hr12'>12</a>,
					<a href='javascript:switchHours(18);' id='hr18'>18</a>,
					<a href='javascript:switchHours(24);' id='hr24'>24</a>
			</td>
			</tr>
			<tr>
			<th><#Traffic_Avg#>:&nbsp;</th>
			<td>
					<a href='javascript:switchAvg(1)' id='avg1'>Off</a>,
					<a href='javascript:switchAvg(2)' id='avg2'>2x</a>,
					<a href='javascript:switchAvg(4)' id='avg4'>4x</a>,
					<a href='javascript:switchAvg(6)' id='avg6'>6x</a>,
					<a href='javascript:switchAvg(8)' id='avg8'>8x</a>
			</td>
			</tr>
			<tr>
			<th><#Traffic_Max#>:&nbsp;</th>
			<td>
					<a href='javascript:switchScale(0)' id='scale0'>Uniform</a>,
					<a href='javascript:switchScale(1)' id='scale1'>Per IF</a>
			</td>
			</tr>
			<tr>
			<th><#Traffic_SvgDisp#>:&nbsp;</th>
			<td>
					<a href='javascript:switchDraw(0)' id='draw0'>Solid</a>,
					<a href='javascript:switchDraw(1)' id='draw1'>Line</a>
			</td>
			</tr>
			<tr>
			<th><#Traffic_Color#>:&nbsp; </th>
			<td>
					<a href='javascript:switchColor()' id='drawcolor'> </a><small><a href='javascript:switchColor(1)' id='drawrev'><#Traffic_Reverse#></a></small>
			</td>
			</div>	
						</table>
					</td>
				</tr>
 
				<!--tr>
					<td bgcolor="#FFFFFF">
						<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<tr align="right">
							<td colspan="2">
								 <input type="submit" class="button" onclick="onSubmitCtrl(this, ' Refresh ')" value="<#CTL_refresh#>" name="action" />
							</td>
		 				 	</tr>
						</table>
					</td>
				</tr-->

				</td>
				</tr>
			</table>	
		</td>
	</tr>
			
	</table>				
		</td>
    <td>&nbsp</td>
	</tr>
</table>
<div id="footer"></div>
</body>
</html>


