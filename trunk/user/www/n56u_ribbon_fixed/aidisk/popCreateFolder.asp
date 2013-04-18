<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<title>Add New Folder</title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">

<script type="text/javascript" src="../state.js"></script>
<script type="text/javascript">
var selectedPool = parent.getSelectedPool();
var folderlist = parent.get_sharedfolder_in_pool(selectedPool);

function initial(){
	showtext($("poolName"), selectedPool);
	
	clickevent();
}

function clickevent(){
	$("Submit").onclick = function(){
			if(validForm()){
				document.createFolderForm.pool.value = selectedPool;
				
				/*alert('action = '+document.createFolderForm.action+'\n'+
					  'pool = '+document.createFolderForm.pool.value+'\n'+
					  'folder = '+document.createFolderForm.folder.value
					  );//*/
				
				parent.showLoading();
				document.createFolderForm.submit();
				parent.hidePop("apply");
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
	
	var re = new RegExp("[^a-zA-Z0-9 _-]+", "gi");
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
</script>
</head>

<body style="background: 0 none;"  onLoad="initial();">
<form name="createFolderForm" method="post" action="create_sharedfolder.asp" target="hidden_frame">
<input type="hidden" name="pool" id="pool">
	<table width="100%" class="table well aidisk_table" cellpadding="0" cellspacing="0">
	<thead>
    <tr>
        <td>
            <b><#AddFolderTitle#> <#in#> <span id="poolName"></span></b>
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
