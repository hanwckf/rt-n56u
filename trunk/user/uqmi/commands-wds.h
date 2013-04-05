#define __uqmi_wds_commands \
	__uqmi_command(wds_start_network, start-network, required, QMI_SERVICE_WDS), \
	__uqmi_command(wds_set_auth, auth-type, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_username, username, required, CMD_TYPE_OPTION), \
	__uqmi_command(wds_set_password, password, required, CMD_TYPE_OPTION)

#define wds_helptext \
		"  --start-network <apn>:            Start network connection (use with options below)\n" \
		"    --auth-type pap|chap|both|none: Use network authentication type\n" \
		"    --username <name>:              Use network username\n" \
		"    --password <password>:          Use network password\n" \

