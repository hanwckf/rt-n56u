<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script>
var result = [<% get_folder_tree(); %>];
var layer_order = '<% get_parameter("layer_order"); %>';
var motion = '<% get_parameter("motion"); %>';

function get_folder_tree_success(){
	parent.get_tree_items(result, motion);
}
</script>
</head>

<body onload="get_folder_tree_success();">
</body>
</html>
