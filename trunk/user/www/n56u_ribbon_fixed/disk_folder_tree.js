var isLoading = 0;
var FromObject = "0";
var Items = null;
var lastClickedObj = 0;
var clickedFolderBarCode = new Array();

function popupWindow(w,u){
	disableCheckChangedStatus();
	
	winW_H();
	
	$(w).style.width = winW+"px";
	$(w).style.height = winH+"px";
	$(w).style.visibility = "visible";
	
	$('popupframe').src = u;
}

function hidePop(flag){
	if(flag != "apply")
		enableCheckChangedStatus();

	setTimeout(function(){
		document.getElementById("popupframe").src = "";
	}, 100);

	$('OverlayMask').style.visibility = "hidden";
}

function GetFolderItem(selectedObj, haveSubTree){
	var layer_order, layer = 0;
	
	showClickedObj(selectedObj);
	
	layer_order = selectedObj.id.substring(1);
	layer = get_layer(layer_order);
	
	if(layer == 0)
		alert("Machine: Wrong");
	else if(layer == 1){
		// chose Disk
		setSelectedDisk(selectedObj);
		onEvent();
	}
	else if(layer == 2){
		// chose Partition
		setSelectedPool(selectedObj);
		onEvent();
	}
	else if(layer == 3){
		// chose Shared-Folder
		setSelectedFolder(selectedObj);
		onEvent();
		showApplyBtn();
	}
	
	if(haveSubTree)
		GetTree(layer_order, 1);
}

function getSelectedStatusOfPool(pool){
	var status = "";
	for(var i = 0; i < pool_names().length; ++i){
		if(pool_names()[i] == pool){
			status = pool_status()[i];
			break;
		}
	}
	
	return status;
}

function showClickedObj(clickedObj){
	if(this.lastClickedObj != 0)
		this.lastClickedObj.className = "lastfolderClicked";  //this className set in AiDisk_style.css
	
	clickedObj.className = "folderClicked";

	this.lastClickedObj = clickedObj;
}

function GetTree(layer_order, v){
	if(layer_order == "0"){
		this.FromObject = layer_order;
		$('d'+layer_order).innerHTML = '<span class="FdWait">. . . . . . . . . .</span>';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
		
		return;
	}
	
	if($('a'+layer_order).className == "FdRead"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";
		
		this.FromObject = layer_order;
		
		$('e'+layer_order).innerHTML = '<img src="/images/Tree/folder_wait.gif">';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
	}
	else if($('a'+layer_order).className == "FdOpen"){
		$('a'+layer_order).className = "FdClose";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"0.gif";
		
		$('e'+layer_order).style.position = "absolute";
		$('e'+layer_order).style.visibility = "hidden";
	}
	else if($('a'+layer_order).className == "FdClose"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";
		
		$('e'+layer_order).style.position = "";
		$('e'+layer_order).style.visibility = "";
	}
	else
		alert("Error when show the folder-tree!");
}

function get_disk_tree(){
	if(this.isLoading == 0){
		get_layer_items("0", "gettree");
		setTimeout('get_disk_tree();', 1000);
	}
}

function get_layer_items(new_layer_order, motion){
	disableCheckChangedStatus();
	
	document.aidiskForm.action = "/aidisk/getsharearray.asp";
	
	$("motion").value = motion;
	$("layer_order").value = new_layer_order;
	document.aidiskForm.submit_fake.click();
}

function get_tree_items(treeitems, motion){
	this.isLoading = 1;
	this.Items = treeitems;
	
	if(motion == "lookup")
		;
	else if(motion == "gettree"){
		if(this.Items && this.Items.length > 0)
			BuildTree();
	}
}

function BuildTree(){
	var ItemText, ItemBarCode, ItemSub, ItemIcon;
	var vertline, isSubTree;
	var layer;
	var shown_permission = "";
	
	var TempObject = '<table cellpadding=0 cellspacing=0 border=0>\n';
	
	for(var i = 0; i < this.Items.length; ++i){
		this.Items[i] = this.Items[i].split("#");
		
		var Item_size = 0;
		Item_size = this.Items[i].length;
		if(Item_size > 3){
			var temp_array = new Array(3);
			
			temp_array[2] = this.Items[i][Item_size-1];
			temp_array[1] = this.Items[i][Item_size-2];
			
			temp_array[0] = "";
			for(var j = 0; j < Item_size-2; ++j){
				if(j != 0)
					temp_array[0] += "#";
				temp_array[0] += this.Items[i][j];
			}
			this.Items[i] = temp_array;
		}
		
		ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemSub = parseInt((this.Items[i][2]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,""));

		layer = get_layer(ItemBarCode.substring(1));		
		if(layer == 1)
			ItemIcon = '9'; //usb
		else if(layer == 2)
			ItemIcon = '4'; //part
		else
			ItemIcon = '10'; //folder
		
		SubClick = ' onclick="GetFolderItem(this, ';
		if(ItemSub <= 0){
			SubClick += '0);"';
			isSubTree = 'n';
		}
		else{
			SubClick += '1);"';
			isSubTree = 's';
		}
		
		if(i == this.Items.length-1){
			vertline = '';
			isSubTree += '1';
		}
		else{
			vertline = ' background="/images/Tree/vert_line.gif"';
			isSubTree += '0';
		}
		
		TempObject += 
'<tr>\n'+
	'<td width=19 height=16 valign=top>\n'+
		'<img id="a'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' class="FdRead" src="/images/Tree/vert_line_'+isSubTree+'0.gif">\n'+
	'</td>\n'+
	
	'<td>\n';
		
		var short_ItemText = "";
		var shown_ItemText = "";
		
		if(layer == 3){
			if(ItemText.length > 19)
		 		short_ItemText = ItemText.substring(0,16)+"...";
		 	else
		 		short_ItemText = ItemText;
		 	
		 	shown_ItemText = showhtmlspace(short_ItemText);
			
			TempObject += 
		'<div id="b'+ItemBarCode+'" style="float:left; width:170px; overflow:hidden;" class="FdText">\n'+
			'<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="/bootstrap/img/wl_device/'+ItemIcon+'.gif" align=top>\n'+
			'<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n'+
		'</div>\n';
			
			TempObject += 
		'<div id=\"f'+ItemBarCode+'" class="FileStatus" style="margin-left: 10px;" onclick="getChangedPermission(this);"></div>\n\n';
		}
		else{
			shown_ItemText = showhtmlspace(ItemText);
			
			TempObject += 
		'<div id="b'+ItemBarCode+'" class="FdText">\n'+
			'<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="/bootstrap/img/wl_device/'+ItemIcon+'.gif" align=top>\n'+
			'<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n'+
		'</div>\n';			
			TempObject += 
		'<div id="e'+ItemBarCode+'" class="FdTemp"></div>\n';
		}
		
		TempObject += 
	'</td>\n'+
'</tr>\n';
	}
	
	TempObject += 
'</table>\n';
	
	$("e"+this.FromObject).innerHTML = TempObject;
	
	// additional object
	if(layer == 3){
		for(var i = 0; i < this.Items.length; ++i){
			ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
			ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
			
			// record the barcode of the shown folder
			add_folderBarCode_list(this.selectedPool, ItemText, ItemBarCode);
			
			// decide if show the permission out
			if(this.selectedAccount.length > 0)
				shown_permission = get_permission_of_folder(this.selectedAccount, this.selectedPool, ItemText, PROTOCOL);
			else
				shown_permission = 3;
			
			showPermissionRadio(ItemBarCode, shown_permission);
		}
	}
	
	//enableCheckChangedStatus();
}

function showPermissionRadio(barCode, permission){
	var code = "";
	var parentBarCode = barCode.substring(0, barCode.lastIndexOf('_'));
	var parentPoolName = $('d'+parentBarCode).firstChild.nodeValue;
	var parentPoolStatus = getSelectedStatusOfPool(parentPoolName);

	code += '<input type="radio" style="margin-top: 0px; margin-left: '+(PROTOCOL == 'cifs' ? 20 : 15)+'px" name="g'+barCode+'" value="3"';
	if(permission == 3)
		code += ' checked';
	else if(PROTOCOL == "cifs" && permission == 2)
		code += ' checked';

	if(this.selectedAccount.length <= 0 || (this.selectedAccount == "anonymous" && PROTOCOL != 'cifs') || parentPoolStatus != "rw")
		code += ' disabled';

	code += '>';

	if(PROTOCOL == "ftp"){
		code += '<input type="radio" style="margin-top: 0px; margin-left: 30px" name="g'+barCode+'" value="2"';
		if(permission == 2)
			code += ' checked';
		
		if(this.selectedAccount.length <= 0 || this.selectedAccount == "anonymous" || parentPoolStatus != "rw")
			code += ' disabled';
		
		code += '>';
	}
	else if(PROTOCOL == "cifs")
		code += '<span></span>\n';

	code += '<input type="radio" style="margin-top: 0px; margin-left: '+(PROTOCOL == 'cifs' ? 40 : 30)+'px" name="g'+barCode+'" value="1"';
	if(permission == 1)
		code += ' checked';

	if(this.selectedAccount.length <= 0 || parentPoolStatus != "rw")
		code += ' disabled';

	code += '>';

	if(PROTOCOL == "cifs")
		code += '<span></span>\n';

	code += '<input type="radio" style="margin-top: 0px; margin-left: '+(PROTOCOL == 'cifs' ? 40 : 30)+'px" name="g'+barCode+'" value="0"';
	if(permission == 0)
		code += ' checked';

	if(this.selectedAccount.length <= 0 || parentPoolStatus != "rw")
		code += ' disabled';

	code += '>';

	$("f"+barCode).innerHTML = code;
}

function getChangedPermission(selectedObj){
	var folderBarCode = selectedObj.id.substring(1);
	var folderObj = $("d"+folderBarCode);
	var radioName = "g"+folderBarCode;
	var permission, orig_permission;
	
	if(!this.selectedAccount)
		return;
		
	setSelectedFolder(folderObj);
	
	permission = getValueofRadio(radioName);
	if(permission == -1){
		alert("Can't read the permission when change the radio!");	// system error msg. must not be translate
		return;
	}
	
	if(!this.changedPermissions[this.selectedAccount])
		this.changedPermissions[this.selectedAccount] = new Array();
	
	if(!this.changedPermissions[this.selectedAccount][this.selectedPool])
		this.changedPermissions[this.selectedAccount][this.selectedPool] = new Array();
	
	this.changedPermissions[this.selectedAccount][this.selectedPool][this.selectedFolder] = permission;
	if(this.controlApplyBtn == 0 || this.controlApplyBtn == 1)
		this.controlApplyBtn += 2;
	
	showApplyBtn();
	onEvent();
}

function getValueofRadio(radioName){
	var radioObjs = getElementsByName_iefix("input", radioName);
	var value;
	for(var i = 0; i < radioObjs.length; ++i)
		if(radioObjs[i].checked == true)
			//return parseInt(radioObjs[i].value);
			return radioObjs[i].value;
	
	return -1;
}

function get_layer(layer_order){
	var tmp, layer;
	
	layer = 0;
	while(layer_order.indexOf('_') != -1){
		layer_order = layer_order.substring(layer_order.indexOf('_'), layer_order.length);
		++layer;
		layer_order = layer_order.substring(1);
	}
	
	return layer;
}

function add_folderBarCode_list(poolName, folderName, folderBarCode){
	if(!this.clickedFolderBarCode[poolName]){
		this.clickedFolderBarCode[poolName] = new Array();
	}
	
	this.clickedFolderBarCode[poolName][folderName] = folderBarCode;
}

function get_folderBarCode_in_pool(poolName, folderName){
	if(this.clickedFolderBarCode[poolName])
		if(this.clickedFolderBarCode[poolName][folderName])
			return this.clickedFolderBarCode[poolName][folderName];
	
	return "";
}
