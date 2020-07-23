var AutoUpdate = null;

function showDashboard(){
    showView("MainPage");
    Dashbaord_update_values();
    if(AutoUpdate != null){
        clearInterval(AutoUpdate);
    }
    AutoUpdate = setInterval(AutomaticUpdate, 10000); //every 30 seconds
}

function AutomaticUpdate(){
    if(AutoUpdate==null){
        return;
    }
    var el = document.getElementById("MainPage");
    if(el.style.display === "none" ){
        clearInterval(AutoUpdate);
        AutoUpdate=null;
    }
    Dashbaord_update_values();
}

//We will update the values every 60 seconds....
function Dashbaord_update_values(){
    load_mapping_data(collect_sensors_to_read);
}

function collect_sensors_to_read(callback_on_done){
 //Mapping is up to date now we go for the sensors
 var WindSpeedSensor = SearchForWindSpeed();
 var WindDirection = SearchForWindDirection();
 var Temperature =  SearchForTemperatur();
 var Pressure = SearchForPressure();
 var Humidity = SearchForHumidity();
    var MappingDataToLoad=[ [ WindSpeedSensor,null ], [ WindDirection,null ], [ Temperature,null ], [ Pressure,null ],  [ Humidity,null ]  ];

 //We need to build our path list for the request
    loadMultipleData(MappingDataToLoad,function() { 
        var parsed_data=Dashboard_parse_loaded_data_to_json( MappingDataToLoad ); 
        //We now have json objects if all went well
        //if data is null there was an error inside the JSON
        //The internal ones we try to keep displayed
    
        if(parsed_data[0][1]==null){
            UpdateGaug("cvs_windspeed",-1);
        } else {
            UpdateGauge("cvs_windspeed",parsed_data[0][1].value);
        }

        if(parsed_data[1][1]==null){
            UpdateGaug("cvs_winddirection",-1);
        } else {
            UpdateGauge("cvs_winddirection",parsed_data[1][1].value);
        }

        if(parsed_data[4][1]==null){
            UpdateGaug("cvs_humidity",-1);
        } else {
            UpdateGauge("cvs_humidity",parsed_data[4][1].value);
        }

        if(parsed_data[2][1]==null){
            UpdateGauge("cvs_temperature",-273);
        } else {
            UpdateGauge("cvs_temperature",parsed_data[2][1].value);
        }

        if(parsed_data[3][1]==null){
            UpdateGauge("cvs_pressure",0);
        } else {
            UpdateGauge("cvs_pressure",parsed_data[3][1].value);
        }

        

    });
}

function UpdateGauge(name , val ){
    document.gauges.forEach(function (gauge) {
        if( gauge.canvas.element.id == name){
          gauge.value = val;
        }
      });

}

function Dashboard_parse_loaded_data_to_json(MappingDataToLoad){
    for(var z=0;z<MappingDataToLoad.length;z++){
        try{
            MappingDataToLoad[z][1]=JSON.parse(MappingDataToLoad[z][1]);
        } catch {
            MappingDataToLoad[z][1]=null;
        }
    }
    return MappingDataToLoad;
}

function SearchForWindSpeed(){
    //Check if the sensor itself is mapped !?
    //5 is windspeed
    var Result = SearchForFirstSensor(5);
    //we need to grab the current data....
    if(Result.found===false){
        return "";
    } else {
        //We build the path for the request
        return "/mapping/"+Result.mMappedChannel+"/value";
    }
}

function SearchForWindDirection(){
    var Result = SearchForFirstSensor(7); 
    //we need to grab the current data....
    if(Result.found===false){
        return "";
    } else {
        //We build the path for the request
        return "/mapping/"+Result.mMappedChannel+"/value";
    }
}

function SearchForTemperatur(){
    var Result = SearchForFirstSensor(0); 
    //we need to grab the current data....
    if(Result.found===false){
        return "";
    } else {
        //We build the path for the request
        return "/mapping/"+Result.mMappedChannel+"/value";
    }
}

function SearchForPressure(){
    var Result = SearchForFirstSensor(1); 
    //we need to grab the current data....
    if(Result.found===false){
        return "";
    } else {
        //We build the path for the request
        return "/mapping/"+Result.mMappedChannel+"/value";
    }

}

function SearchForHumidity(){
    //If we have a BME280 or WSEN_PADS we will use these
    var Result = SearchForFirstSensor(2); 
    //we need to grab the current data....
    if(Result.found===false){
        return "";
    } else {
        //We build the path for the request
        return "/mapping/"+Result.mMappedChannel+"/value";
    }
}