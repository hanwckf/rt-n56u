wan_status_t = '<% nvram_get_x("", "wan_status_t"); %>';
wan_subnet_t = '<% nvram_get_x("", "wan_subnet_t"); %>';
lan_subnet_t = '<% nvram_get_x("", "lan_subnet_t"); %>';
detect_if_wan = <% detect_if_wan(); %>;
manually_disconnect_wan = <% nvram_get_x("", "manually_disconnect_wan"); %>0;
done_auto_mac = <% nvram_get_x("", "done_auto_mac"); %>;
link_internet = <% nvram_get_x("", "link_internet"); %>;
