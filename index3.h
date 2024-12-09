const char MAIN_page3[] PROGMEM = R"=====(
<script>
function handleClick(){
    var pwdf=document.getElementById("pwd");
    var pwdc=document.getElementById("showpwd");
    //use this value
    if (pwdc.checked) {
	pwdf.setAttribute("type","text");
	}
    else {
	pwdf.setAttribute("type","password");
	}
}
</script>
</body>
</html>
)=====";
