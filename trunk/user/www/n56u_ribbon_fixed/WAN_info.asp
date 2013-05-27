wan_status_t = '<% nvram_get_x("", "wan_status_t"); %>';
wan_subnet_t = '<% nvram_get_x("", "wan_subnet_t"); %>';
lan_subnet_t = '<% nvram_get_x("", "lan_subnet_t"); %>';
detect_if_wan = <% detect_if_wan(); %>;
link_internet = <% nvram_get_x("", "link_internet"); %>;
