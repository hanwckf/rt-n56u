toInternet = <% detect_wan_connection(); %>;
link_internet = <% nvram_get_x("", "link_internet"); %>;

wan_subnet = '<% nvram_get_x("", "wan_subnet_t"); %>';
lan_subnet = '<% nvram_get_x("", "lan_subnet_t"); %>';
