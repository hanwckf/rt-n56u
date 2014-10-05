<!DOCTYPE html>
<html>
<head>
<title>Add New Account</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="../state.js"></script>
<script>
function initial(){
	$("account").focus();
	
	clickevent();
}

function clickevent(){
	$("account").onkeypress = function(ev){
		var charCode = get_pressed_keycode(ev);
		if (charCode == 13){
			$("password").focus();
			return false;
		} else if (charCode == 27){
			parent.hidePop('OverlayMask');
			return false;
		}
	};
	$("password").onkeypress = function(ev){
		var charCode = get_pressed_keycode(ev);
		if (charCode == 13){
			$("confirm_password").focus();
			return false;
		} else if (charCode == 27){
			parent.hidePop('OverlayMask');
			return false;
		}
	};
	$("confirm_password").onkeypress = function(ev){
		var charCode = get_pressed_keycode(ev);
		if (charCode == 13){
			applyRule();
			return false;
		} else if (charCode == 27){
			parent.hidePop('OverlayMask');
			return false;
		}
	};
}

function validForm(){
	$("account").value = trim($("account").value);
	tempPasswd = trim($("password").value);
	$("confirm_password").value = trim($("confirm_password").value);
	
	// account name
	if($("account").value.length == 0){
		alert("<#File_Pop_content_alert_desc1#>");
		$("account").focus();
		return false;
	}
	
	if($("account").value == "root" || $("account").value == "admin" || $("account").value == "family" || $("account").value == "Family"){
		alert("<#USB_Application_account_alert#>");
		$("account").focus();
		return false;
	}
	
	if(trim($("account").value).length <= 1){
		alert("<#File_Pop_content_alert_desc2#>");
		$("account").focus();
		return false;
	}
	
	if(trim($("account").value).length > 20){
		alert("<#File_Pop_content_alert_desc3#>");
		$("account").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9-]+","gi");
	if(re.test($("account").value)){
		alert("<#File_Pop_content_alert_desc4#>");
		$("account").focus();
		return false;
	}
	
	if(checkDuplicateName($("account").value, parent.get_accounts())){
		alert("<#File_Pop_content_alert_desc5#>");
		$("account").focus();
		return false;
	}
	
	// password
	if(trim(tempPasswd).length <= 0 || trim($("confirm_password").value).length == 0){
		alert("<#File_Pop_content_alert_desc6#>");
		$("password").focus();
		return false;
	}
	
	if(tempPasswd != $("confirm_password").value){
		alert("<#File_Pop_content_alert_desc7#>");
		$("confirm_password").focus();
		return false;
	}
	
	if(tempPasswd.length != $("password").value.length){
		alert("<#File_Pop_content_alert_desc8#>");
		$("password").focus();
		return false;
	}
	
	var re = new RegExp("[^a-zA-Z0-9]+","gi");
	if(re.test($("password").value)){
		alert("<#File_Pop_content_alert_desc9#>");
		$("password").focus();
		return false;
	}
	
	return true;
}

function get_pressed_keycode(ev){
	var charCode = 0;
	if(ev && ev.which){
		charCode = ev.which;
	} else if(window.event){
		ev = window.event;
		charCode = ev.keyCode;
	}
	return charCode;
}

function applyRule(){
	if(validForm()){
		parent.showLoading();
		document.createAccountForm.submit();
		parent.hidePop("apply");
	}
}

</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body style="background: 0 none;" onLoad="initial();">
<form method="post" name="createAccountForm" action="create_account.asp" target="hidden_frame">
  <table width="90%" class="table well aidisk_table" cellpadding="0" cellspacing="0">
    <tr>
        <td width="50%"><h4><#AddAccountTitle#></h4></td>
        <td style="text-align: right"><a href="javascript:void(0)" onclick="parent.hidePop('OverlayMask');"><i class="icon icon-remove"></i></a></td>
    </tr>
    <tr valign="middle">
        <td colspan="2"><#AddAccountAlert#></td>
    </tr>
    <tr>
        <th><#AiDisk_Account#>:</th>
        <td><input class="input" name="account" id="account" type="text" maxlength="20" style="width: 150px;"></td>
    </tr>
    <tr>
        <th><#AiDisk_Password#>:</th>
        <td><input class="input" name="password" id="password" type="password" maxlength="20" style="width: 150px;"></td>
    </tr>
    <tr>
        <th><#Confirmpassword#>:</th>
        <td><input class="input" name="confirm_password" id="confirm_password" type="password" maxlength="20" style="width: 150px;"></td>
    </tr>
    <tr>
        <td colspan="2" style="text-align: center"><input name="button" type="button" class="btn btn-primary" style="width: 170px;" onclick="applyRule();" value="<#CTL_add#>"></td>
    </tr>
  </table>
</form>
</body>
</html>
