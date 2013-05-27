// Use AJAX to detect WAN connection
var time_interval = 5; // second

var wan_status_t = "";
var wan_subnet_t = "";
var lan_subnet_t = "";
var detect_if_wan = 0;

function refresh_waninfo(){
	document.internetForm.connectbutton_link.disabled = false;
	document.internetForm.connectbutton_nolink.disabled = false;
}

function detectWANstatus(){
	var $j = jQuery.noConflict();
	$j.ajax({
		url: '/WAN_info.asp',
		dataType: 'script',
		
		error: function(xhr){
			setTimeout("detectWANstatus();", time_interval*1000);
		},
		success: function(response){
			refresh_waninfo();
			setTimeout("detectWANstatus();", time_interval*1000);
		}
	});
}
