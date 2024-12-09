const char MAIN_page2[] PROGMEM = R"=====(
<body>
<h1>WiFlip configurator V1.2</h1>
<h3>WiFlip Mac: %s</h3>
<h3>Enter the credentials of your private network</h3>
<form action="/config_page" method="post">
<div style="margin-left: auto; margin-right: 0;">
  <label for="ssid">SSID:</label>
  <input type="text" id="ssid" name="ssid" value="%s"><br><br>
  <label for="pwd">Password:</label>
  <input type="password" id="pwd" name="pwd"><br>
  <input type="checkbox" id="showpwd" name="showpwd" value="showpwd" unchecked onclick="handleClick(this);">
  <label for="showpwd" class="mintxt">Show password</label><br><br>
  <label for="portn">Port Number:</label>
  <input type="number" name="portn" min="1" max="65535"  value="%d"><br><br>
  <div>
      <input type="submit" value="Set" class="buttonc">
  </div>
</div>
</form>
<p>Current IP connection is %s</p>
)=====";
