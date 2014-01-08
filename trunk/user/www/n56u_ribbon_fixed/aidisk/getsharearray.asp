<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<script type="text/javascript">
var result = [<% get_share_tree(); %>];
var layer_order = '<% get_parameter("layer_order"); %>';
var motion = '<% get_parameter("motion"); %>';

function get_share_tree_success(){
	parent.get_tree_items(result, motion);
}
</script>
</head>

<body onload="get_share_tree_success();">
</body>
</html>
