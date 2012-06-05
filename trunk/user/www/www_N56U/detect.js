// Use AJAX to detect LAN connection
function isWLclient(){  //detect login client is by wireless or wired
	if(wireless.length > 0){
		for(var i=0; i < wireless.length; i++){
			if(wireless[i][0] == login_mac_str())
				return true;  //wireless
		}
	}
	
	return false; //wired
}

function detectLANstatus(forward_page, page_flag)
{
	if($("drword")){
		$("drword").innerHTML = "<#DrSurf_sweet_advise2#><br/><br/>";
	}
	
	if(page_flag == "detectWAN"){
		send_for_detectWAN();
	}
	else if(document.forms[0].current_page.value == "/QIS_wizard.htm"){
		setTimeout("gotoFinish('"+forward_page+"', '"+page_flag+"');", 1000);
	}
	else if(forward_page.length > 0){
		setTimeout("location.href = '"+forward_page+"';", 1000);
	}
}

function send_for_detectWAN(){
	document.redirectForm.action = "detectWAN.asp";
	document.redirectForm.target = "contentM";
	document.redirectForm.submit();
}

function reply_of_detectWAN(result){
	document.redirectForm.action = "QIS_wizard.htm";
	document.redirectForm.target = "";
	
	if(result >= 1){
		document.redirectForm.flag.value = "remind";
	}
	else{
		document.redirectForm.flag.value = "auto_way_static";
		document.redirectForm.prev_page.value = "start_apply.htm";
	}
	
	document.redirectForm.submit();
}
