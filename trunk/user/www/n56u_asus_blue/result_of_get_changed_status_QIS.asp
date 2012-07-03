<% wanlink(); %>

new_ifWANConnect = <% detect_if_wan(); %>;
detectType = "<% detect_dhcp_pppoe(); %>";
new_wan_status_log = "<% get_wan_status_log(); %>";
manually_stop_wan = '<% nvram_get_x("", "manually_disconnect_wan"); %>';

wan_subnet = '<% nvram_get_x("", "wan_subnet_t"); %>';
lan_subnet = '<% nvram_get_x("", "lan_subnet_t"); %>';

wan_lease = parseInt('<% nvram_get_x("", "wan0_lease"); %>');
