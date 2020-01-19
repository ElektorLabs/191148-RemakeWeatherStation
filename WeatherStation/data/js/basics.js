var wifi_scan_results = null;
var wifi_clean_ssid_list = null;

function basic_js_loaded(){
    return true;
}

function respondToVisibility (element, callback) {
    var options = {
      root: document.documentElement
    }
  
    var observer = new IntersectionObserver((entries, observer) => {
      entries.forEach(entry => {
        callback(entry.intersectionRatio > 0);
      });
    }, options);
  
    observer.observe(element);
  }
  
 


function showView(view) {
    Array.from(document.getElementsByClassName("views")).forEach(function(v) {v.style.display = "none";});
    var Element = document.getElementById(view);
    if(Element === null){
        console.log("Error view not found");
    } else {
        Element.style.display = "";
    }
   
    Array.from(document.getElementsByClassName("menuBtn")).forEach(function(b) {b.classList.remove("active")});
    var ElBtn =  document.getElementById(view + "Btn");
    if(ElBtn === null ){
        console.log("Error Btn not found");
    } else {
        ElBtn.classList.add("active");
    }
}

function sendRequest(addr, func = null) {
    console.log("requesting: " + addr);
    requestPending = true;
    var xhr = new XMLHttpRequest();
    //xhr.timeout = 5000; //ms
    xhr.open("GET", addr, true);
    xhr.onload = function() {
        console.log("Request finished");
        requestPending = false;
        if (func != null)
            func(this.responseText);
    }
    xhr.onerror = function() {
        console.log("Request finished");
        requestPending = false;
        console.log("error");
    }
    xhr.ontimeout = function() {
        console.log("Request finished");
        requestPending = false;
        console.log("timeout");
    }
    xhr.send();
}

//open and close the main menu
function openMenu() {
    document.getElementById("menu").classList.add("open");
}
function closeMenu() {
    document.getElementById("menu").classList.remove("open");
}

//show and hide the notification bar
function openNotification(msg, timeout=2500) {
    document.getElementById("notification").innerHTML = msg;
    document.getElementById("notification").classList.add("open");
    if (timeout != 0)
        setTimeout(closeNotification, timeout);
}
function closeNotification() {
    document.getElementById("notification").classList.remove("open");
}

function restart() {
    sendRequest("restart", openNotification);
}




function httpGetAsync(theUrl, callback, param, workspace)
{
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200){
            if(callback != null){
                callback(xmlHttp.responseText, param, workspace);
            }
        }
        if (xmlHttp.readyState == 4 && xmlHttp.status == 404){
            if(callback != null){
                callback("", param, workspace);
            }
        }
        if (xmlHttp.readyState == 4 && xmlHttp.status == 500){
            if(callback != null){
                callback("", param, workspace);
            }
        }

    }

    xmlHttp.onerror = function() {
        if(callback != null){
            callback("", param, workspace);
        }
    }

    xmlHttp.ontimeout = function() {
        if(callback != null){
            callback("", param, workspace);
        }
    }

    xmlHttp.open("GET", theUrl, true); // true for asynchronous 
    xmlHttp.send(null);
}

function sendData(url,data,callbackDone=null,callBackError=null) {       
  var XHR = new XMLHttpRequest();
  var urlEncodedData = "";
  var urlEncodedDataPairs = [];
  var name;

  for(name in data) {
    urlEncodedDataPairs.push(encodeURIComponent(data[name].key) + '=' + encodeURIComponent(data[name].value));
  }

  urlEncodedData = urlEncodedDataPairs.join('&').replace(/%20/g, '+');

  XHR.addEventListener('load', function(event) {
    if(callbackDone!=null){
        callbackDone;
    }

  });

  XHR.addEventListener('error', function(event) {
    alert('Oops! Something goes wrong.');
    if(callBackError!=null){
        callBackError();
    }
  });

  XHR.open('POST', url);
  XHR.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
  XHR.send(urlEncodedData);
  
}
        


function loadMultipleData( URL_To_Load , CallBackOnDone){
    var URL_To_Load;
    var CallBackOnDone;
    var DoneArray;
    
    var workspace = {URL_To_Load, DoneArray, CallBackOnDone};

    workspace.DoneArray = new Array( workspace.URL_To_Load.length);
    for(var z=0;z<workspace.DoneArray.length;z++){
        workspace.DoneArray[z]=false;
    }
    StartRequest( workspace );
}

function StartRequest( workspace ){
    var done = true;
    for( var x=0;x<workspace.URL_To_Load.length;x++){
        if(workspace.URL_To_Load[x][0]==""){ //No URL inside....
            workspace.DoneArray[x] = true;
            workspace.URL_To_Load[x][1]="";
        }

        if(workspace.URL_To_Load[x][0]==null){ //No URL inside....
            workspace.DoneArray[x] = true;
            workspace.URL_To_Load[x][1]="";
        }
        
        if(workspace.DoneArray[x] == false){
            httpGetAsync(workspace.URL_To_Load[x][0],ProcessResponse,x, workspace);
            workspace.DoneArray[x] = true;
            done = false;
            break;
        }
    }
    if(true==done){
        if(workspace.CallBackOnDone != null){
            
            workspace.CallBackOnDone(workspace.URL_To_Load);
        }
    }
}


function ProcessResponse( data, param ,workspace){
    //param holds the index for the result array
    workspace.URL_To_Load[param][1]=data;
    StartRequest(workspace);
}


function GenerateHostUrl( part ){
    var protocol = location.protocol;
    var slashes = protocol.concat("//");
    var host = slashes.concat(window.location.hostname);
    var url = host +  part;
    return url;
}
