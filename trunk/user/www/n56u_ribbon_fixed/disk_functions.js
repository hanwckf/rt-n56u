var selectedDiskOrder = "";
var selectedPoolOrder = "";
var selectedFolderOrder = "";

function setSelectedDiskOrder(selectedDiskId){
	this.selectedDiskOrder = parseInt(selectedDiskId.substring(selectedDiskId.length-1));
}

function getSelectedDiskOrder(){
	return this.selectedDiskOrder;
}

// for folder tree {
var selectedDisk = "";
var selectedPool = "";
var selectedFolder = "";

function setSelectedDisk(selectedObj){
	this.selectedDisk = selectedObj.firstChild.nodeValue;
	this.selectedPool = "";
	this.selectedFolder = "";
}

function getSelectedDisk(){
	return this.selectedDisk;
}

function setSelectedPool(selectedObj){
	var disk_id;
	
	disk_id = getDiskIDfromOtherID(selectedObj.id);
	
	this.selectedDisk = $(disk_id).firstChild.nodeValue;
	this.selectedPool = selectedObj.firstChild.nodeValue;
	this.selectedFolder = "";
}

function getSelectedPool(){
	return this.selectedPool;
}

function setSelectedFolder(selectedObj){
	var disk_id, part_id, share_id;
	
	disk_id = getDiskIDfromOtherID(selectedObj.id);
	part_id = getPoolIDfromOtherID(selectedObj.id);
	share_id = getShareIDfromOtherID(selectedObj.id);
	
	this.selectedDisk = $(disk_id).firstChild.nodeValue;
	this.selectedPool = $(part_id).firstChild.nodeValue;
	this.selectedFolder = get_sharedfolder_in_pool(this.selectedPool)[share_id];
}

function getSelectedFolder(){
	return this.selectedFolder;
}

function getDiskIDfromOtherID(objID){
	var disk_id_pos, disk_id;
	
	disk_id_pos = objID.indexOf("_", 3);
	disk_id = objID.substring(0, disk_id_pos);
	
	return disk_id;
}

function getPoolIDfromOtherID(objID){
	var part_id_pos, part_id;
	
	part_id_pos = objID.lastIndexOf("_");
	part_id = objID.substring(0, part_id_pos);
	
	return part_id;
}

function getShareIDfromOtherID(objID){
	var share_id_pos, share_id;
	
	share_id_pos = objID.lastIndexOf("_")+1;
	share_id = objID.substring(share_id_pos);
	
	return share_id;
}
// for folder tree }

function computepools(diskorder, flag){
	var pools = new Array();
	var pools_size = new Array();
	var pools_available = new Array();
	var pools_type = new Array();
	var pools_size_in_use = new Array();
	
	for(var i = 0; i < pool_names().length; ++i){
		if(per_pane_pool_usage_kilobytes(i, diskorder)[0] && per_pane_pool_usage_kilobytes(i, diskorder)[0] > 0){
			pools[pools.length] = pool_names()[i];
			pools_size[pools_size.length] = per_pane_pool_usage_kilobytes(i, diskorder)[0];
			pools_available[pools_available.length] = per_pane_pool_usage_kilobytes(i, diskorder)[0]-pool_kilobytes_in_use()[i];
			pools_type[pools_type.length] = pool_types()[i];
			pools_size_in_use[pools_size_in_use.length] = pool_kilobytes_in_use()[i];
		}
	}
	
	if(flag == "name") return pools;
	if(flag == "size") return pools_size;
	if(flag == "available") return pools_available;
	if(flag == "type") return pools_type;
	if(flag == "size_in_use") return pools_size_in_use;
}

function computeallpools(all_disk_order, flag){
	var pool_array = computepools(all_disk_order, flag);
	var total_size = 0;
	
	if(all_disk_order >= foreign_disks().length)
		return getDiskTotalSize(all_disk_order);
	
	for(var i = 0; i < pool_array.length; ++i)
		total_size += pool_array[i];
	
	return simpleNum(total_size);
}

function getDiskMountedNum(all_disk_order){
	if(all_disk_order < foreign_disks().length)
		return foreign_disk_total_mounted_number()[all_disk_order];
	else
		return 0;
}

function getDiskName(all_disk_order){
	var disk_name;
	
	if(all_disk_order < foreign_disks().length)
		disk_name = foreign_disks()[all_disk_order];
	else
		disk_name = blank_disks()[all_disk_order-foreign_disks().length];
	
	return disk_name;
}

function getDiskPort(all_disk_order){
	var disk_port;
	
	if(all_disk_order < foreign_disks().length)
		disk_port = foreign_disk_interface_names()[all_disk_order];
	else
		disk_port = blank_disk_interface_names()[all_disk_order-foreign_disks().length];
	
	return disk_port;
}

function getDiskDevice(all_disk_order){
	var disk_device;
	
	if(all_disk_order < foreign_disks().length)
		disk_device = foreign_disk_device_names()[all_disk_order];
	else
		disk_device = blank_disk_device_names()[all_disk_order-foreign_disks().length];
	
	return disk_device;
}

function getDiskModelName(all_disk_order){
	var disk_model_name;
	
	if(all_disk_order < foreign_disks().length)
		disk_model_name = foreign_disk_model_info()[all_disk_order];
	else
		disk_model_name = blank_disk_model_info()[all_disk_order-foreign_disk_model_info().length];
		
	return disk_model_name;
}

function getDiskTotalSize(all_disk_order){
	var TotalSize;
	
	if(all_disk_order < foreign_disks().length)
		TotalSize = simpleNum(foreign_disk_total_size()[all_disk_order]);
	else
		TotalSize = simpleNum(blank_disk_total_size()[all_disk_order-foreign_disk_total_size().length]);
		
	return TotalSize;
}
