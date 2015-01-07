var winH,winW;

<% get_flash_time(); %>

function winW_H(){
	if(parseInt(navigator.appVersion) > 3){
		winW = document.documentElement.scrollWidth;
		if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
			winH = document.documentElement.clientHeight;
		else
			winH = document.documentElement.scrollHeight;
	}
}

function LoadingTime(seconds, flag){
	showtext($("proceeding_main_txt"), "<#Main_alert_proceeding_desc1#>");
	$("Loading").style.visibility = "visible";

	y = y+progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			showtext($("proceeding_main_txt"), "<#Main_alert_proceeding_desc4#>");
			showtext($("proceeding_txt"), Math.round(y)+"%");
			$("proceeding_bar").style.width=Math.round(y)+"%";
			--seconds;
			setTimeout("LoadingTime("+seconds+", '"+flag+"');", 1000);
		}else{
			showtext($("proceeding_main_txt"), translate("<#Main_alert_proceeding_desc3#>"));
			showtext($("proceeding_txt"), "");
			y = 0;
			
			if(flag != "waiting")
				setTimeout("hideLoading();",1000);
		}
	}
}

function LoadingProgress(seconds){
	y = y + progress;
	if(typeof(seconds) == "number" && seconds >= 0){
		if(seconds != 0){
			$("LoadingBar").style.visibility = "visible";
			$("proceeding_img").style.width = Math.round(y) + "%";
			$("proceeding_img_text").innerHTML = Math.round(y) + "%";
			--seconds;
			setTimeout("LoadingProgress("+seconds+");", 1000);
		}
		else{
			$("proceeding_img_text").innerHTML = "<#Main_alert_proceeding_desc3#>";
			y = 0;
			setTimeout("hideLoadingBar();",1000);
			location.href = "index.asp";
		}
	}
}

function showLoading(seconds, flag){
	disableCheckChangedStatus();

	// hide IE scrollbars
	htmlbodyforIE = document.getElementsByTagName("html");
	htmlbodyforIE[0].style.overflow = "hidden";

	winW_H();
	var blockmarginTop;
	var sheight = document.documentElement.scrollHeight;
	var cheight = document.documentElement.clientHeight

	//blockmarginTop = (navigator.userAgent.indexOf("Safari")>=0)?document.documentElement.scrollHeight - document.documentElement.clientHeight+200:document.documentElement.scrollTop+200;
	blockmarginTop = (navigator.userAgent.indexOf("Safari")>=0)?(sheight-cheight<=0)?200:sheight-cheight+200:document.documentElement.scrollTop+200;

	//Lock modified it for Safari4 display issue.
	$("loadingBlock").style.marginTop = blockmarginTop+"px";
	$("Loading").style.width = winW+"px";
	$("Loading").style.height = winH+"px";

	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	LoadingTime(seconds, flag);
}

function showLoadingBar(seconds){
	disableCheckChangedStatus();

	// hide IE scrollbars
	htmlbodyforIE = document.getElementsByTagName("html");
	htmlbodyforIE[0].style.overflow = "hidden";

	winW_H();
	//var blockmarginTop;
	//blockmarginTop = document.documentElement.scrollTop + 200;
	//$("loadingBarBlock").style.marginTop = blockmarginTop+"px";
	$("LoadingBar").style.width = winW+"px";
	$("LoadingBar").style.height = winH+"px";

	loadingSeconds = seconds;
	progress = 100/loadingSeconds;
	y = 0;
	LoadingProgress(seconds);
}

function showResetBar(){
	showLoadingBar(board_boot_time());
}

function showUpgradeBar(){
	showLoadingBar(board_flash_time());
}

function hideLoadingBar(){
	enableCheckChangedStatus();
	$("LoadingBar").style.visibility = "hidden";
}

function stopLoadingBar(){
	LoadingProgress(0);
}

function hideLoading(flag){
	enableCheckChangedStatus();
	$("Loading").style.visibility = "hidden";
}

function simpleSSID(obj){
	var SSID = document.loginform.wl_ssid.value;

	if(SSID.length < 16)
		showtext(obj, SSID);
	else{
		obj.title = SSID;
		showtext(obj, SSID.substring(0, 16)+"...");
	}
}
