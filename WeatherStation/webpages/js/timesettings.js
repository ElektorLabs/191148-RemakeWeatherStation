function timesettings_js_loaded(){
    return true;
}
function showTimeSettings(){
    sendRequest("timesettings", read_timesettings);
    showView("TimeSettings");
}

function read_timesettings(msg){
    try{ 
        var jsonObj = JSON.parse(msg);
        document.getElementById("NTP_ON").checked = jsonObj.ntpena;
        document.getElementById("ZONE_OVERRRIDE").checked = jsonObj.zoneoverride;
        document.getElementById("DLSOverrid").checked = jsonObj.dlsdis;
        document.getElementById("ManualDLS").checked = jsonObj.dlsmanena;
  
        var element = document.getElementById("GMT_OFFSET");
        element.value = jsonObj.gmtoffset;
        
        var element = document.getElementById("timezoneid");
        element.value = jsonObj.tzidx;
    
        var element = document.getElementById("dls_offset");
        element.value = jsonObj.dlsmanidx;
    
        var element = document.getElementById("date");
        element.value = jsonObj.date;
   
        var element = document.getElementById("time");
        element.value=jsonObj.time;
    
        var element = document.getElementById("NTPServerName");
        element.value = jsonObj.ntpname;

        var element = document.getElementById("NTP_UPDTAE_SPAN");
        element.value = jsonObj.ntp_update_span;

    } catch {
        //Data is not in good shape....
        document.getElementById("NTP_ON").checked = false;
        document.getElementById("ZONE_OVERRRIDE").checked = false;
        document.getElementById("DLSOverrid").checked = false;
        document.getElementById("ManualDLS").checked = false;
  
        var element = document.getElementById("GMT_OFFSET");
        element.value = 0;
        
        var element = document.getElementById("timezoneid");
        element.value = 0;
    
        var element = document.getElementById("dls_offset");
        element.value = 0;
    
        var element = document.getElementById("date");
        element.value = "1970-01-01";
   
        var element = document.getElementById("time");
        element.value="00:00:00";
    
        var element = document.getElementById("NTPServerName");
        element.value = "";

        var element = document.getElementById("NTP_UPDTAE_SPAN");
        element.value = 60;

        
    }
    
  

}

function LoadParameter(){
    sendRequest("timesettings", read_timesettings);
    
}

function SubmitTimeZone(){
   var protocol = location.protocol;
   var slashes = protocol.concat("//");
   var host = slashes.concat(window.location.hostname);
   var url = host + "/timezone.dat";
   
   var element = document.getElementById("timezoneid");
   var data = [];
   data.push({key:"timezoneid",
              value:  element.value});
   
   sendData(url,data); 
}                 

function SubmitDateTime(){
   var protocol = location.protocol;
   var slashes = protocol.concat("//");
   var host = slashes.concat(window.location.hostname);
   var url = host + "/settime.dat";
   
   var element = document.getElementById("date");
   var d = element.value;
  
   
   var element = document.getElementById("time");
   var t=element.value;

   var parts = t.split(':');
   //This will result in ideal conditions in three parts
   if(parts.lenght < 3){
       //We assume only hours and minutes here ( iphone style )
       t=parts[0]+":"+parts[1]+":00";
   }

   var data = [];
   data.push({key:"date",
              value: d});
   data.push({key:"time",
              value: t});
   
   sendData(url,data); 


}

function SubmitNTP(){
   var protocol = location.protocol;
   var slashes = protocol.concat("//");
   var host = slashes.concat(window.location.hostname);
   var url = host + "/ntp.dat";
   
   var ntp_ena=document.getElementById("NTP_ON").checked;
             
   var element = document.getElementById("NTPServerName");
   var servername = element.value;

   var element = document.getElementById("NTP_UPDTAE_SPAN");
   var timespan = element.value;
   
   
   
   var data = [];
   if(true===ntp_ena){
   data.push({key:"NTP_ON",
              value: ntp_ena});
   }
              
   data.push({key:"ntp_update_delta",
              value: timespan});
              
   data.push({key:"NTPServerName",
              value: servername});
   
   sendData(url,data); 


}

function SubmitOverrides(){
   var protocol = location.protocol;
   var slashes = protocol.concat("//");
   var host = slashes.concat(window.location.hostname);
   var url = host + "/overrides.dat";
   
   var zoneoverride = document.getElementById("ZONE_OVERRRIDE").checked;
   var dls_override = document.getElementById("DLSOverrid").checked;
   var dlsmanena = document.getElementById("ManualDLS").checked;
   
  
   var element = document.getElementById("GMT_OFFSET");
   var gmtoffset = element.value;
   
   var element = document.getElementById("dls_offset");
   var dlsoffsetidx=element.value;
   
   
   
   
   var data = [];
   if(true===zoneoverride){
   data.push({key:"ZONE_OVERRRIDE",
              value: zoneoverride});
   }
   
   if(true===dls_override){
   data.push({key:"dlsdis",
              value: dls_override});
   }
   
    if(true===dlsmanena){
   data.push({key:"dlsmanena",
              value: dlsmanena});
   }
              
   data.push({key:"dlsmanidx",
              value: dlsoffsetidx});
              
   data.push({key:"gmtoffset",
              value: gmtoffset});
   
   sendData(url,data); 


}