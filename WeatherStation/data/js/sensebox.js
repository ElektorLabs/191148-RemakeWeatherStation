var sensebox_settings ;
var sensebox_mapping ;
var station_mapping ;


var sensebox_settings_json = ["testdata/sensebox/settings.json", null];
var sensebox_mapping_json = ["testdata/sensebox/mapping.json",null];
var sensebox_station_mapping_json = [ "testdata/mappingdata.json", null];
var DataToLoad = [sensebox_settings_json, sensebox_mapping_json, sensebox_station_mapping_json ];

function sensebox_js_loaded(){
    return true;
}

function showSenseBox(){
    showView("SenseBox");
    ClearMappingSB();
    load_mapping_data(function(){
        loadMultipleData(DataToLoad,SenseBoxDisplaySettings);
    });
    
}

function ClearMappingSB(){
    const myNode = document.getElementById("SenseboxMappingTableBtn");
    while (myNode.firstChild) {
      myNode.removeChild(myNode.firstChild);
    } 
}


function SenseBoxDisplaySettings(URL_to_Load){

    //We remove parts from the tabel an generate new ones
const myNode = document.getElementById("SenseboxMappingTableBtn");
//Next is to rebuild the table....
sensebox_settings = JSON.parse(sensebox_settings_json[1]);
sensebox_mapping = JSON.parse(sensebox_mapping_json[1]);
station_mapping = JSON.parse(sensebox_station_mapping_json[1]);

document.getElementById("SenseboxUploadInterval").onchange = null;
document.getElementById("SenseboxUploadInterval").value = sensebox_settings.SENSEBOX_TXINTERVALL;
document.getElementById("SenseboxUploadInterval").onchange = SenseBoxUploadIntervalChanged;

document.getElementById("SenseboxUploadEnable").onchange = null;
document.getElementById("SenseboxUploadEnable").value = sensebox_settings.SENSEBOX_ENA;
document.getElementById("SenseboxUploadEnable").onchange = SenseboxUploadEnableChanged;


for(var x =0; x< sensebox_mapping.Mapping.length;x++){
    var row = myNode.insertRow(-1);
    var cell1 = row.insertCell(0);
    var cell2 = row.insertCell(1);
    var cell3 = row.insertCell(2);
    cell1.innerHTML = 'ID Key <input type="text" onchange="KeyID_Changed('+ x +')"  onblur="KeyID_MayChanged('+ x +')" id="SenseBoxID_' + x + '" value = "'+ sensebox_mapping.Mapping[x].SensorID+'">';
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
    selectList.setAttribute('onchange','SenseboxChSelectChanged('+x+');');
    var sensbox_mapped_ch  = sensebox_mapping.Mapping[x].STA_Channel;
    var option = document.createElement("option");
    var mapping_exists= false;
    for(var t=0;t<station_mapping.Mapping.length;t++){
        var vmpg = channel_valid_mapped( t );
        if(station_mapping.Mapping[t].Bus!=0){
            if(sensbox_mapped_ch === t ){
                mapping_exists = true;
            }
            if(vmpg.valid === true ){
                option = document.createElement("option");
                option.setAttribute("value",t);
                option.text = t+" ( " +vmpg.name+" )";
                selectList.appendChild(option);
            }
        }
       

    }
    if(mapping_exists === false){

        option = document.createElement("option");
        option.setAttribute("value",sensbox_mapped_ch);
        option.text = sensbox_mapped_ch + "(unmapped)";
        selectList.appendChild(option);
    }
    cell2.appendChild(selectList);
    //select the value we need....
    selectList.value = sensbox_mapped_ch;
    


    
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
    EnaListHtml.setAttribute("id", "SenseboxEnaList"+x);
    EnaListHtml.setAttribute('onchange','SenseboxEnaListSelectionChanged('+x+');');
    cell3.appendChild(EnaListHtml);
    if(true === sensebox_mapping.Mapping[x].Enabled){
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

function SenseboxChSelectChanged( Channel ){
    //Mapping channel has been changed 
    var el = document.getElementById("SenseboxChSelect"+Channel);

}

function SenseboxEnaListSelectionChanged( Channel ){
    //Enabled has been changed 
    var el = document.getElementById("SenseboxEnaList"+Channel);
}

function SenseboxUploadEnableChanged( ){
    var el = document.getElementById("SenseboxUploadEnable");

}

function SenseBoxUploadIntervalChanged( ){
    var el = document.getElementById("SenseboxUploadInterval");
    if(true == el.validity.valid ){
        //We can process the input....
    }

}