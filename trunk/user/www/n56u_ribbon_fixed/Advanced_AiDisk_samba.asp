<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#> - <#menu5_4_1#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.min.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/disk_folder_tree.js"></script>
<script>
var $j = jQuery.noConflict();

<% disk_pool_mapping_info(); %>
<% available_disk_names_and_sizes(); %>

<% get_AiDisk_status(); %>
<% get_permissions_of_account("cifs"); %>

var PROTOCOL = "cifs";

var NN_status = get_cifs_status();
var AM_to_cifs = get_share_management_status("cifs");

var accounts = [<% get_all_accounts("cifs"); %>];

var lastClickedAccount = 0;
var selectedAccount = "";

var changedPermissions = new Array();

var folderlist = new Array();

function initial(){
	show_banner(1);
	show_menu(5,6,2);
	show_footer();

	// show page's control
	showShareStatusControl();
	showAccountControl();

	// show accounts
	showAccountMenu();

	// show the kinds of permission
	showPermissionTitle();

	// the click event of the buttons
	onEvent();

	// show folder's tree
	setTimeout('get_disk_tree();', 1000);
}

function submitRules(){
	document.aidiskForm.submit();
}

function get_accounts(){
	return this.accounts;
}

function showShareStatusControl(){
	if (this.NN_status == 1){
		$("tableMask").style.width = "0px";
	}
	else{
		$("tableMask").style.width = "500px";
	}
}

function showAccountMenu(){
	var account_menu_code = "";
	
	if(this.accounts.length <= 0)
		account_menu_code += '<div class="noAccount" id="noAccount"><#Noaccount#></div>\n'
	else
		for(var i = 0; i < this.accounts.length; ++i){
			account_menu_code += '<div class="accountName" id="';
			account_menu_code += "account"+i;
			account_menu_code += '" onClick="setSelectAccount(this);">'
			account_menu_code += this.accounts[i];
			account_menu_code += '</div>\n';
		}
	
	$("account_menu").innerHTML = account_menu_code;
	
	if(this.accounts.length > 0){
		if(AM_to_cifs == 2 || AM_to_cifs == 4)
			setSelectAccount($("account0"));
	}
}

function showAccountControl(){
	switch(this.AM_to_cifs){
		case 1:
		case 3:
			$("accountMask").style.display = "block";
			break;
		case 2:
		case 4:
			$("accountMask").style.display = "none";
			break;
	}
}

function showPermissionTitle(){
	var code = "";
	code += '<table border="0"><tr>';
	code += '<td width="40" valign="top" style="text-align: center; padding-top: 0px; padding-bottom: 0px;"><span class="label label-info">R/W</span></td>';
	code += '<td width="40" valign="top" style="text-align: center; padding-top: 0px; padding-bottom: 0px;"><span class="label label-info">R</span></td>';
	code += '<td width="40" valign="top" style="text-align: center; padding-top: 0px; padding-bottom: 0px;"><span class="label label-info">No</span></td>';
	code += '</tr></table>';
	$("permissionTitle").innerHTML = code;
}

var controlApplyBtn = 0;
function showApplyBtn(){
	if(this.controlApplyBtn == 3)
		$("changePermissionBtn").disabled = 0;
	else
		$("changePermissionBtn").disabled = 1;
}

function setSelectAccount(selectedObj){
	this.selectedAccount = selectedObj.firstChild.nodeValue;
	
	if(this.controlApplyBtn == 0 || this.controlApplyBtn == 2)
		this.controlApplyBtn += 1;
	showApplyBtn();
	onEvent();
	
	show_permissions_of_account(selectedObj, PROTOCOL);
	contrastSelectAccount(selectedObj);
}

function getSelectedAccount(){
	return this.selectedAccount;
}

function show_permissions_of_account(selectedObj, protocol){
	var accountName = selectedObj.firstChild.nodeValue;
	var poolName;
	var permissions;
	
	for(var i = 0; i < pool_names().length; ++i){
		poolName = pool_names()[i];
		if(!this.clickedFolderBarCode[poolName])
			continue;
		
		permissions = get_account_permissions_in_pool(accountName, poolName);
		for(var j = 0; j < permissions.length; ++j){
			var folderBarCode = get_folderBarCode_in_pool(poolName, permissions[j][0]);
			
			if(protocol == "cifs")
				showPermissionRadio(folderBarCode, permissions[j][1]);
			else if(protocol == "ftp")
				showPermissionRadio(folderBarCode, permissions[j][2]);
			else{
				alert("Wrong protocol when get permission!");	// system error msg. must not be translate
				return;
			}
		}
	}
}

function get_permission_of_folder(accountName, poolName, folderName, protocol){
	var permissions = get_account_permissions_in_pool(accountName, poolName);
	
	for(var i = 0; i < permissions.length; ++i)
		if(permissions[i][0] == folderName){
			if(protocol == "cifs")
				return permissions[i][1];
			else if(protocol == "ftp")
				return permissions[i][2];
			else{
				alert("Wrong protocol when get permission!");	// system error msg. must not be translate
				return;
			}
		}
	
	alert("Wrong folderName when get permission!");	// system error msg. must not be translate
}

function contrastSelectAccount(selectedObj){
	if(this.lastClickedAccount != 0){
		this.lastClickedAccount.style.marginRight = "0px";
		this.lastClickedAccount.style.background = "url(/images/AiDisk/user_icon0.gif) #F8F8F8 left no-repeat";
		this.lastClickedAccount.style.cursor = "pointer";
		this.lastClickedAccount.style.fontWeight ="normal";
	}
	
	//selectedObj.style.marginRight = "-1px";
	selectedObj.style.background = "url(/images/AiDisk/user_icon.gif) #FFF left no-repeat";
	selectedObj.style.cursor = "default";
	selectedObj.style.paddingLeft = "20px";
	
	this.lastClickedAccount = selectedObj;
}

function submitChangePermission(protocol){
	var orig_permission;
	
	for(var i = 0; i < accounts.length; ++i){
		if(!this.changedPermissions[accounts[i]])
			continue;
		
		for(var j = 0; j < pool_names().length; ++j){
			if(!this.changedPermissions[accounts[i]][pool_names()[j]])
				continue;
			
			folderlist = get_sharedfolder_in_pool(pool_names()[j]);
			
			for(var k = 0; k < folderlist.length; ++k){
				if(!this.changedPermissions[accounts[i]][pool_names()[j]][folderlist[k]])
					continue;
				
				orig_permission = get_permission_of_folder(accounts[i], pool_names()[j], folderlist[k], PROTOCOL);
				if(this.changedPermissions[accounts[i]][pool_names()[j]][folderlist[k]] == orig_permission)
					continue;
				
				// the item which was set already
				if(this.changedPermissions[accounts[i]][pool_names()[j]][folderlist[k]] == -1)
					continue;
				
				document.aidiskForm.action = "/aidisk/set_account_permission.asp";
				$("account").value = accounts[i];
				$("pool").value = pool_names()[j];
				$("folder").value = folderlist[k];
				$("protocol").value = protocol;
				$("permission").value = this.changedPermissions[accounts[i]][pool_names()[j]][folderlist[k]];
				
				// mark this item which is set
				this.changedPermissions[accounts[i]][pool_names()[j]][folderlist[k]] = -1;
				showLoading();
				document.aidiskForm.submit_fake.click();
				return;
			}
		}
	}
	
	refreshpage();
}

function changeActionButton(selectedObj, type, action, flag){
	if(type == "User")
		if(this.accounts.length <= 0)
			if(action == "Del" || action == "Mod")
				return;
	
	if(typeof(flag) == "number")
		selectedObj.src = '/images/AiDisk/'+type+action+'_'+flag+'.gif';
	else
		selectedObj.src = '/images/AiDisk/'+type+action+'.gif';
}

function resultOfCreateAccount(){
	refreshpage();
}

function onEvent(){
	// account action buttons
	if((AM_to_cifs == 2 || AM_to_cifs == 4) && accounts.length < 50){
		changeActionButton($("createAccountBtn"), 'User', 'Add', 0);
		
		$("createAccountBtn").onclick = function(){
				popupWindow('OverlayMask','/aidisk/popCreateAccount.asp');
			};
		$("createAccountBtn").onmouseover = function(){
				changeActionButton(this, 'User', 'Add', 1);
			};
		$("createAccountBtn").onmouseout = function(){
				changeActionButton(this, 'User', 'Add', 0);
			};
	}
	else{
		changeActionButton($("createAccountBtn"), 'User', 'Add');		
		$("createAccountBtn").onclick = function(){};
		$("createAccountBtn").onmouseover = function(){};
		$("createAccountBtn").onmouseout = function(){};
		$("createAccountBtn").title = (accounts.length < 50)?"<#AddAccountTitle#>":"<#account_overflow#>";
	}
	
	if(this.accounts.length > 0 && this.selectedAccount.length > 0){
		changeActionButton($("deleteAccountBtn"), 'User', 'Del', 0);
		changeActionButton($("modifyAccountBtn"), 'User', 'Mod', 0);
		
		$("deleteAccountBtn").onclick = function(){
				if(!selectedAccount){
					alert("No chosen account!");
					return;
				}
				
				popupWindow('OverlayMask','/aidisk/popDeleteAccount.asp');
			};
		$("deleteAccountBtn").onmouseover = function(){
				changeActionButton(this, 'User', 'Del', 1);
			};
		$("deleteAccountBtn").onmouseout = function(){
				changeActionButton(this, 'User', 'Del', 0);
			};
		
		$("modifyAccountBtn").onclick = function(){
				if(!selectedAccount){
					alert("No chosen account!");
					return;
				}
				
				popupWindow('OverlayMask','/aidisk/popModifyAccount.asp');
			};
		$("modifyAccountBtn").onmouseover = function(){
				changeActionButton(this, 'User', 'Mod', 1);
			};
		$("modifyAccountBtn").onmouseout = function(){
				changeActionButton(this, 'User', 'Mod', 0);
			};
	}
	else{
		changeActionButton($("deleteAccountBtn"), 'User', 'Del');
		changeActionButton($("modifyAccountBtn"), 'User', 'Mod');
		
		$("deleteAccountBtn").onclick = function(){};
		$("deleteAccountBtn").onmouseover = function(){};
		$("deleteAccountBtn").onmouseout = function(){};
		
		$("modifyAccountBtn").onclick = function(){};
		$("modifyAccountBtn").onmouseover = function(){};
		$("modifyAccountBtn").onmouseout = function(){};
	}
	
	// folder action buttons
	if(this.selectedPool.length > 0){
		changeActionButton($("createFolderBtn"), 'Folder', 'Add', 0);
		
		$("createFolderBtn").onclick = function(){
				if(!selectedDisk){
					alert("No chosen Disk for creating the shared-folder!");
					return;
				}
				if(!selectedPool){
					alert("No chosen Partition for creating the shared-folder!");
					return;
				}
				
				popupWindow('OverlayMask','/aidisk/popCreateFolder.asp');
			};
		$("createFolderBtn").onmouseover = function(){
				changeActionButton(this, 'Folder', 'Add', 1);
			};
		$("createFolderBtn").onmouseout = function(){
				changeActionButton(this, 'Folder', 'Add', 0);
			};
	}
	else{
		changeActionButton($("createFolderBtn"), 'Folder', 'Add');
		
		$("createFolderBtn").onclick = function(){};
		$("createFolderBtn").onmouseover = function(){};
		$("createFolderBtn").onmouseout = function(){};
	}
	
	if(this.selectedFolder.length > 0){
		changeActionButton($("deleteFolderBtn"), 'Folder', 'Del', 0);
		changeActionButton($("modifyFolderBtn"), 'Folder', 'Mod', 0);
		
		$("deleteFolderBtn").onclick = function(){
				if(!selectedFolder){
					alert("No chosen folder!");
					return;
				}
				
				popupWindow('OverlayMask','/aidisk/popDeleteFolder.asp');
			};
		$("deleteFolderBtn").onmouseover = function(){
				changeActionButton(this, 'Folder', 'Del', 1);
			};
		$("deleteFolderBtn").onmouseout = function(){
				changeActionButton(this, 'Folder', 'Del', 0);
			};
		
		$("modifyFolderBtn").onclick = function(){
				if(!selectedFolder){
					alert("No chosen folder!");
					return;
				}
				
				popupWindow('OverlayMask','/aidisk/popModifyFolder.asp');
			};
		$("modifyFolderBtn").onmouseover = function(){
				changeActionButton(this, 'Folder', 'Mod', 1);
			};
		$("modifyFolderBtn").onmouseout = function(){
				changeActionButton(this, 'Folder', 'Mod', 0);
			};
	}
	else{
		changeActionButton($("deleteFolderBtn"), 'Folder', 'Del');
		changeActionButton($("modifyFolderBtn"), 'Folder', 'Mod');
		
		$("deleteFolderBtn").onclick = function(){};
		$("deleteFolderBtn").onmouseover = function(){};
		$("deleteFolderBtn").onmouseout = function(){};
		
		$("modifyFolderBtn").onclick = function(){};
		$("modifyFolderBtn").onmouseover = function(){};
		$("modifyFolderBtn").onmouseout = function(){};
	}
	
	$("changePermissionBtn").onclick = function(){
			submitChangePermission(PROTOCOL);
		};
}

function unload_body(){
	$("createAccountBtn").onclick = function(){};
	$("createAccountBtn").onmouseover = function(){};
	$("createAccountBtn").onmouseout = function(){};
	
	$("deleteAccountBtn").onclick = function(){};
	$("deleteAccountBtn").onmouseover = function(){};
	$("deleteAccountBtn").onmouseout = function(){};
	
	$("modifyAccountBtn").onclick = function(){};
	$("modifyAccountBtn").onmouseover = function(){};
	$("modifyAccountBtn").onmouseout = function(){};
	
	$("createFolderBtn").onclick = function(){};
	$("createFolderBtn").onmouseover = function(){};
	$("createFolderBtn").onmouseout = function(){};
	
	$("deleteFolderBtn").onclick = function(){};
	$("deleteFolderBtn").onmouseover = function(){};
	$("deleteFolderBtn").onmouseout = function(){};
	
	$("modifyFolderBtn").onclick = function(){};
	$("modifyFolderBtn").onmouseover = function(){};
	$("modifyFolderBtn").onmouseout = function(){};
}
</script>
<style>
    .FdTemp th,
    .FdTemp td,
    .t-border-top-none th,
    .t-border-top-none td
    {border-top: 0 none; margin-bottom: 0px; vertical-align: top;}

    .FdTemp table td {padding: 0px 0px 0px 0px;}
    .nav-tabs > li > a {
          padding-right: 6px;
          padding-left: 6px;
    }
</style>
</head>

<body onLoad="initial();" onunload="unload_body();">

<div class="wrapper">
    <div class="container-fluid" style="padding-right: 0px">
        <div class="row-fluid">
            <div class="span3"><center><div id="logo"></div></center></div>
            <div class="span9" >
                <div id="TopBanner"></div>
            </div>
        </div>
    </div>

    <div id="Loading" class="popup_bg"></div>

    <iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>

    <form method="post" name="aidiskForm" action="" target="hidden_frame">
    <input type="hidden" name="motion" id="motion" value="">
    <input type="hidden" name="layer_order" id="layer_order" value="">
    <input type="hidden" name="protocol" id="protocol" value="">
    <input type="hidden" name="mode" id="mode" value="">
    <input type="hidden" name="flag" id="flag" value="">
    <input type="hidden" name="account" id="account" value="">
    <input type="hidden" name="pool" id="pool" value="">
    <input type="hidden" name="folder" id="folder" value="">
    <input type="hidden" name="permission" id="permission" value="">
    <input type="hidden" name="submit_fake" value="" onclick="submitRules();">

    <div class="container-fluid">
        <div class="row-fluid">
            <div class="span3">
                <!--Sidebar content-->
                <!--=====Beginning of Main Menu=====-->
                <div class="well sidebar-nav side_nav" style="padding: 0px;">
                    <ul id="mainMenu" class="clearfix"></ul>
                    <ul class="clearfix">
                        <li>
                            <div id="subMenu" class="accordion"></div>
                        </li>
                    </ul>
                </div>
            </div>

            <div class="span9">
                <!--Body content-->
                <div class="row-fluid">
                    <div class="span12">
                        <div class="box well grad_colour_dark_blue">
                            <h2 class="box_head round_top"><#menu5_4#> - <#menu5_4_1#></h2>
                            <div class="round_bottom">
                                <div class="row-fluid">
                                    <div id="tabMenu" class="submenuBlock"></div>
                                    <!--table cellpadding="4" cellspacing="0" class="table" style="margin-bottom: 0px;">
                                    </table-->

                                    <!-- The table of share. -->
                                    <div id="shareStatus">
                                        <!-- The mask of all share table. -->
                                        <div id="tableMask"></div>

                                        <!-- The mask of accounts. -->
                                        <div id="accountMask"></div>

                                        <!-- The action buttons of accounts and folders. -->
                                        <table cellpadding="2" cellspacing="0" class="table t-border-top-none" style="margin-bottom: 0px; border-top: 0 none;">
                                            <tr>
                                                <!-- The action buttons of accounts. -->
                                                <td width="35%" style="vertical-align: top;">
                                                    <img id="createAccountBtn" src="/images/AiDisk/UserAdd.gif" hspace="1" title="<#AddAccountTitle#>">
                                                    <img id="deleteAccountBtn" src="/images/AiDisk/UserDel.gif" hspace="1" title="<#DelAccountTitle#>">
                                                    <img id="modifyAccountBtn" src="/images/AiDisk/UserMod.gif" hspace="1" title="<#ModAccountTitle#>">
                                                </td>

                                                <!-- The action buttons of folders. -->
                                                <td>
                                                    <img id="createFolderBtn" src="/images/AiDisk/FolderAdd.gif" hspace="1" title="<#AddFolderTitle#>">
                                                    <img id="deleteFolderBtn" src="/images/AiDisk/FolderDel.gif" hspace="1" title="<#DelFolderTitle#>">
                                                    <img id="modifyFolderBtn" src="/images/AiDisk/FolderMod.gif" hspace="1" title="<#ModFolderTitle#>">
                                                </td>
                                            </tr>
                                        </table>
                                    </div>
                                    <!-- The table of accounts and folders. -->
                                    <table cellpadding="0" cellspacing="1" class="table" style="height: 500px;">
                                        <tr>
                                            <!-- The table of accounts. -->
                                            <td width="35%" style="vertical-align: top;">
                                                <div id="account_menu"></div>
                                            </td>

                                            <!-- The table of folders. -->
                                            <td style="vertical-align: top;">
                                                <table cellspacing="0" cellpadding="0" class="table t-border-top-none" style="margin-bottom: 0px; border-top: 0 none;">
                                                    <tr>
                                                        <td width="220" align="left" style="padding-left: 0px; padding-top: 0px; padding-bottom: 0px;">
                                                            <div class="machineName"><span><% nvram_get_x("", "productid"); %></span></div>
                                                        </td>
                                                        <td style="padding-top: 0px; padding-bottom: 0px;">
                                                            <div id="permissionTitle"></div>
                                                        </td>
                                                    </tr>
                                                </table>

                                                <!-- the tree of folders -->
                                                <div id="e0" class="FdTemp" style="font-size:9pt; margin-top:1px;"></div>

                                                <div style="text-align:right; margin:10px auto; border-top:1px dotted #CCC; width:95%; padding:2px;">
                                                    <center><input name="changePermissionBtn" id="changePermissionBtn" type="button" value="<#CTL_apply#>" class="btn btn-primary" style="width: 219px" disabled="disabled"></center>
                                                </div>
                                            </td>
                                        </tr>
                                    </table>

                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    </form>

    <div id="footer"></div>

    <!-- mask for disabling AiDisk -->
    <div id="OverlayMask" class="popup_bg">
        <div align="center">
            <iframe src="" frameborder="0" scrolling="no" id="popupframe" width="400" height="400" allowtransparency="true" style="margin-top:150px;"></iframe>
        </div>
    <!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
    </div>
</div>
</body>
</html>
