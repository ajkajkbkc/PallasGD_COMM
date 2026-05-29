
#ifndef __WEBPGE_H
#define __WEBPGE_H

/* Includes ------------------------------------------------------------------*/



/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/


/* Private defines -----------------------------------------------------------*/


#define INDEX_HTML  "<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\
"<title>用户登录</title>"\
"<style type='text/css'>"\
"body {text-align:center; background-color:#3399cc;font-family:Verdana;}"\
"#main {margin-right:auto;margin-left:auto;margin-top:300px;}"\
"label{display:inline-block;width:80px;}"\
"#main h3{color:#000000; text-decoration:none;}"\
"</style>"\
"<script>"\
"};"\
"</script>"\
"</head>"\
"<body>"\
"<div id='main'>"\
"<div style='background:snow; display:block; padding:10px 20px;'>"\
"<h3>用户信息验证</h3>"\
"<form id='frmLogin' method='POST' action='login.cgi'>"\
"<p><label for='txtUser'>账号:</label><input type='text' id='txtUser' name='user' size='16' /></p>"\
"<p><label for='txtPword'>密码:</label><input type='password' id='txtPword' name='pword' size='16' maxlength='8' /></p>"\
"<p><input type='submit' value='登录' /></p>"\
"</form>"\
"</div>"\
"</div>"\
"<div style='margin:5px 5px;'>"\
"<!-- &copy; 2019 FX Shenzhen Team -->"\
"</body>"\
"</html>"\



#define CONFIG_HTML  "<!DOCTYPE html>"\
"<html>"\
"<head>"\
"<title>W5500EVB::网页配置</title>"\
"<meta http-equiv='Content-Type' content='text/html; charset=GB2312'/>"\
"<style type='text/css'>"\
"body {text-align:left; background-color:#c0deed;font-family:Verdana;}"\
"#main {margin-right:auto;margin-left:auto;margin-top:30px;}"\
"label{display:inline-block;width:150px;}"\
"#main h3{color:#66b3ff; text-decoration:underline;}"\
"</style>"\
"<script>"\
"function $(id) { return document.getElementById(id); };"\
"function settingsCallback(o) {"\
"if ($('txtVer')) $('txtVer').value = o.ver;"\
"if ($('txtMac')) $('txtMac').value = o.mac;"\
"if ($('txtIp')) $('txtIp').value = o.ip;"\
"if ($('txtSub')) $('txtSub').value = o.sub;"\
"if ($('txtGw')) $('txtGw').value = o.gw;"\
"};"\
"</script>"\
"</head>"\
"<body>"\
"<div id='main'>"\
"<div style='background:snow; display:block;padding:10px 20px;'>"\
"<h3>配置网络参数</h3>"\
"<form id='frmSetting' method='POST' action='config.cgi'>"\
"<p><label for='txtIp'>固件版本号:</label><input type='text' id='txtVer' name='ver' size='16' disabled='disabled' /></p>"\
"<p><label for='txtIp'>MAC地址:</label><input type='text' id='txtMac' name='mac' size='16' disabled='disabled' /></p>"\
"<p><label for='txtIp'>IP地址:</label><input type='text' id='txtIp' name='ip' size='16' /></p>"\
"<p><label for='txtSub'>子网掩码:</label><input type='text' id='txtSub' name='sub' size='16' /></p>"\
"<p><label for='txtGw'>默认网关:</label><input type='text' id='txtGw' name='gw' size='16' /></p>"\
"<p><input type='submit' value='保存并重启' /></p>"\
"</form>"\
"</div>"\
"</div>"\
"<div style='margin:5px 5px;'>"\
"<!-- &copy; 2019 FX Shenzhen Team -->"\
"</div>"\
"<script type='text/javascript' src='w5500.js'></script>"\
"</body>"\
"</html>"

/* Private functions ---------------------------------------------------------*/



#endif /* __WEBPGE_H */
