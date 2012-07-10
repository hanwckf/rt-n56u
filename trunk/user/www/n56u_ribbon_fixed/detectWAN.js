// Use AJAX to detect WAN connection
var time_interval = 5; // second

var wan_status_t = "";
var wan_subnet_t = "";
var lan_subnet_t = "";
var detect_if_wan = 0;
var manually_disconnect_wan = 0;

function refresh_waninfo(){	
	if(link_internet == "0"){
		//$("#connectstatus")[0].innerHTML = "<#Connected#>";
		//$("#connectbutton")[0].value = "<#Disconnect#>";
		document.internetForm.connectbutton_link.disabled = false;
		document.internetForm.connectbutton_nolink.disabled = true;	
		//parent.showMapWANStatus(1);
	}
	else if(link_internet == "1"){
		//$("#connectstatus")[0].innerHTML = "<#Disconnected#>";
		//$("#connectbutton")[0].value = "<#Connect#>";
		document.internetForm.connectbutton_link.disabled = true;	
		document.internetForm.connectbutton_nolink.disabled = false;	
		//parent.showMapWANStatus(0);
	}
	else{
		document.internetForm.connectbutton_link.disabled = true;	
		document.internetForm.connectbutton_nolink.disabled = true;	
	}
	//$("#connectbutton")[0].style.display = "";
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
