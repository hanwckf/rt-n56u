<!DOCTYPE html>
<html>
<head>
<title><#Web_Title#></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">

<link rel="shortcut icon" href="images/favicon.ico">
<link rel="icon" href="images/favicon.png">

<style type="text/css">
body {
  background: url("/bootstrap/img/dark-bg.jpg") repeat scroll center top transparent;
  min-width: 1060px;
}

.alert {
  padding: 10px 20px 2px 20px;
  margin-bottom: 10px;
  margin-top: 10px;
  color: #c09853;
  text-shadow: 0 1px 0 rgba(255, 255, 255, 0.5);
  background-color: #fcf8e3;
  border: 1px solid #fbeed5;
  -webkit-border-radius: 4px;
  -moz-border-radius: 4px;
  border-radius: 4px;
  width: 460px;
  height: 100px;
  font-family:Arial, Verdana, Helvetica, sans-serif;
  text-align:left;
}

.alert-info {
  color: #3a87ad;
  background-color: #d9edf7;
  border-color: #bce8f1;
}

#logined_ip_str
{
    color: #BD362F;
}
</style>

<script>
<% login_state_hook(); %>

function initial(){
  document.getElementById("logined_ip_str").innerHTML = login_ip_str();
}
</script>
</head>

<body onload="initial()">
<center>
    <div class="alert alert-info">
        <p><#login_hint1#> <span id="logined_ip_str"></span></p>
        <p><#login_hint2#></p>
    </div>
</center>
</body>
</html>
