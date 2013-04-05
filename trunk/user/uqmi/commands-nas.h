#define __uqmi_nas_commands \
	__uqmi_command(nas_set_network_modes, set-network-modes, required, QMI_SERVICE_NAS), \
	__uqmi_command(nas_initiate_network_register, network-register, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_get_signal_info, get-signal-info, no, QMI_SERVICE_NAS), \
	__uqmi_command(nas_get_serving_system, get-serving-system, no, QMI_SERVICE_NAS) \

#define nas_helptext \
		"  --set-network-modes <modes>:      Set preferred network mode (Syntax: <mode1>[,<mode2>,...])\n" \
		"                                    Available modes: all, lte, umts, gsm, cdma, td-scdma\n" \
		"  --network-register:               Initiate network register\n" \
		"  --get-signal-info:                Get signal strength info\n" \
		"  --get-serving-system:             Get serving system info\n" \

