<!DOCTYPE html>
<html>
<head>
<title>Add New Folder</title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="../state.js"></script>
<script>
var selectedPool = parent.getSelectedPool();
var folderlist = parent.get_sharedfolder_in_pool(selectedPool);

function initial(){
	showtext($("poolName"), selectedPool);
	$("folder").focus();
	
	clickevent();
}

function clickevent(){
	$("Submit").onclick = function(){
		applyRule();
	};
	$("folder").onkeypress = function(ev){
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
	$("folder").value = trim($("folder").value);
	
	// share name
	if($("folder").value.length == 0){
		alert("<#File_content_alert_desc6#>");
		$("folder").focus();
		return false;
	}
	
	var re = new RegExp("[^\u4e00-\u9fa5_a-zA-Z0-9 _-]+", "gi");
	if(re.test($("folder").value)){
		alert("<#File_content_alert_desc7#>");
		$("folder").focus();
		return false;
	}
	
	if(parent.checkDuplicateName($("folder").value, folderlist)){
		alert("<#File_content_alert_desc8#>");
		$("folder").focus();
		return false;
	}
	
	if(trim($("folder").value).length > 12)
		if (!(confirm("<#File_content_alert_desc10#>")))
			return false;
	
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
		document.createFolderForm.pool.value = selectedPool;
		
		parent.showLoading();
		document.createFolderForm.submit();
		parent.hidePop("apply");
	}
}

</script>
</head>

<body style="background: 0 none;"  onLoad="initial();">
<form name="createFolderForm" method="post" action="create_sharedfolder.asp" target="hidden_frame">
    <input type="hidden" name="pool" id="pool">
    <table width="100%" class="table well aidisk_table" cellpadding="0" cellspacing="0">
    <thead>
    <tr>
        <td>
            <b><#AddFolderTitle#>&nbsp;<#in#>&nbsp;<span id="poolName"></span></b>
        </td>
        <td style="text-align: right">
            <a href="javascript:void(0)" onclick="parent.hidePop('OverlayMask');"><i class="icon icon-remove"></i></a>
        </td>
    </tr>
    </thead>
    <tr>
      <td colspan="2"><#AddFolderAlert#></td>
    </tr>
    <tr>
      <th width="50%"><#FolderName#>: </th>
      <td width="50%"><input class="input" type="text" name="folder" id="folder"></td>
    </tr>
    <tr>
      <th colspan="2" style="text-align: center;"><input id="Submit" type="button" class="btn btn-primary" style="width: 170px;" value="<#CTL_add#>"></th>
    </tr>
  </table>
</form>
</body>
</html>
