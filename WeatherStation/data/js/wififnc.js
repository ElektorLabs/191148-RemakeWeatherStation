//show the wifi settings data
function showWiFiSettings() {
    showView("WiFiSettings");
    getWiFiSettings();
    getSSIDList();
}

var wifi_scan_results = null;
var wifi_clean_ssid_list = null;

function clearWiFiSettings(){
    var ssid ="";
    var pass = "";
    console.log("Clear WiFi Settings");
    var request = "setWiFiSettings?ssid=" + ssid + "&pass=" + pass;
    sendRequest(request, openNotification);
}

//get and set wifi settings
function setWiFiSettings() {
    var ssid = document.getElementById("ssid").value;
    var pass = document.getElementById("pass").value;
    console.log("ssid: " + ssid + ", pass: " + pass);
    var request = "setWiFiSettings?ssid=" + ssid + "&pass=" + pass;
    sendRequest(request, openNotification);
}
function getWiFiSettings() {
    openNotification("Getting data...", 0);
    sendRequest("getWiFiSettings", getWiFiSettingsHandler);
}
function getWiFiSettingsHandler(data) {
    try{
        obj = JSON.parse(data);
        if(obj.SSID != ""){
        document.getElementById("currentSSID").innerHTML = "SSID: " + obj.SSID +"<br> Channel: "+obj.CHANNEL+"<br>RSSI: "+obj.RSSI+"dBm";
        }else{
            document.getElementById("currentSSID").innerHTML = "No network configured";
        }
    } catch {
        //No valid JSON received!
        document.getElementById("currentSSID").innerHTML = "SSID: Unknown<br> Channel: Unknown <br>RSSI: Unknown";
    }
    
}

function onlyUnique(value, index, self) { 
    return self.indexOf(value) === index;
}

//get a list of available SSIDs
function getSSIDList() {
    openNotification("Getting network names...", 0);
    sendRequest("getSSIDList", getSSIDListHandler);
}
function getSSIDListHandler(data) {
    openNotification("Done");
    setTimeout(closeNotification, 1000);
    if (data == null)
        return;
    try{
    wifi_scan_results = JSON.parse(data);
    /* We cleanup the list and remove double entrys here */
    wifi_clean_ssid_list=[];
    for(var x=0;x<wifi_scan_results.Networks.length;x++){
     wifi_clean_ssid_list.push( wifi_scan_results.Networks[x].SSID );  
    }
    wifi_clean_ssid_list = wifi_clean_ssid_list.filter(onlyUnique);
    
    
    var list = document.getElementById("ssid");
    list.innerHTML = "";
    if (wifi_scan_results.ScanCount == 0) {
        list.innerHTML = "<option value='0'>-</option>";
        return;
    }
    for (var i = 0; i < wifi_clean_ssid_list.length; i++) {
        list.innerHTML += "<option value='" + wifi_clean_ssid_list[i] + "'>" + wifi_clean_ssid_list[i] + "</option>";
    }
    } catch {
         
        var list = document.getElementById("ssid");
        list.innerHTML = "";
    }
    
    
    
}

