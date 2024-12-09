

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html> <html>
<head><meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>WiFlip Control</title>
<style>html{font-family:Helvetica; display:inline-block; margin:0px auto; text-align:center;}
body{margin-top: 50px;} h1{color: #444444; margin: 50px auto 30px;} h3{color:#444444; margin-bottom: 50px;}
.button{display:block; width:80px; background-color:#f48100; border:none; color:white; padding: 13px 30px; text-decoration:none; font-size:25px; margin: 0px auto 35px; cursor:pointer; border-radius:4px;}
.button-on{background-color:#f48100;}
.button-on:active{background-color:#f48100;}
.button-off{background-color:#26282d;}
.button-off:active{background-color:#26282d;}
.buttonc {
  display: inline-block;
  padding: 15px 25px;
  font-size: 24px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  outline: none;
  color: #fff;
  background-color: #f48100;
  border: none;
  border-radius: 15px;
  box-shadow: 0 9px #999;
}
.buttonc:hover {background-color: #3e8e41}
.buttonc:active {
  background-color: #141100;
  box-shadow: 0 5px #666;
  transform: translateY(4px);
}
</style>
</head>
<body>
<h1>WiFlip configurator</h1>
<h3>WiFlip Mac: %s</h3>
<h3>Enter the credentials of your private network</h3>
<form action="/config_page" method="post">
  <label for="ssid">SSID:</label>
  <input type="text" id="ssid" name="ssid" value="%s"><br><br>
  <label for="pwd">Password:</label>
  <input type="password" id="pwd" name="pwd"><br>
  <input type="checkbox" id="showpwd" name="showpwd" value="showpwd" unchecked onclick="handleClick(this);">
  <label for="showpwd">Show password</label><br><br>
  <input type="submit" value="Set" class="buttonc">
</form>
<p>Current IP connection is %s</p>
<script>
function handleClick(){
    var pwdf=document.getElementById("pwd");
    var pwdc=document.getElementById("showpwd");
    //use this value
    if (pwdc.checked) {
	pwdf.setAttribute("color","red");
	}
    else {
	pwdf.setAttribute("color","green");
	}
}
</script>
</body>
</html>
)=====";
