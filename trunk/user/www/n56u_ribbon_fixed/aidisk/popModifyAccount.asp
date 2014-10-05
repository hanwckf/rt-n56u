<!DOCTYPE html>
<html>
<head>
<title>Mod New Account</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="../state.js"></script>
<script>
var selectedAccount = parent.getSelectedAccount();

function initial(){
	$("new_account").value = selectedAccount;
	
	$("new_account").focus();
	
	clickevent();
}

function clickevent(){
	$("new_account").onkeypress = function(ev){
		var charCode = get_pressed_keycode(ev);
		if (charCode == 13){
			$("new_password").focus();
			return false;
		} else if (charCode == 27){
			parent.hidePop('OverlayMask');
			return false;
		}
	};
	$("new_password").onkeypress = function(ev){
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
	if($("new_account").value.length > 0)
		$("new_account").value = trim($("new_account").value);
	if($("new_password").value.length > 0)
		$("new_password").value = trim($("new_password").value);
	$("confirm_password").value = trim($("confirm_password").value);
	
	// new_account name
	if($("new_account").value.length > 0){
		if(trim($("new_account").value).length > 20){
			alert("<#File_Pop_content_alert_desc3#>");
			$("new_account").focus();
			return false;
		}
		
		var re = new RegExp("[^a-zA-Z0-9-]+","gi");
		if(re.test($("new_account").value)){
			alert("<#File_Pop_content_alert_desc4#>");
			$("new_account").focus();
			return false;
		}
		
		if(checkDuplicateName($("new_account").value, parent.get_accounts()) &&
				$("new_account").value != selectedAccount){
			alert("<#File_Pop_content_alert_desc5#>");
			$("new_account").focus();
			return false;
		}
	}
	
	// password
	if($("new_password").value != $("confirm_password").value){
		alert("<#File_Pop_content_alert_desc7#>");
		
		if($("new_password").value.length <= 0)
			$("new_password").focus();
		else
			$("confirm_password").focus();
		return false;
	}
	
	if($("new_account").value.length <= 0 && $("new_password").value.length <= 0){
		return false;
	}
	
	return true;
}

function checkDuplicateName(newname, teststr){
	var existing_string = teststr.join(',');
	existing_string = "," + existing_string + ",";
	var newstr = "," + trim(newname) + ","; 

	var re = new RegExp(newstr,"gi")
	var matchArray =  existing_string.match(re);
	if (matchArray != null)
		return true;
	else
		return false;
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
		$("account").value = selectedAccount;
		
		parent.showLoading();
		document.modifyAccountForm.submit();
		parent.hidePop("apply");
	}
}

</script>
</head>

<body style="background: 0 none;" onLoad="initial();">
<form method="post" name="modifyAccountForm" action="modify_account.asp" target="hidden_frame">
  <input name="account" id="account" type="hidden" value="">
  <table width="90%" class="table well aidisk_table" cellpadding="0" cellspacing="0">
    <tr>
        <td width="50%"><h4><#ModAccountTitle#></h4></td>
        <td style="text-align: right"><a href="javascript:void(0)" onclick="parent.hidePop('OverlayMask');"><i class="icon icon-remove"></i></a></td>
    </tr>
    <tr valign="middle">
        <td colspan="2"><#ModAccountAlert#></td>
    </tr>
    <tr>
        <th><#AiDisk_Account#>:</th>
        <td ><input class="input" name="new_account" id="new_account" type="text" maxlength="20" style="width: 150px;"></td>
    </tr>
    <tr>
        <th><#ModAccountPassword#>:</th>
        <td><input class="input" name="new_password" id="new_password" type="password" maxlength="20" style="width: 150px;"></td>
    </tr>
    <tr>
        <th><#Confirmpassword#>: </th>
        <td><input class="input" name="confirm_password" id="confirm_password" maxlength="20" type="password" style="width: 150px;"></td>
    </tr>
    <tr>
        <th colspan="2" style="text-align: center"><input name="button" type="button" class="btn btn-primary" onclick="applyRule();" value="<#CTL_modify#>"></th>
    </tr>
  </table>
</form>
</body>
</html>
