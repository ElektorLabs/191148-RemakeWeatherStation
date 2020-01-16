//show the wifi settings data
var connectedsensors=null;
var supportedsensors=null;
var mapping=null;

var connectedsensors_json = ["testdata/supportedsensors.json", null];
var supportedsensors_json = ["testdata/connectedsensors.json",null];
var mapping_json = [ "testdata/mappingdata.json", null];
var MappingDataToLoad = [connectedsensors_json, supportedsensors_json, mapping_json ];

function load_mapping_data( callback_on_done){
    
    loadMultipleData(MappingDataToLoad,function() { mapping_parse_loaded_data_to_json(); 
        if(callback_on_done != null){
            callback_on_done();
        } });
}

function mapping_parse_loaded_data_to_json(){
    connectedsensors=JSON.parse( connectedsensors_json[1] );
    supportedsensors=JSON.parse( supportedsensors[1] );
    mapping =JSON.parse( mapping_json[1] );
}

function showSensorMapping() {
    showView("SensorMapping");
    //Next is to grab the data and generate the lists
    ClearMappingDiv();
    load_mapping_data(MappingGeneratePage);

}

function ClearMappingDiv(){
    const myNode = document.getElementById("ChannelMappingTableBtn");
    while (myNode.firstChild) {
      myNode.removeChild(myNode.firstChild);
    }
    //We need to perform a request for /devices/connectedsensors.json
    //and also load devices/supportedsensors.json
 
}

function LoadMappigData(){
    connectedsensors=null;
    supportedsensors=null;
    mapping=null;
    sendRequest("testdata/connectedsensors.json",ConnectedSensorsLoaded);
    
   

}

function MappingGeneratePage(){
//We remove parts from the tabel an generate new ones
const myNode = document.getElementById("ChannelMappingTableBtn");
//Next is to rebuild the table....

for(var x =0; x< 64;x++){
    var row = myNode.insertRow(-1);
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    cell1.innerHTML = "Channel "+x;
    //We need to add the selection element....

   

    //Create array of options to be added
   
    //Create and append select list
    var selectList = document.createElement("select");
    selectList.setAttribute("id", "mySelect");
    selectList.setAttribute('onchange','MappingSelectionChanged('+x+');');
    cell2.appendChild(selectList);

    //Create and append the options
    var o = document.createElement("option");
    o.setAttribute("value", "unmapped");
    o.text = "unmapped";
    selectList.appendChild(o);
    
    for (var i = 0; i < connectedsensors.SensorList.length ; i++) {
        
        var option = document.createElement("option");
        var val = connectedsensors.SensorList[i].Bus + "," + connectedsensors.SensorList[i].ValueType + "," + connectedsensors.SensorList[i].Channel ;
        option.setAttribute("value",val);
        option.text = connectedsensors.SensorList[i].Name;
        selectList.appendChild(option);
    }

    //We now get the mapped channel and select the propper entry 
    var mBus = mapping.Mapping[x].Bus;
    var mChannel = mapping.Mapping[x].Channel;
    var mValueType = mapping.Mapping[x].ValueType;
    var foundconnected = false
    //Lets try to find an entry in the connected sensors
    for (var i = 0; i < connectedsensors.SensorList.length ; i++) {
        if( (  connectedsensors.SensorList[i].Bus === mBus )        && 
            (connectedsensors.SensorList[i].Channel === mChannel )  &&
            (connectedsensors.SensorList[i].ValueType === mValueType ) ){
                //We have a connected sensor all good......
                selectList.value= mBus+","+mValueType+","+mChannel;
                foundconnected = true;
                break;

            }
    }
    
    if(foundconnected === false){
        //Lookup in the supported ones.....
        for (var i = 0; i < supportedsensors.SensorList.length ; i++) {
            if( (  supportedsensors.SensorList[i].Bus === mBus )        && 
                (supportedsensors.SensorList[i].Channel === mChannel )  &&
                (supportedsensors.SensorList[i].ValueType === mValueType ) ){
                    //We need to add a new entry to the list....
                    var option = document.createElement("option");
                    var val = supportedsensors.SensorList[i].Bus + "," + supportedsensors.SensorList[i].ValueType + "," + supportedsensors.SensorList[i].Channel ;
                    option.setAttribute("value",val);
                    option.text = "(unconnected) "+supportedsensors.SensorList[i].Name;
                    selectList.appendChild(option);

                    selectList.value= val;
                    foundconnected = true;
                    break;
    
                }
        }

    }
    
    if(foundconnected===false){
        selectList.value=  "unmapped";
    }


  }
}

function MappingSelectionChanged( id ){
   //We need here to submitt the new settings !
}