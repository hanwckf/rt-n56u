<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_4_1#></title>
<link rel="stylesheet" type="text/css" href="/bootstrap/css/bootstrap.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/main.css">
<link rel="stylesheet" type="text/css" href="/bootstrap/css/engage.itoggle.css">

<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/bootstrap/js/bootstrap.min.js"></script>
<script type="text/javascript" src="/bootstrap/js/engage.itoggle.min.js"></script>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/disk_functions.js"></script>
<script type="text/javascript" src="/aidisk/AiDisk_folder_tree.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script>
    var $j = jQuery.noConflict();
    $j(document).ready(function() {
        $j('#enable_samba_on_of').iToggle({
            easing: 'linear',
            speed: 70,
            onClickOn: function(){
                $j("#enable_samba_fake").attr("checked", "checked").attr("value", 1);
                $j("#enable_samba_1").attr("checked", "checked");
                $j("#enable_samba_0").removeAttr("checked");
                switchAppStatus(1);
            },
            onClickOff: function(){
                $j("#enable_samba_fake").removeAttr("checked").attr("value", 0);
                $j("#enable_samba_0").attr("checked", "checked");
                $j("#enable_samba_1").removeAttr("checked");
                switchAppStatus(0);
            }
        });
        $j("#enable_samba_on_of label.itoggle").css("background-position", $j("input#enable_samba_fake:checked").length > 0 ? '0% -27px' : '100% -27px');
    });
</script>

<script type="text/javascript">
wan_route_x = '<% nvram_get_x("IPConnection", "wan_route_x"); %>';
wan_nat_x = '<% nvram_get_x("IPConnection", "wan_nat_x"); %>';
wan_proto = '<% nvram_get_x("Layer3Forwarding",  "wan_proto"); %>';

<% disk_pool_mapping_info(); %>
<% available_disk_names_and_sizes(); %>

<% get_AiDisk_status(); %>
<% initial_folder_var_file(); %>
<% get_permissions_of_account(); %>

var PROTOCOL = "cifs";

var NN_status = get_cifs_status();  // Network-Neighborhood
var AM_to_cifs = get_share_management_status("cifs");  // Account Management for Network-Neighborhood

var accounts = [<% get_all_accounts(); %>];

var lastClickedAccount = 0;
var selectedAccount = "";

// changedPermissions[accountName][poolName][folderName] = permission
var changedPermissions = new Array();

var folderlist = new Array();

function initial(){
	show_banner(1);
	show_menu(5, 5, 2);
	show_footer();
	
	// show page's control
	showShareStatusControl();
	showAccountControl();
	
	// show accounts
	showAccountMenu();
	
	// show the kinds of permission
	showPermissionTitle();
	
	// show folder's tree
	setTimeout('get_disk_tree();', 1000);
	
	// the click event of the buttons
	onEvent();
	check_usb();
}

function check_usb()
{
	var usb_path1 = '<% nvram_get_x("", "usb_path1"); %>';
	var usb_path2 = '<% nvram_get_x("", "usb_path2"); %>';
	if (usb_path1 != "storage" && usb_path2 != "storage"){
		$("accountbtn").disabled = true;
		$j('#enable_samba_on_of').iState(0).iClickable(0);
	}
}

function show_footer(){
	footer_code = '<div align="center" class="bottom-image"></div>';
	footer_code +='<div align="center" class="copyright"><#footer_copyright_desc#></div>';
	
	$("footer").innerHTML = footer_code;

	flash_button();
}

function get_disk_tree(){
	if(this.isLoading == 0){
		get_layer_items("0", "gettree");
		setTimeout('get_disk_tree();', 1000);
	}
}

function get_accounts(){
	return this.accounts;
}

function switchAppStatus(value){
	showLoading();
	document.aidiskForm.action = "/aidisk/switch_AiDisk_app.asp";
	document.aidiskForm.protocol.value = PROTOCOL;
	if (value == 1)
		document.aidiskForm.flag.value = "on";
	else
		document.aidiskForm.flag.value = "off";
	document.aidiskForm.submit();
}

function resultOfSwitchAppStatus(){
	refreshpage();
}

function showShareStatusControl(){
	if (this.NN_status == 1){
		$("tableMask").style.width = "0px";
		$("accountbtn").disabled = false;
	}
	else{
		$("tableMask").style.width = "500px";
		$("accountbtn").disabled = true;
	}
	
	showSamba();
}

function showSamba(){
	$("ie_link").href = '\\\\'+decodeURIComponent(document.form.computer_name.value);
	$("computer_show1").value = '\\\\'+decodeURIComponent(document.form.computer_name.value);
	$("computer_show2").value = '\\\\'+decodeURIComponent(document.form.computer_name.value);
	
	if(this.NN_status == 1){
		$("ShareClose").style.display = "none";
		$("Sambainfo").style.display = "block";
		
		if(navigator.appName.indexOf("Microsoft") >= 0)
			$("ie_hint").style.display = "block";
		else
			$("notie_hint").style.display = "block";
	}
	else{
		$("ShareClose").style.display = "block";
		$("Sambainfo").style.display = "none";
	}
}

function switchAccount(value){
	document.aidiskForm.action = "/aidisk/switch_share_mode.asp";
	$("protocol").value = PROTOCOL;
	
	if (value=="1")
		$("mode").value = "share";
	else{
		$("mode").value = "account";
		if(this.accounts.length==0)
			alert("<#enable_noaccount_alert#>");
	}
	
	showLoading();
	document.aidiskForm.submit();
}

function resultOfSwitchShareMode(){
	refreshpage();
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
	
	if(PROTOCOL == "cifs"){
		code += '<td width="56" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">R/W</span></td>';
		code += '<td width="56" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">R</span></td>';
		code += '<td width="56" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">No</span></td>';
	}else if(PROTOCOL == "ftp"){
		code += '<td width="42" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">R/W</span></td>';
		code += '<td width="42" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">W</span></td>';
		code += '<td width="42" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">R</span></td>';
		code += '<td width="42" align="center" valign="top" style="padding-top: 0px;"><span class="label label-info">No</span></td>';
	}
	
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
				/*alert("account = "+$("account").value+"\n"+
					  "pool = "+$("pool").value+"\n"+
					  "folder = "+$("folder").value+"\n"+
					  "protocol = "+$("protocol").value+"\n"+
					  "permission = "+$("permission").value);//*/
				showLoading();
				document.aidiskForm.submit();
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

    <div style="position: absolute;">
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
        </form>

        <form name="form">
        <input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get_x("LANGUAGE", "preferred_lang"); %>">
        <input type="hidden" name="firmver" value="<% nvram_get_x("",  "firmver"); %>">
        <input type="hidden" name="computer_name" value="<% nvram_char_to_ascii("Storage", "computer_name"); %>">
        </form>
    </div>

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
                                    <table cellpadding="4" cellspacing="0" class="table" style="margin-bottom: 0px;">
                                        <tr>
                                            <th width="35%">
                                                <#enableCIFS#>
                                            </th>
                                            <td>
                                                <div class="main_itoggle">
                                                    <div id="enable_samba_on_of">
                                                        <input type="checkbox" id="enable_samba_fake" <% nvram_match_x("Storage", "enable_samba", "1", "value=1 checked"); %><% nvram_match_x("Storage", "enable_samba", "0", "value=0"); %>>
                                                    </div>
                                                </div>

                                                <div style="position: absolute; margin-left: -10000px;">
                                                    <input type="radio" name="enable_samba" id="enable_samba_1" value="1" <% nvram_match_x("Storage", "enable_samba", "1", "checked"); %>/><#checkbox_Yes#>
                                                    <input type="radio" name="enable_samba" id="enable_samba_0" value="0" <% nvram_match_x("Storage", "enable_samba", "0", "checked"); %>/><#checkbox_No#>
                                                </div>
                                            </td>
                                        </tr>

                                        <tr>
                                            <th>
                                                <#StorageShare#>
                                            </th>
                                            <td>
                                                <select id="accountbtn" name="st_samba_mode" class="input" style="width: 300px;" onchange="switchAccount(this.value);">
                                                    <option value="1" <% nvram_match_x("Storage", "st_samba_mode", "1", "selected"); %>><#StorageShare1#></option>
                                                    <option value="4" <% nvram_match_x("Storage", "st_samba_mode", "4", "selected"); %>><#StorageShare2#></option>
                                                </select>
                                            </td>
                                        </tr>
                                    </table>

                                    <!-- The table of share. -->
                                    <div id="shareStatus">
                                        <!-- The mask of all share table. -->
                                        <div id="tableMask"></div>

                                        <!-- The mask of accounts. -->
                                        <div id="accountMask"></div>

                                        <!-- The action buttons of accounts and folders. -->
                                        <table cellpadding="2" cellspacing="0" class="table" style="margin-bottom: 0px; border-top: 1px solid #DDD;">
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
                                                        <td width="175" align="left" style="padding-left: 0px;">
                                                            <div class="machineName"><span><% nvram_get_f("general.log","productid"); %></span></div>
                                                        </td>
                                                        <td>
                                                            <div id="permissionTitle"></div>
                                                        </td>
                                                    </tr>
                                                </table>

                                                <!-- the tree of folders -->
                                                <div id="e0" class="FdTemp" style="font-size:10pt; margin-top:2px;"></div>

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


    <div id="help_td" style="position: absolute; margin-left: -10000px;">
        <!--==============Beginning of hint content=============-->
          <form name="hint_form"></form>
          <div id="helpicon" onClick="openHint(0,0);" title="Click to open Help" style="display:none;margin-top:20px;"><img src="images/help.gif" /></div>
          <div id="hintofPM">
          <table class="Help" bgcolor="#999999" width="180" border="0" cellpadding="0" cellspacing="1" style="margin-top:20px;">
            <thead>
            <tr>
              <td>
                <div id="helpname" class="AiHintTitle">How to Share?</div>
                    <a href="javascript:closeHint();">
                    <img src="images/button-close.gif" class="closebutton">
                    </a>
              </td>
            </tr>
            </thead>
            <tr>
              <td valign="top">
                <div id="hint_body" class="hint_body2">
                  <!-- the condition when stop share. -->
                  <div id="ShareClose" class="ShareClose" style="display:none; "><#SambaClose#></div>
                    <!-- the condition when run share. -->
                    <div id="Sambainfo" style="display:none;">
                    <!-- the info of Samba. -->
                        <div id="ie_hint" style="display:none;">
                            <a id="ie_link" href="" target="_blank"><#Clickhere#></a>
                            <#HowToSharebySamba1#>
                            <input type="text" id="computer_show1" readonly="1"  size="11" class="sharelink" onmouseover="this.select();" value="">
                            <#HowToSharebySamba2#>
                    </div>
                    <div id="notie_hint" style="display:none;">
                            <#HowToSharebySamba_notIE1#>
                            <input type="text" id="computer_show2" readonly="1"  size="11" class="sharelink" onmouseover="this.select();" value="">
                            <#HowToSharebySamba_notIE2#>
                        </div>
                    </div>
                  <!--p><#AiDisk_Step1_help2#></p-->
                </div>
                <iframe id="statusframe" name="statusframe" class="statusframe" src="" frameborder="0"></iframe>
              </td>
            </tr>
          </table>
          </div>
        <!--==============Ending of hint content=============-->
    </div>


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
