<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>AiDisk Wizard</title>
<link rel="stylesheet" type="text/css" href="aidisk.css">
<script type="text/javascript" src="/state.js"></script>
<script>
var next_page = "";
var dummyShareway = 0;
var ddns_return_code = '<% nvram_get_ddns("LANHostConfig", "ddns_return_code"); %>';

function initial(){
	//parent.show_help_iframe(2);
	parent.hideLoading();
	
	parent.restore_help_td();	
	parent.openHint(15, 2);
	
	parent.$("dummyShareway").value = "<% nvram_get_x("", "dummyShareway"); %>";
	if(parent.$("dummyShareway").value == "")
		parent.$("dummyShareway").value = 0;
	showTextinWizard(parent.$("dummyShareway").value);
}

function showTextinWizard(flag){
	dummyShareway = flag;
	
	if(dummyShareway == 0){
		parent.$("dummyShareway").value = dummyShareway;
		
		document.getElementsByName('dummyoption')[dummyShareway].focus();
		document.getElementsByName('dummyoption')[dummyShareway].checked = true;
		
		$("share1").style.display = "none";				
		$("target1").style.display = "none";
		$("target2").style.display = "none";
	}
	else if(dummyShareway == 1){
		parent.$("dummyShareway").value = dummyShareway;
		
		document.getElementsByName('dummyoption')[dummyShareway].focus();
		document.getElementsByName('dummyoption')[dummyShareway].checked = true;
		
		/*showtext($("user1"), "Admin");
		$("userpasswd1").value =  "Admin";//*/
		showtext($("user1"), "admin");
		$("userpasswd1").value =  "admin";//*/
		
		showtext($("user2"), "Family");
		$("userpasswd2").value =  "Family";

		$("share1").style.display = "block";		
		$("target1").style.display = "";
		$("target2").style.display = "";
	}
	else if(dummyShareway == 2){
		parent.$("dummyShareway").value = dummyShareway;
		
		document.getElementsByName('dummyoption')[dummyShareway].focus();
		document.getElementsByName('dummyoption')[dummyShareway].checked = true;
		
		/*showtext($("user1"), "Admin");
		$("userpasswd1").value =  "Admin";//*/
		showtext($("user1"), "admin");
		$("userpasswd1").value =  "admin";//*/

		$("share1").style.display = "";		
		$("target1").style.display = "";
		$("target2").style.display = "none";
	}
	else
		alert("System error: No this choice");	// no translate*/
}

function passTheResult(){
	if(dummyShareway == 0){
		parent.$("accountNum").value = 0;
		
		parent.$("account0").value = "";
		parent.$("passwd0").value = "";
		parent.$("permission0").value = "";
		
		parent.$("account1").value = "";
		parent.$("passwd1").value = "";
		parent.$("permission1").value = "";
	}
	else if(dummyShareway == 1){
		parent.$("accountNum").value = 2;
		
		if(checkPasswdValid($("userpasswd1").value)){
			parent.$("account0").value = $("user1").firstChild.nodeValue;
			parent.$("passwd0").value = $("userpasswd1").value;
			parent.$("permission0").value = "3";
		}
		else{
			$("userpasswd1").focus();
			return;
		}
		
		if(checkPasswdValid($("userpasswd2").value)){
			parent.$("account1").value = $("user2").firstChild.nodeValue;
			parent.$("passwd1").value = $("userpasswd2").value;
			parent.$("permission1").value = "1";
		}
		else{
			$("userpasswd2").focus();
			return;
		}
	}
	else if(dummyShareway == 2){
		parent.$("accountNum").value = 1;
		
		if(checkPasswdValid($("userpasswd1").value)){
			parent.$("account0").value = $("user1").firstChild.nodeValue;
			parent.$("passwd0").value = $("userpasswd1").value;
			parent.$("permission0").value = "3";
		}
		else{
			$("userpasswd1").focus();
			return;
		}
		
		parent.$("account1").value = "";
		parent.$("passwd1").value = "";
		parent.$("permission1").value = "";
	}
	
	document.smartForm.action = "/aidisk/Aidisk-3.asp";
	document.smartForm.submit();
}

function go_pre_page(){
	document.smartForm.action = "/aidisk/Aidisk-1.asp";
	document.smartForm.submit();
}

function checkPasswdValid(passwd){
	var tempPasswd = trim(passwd);
	
	// password
	if(tempPasswd.length != passwd.length){
		alert("<#File_Pop_content_alert_desc8#>");
		
		return false;
	}
	
	if(trim(tempPasswd).length <= 0){
		alert("<#File_Pop_content_alert_desc6#>");
		
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9]+","gi");
	if(re.test(tempPasswd)){
		alert("<#File_Pop_content_alert_desc9#>");
		
		return false;
	}
	
	return true;
}
</script>
</head>

<body onload="initial();">
<form method="GET" name="smartForm" id="smartForm" action="Aidisk-3.asp">
<input type="hidden" name="accountNum" id="accountNum" value="">
<input type="hidden" name="account0" id="account0" value="">
<input type="hidden" name="passwd0" id="passwd0" value="">
<input type="hidden" name="permission0" id="permission0" value="">
<input type="hidden" name="account1" id="account1" value="">
<input type="hidden" name="passwd1" id="passwd1" value="">
<input type="hidden" name="permission1" id="permission1" value="">
</form>

<table width="400" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td width="100%" >
      <table width="95%" border="0" align="center" cellpadding="0" cellspacing="0" >
        <tr>
          <td colspan="2" valign="top">
          	<table width="250" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="64"><img src="../images/aidisk-01_r.gif" width="64" height="59" /></td>
                <td width="29"><img src="../images/aidisk-arrow-1.gif" width="29" height="29" /></td>
                <td width="64"><img src="../images/aidisk-02.gif" width="64" height="59" /></td>
                <td width="29"><img src="../images/aidisk-arrow-2.gif" width="29" height="29" /></td>
                <td><img src="../images/aidisk-03.gif" width="64" height="59" /></td>
              </tr>
            </table>
          </td>
        </tr>
        <tr>
          <td colspan="2" class="title"><#Step1_desp#></td>
        </tr>
        
        <tr>
          <td colspan="2" valign="top" class="textbox">
            <p><#Step2_method#></p>
            <p><input type="radio" id="d1" name="dummyoption" value="0" onclick="showTextinWizard(this.value);"/> 
            	    <label for="d1"><#Step2_method1#></p></label>
            <p><input type="radio" id="d2" name="dummyoption" value="1" onclick="showTextinWizard(this.value);"/> 
            	    <label for="d2"><#Step2_method2#></p></label>
            <p><input type="radio" id="d3" name="dummyoption" value="2" onclick="showTextinWizard(this.value);"/> 
            	    <label for="d3"><#Step2_method3#></p></label>
            <div id="share1">
              <table width="95%" border="1" align="center" cellpadding="2" cellspacing="0" bordercolor="#7ea7bd" class="table1px" >
                <tr>
                  <th width="100"><#AiDisk_Account#></th>
                  <th><#AiDisk_Password#></th>
                  <th width="50" ><#AiDisk_Read#></th>
                  <th width="50" ><#AiDisk_Write#></th>
                </tr>
                
                <tr id="target1">
                  <td><span id="user1"></span></td>
                  <td><input type="text" name="userpasswd1" id="userpasswd1" value="" class="inputtext"></td>
                  <td align="center"><img src="/images/checked.gif"></td>
                  <td align="center"><img src="/images/checked.gif"></td>
                </tr>
                
                <tr id="target2">
                  <td><span id="user2"></span></td>
                  <td><input type="text" name="userpasswd2" id="userpasswd2" value="" class="inputtext"></td>
                  <td align="center"><img src="/images/checked.gif"></td>
                  <td align="center">&nbsp;</td>
                </tr>
              </table>  
          </div>          </td>
        </tr>
        
        <tr valign="middle">
          <td height="50" align="right">
        	<div class="short_btn">
        	  <a href="javascript:go_pre_page();"><#btn_pre#></a>
        	</div>
          </td>
          <td>
        	<div class="short_btn">
        	  <a href="javascript:passTheResult();"><#btn_next#></a>
        	</div>
          </td>
        </tr>
      </table>
    </td>
    <td width="20">&nbsp;</td>
  </tr>
</table>
</body>
</html>
