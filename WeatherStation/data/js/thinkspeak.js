var thinkspeak_settings ;
var thinkspeak_mapping ;



var thinkspeak_settings_json = ["testdata/thinkspeak/settings.json", null];
var thinkspeak_mapping_json = ["testdata/thinkspeak/mapping.json",null];
var station_mappin_json = [ "testdata/mappingdata.json", null];
var TSDataToLoad = [thinkspeak_settings_json, thinkspeak_mapping_json, station_mappin_json ];


function showThinkspeak(){
    showView("Thinkspeak");
    ClearMappingTB();
    loadMultipleData(TSDataToLoad,ThinkspeakDisplaySettings);
}

function ClearMappingTB(){
    const myNode = document.getElementById("ThinkspeakMappingTableBtn");
    while (myNode.firstChild) {
      myNode.removeChild(myNode.firstChild);
    } 
}


function ThinkspeakDisplaySettings(URL_to_Load){

    //We remove parts from the tabel an generate new ones
const myNode = document.getElementById("ThinkspeakMappingTableBtn");
//Next is to rebuild the table....
thinkspeak_settings = JSON.parse(thinkspeak_settings_json[1]);
thinkspeak_mapping = JSON.parse(thinkspeak_mapping_json[1]);
station_mapping = JSON.parse(station_mappin_json[1]);

for(var x =0; x< 8;x++){
    var row = myNode.insertRow(-1);
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    cell1.innerHTML = "<a>Field "+x+"</a>";
    //We need to add the selection element....

    var Text = document.createElement("a");
    Text.innerHTML="Channel  "
    cell2.appendChild(Text);
   
    //Create array of options to be added
   //All mapped channels will be in the list.....
    //And the selected one if not mapped
    //Create and append select list
    var selectList = document.createElement("select");
    selectList.setAttribute("id", "SenseboxChSelect"+x);
    selectList.setAttribute('onchange','MappingChannelChanged('+x+');');
    var thinkspeak_mapped_ch  = thinkspeak_mapping.Mapping[x].STA_Channel;
    var option = document.createElement("option");

    var mapping_exists= false;
    for(var t=0;t<station_mapping.Mapping.length;t++){

        if(station_mapping.Mapping[t].Bus!=0){
            if(thinkspeak_mapped_ch === t ){
                mapping_exists = true;
            }
            option = document.createElement("option");
            option.setAttribute("value",t);
            option.text = t;
            selectList.appendChild(option);
        }
       
    }
    if(mapping_exists === false){
        option = document.createElement("option");
        option.setAttribute("value",thinkspeak);
        option.text = thinkspeak_mapped_ch + "(unmapped)";
        selectList.appendChild(option);
    }
    cell2.appendChild(selectList);
    //select the value we need....
    selectList.value = thinkspeak_mapped_ch;
    
    var Text = document.createElement("a");
    Text.innerHTML=" Enabled   "
    cell3.appendChild(Text);
    EnaListHtml = document.createElement("select");
    var option = document.createElement("option");
    var val = "true";
    option.setAttribute("value",val);
    option.text = "True";
    EnaListHtml.appendChild(option);
    option = document.createElement("option");
    var val = "false";
    option.setAttribute("value",val);
    option.text = "False";
    EnaListHtml.appendChild(option);
    


    //Create and append select list
    EnaListHtml.setAttribute("id", "EnaList"+x);
    EnaListHtml.setAttribute('onchange','MappingSelectionChanged('+x+');');
    cell3.appendChild(EnaListHtml);
    if(true === thinkspeak_mapping.Mapping[x].Enabled){
        EnaListHtml.value = "true";
    } else {
        EnaListHtml.value = "false";
    }  
   }

}

function KeyID_MayChanged( Channel ){
    //Key has may changed as we use to onbluc function.....
    myNode = document.getElementById("SenseBoxID_"+Channel);
    var id = myNode.value;
    var org_id = sensebox_mapping.Mapping[Channel].SensorID;
    if(id !=org_id ){
       //We need to write the new value back 
    }
    myNode.style.color="black";



}

function KeyID_Changed( Channel ){
    //We change the color to red.....
    myNode = document.getElementById("SenseBoxID_"+Channel);
    myNode.style.color="red";
}

function MappingChannelChanged( Channel ){
//Mapping channel has been changed 

}

function EnableChannelChanged( Channel ){
//Enabled has been changed 
}