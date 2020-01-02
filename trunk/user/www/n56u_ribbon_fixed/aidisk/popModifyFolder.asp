<!DOCTYPE html>
<html>
<head>
<title>Rename Folder</title>
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
var selectedFolder = parent.getSelectedFolder();
var folderlist = parent.get_sharedfolder_in_pool(selectedPool);

function initial(){
	showtext($("selected_Pool"), selectedPool);
	showtext($("selected_Folder"), showhtmlspace(showhtmland(selectedFolder)));
	$("new_folder").focus();
	
	clickevent();
}

function clickevent(){
	$("Submit").onclick = function(){
		applyRule();
	};
	$("new_folder").onkeypress = function(ev){
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
	$("new_folder").value = trim($("new_folder").value);
	
	// share name
	if($("new_folder").value.length == 0){
		alert("<#File_content_alert_desc6#>");
		$("new_folder").focus();
		return false;
	}
	
	var re = new RegExp("[^\u4e00-\u9fa5_a-zA-Z0-9 _-]+","gi");
	if(re.test($("new_folder").value)){
		alert("<#File_content_alert_desc7#>");
		$("new_folder").focus();
		return false;
	}
	
	if(parent.checkDuplicateName($("new_folder").value, folderlist)){
		alert("<#File_content_alert_desc8#>");
		$("new_folder").focus();
		return false;
	}
	
	if(trim($("new_folder").value).length > 12)
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
		$("pool").value = selectedPool;
		$("folder").value = selectedFolder;
		
		parent.showLoading();
		document.modifyFolderForm.submit();
		parent.hidePop("apply");
	}
}

</script>
</head>

<body style="background: 0 none;" onLoad="initial();">
<form method="post" name="modifyFolderForm" action="modify_sharedfolder.asp" target="hidden_frame">
    <input type="hidden" name="pool" id="pool" value="">
    <input type="hidden" name="folder" id="folder" value="">
    <table class="table well aidisk_table" cellpadding="0" cellspacing="0">
    <thead>
    <tr>
      <td width="50%">
          <b><#ModFolderTitle#></b>
      </td>
      <td style="text-align: right">
          <a href="javascript:void(0)" onclick="parent.hidePop('OverlayMask');"><i class="icon icon-remove"></i></a>
      </td>
    </tr>
    </thead>
    <tbody>
      <tr>
        <td  colspan="2" height="30"><#ModFolderAlert#></td>
      </tr>
      <tr>
        <th width="50%"><#PoolName#>: </th>
        <td colspan="3"><span id="selected_Pool"></span></td>
      </tr>
      <tr>
        <th><#FolderName#>: </th>
        <td colspan="3"><span id="selected_Folder"></span></td>
      </tr>
      <tr>
        <th><#NewFolderName#>: </th>
        <td><input class="input" type="text" name="new_folder" id="new_folder"></td>
      </tr>
      <tr>
        <th colspan="2" style="text-align: center;"><input id="Submit" type="button" class="btn btn-primary" style="width: 170px;" value="<#CTL_modify#>"></th>
      </tr>
    </tbody>
    </table>
</form>
</body>
</html>
