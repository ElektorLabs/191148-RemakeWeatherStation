function sdcard_js_loaded(){
    return true;
}

function showSDCARD(){
    showView("SDCARDView");
    RequestStettings();
}


function RequestStettings(){
    sendRequest("/sdlog/settings.json", SDSettingsLoaded);
}

function RequestSdStatus(){
    sendRequest("/sdlog/sd/status", UpadteSDStatus);
}

function SDSettingsLoaded(data){
    try{
        SettingsJSON = JSON.parse(data);
        var elen = document.getElementById("SDCardLoggingEnable");
        var eliv = document.getElementById("SDcardLoggingInterval");

        elen.value = SettingsJSON.Enable;
        eliv.value = SettingsJSON.LogInterval;
    } catch{
        var elen = document.getElementById("SDCardLoggingEnable");
        var eliv = document.getElementById("SDcardLoggingInterval");
        elen.value = false;
        eliv.value = 60;

    }
    RequestSdStatus();
}


function SDCardEnaChanged(){
   var url = GenerateHostUrl("/sdlog/settings.dat");
   var el = document.getElementById("SDCardLoggingEnable");
    
    var data = [];
    data.push({key:"SDLOG_ENA",
               value: el.value});
    sendData(url,data); 
}

function SDCardLogIntChanged(){
    var url = GenerateHostUrl("/sdlog/settings.dat");
    var el = document.getElementById("SDcardLoggingInterval");

    var data = [];
    data.push({key:"SDLOG_INT",
               value: el.value});
    sendData(url,data);    
}

function UpadteSDStatus(data){
    try{
        StatusJSON = JSON.parse(data);
        var sdmounted = StatusJSON.SDCardMounted;
        var freesize = StatusJSON.SDCardFree;
        var sdcapaciyt = StatusJSON.SDCardSize;

        document.getElementById("SDCapacity").innerHTML=sdcapaciyt+"MB Total";
        document.getElementById("SDCapacityUsed").innerHTML=freesize+"MB Free";
        if(sdmounted===true){
            document.getElementById("SDMountStatus").innerHTML="SD-Card mounted";
        } else {
            document.getElementById("SDMountStatus").innerHTML="SD-Card unmounted";
        }


      

    } catch {
        //File Broken!
        document.getElementById("SDCapacity").innerHTML="0 MB Total";
        document.getElementById("SDCapacityUsed").innerHTML="0 MB Free";
        document.getElementById("SDMountStatus").innerHTML="API Error";
        
    }
}

function SDCardMount(){
    // /sdlog/sd/mount
    sendRequest("/sdlog/sd/mount",RequestSdStatus);

}

function SDCardUMount(){
    // /sdlog/sd/umount
    sendRequest("/sdlog/sd/umount",RequestSdStatus);

}