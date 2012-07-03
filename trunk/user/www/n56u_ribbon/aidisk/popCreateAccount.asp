<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Add New Account</title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="../state.js"></script>
<script type="text/javascript">
function clickevent(){
	$("Submit").onclick = function(){
			if(validForm()){
				/*alert('action = '+document.createAccountForm.action+'\n'+
					  'account = '+$("account").value+'\n'+
					  'password = '+$("password").value
					  );//*/
				
				
				parent.showLoading();
				document.createAccountForm.submit();
				parent.hidePop("apply");
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
</script>
<style>
    .table th, .table td{vertical-align: middle;}
    .table input, .table select {margin-bottom: 0px;}
</style>
</head>

<body style="background: 0 none;" onLoad="clickevent();">
<form method="post" name="createAccountForm" action="create_account.asp" target="hidden_frame">
  <table width="90%" class="table well aidisk_table" cellpadding="0" cellspacing="0">
   <thead>
    <tr>
        <td width="95%">
            <h4><#AddAccountTitle#></h4>
        </td>
        <td style="text-align: right">
            <a href="javascript:void(0)" onclick="parent.hidePop('OverlayMask');"><i class="icon icon-remove"></i></a>
        </td>
      </tr>
	</thead>

    <tr align="center">
      <td height="25" colspan="2"><#AddAccountAlert#></td>
    </tr>
    <tr>
      <th><#AiDisk_Account#>: </th>
      <td><input class="input" name="account" id="account" type="text" maxlength="20"></td>
    </tr>
    <tr>
      <th><#AiDisk_Password#>: </th>
      <td><input class="input" name="password" id="password" type="password" maxlength="20"></td>
    </tr>
    <tr>
      <th><#Confirmpassword#>: </th>
      <td><input class="input" name="confirm_password" id="confirm_password" type="password" maxlength="20"></td>
    </tr>
    <tr>
        <td colspan="2" style="text-align: center"><input id="Submit" type="button" class="btn btn-primary" style="width: 170px;" value="<#CTL_add#>"></td>
    </tr>
  </table>
</form>
</body>
</html>
