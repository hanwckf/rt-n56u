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
<script>
var next_page = "";
var dummyShareway = 0;
var ddns_return_code = '<% nvram_get_ddns("", "ddns_return_code"); %>';

function initial(){
	parent.hideLoading();
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

<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}

    table.inside {margin-top: 20px;}
    table.inside th{border-top: 0 none;}

    td.steps .badge {font-size: 300%; margin-left: 10px; padding: 2px 15px 3px 15px; border-radius: 26px;}

    .controls {padding-left:  15px;}
</style>

</head>

<body class="body_iframe" onload="initial();">
<form method="GET" name="smartForm" id="smartForm" action="Aidisk-3.asp">
<input type="hidden" name="accountNum" id="accountNum" value="">
<input type="hidden" name="account0" id="account0" value="">
<input type="hidden" name="passwd0" id="passwd0" value="">
<input type="hidden" name="permission0" id="permission0" value="">
<input type="hidden" name="account1" id="account1" value="">
<input type="hidden" name="passwd1" id="passwd1" value="">
<input type="hidden" name="permission1" id="permission1" value="">
</form>

<table width="100%" class="table table-condensed" style="margin-top: 35px;" cellpadding="0" cellspacing="0" >
    <tr>
        <td colspan="2" style="border-top: 0 none; text-align: center" class="steps">
            <span class="badge badge-info">1</span>
            <span class="badge" style="margin-left: 30px;">2</span>
            <span class="badge" style="margin-left: 30px;">3</span>
        </td>
    </tr>
    <tr>
        <td colspan="2" style="border-top: 0 none; padding-top: 25px;"><div class="alert alert-info" style="margin-top: 10px;"><#Step1_desp#></div></td>
    </tr>

    <tr>
        <td colspan="2" class="textbox">
            <p style="padding-left:  15px;"><#Step2_method#></p>

            <div class="controls">
                <label class="inline radio"><input type="radio" id="d1" name="dummyoption" value="0" onclick="showTextinWizard(this.value);"/><#Step2_method1#></label>
            </div>
            <div class="controls">
                <label class="inline radio"><input type="radio" id="d2" name="dummyoption" value="1" onclick="showTextinWizard(this.value);"/><#Step2_method2#></label>
            </div>
            <div class="controls">
                <label class="inline radio"><input type="radio" id="d3" name="dummyoption" value="2" onclick="showTextinWizard(this.value);"/><#Step2_method3#></label>
            </div>

            <div id="share1">
                <table width="100%" align="center" cellpadding="2" cellspacing="0" class="table inside" >
                    <tr>
                        <th width="100"><#AiDisk_Account#></th>
                        <th><#AiDisk_Password#></th>
                        <th width="50" style="text-align: center"><#AiDisk_Read#></th>
                        <th width="50" style="text-align: center"><#AiDisk_Write#></th>
                    </tr>

                    <tr id="target1">
                        <td><span id="user1"></span></td>
                        <td><input type="text" name="userpasswd1" id="userpasswd1" value="" class="inputtext"></td>
                        <td style="text-align: center"><img src="/images/checked.gif"></td>
                        <td style="text-align: center"><img src="/images/checked.gif"></td>
                    </tr>

                    <tr id="target2">
                        <td><span id="user2"></span></td>
                        <td><input type="text" name="userpasswd2" id="userpasswd2" value="" class="inputtext"></td>
                        <td style="text-align: center"><img src="/images/checked.gif"></td>
                        <td style="text-align: center">&nbsp;</td>
                    </tr>
                </table>
            </div>
        </td>
    </tr>
    <tr>
        <td width="50%" style="text-align: right; border-top: 1px dashed #ddd">
            <a class="btn" style="width: 170px;" href="javascript:go_pre_page();"><#btn_pre#></a>
        </td>
        <td style="border-top: 1px dashed #ddd">
            <a class="btn btn-primary" style="width: 170px;" href="javascript:passTheResult();"><#btn_next#></a>
        </td>
    </tr>
</table>

</body>
</html>
