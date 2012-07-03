
	router_ip = "<% nvram_get_x("LANHostConfig","lan_gateway_t"); %>";
	
	function testRemote(){
		return router_ip;
	}
	