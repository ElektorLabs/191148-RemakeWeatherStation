function showMQTT(){
    sendRequest("mqtt/settings", read_mqttsettings);
    showView("MQTTView");
}


function mqtt_pass_onclick(){
    document.getElementById("MQTT_PASS").value = "";
    document.getElementById("MQTT_PASS").type = "text";
}

function read_mqttsettings( msg ){
    try{
        var jsonObj = JSON.parse(msg);
        document.getElementById("MQTT_USER").value = jsonObj.mqttuser;
        document.getElementById("MQTT_HOST").value = jsonObj.mqtthost;
        document.getElementById("MQTT_SERVER").value = jsonObj.mqttserver;
        document.getElementById("MQTT_PORT").value = jsonObj.mqttport;
        document.getElementById("MQTT_PASS").value = jsonObj.mqttpass;
        document.getElementById("MQTT_TOPIC").value = jsonObj.mqtttopic;
        document.getElementById("MQTT_PASS").type = "password";
        document.getElementById("MQTT_ENA").value = jsonObj.mqttena;
        document.getElementById("MQTT_UPDATE_INT").value = jsonObj.mqtttxintervall;  
        document.getElementById("MQTT_IOBROKER").value = jsonObj.mqtte_iobrokermode;  
    } catch{
        document.getElementById("MQTT_USER").value = "";
        document.getElementById("MQTT_HOST").value = "Unknown";
        document.getElementById("MQTT_SERVER").value = "0.0.0.0";
        document.getElementById("MQTT_PORT").value = 1883;
        document.getElementById("MQTT_PASS").value = "";
        document.getElementById("MQTT_TOPIC").value = "Unknown";
        document.getElementById("MQTT_PASS").type = "password";
        document.getElementById("MQTT_ENA").value = false;
        document.getElementById("MQTT_UPDATE_INT").value = 60;
        document.getElementById("MQTT_IOBROKER").value = false;
    }
    
}


function SubmitMQTT( ){
    var protocol = location.protocol;
    var slashes = protocol.concat("//");
    var host = slashes.concat(window.location.hostname);
    var url = host + "/mqtt/settings";
    var mqtt_user = document.getElementById("MQTT_USER").value;
    var mqtt_host = document.getElementById("MQTT_HOST").value;
    var mqtt_server = document.getElementById("MQTT_SERVER").value;
    var mqtt_port = document.getElementById("MQTT_PORT").value;
    var mqtt_topic =  document.getElementById("MQTT_TOPIC").value;
    var mqtt_ena = document.getElementById("MQTT_ENA").value;
    var mqtt_txintervall =  document.getElementById("MQTT_UPDATE_INT").value;
    var mqtt_iobrokermode = document.getElementById("MQTT_IOBROKER").value;
    var data = [];
    data.push({key:"MQTT_ENA",
                   value: mqtt_ena}); 
    data.push({key:"MQTT_IOBROKER",
                    value: mqtt_iobrokermode});
    
    data.push({key:"MQTT_PORT",
                   value: mqtt_port});
   
    
    data.push({key:"MQTT_USER",
               value: mqtt_user});
               
    data.push({key:"MQTT_HOST",
               value: mqtt_host});
    
    
    data.push({key:"MQTT_SERVER",
               value: mqtt_server});
               
    data.push({key:"MQTT_TOPIC",
               value: mqtt_topic});
             
             
    data.push({key:"MQTT_TXINTERVALL",
               value: mqtt_txintervall});
    
    if( "password" != document.getElementById("MQTT_PASS").type){
        var mqtt_pass = document.getElementById("MQTT_PASS").value;
        data.push({key:"MQTT_PASS",
               value: mqtt_pass});
        
        document.getElementById("MQTT_PASS").value = "********";
        document.getElementById("MQTT_PASS").type = "password";
        
    }            
               
    sendData(url,data); 
     
     
    

}

